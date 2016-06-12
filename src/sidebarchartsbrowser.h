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

#ifndef SIDEBAR_CHARTS_BROWSER_H
#define SIDEBAR_CHARTS_BROWSER_H

#include <QAbstractTableModel>
#include <QWidget>

// Forward declarations.
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;

namespace Caneda
{
    // Forward declations
    class ChartView;

    //! \def ChartSeriesMap This is a typedef to map waveforms with selected status.
    typedef QMap<QString, bool> ChartSeriesMap;

    /*!
     * \brief Model to provide the abstract interface for waveform items
     * in a table.
     *
     * This class derives from QAbstractTableModel and provides the abstract
     * interface for waveform items in a table model class. While the
     * SidebarChartsBrowser class implements the user interface, this class
     * interacts with the data itself.
     *
     * This class defines a standard interface that must used to be able to
     * interoperate with other components in Qt's model/view framework. The
     * underlying data model is exposed as a simple table of rows and columns.
     * Each item has a unique index specified by a QModelIndex.
     *
     * Each item has a number of data elements associated with it and they can
     * be retrieved by specifying a role (see Qt::ItemDataRole) to the model's
     * data() function. Data for each role is set using a particular
     * Qt::ItemDataRole. Data for individual roles are set individually with
     * setData(). Items can be queried with flags() (see Qt::ItemFlag) to see
     * if they can be selected, dragged, or manipulated in other ways.
     *
     * \sa QAbstractTableModel, SidebarChartsBrowser
     */
    class SidebarChartsModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit SidebarChartsModel(ChartSeriesMap chartSeriesMap, QObject *parent = 0);

        int rowCount(const QModelIndex& = QModelIndex() ) const { return m_chartSeriesMap.size(); }
        int columnCount(const QModelIndex& = QModelIndex() ) const { return 2; }

        QVariant data(const QModelIndex&, int role) const;
        QVariant headerData(int section, Qt::Orientation o, int role) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private:
        friend class SidebarChartsBrowser;

        ChartSeriesMap m_chartSeriesMap;
        QList<QString> keys;
    };

    /*!
     * \brief Dialog to select waveforms and set properties in a
     * ChartView plot.
     *
     * This dialog presents to the user the properties of the selected
     * simulation plot (ChartView) and the visible waveforms.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while SidebarChartsModel class
     * handles the data interaction itself.
     *
     * \sa ChartView, ChartSeriesMap, SidebarChartsModel,
     * QSortFilterProxyModel
     */
    class SidebarChartsBrowser : public QWidget
    {
        Q_OBJECT

    public:
        explicit SidebarChartsBrowser(ChartView *parent = 0);

        void updateChartSeriesMap();

    private Q_SLOTS:
        void filterTextChanged();

        void selectAll();
        void selectNone();
        void selectVoltages();
        void selectCurrents();

        void updateChartView();

    private:
        SidebarChartsModel *m_model;
        QSortFilterProxyModel *m_proxyModel;
        QTableView *m_tableView;

        ChartSeriesMap m_chartSeriesMap;

        QLineEdit *m_filterEdit;
        QPushButton *buttonAll, *buttonNone, *buttonVoltages, *buttonCurrents;
    };

} // namespace Caneda

#endif //SIDEBAR_CHARTS_BROWSER_H
