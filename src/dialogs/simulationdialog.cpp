/***************************************************************************
 * Copyright (C) 2015 by Pablo Daniel Pareja Obregon                       *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "simulationdialog.h"

#include "csimulationitem.h"
#include "csimulationview.h"
#include "documentviewmanager.h"
#include "global.h"
#include "idocument.h"

#include <QSortFilterProxyModel>

#include <qwt_plot_curve.h>

namespace Caneda
{
    //*************************************************************
    //******************** SimulationModel ************************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param simulationList WaveformsMap wich contains waveforms to be set
     * visible/not-visible.
     * \param parent Parent of this object.
     *
     * \sa WaveformsMap
     */
    SimulationModel::SimulationModel(WaveformsMap simulationList, QObject *parent) :
        QAbstractTableModel(parent),
        m_simMap(simulationList),
        keys(simulationList.keys())
    {
    }

    /*!
     * \brief Returns the data stored for the item referred by index.
     *
     * This class returns the item data corresponding to index position.
     * For example, if we are editing an item in the first column, the
     * data corresponds to the waveform name, hence the return value is
     * the key of the waveform map in the form of a QString.
     *
     * \param index Item to return data from
     * \param role Role of the item (editable, checkable, etc).
     * \return data stored for given item
     */
    QVariant SimulationModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid() || index.row() >= m_simMap.size()) {
            return QVariant();
        }

        QString key = keys.at(index.row());

        if(role == Qt::DisplayRole && index.column() == 0) {
            return key;
        }
        else if(role == Qt::CheckStateRole && index.column() == 1) {
            return m_simMap[key] ? Qt::Checked : Qt::Unchecked;
        }

        return QVariant();
    }

    /*!
     * \brief Returns header data (text) for the given column
     *
     * This method defines column header text to be displayed on the
     * associated table view.
     */
    QVariant SimulationModel::headerData(int section, Qt::Orientation o, int role) const
    {
        if(role != Qt::DisplayRole) {
            return QVariant();
        }

        if(o == Qt::Vertical) {
            return QAbstractTableModel::headerData(section, o, role);
        }
        else {
            switch(section) {
                case 0: return tr("Name");
                case 1: return tr("Visible");
            }
        }
        return QVariant();
    }

    /*!
     * \brief Returns item flags according to its position. This flags
     * are responsible for the item editable or checkable state.
     *
     * \param index Item for which its flags must be returned.
     * \return Qt::ItemFlags Item's flags.
     */
    Qt::ItemFlags SimulationModel::flags(const QModelIndex& index) const
    {
        if(!index.isValid()) {
            return Qt::ItemIsEnabled;
        }

        Qt::ItemFlags flags = QAbstractTableModel::flags(index);

        // Column 1 is checkable (visibility CheckBox)
        if(index.column() == 1) {
            flags |= Qt::ItemIsUserCheckable;
        }
        // Column 0 is not editable
        else {
            flags |= Qt::ItemIsEnabled;
        }

        return flags;
    }

    /*!
     * \brief Sets data in a SimulationModel item.
     *
     * Sets the data in a SimulationModel item, ie. modifies the user edited
     * data.
     *
     * \param index Item to be edited.
     * \param value New value to be set.
     * \param role Role of the item. Helps identify what are we editing (editable
     * item, checkable item, etc).
     * \return True on success, false otherwise.
     *
     * \sa WaveformsMap
     */
    bool SimulationModel::setData(const QModelIndex& index, const QVariant& value,
            int role)
    {
        if(index.isValid()){
            // If editing the property visibility, set new visibility
            if(role == Qt::CheckStateRole && index.column() == 1) {
                m_simMap[keys[index.row()]] = value.toBool();
            }

            emit dataChanged(index, index);
            return true;
        }

        return false;
    }

    //*************************************************************
    //******************** SimulationDialog ***********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object, the simulation view
     * being modified by this dialog.
     */
    SimulationDialog::SimulationDialog(CSimulationView *parent) :
        QDialog(parent)
    {
        // Initialize designer dialog
        ui.setupUi(this);

        // Set log axis checkboxes state
        ui.checkBoxXscale->setChecked(parent->isLogAxis(QwtPlot::xBottom));
        ui.checkBoxYleftScale->setChecked(parent->isLogAxis(QwtPlot::yLeft));
        ui.checkBoxYrightScale->setChecked(parent->isLogAxis(QwtPlot::yRight));

        // Set lineedit properties
        ui.m_filterEdit->setClearButtonEnabled(true);

        // Create new table model
        QwtPlotItemList list = parent->itemList(QwtPlotItem::Rtti_PlotCurve);
        WaveformsMap m_simulationList;

        for(int i=0; i<list.size(); ++i) {
            m_simulationList.insert(list.at(i)->title().text(),
                                    list.at(i)->isVisible());
        }

        m_model = new SimulationModel(m_simulationList, this);

        // Create proxy model and set its properties
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        // Apply table properties and set proxy model
        ui.tableView->setModel(m_proxyModel);
        ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui.tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
        ui.tableView->resizeColumnsToContents();

        connect(ui.m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));
        connect(ui.buttonAll, SIGNAL(clicked()), this, SLOT(selectAll()));
        connect(ui.buttonNone, SIGNAL(clicked()), this, SLOT(selectNone()));
    }

    /*!
     * \brief Accept dialog
     *
     * Accept dialog and set new plot properties and waveforms' visibility
     * values according to the user input.
     */
    void SimulationDialog::accept()
    {
        CSimulationView *parent = static_cast<CSimulationView*>(this->parent());

        // Set waveforms visibility
        QwtPlotItemList list = parent->itemList(QwtPlotItem::Rtti_PlotCurve);
        for(int i=0; i<list.size(); ++i) {
            list.at(i)->setVisible(m_model->m_simMap[list.at(i)->title().text()]);
        }

        // Set log axis checkboxes state
        parent->setLogAxis(QwtPlot::xBottom, ui.checkBoxXscale->isChecked());
        parent->setLogAxis(QwtPlot::yLeft, ui.checkBoxYleftScale->isChecked());
        parent->setLogAxis(QwtPlot::yRight, ui.checkBoxYrightScale->isChecked());

        // Accept dialog
        parent->replot();
        QDialog::accept();
    }

    //! \brief Filters properties according to user input on a QLineEdit.
    void SimulationDialog::filterTextChanged()
    {
        QString text = ui.m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    //! \brief Select all available waveforms
    void SimulationDialog::selectAll()
    {
        m_model->beginResetModel();

        WaveformsMap::const_iterator it = m_model->m_simMap.begin(),
            end = m_model->m_simMap.end();

        while(it != end) {
            m_model->m_simMap[it.key()] = true;
            ++it;
        }

        m_model->endResetModel();
    }

    //! \brief Deselect all waveforms
    void SimulationDialog::selectNone()
    {
        m_model->beginResetModel();

        WaveformsMap::const_iterator it = m_model->m_simMap.begin(),
            end = m_model->m_simMap.end();

        while(it != end) {
            m_model->m_simMap[it.key()] = false;
            ++it;
        }

        m_model->endResetModel();
    }

} // namespace Caneda
