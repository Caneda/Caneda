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

#include "aboutdialog.h"

#include "global.h"

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of the dialog.
     */
    AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
    {
        ui.setupUi(this);

        // Header part of the dialog
        ui.labelIcon->setPixmap(QPixmap(Caneda::imageDirectory() + "caneda.png"));

        ui.labelTitle->setText("<span style=\"font-weight:0;font-size:16pt;\">Caneda "
                               + Caneda::version() + "</span>");

        // Text of the license in a read-only text edit
        QFile *file = new QFile(Caneda::baseDirectory() + "COPYING");
        QString text;
        if(!file->exists()) {
            text = QString(tr("The text file that contains the license is not found."));
        }
        else if(!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            text = QString(tr("The text file that contains the license exists but could not be opened."));
        }
        else {
            QTextStream in(file);
            while(!in.atEnd()) {
                text += in.readLine() + QChar('\n');
            }
            file->close();
        }

        ui.textEditLicense->setPlainText(text);
    }

} // namespace Caneda
