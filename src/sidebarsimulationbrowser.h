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

#ifndef SIDEBAR_SIMULATION_BROWSER_H
#define SIDEBAR_SIMULATION_BROWSER_H

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
    class CSimulationView;

    //! \def WaveformsMap This is a typedef to map waveforms with selected status.
    typedef QMap<QString, bool> WaveformsMap;

    /*!
     * \brief Model to provide the abstract interface for waveform items
     * in a table.
     *
     * This class derives from QAbstractTableModel and provides the abstract
     * interface for waveform items in a table model class. While the
     * SidebarSimulationBrowser class implements the user interface, this class
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
     * \sa QAbstractTableModel, SidebarSimulationBrowser
     */
    class SidebarSimulationModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        SidebarSimulationModel(WaveformsMap simulationList, QObject *parent = 0);

        int rowCount(const QModelIndex& = QModelIndex() ) const { return m_simMap.size(); }
        int columnCount(const QModelIndex& = QModelIndex() ) const { return 2; }

        QVariant data(const QModelIndex&, int role) const;
        QVariant headerData(int section, Qt::Orientation o, int role) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private:
        friend class SidebarSimulationBrowser;

        WaveformsMap m_simMap;
        QList<QString> keys;
    };

    /*!
     * \brief Dialog to select waveforms and set properties in a
     * CSimulationView plot.
     *
     * This dialog presents to the user the properties of the selected
     * simulation plot (CSimulationView) and the visible waveforms.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while SidebarSimulationModel class
     * handles the data interaction itself.
     *
     * \sa CSimulationView, WaveformsMap, SidebarSimulationModel,
     * QSortFilterProxyModel
     */
    class SidebarSimulationBrowser : public QWidget
    {
        Q_OBJECT

    public:
        SidebarSimulationBrowser(CSimulationView *parent = 0);

        void updateWaveformsList();

    private Q_SLOTS:
        void filterTextChanged();
        void selectAll();
        void selectNone();

        void updateSimulationView();

    private:
        SidebarSimulationModel *m_model;
        QSortFilterProxyModel *m_proxyModel;
        QTableView *m_tableView;
        WaveformsMap m_simulationList;

        QLineEdit *m_filterEdit;
        QPushButton *buttonAll, *buttonNone;
    };

} // namespace Caneda

#endif //SIDEBAR_SIMULATION_BROWSER_H
