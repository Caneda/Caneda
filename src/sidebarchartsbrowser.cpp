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

#include "sidebarchartsbrowser.h"

#include "chartview.h"
#include "documentviewmanager.h"
#include "iview.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
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
    //******************* SidebarChartsModel **********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param chartSeriesMap ChartSeriesMap which contains waveforms to be set
     * visible/not-visible.
     * \param parent Parent of this object.
     *
     * \sa ChartSeriesMap
     */
    SidebarChartsModel::SidebarChartsModel(ChartSeriesMap chartSeriesMap, QObject *parent) :
        QAbstractTableModel(parent),
        m_chartSeriesMap(chartSeriesMap),
        keys(chartSeriesMap.keys())
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
    QVariant SidebarChartsModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid() || index.row() >= m_chartSeriesMap.size()) {
            return QVariant();
        }

        QString key = keys.at(index.row());

        if(role == Qt::DisplayRole && index.column() == 0) {
            return key;
        }
        else if(role == Qt::CheckStateRole && index.column() == 1) {
            return m_chartSeriesMap[key] ? Qt::Checked : Qt::Unchecked;
        }

        return QVariant();
    }

    /*!
     * \brief Returns header data (text) for the given column
     *
     * This method defines column header text to be displayed on the
     * associated table view.
     */
    QVariant SidebarChartsModel::headerData(int section, Qt::Orientation o, int role) const
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
    Qt::ItemFlags SidebarChartsModel::flags(const QModelIndex& index) const
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
     * \brief Sets data in a SidebarChartsModel item.
     *
     * Sets the data in a SidebarChartsModel item, ie. modifies the user
     * edited data.
     *
     * \param index Item to be edited.
     * \param value New value to be set.
     * \param role Role of the item. Helps identify what are we editing (editable
     * item, checkable item, etc).
     * \return True on success, false otherwise.
     *
     * \sa ChartSeriesMap
     */
    bool SidebarChartsModel::setData(const QModelIndex& index, const QVariant& value,
            int role)
    {
        if(index.isValid()){
            // If editing the property visibility, set new visibility
            if(role == Qt::CheckStateRole && index.column() == 1) {
                m_chartSeriesMap[keys[index.row()]] = value.toBool();
            }

            emit dataChanged(index, index);
            return true;
        }

        return false;
    }

    //*************************************************************
    //****************** SidebarChartsBrowser *********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object, the ChartView
     * being modified by this dialog.
     */
    SidebarChartsBrowser::SidebarChartsBrowser(ChartView *parent) :
        QWidget(parent)
    {
        QVBoxLayout *layoutTop = new QVBoxLayout(this);
        QHBoxLayout *layoutHorizontal = new QHBoxLayout();
        QVBoxLayout *layoutButtons = new QVBoxLayout();

        // Set lineedit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        layoutTop->addWidget(m_filterEdit);

        // Create proxy model and set its properties.
        // The model is set in updateWaveformsList().
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        // Apply table properties and set proxy model
        m_tableView = new QTableView(this);
        m_tableView->setModel(m_proxyModel);
        m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
        m_tableView->verticalHeader()->setVisible(false);
        layoutHorizontal->addWidget(m_tableView);

        // Add selection buttons
        QLabel *labelButtons = new QLabel(tr("Select:"), this);
        buttonAll = new QPushButton("All", this);
        buttonNone = new QPushButton("None", this);
        buttonVoltages = new QPushButton("Voltages", this);
        buttonCurrents = new QPushButton("Currents", this);
        layoutButtons->addWidget(labelButtons);
        layoutButtons->addWidget(buttonAll);
        layoutButtons->addWidget(buttonNone);
        layoutButtons->addSpacing(20);
        layoutButtons->addWidget(buttonVoltages);
        layoutButtons->addWidget(buttonCurrents);
        layoutButtons->addStretch();

        // Complete the layout of elements
        layoutHorizontal->addLayout(layoutButtons);
        layoutTop->addLayout(layoutHorizontal);

        // Signals and slots connections
        connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));

        connect(buttonAll, SIGNAL(clicked()), this, SLOT(selectAll()));
        connect(buttonNone, SIGNAL(clicked()), this, SLOT(selectNone()));
        connect(buttonVoltages, SIGNAL(clicked()), this, SLOT(selectVoltages()));
        connect(buttonCurrents, SIGNAL(clicked()), this, SLOT(selectCurrents()));

        connect(m_tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(updateChartView()));

        setWindowTitle(tr("Displayed Waveforms"));
    }

    //! \brief Filters properties according to user input on a QLineEdit.
    void SidebarChartsBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    //! \brief Select all available waveforms
    void SidebarChartsBrowser::selectAll()
    {
        m_model->beginResetModel();

        ChartSeriesMap::const_iterator it = m_model->m_chartSeriesMap.begin(),
            end = m_model->m_chartSeriesMap.end();

        while(it != end) {
            m_model->m_chartSeriesMap[it.key()] = true;
            ++it;
        }

        m_model->endResetModel();

        updateChartView();
    }

    //! \brief Deselect all waveforms
    void SidebarChartsBrowser::selectNone()
    {
        m_model->beginResetModel();

        ChartSeriesMap::const_iterator it = m_model->m_chartSeriesMap.begin(),
            end = m_model->m_chartSeriesMap.end();

        while(it != end) {
            m_model->m_chartSeriesMap[it.key()] = false;
            ++it;
        }

        m_model->endResetModel();

        updateChartView();
    }

    //! \brief Select all available voltage waveforms
    void SidebarChartsBrowser::selectVoltages()
    {
        m_model->beginResetModel();

        ChartSeriesMap::const_iterator it = m_model->m_chartSeriesMap.begin(),
            end = m_model->m_chartSeriesMap.end();

        while(it != end) {
            if(it.key().startsWith("v")) {
                m_model->m_chartSeriesMap[it.key()] = true;
            }
            else {
                m_model->m_chartSeriesMap[it.key()] = false;
            }

            ++it;
        }

        m_model->endResetModel();

        updateChartView();
    }

    //! \brief Select all available current waveforms
    void SidebarChartsBrowser::selectCurrents()
    {
        m_model->beginResetModel();

        ChartSeriesMap::const_iterator it = m_model->m_chartSeriesMap.begin(),
            end = m_model->m_chartSeriesMap.end();

        while(it != end) {
            if(it.key().startsWith("i")) {
                m_model->m_chartSeriesMap[it.key()] = true;
            }
            else {
                m_model->m_chartSeriesMap[it.key()] = false;
            }

            ++it;
        }

        m_model->endResetModel();

        updateChartView();
    }

    /*!
     * \brief Update ChartSeries map
     *
     * This method updates the waveforms list given a ChartView. This is
     * usually used when changing between views to keep the list of available
     * waveforms synchronized with the currently selected chart.
     */
    void SidebarChartsBrowser::updateChartSeriesMap()
    {
        // Get the current view
        DocumentViewManager *manager = DocumentViewManager::instance();
        ChartView *view = static_cast<ChartView*>(manager->currentView()->toWidget());

        // Populate the waveforms list
        QwtPlotItemList list = view->itemList(QwtPlotItem::Rtti_PlotCurve);
        m_chartSeriesMap.clear();
        for(int i=0; i<list.size(); ++i) {
            m_chartSeriesMap.insert(list.at(i)->title().text(),
                                    list.at(i)->isVisible());
        }

        // Create a new table model
        m_model = new SidebarChartsModel(m_chartSeriesMap, this);  //! \todo What happens with the old pointer??? Should we destroy it first?
        m_proxyModel->setSourceModel(m_model);

        // Resize the table columns to fit the contents. This must be done
        // here instead of the constructor because the columns are created
        // here when filling the model (in the constructor the columns do
        // not yet exist, resulting in a crash if resized there).
        m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    }

    /*!
     * \brief Filters available items in the sidebar.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. This method allows for an external object to request
     * the selection of the sidebar focus and filtering, for example when
     * inserting items.
     */
    void SidebarChartsBrowser::filterItems()
    {
        m_filterEdit->setFocus();
        m_filterEdit->clear();
    }

    /*!
     * \brief Update chart waveforms visibility (ChartView)
     *
     * This method updates the chart waveforms visibility according to the
     * user input.
     */
    void SidebarChartsBrowser::updateChartView()
    {
        // Get the current view
        DocumentViewManager *manager = DocumentViewManager::instance();
        ChartView *view = static_cast<ChartView*>(manager->currentView()->toWidget());

        // Set waveforms visibility
        QwtPlotItemList list = view->itemList(QwtPlotItem::Rtti_PlotCurve);
        for(int i=0; i<list.size(); ++i) {
            list.at(i)->setVisible(m_model->m_chartSeriesMap[list.at(i)->title().text()]);
        }

        view->replot();
    }

} // namespace Caneda
