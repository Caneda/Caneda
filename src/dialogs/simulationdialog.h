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

#ifndef SIMULATIONDIALOG_H
#define SIMULATIONDIALOG_H

#include "ui_simulationdialog.h"

#include <QAbstractTableModel>

// Forward declarations.
class QSortFilterProxyModel;

namespace Caneda
{
    // Forward declations
    class CSimulationView;

    //! \def WaveformsMap This is a typedef to map waveforms with selected status.
    typedef QMap<QString, bool> WaveformsMap;

    /*!
     * \brief Model to provide the abstract interface for property items
     * in a table.
     *
     * This class derives from QAbstractTableModel and provides the abstract
     * interface for property items in a table model class. While PropertyDialog
     * implements the user interface, this class interacts with the data
     * itself.
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
     * The model has a rowCount() and a columnCount(). Rows can be inserted and
     * removed with insertRows(), and removeRows().
     *
     * \sa QAbstractTableModel, SimulationDialog
     */
    class SimulationModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        SimulationModel(WaveformsMap simulationList, QObject *parent = 0);

        int rowCount(const QModelIndex& = QModelIndex() ) const { return m_simMap.size(); }
        int columnCount(const QModelIndex& = QModelIndex() ) const { return 2; }

        QVariant data(const QModelIndex&, int role) const;
        QVariant headerData(int section, Qt::Orientation o, int role) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private:
        friend class SimulationDialog;

        WaveformsMap m_simMap;
        QList<QString> keys;
    };

    /*!
     * \brief Dialog to modify properties in a PropertyGroup
     *
     * This dialog presents to the user the properties of the selected
     * component/scene. By default, properties are presented with a
     * QLineEdit. For better representation it is recommended to have
     * string properties rather than numeric. In this way, the user can
     * use prefixes, like 'p' for pico, 'u' for micro, etc. Even
     * parametric properties could be used as string properties, using
     * for example brackets as in '{R}'.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while PropertyModel class handles
     * the data interaction itself.
     *
     * \sa PropertyGroup, PropertyModel, QSortFilterProxyModel
     */
    class SimulationDialog : public QDialog
    {
        Q_OBJECT

    public:
        SimulationDialog(CSimulationView *parent = 0);

    public Q_SLOTS:
        void accept();

    private Q_SLOTS:
        void filterTextChanged();

    private:
        SimulationModel *m_model;
        QSortFilterProxyModel *m_proxyModel;

        Ui::SimulationDialog ui;
    };

} // namespace Caneda

#endif //SIMULATIONDIALOG_H
