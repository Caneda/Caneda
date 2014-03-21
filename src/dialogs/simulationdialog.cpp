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

#include "simulationdialog.h"

#include "global.h"

namespace Caneda
{
    //*************************************************************
    //******************** SimulationModel ************************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param simGroup SimulationGroup wich contains the simulation profile to
     * be modified.
     * \param parent Parent of this object.
     *
     * \sa SimulationGroup
     */
    SimulationModel::SimulationModel(SimulationGroup *simGroup, QObject *parent) :
        QAbstractListModel(parent),
        m_simulationList(simGroup->simulationList())
    {
    }

    /*!
     * \brief Returns the data stored for the item referred by index.
     *
     * This class returns the item data corresponding to index position.
     *
     * \param index Item to return data from
     * \param role Role of the item (editable, checkable, etc).
     * \return data stored for given item
     */
    QVariant SimulationModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid() || index.row() >= rowCount()) {
            return QVariant();
        }

        // Return the simulation type
        if(role == Qt::DisplayRole) {
            return m_simulationList.at(index.row()).type();
        }

        return QVariant();
    }

    /*!
     * \brief Inserts a new row (Simulation) with a default text.
     *
     * \param position Position to insert new simulation.
     * \param rows Number of rows to insert.
     * \param index Unused.
     * \return True on success, false otherwise.
     */
    bool SimulationModel::insertRows(int position, int rows, const QModelIndex &index)
    {
        // Insert new simulation
        beginInsertRows(QModelIndex(), position, position+rows-1);

        for(int row=0; row < rows; row++) {
            Simulation newSim("transient","t1=5 t2=10 t3=15");
            m_simulationList.append(newSim);
        }

        endInsertRows();
        return true;
    }

    /*!
     * \brief Deletes a row (Simulation) from the SimulationList.
     *
     * \param position Position from where to delete rows.
     * \param rows Number of rows to delete.
     * \param index Unused.
     * \return True on success, false otherwise.
     */
    bool SimulationModel::removeRows(int position, int rows, const QModelIndex &index)
    {
        beginRemoveRows(QModelIndex(), position, position+rows-1);

        for (int row=0; row < rows; ++row) {
            // Remove simulation
            m_simulationList.removeAt(position);
        }

        endRemoveRows();
        return true;
    }

    //*************************************************************
    //******************** SimulationDialog ***********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object.
     */
    SimulationDialog::SimulationDialog(SimulationGroup *simGroup, QWidget *parent) :
        QDialog(parent), m_simulationGroup(simGroup)
    {
        // Initialize designer dialog
        ui.setupUi(this);

        // Set button properties
        ui.m_addButton->setIcon(Caneda::icon("list-add"));
        ui.m_addButton->setStatusTip(tr("Add a new simulation to the list"));
        ui.m_addButton->setWhatsThis(
                    tr("Add New Simulation\n\nAdds a new simulation to the list"));

        ui.m_removeButton->setIcon(Caneda::icon("list-remove"));
        ui.m_removeButton->setStatusTip(tr("Remove selected simulation from the list"));
        ui.m_removeButton->setWhatsThis(
                    tr("Remove Simulation\n\nRemoves selected simulation from the list"));

        // Create new list model
        m_model = new SimulationModel(m_simulationGroup, this);

        // Apply table properties and set proxy model
        ui.listViewSimulations->setModel(m_model);

        connect(ui.m_addButton, SIGNAL(clicked()), SLOT(addSimulation()));
        connect(ui.m_removeButton, SIGNAL(clicked()), SLOT(removeSimulation()));
    }

    /*!
     * \brief Accept dialog
     *
     * Accept dialog and set the new values according to the user input.
     */
    void SimulationDialog::accept()
    {
        QDialog::accept();
    }

    //! \brief Add a new simulation to the model.
    void SimulationDialog::addSimulation()
    {
        m_model->insertRows(m_model->rowCount(), 1);
    }

    //! \brief Remove a simulation from the model.
    void SimulationDialog::removeSimulation()
    {
        m_model->removeRows(ui.listViewSimulations->currentIndex().row(), 1);
    }

} // namespace Caneda
