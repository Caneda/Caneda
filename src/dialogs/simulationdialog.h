/***************************************************************************
 * Copyright (C) 2014 by Pablo Daniel Pareja Obregon                       *
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

#include "simulation.h"

#include <QAbstractListModel>

namespace Caneda
{
    class SimulationModel : public QAbstractListModel
    {
        Q_OBJECT

    public:
        SimulationModel(SimulationGroup *simGroup, QObject *parent = 0);

        int rowCount(const QModelIndex& = QModelIndex()) const { return m_simulationList.size(); }
        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;

        bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
        bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

    private:
        friend class SimulationDialog;

        SimulationList m_simulationList;
    };

    /*!
     * \brief Dialog to modify Simulation profiles
     *
     * This dialog presents to the user the available simulation profiles,
     * and gives the opportunity to edit them or add new ones.
     *
     * \sa SimulationGroup
     */
    class SimulationDialog : public QDialog
    {
        Q_OBJECT

    public:
        SimulationDialog(SimulationGroup *simGroup, QWidget *parent = 0);

    public Q_SLOTS:
        void accept();

    private Q_SLOTS:
        void addSimulation();
        void removeSimulation();

    private:
        SimulationModel *m_model;
        SimulationGroup *m_simulationGroup;

        Ui::SimulationDialog ui;
    };

} // namespace Caneda

#endif //SIMULATIONDIALOG_H
