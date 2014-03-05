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

#ifndef PORTSYMBOLDIALOG_H
#define PORTSYMBOLDIALOG_H

#include "ui_portsymboldialog.h"

namespace Caneda
{
    /*!
     * \brief Dialog to modify PortSymbol properties
     *
     * This dialog presents to the user the properties of the selected
     * PortSymbol. By default, properties are presented with a QLineEdit.
     *
     * \sa PortSymbol
     */
    class PortSymbolDialog : public QDialog
    {
        Q_OBJECT

    public:
        PortSymbolDialog(QString *label, QWidget *parent = 0);

    public Q_SLOTS:
        void accept();

    private:
        QString *m_label;

        Ui::PortSymbolDialog ui;
    };

} // namespace Caneda

#endif //PORTSYMBOLDIALOG_H
