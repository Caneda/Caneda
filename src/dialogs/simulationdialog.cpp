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

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object.
     */
    SimulationDialog::SimulationDialog(QWidget *parent) :
        QDialog(parent)
    {
        // Initialize designer dialog
        ui.setupUi(this);

        ui.editName->setText(tr("Empty simulation profile"));
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

} // namespace Caneda
