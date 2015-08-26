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

#include "portsymboldialog.h"

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param label The port label being modified by this dialog.
     * \param parent Parent of this object.
     */
    PortSymbolDialog::PortSymbolDialog(QString *label, QWidget *parent) :
        QDialog(parent), m_label(label)
    {
        // Initialize designer dialog
        ui.setupUi(this);

        ui.editName->setClearButtonEnabled(true);
        ui.editName->setText(*m_label);
    }

    /*!
     * \brief Accept dialog
     *
     * Accept dialog and set the new values according to the user input.
     */
    void PortSymbolDialog::accept()
    {
        // Clear and append the new value. This is done using the access
        // functions to modify the value of the object pointed by the pointer
        // instead of the local pointer.
        m_label->clear();
        m_label->append(ui.editName->text());

        QDialog::accept();
    }

} // namespace Caneda
