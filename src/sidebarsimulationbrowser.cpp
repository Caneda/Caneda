/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#include "sidebarsimulationbrowser.h"

#include "csimulationview.h"
#include "documentviewmanager.h"
#include "iview.h"

#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

#include <qwt_plot_curve.h>

namespace Caneda
{
    //*************************************************************
    //**************** SidebarSimulationModel *********************
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
    SidebarSimulationModel::SidebarSimulationModel(WaveformsMap simulationList, QObject *parent) :
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
    QVariant SidebarSimulationModel::data(const QModelIndex& index, int role) const
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
    QVariant SidebarSimulationModel::headerData(int section, Qt::Orientation o, int role) const
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
    Qt::ItemFlags SidebarSimulationModel::flags(const QModelIndex& index) const
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
     * \brief Sets data in a SidebarSimulationModel item.
     *
     * Sets the data in a SidebarSimulationModel item, ie. modifies the user
     * edited data.
     *
     * \param index Item to be edited.
     * \param value New value to be set.
     * \param role Role of the item. Helps identify what are we editing (editable
     * item, checkable item, etc).
     * \return True on success, false otherwise.
     *
     * \sa WaveformsMap
     */
    bool SidebarSimulationModel::setData(const QModelIndex& index, const QVariant& value,
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
    //*************** SidebarSimulationBrowser ********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object, the simulation view
     * being modified by this dialog.
     */
    SidebarSimulationBrowser::SidebarSimulationBrowser(CSimulationView *parent) :
        QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Set lineedit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        layout->addWidget(m_filterEdit);

        QShortcut *filterEditShortcut = new QShortcut(
                    QKeySequence(tr("C", "Insert component shortcut")), this);

        // Create proxy model and set its properties.
        // The model is set in updateWaveformsList().
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        // Apply table properties and set proxy model
        m_tableView = new QTableView();
        layout->addWidget(m_tableView);

        m_tableView->setModel(m_proxyModel);
        m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
        m_tableView->resizeColumnsToContents();

        // Add selection buttons
        buttonAll = new QPushButton("All");
        buttonNone = new QPushButton("None");
        layout->addWidget(buttonAll);
        layout->addWidget(buttonNone);

        // Signals and slots connections
        connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));
        connect(filterEditShortcut, SIGNAL(activated()), m_filterEdit, SLOT(setFocus()));
        connect(filterEditShortcut, SIGNAL(activated()), m_filterEdit, SLOT(clear()));

        connect(buttonAll, SIGNAL(clicked()), this, SLOT(selectAll()));
        connect(buttonNone, SIGNAL(clicked()), this, SLOT(selectNone()));

        connect(m_tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(updateSimulationView()));
    }

    //! \brief Filters properties according to user input on a QLineEdit.
    void SidebarSimulationBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    //! \brief Select all available waveforms
    void SidebarSimulationBrowser::selectAll()
    {
        m_model->beginResetModel();

        WaveformsMap::const_iterator it = m_model->m_simMap.begin(),
            end = m_model->m_simMap.end();

        while(it != end) {
            m_model->m_simMap[it.key()] = true;
            ++it;
        }

        m_model->endResetModel();

        updateSimulationView();
    }

    //! \brief Deselect all waveforms
    void SidebarSimulationBrowser::selectNone()
    {
        m_model->beginResetModel();

        WaveformsMap::const_iterator it = m_model->m_simMap.begin(),
            end = m_model->m_simMap.end();

        while(it != end) {
            m_model->m_simMap[it.key()] = false;
            ++it;
        }

        m_model->endResetModel();

        updateSimulationView();
    }

    /*!
     * \brief Update waveforms list
     *
     * This method updates the waveforms list given a CSimulationView. This is
     * usually used when changing between views to keep the list of available
     * waveforms synchronized with the currently selected simulation.
     */
    void SidebarSimulationBrowser::updateWaveformsList()
    {
        // Get the current view
        DocumentViewManager *manager = DocumentViewManager::instance();
        CSimulationView *view = static_cast<CSimulationView*>(manager->currentView()->toWidget());

        // Populate the waveforms list
        QwtPlotItemList list = view->itemList(QwtPlotItem::Rtti_PlotCurve);
        m_simulationList.clear();
        for(int i=0; i<list.size(); ++i) {
            m_simulationList.insert(list.at(i)->title().text(),
                                    list.at(i)->isVisible());
        }

        // Create a new table model
        m_model = new SidebarSimulationModel(m_simulationList, this);  //! \todo What happens with the old pointer??? Should we destroy it first?
        m_proxyModel->setSourceModel(m_model);
    }

    /*!
     * \brief Update simulation waveforms visibility
     *
     * This method updates the simulation waveforms visibility according to the
     * user input.
     */
    void SidebarSimulationBrowser::updateSimulationView()
    {
        // Get the current view
        DocumentViewManager *manager = DocumentViewManager::instance();
        CSimulationView *view = static_cast<CSimulationView*>(manager->currentView()->toWidget());

        // Set waveforms visibility
        QwtPlotItemList list = view->itemList(QwtPlotItem::Rtti_PlotCurve);
        for(int i=0; i<list.size(); ++i) {
            list.at(i)->setVisible(m_model->m_simMap[list.at(i)->title().text()]);
        }

        view->replot();
    }

} // namespace Caneda
