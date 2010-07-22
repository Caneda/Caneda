/***************************************************************************
 * Copyright 2010 Pablo Daniel Pareja Obregon                              *
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

#include "projectfilenewdialog.h"

#include "caneda-tools/global.h"

#include <QMessageBox>

namespace Caneda
{
    /*!
     * Constructor
     * @param parent  parent Widget of the dialog
     */
    ProjectFileNewDialog::ProjectFileNewDialog(QWidget *parent) :
            QDialog(parent)
    {
        ui.setupUi(this);
        ui.rbNewComponent->setIcon(QIcon(Caneda::bitmapDirectory() + "filenew.png"));
        ui.rbExistingComponent->setIcon(QIcon(Caneda::bitmapDirectory() + "fileopen.png"));
        ui.rbImportFromProject->setIcon(QIcon(Caneda::bitmapDirectory() + "project-new.png"));

        connect(ui.rbNewComponent, SIGNAL(toggled(bool)), ui.editName, SLOT(setEnabled(bool)));

        userchoice = Caneda::NewComponent;
    }

    //! Destructor
    ProjectFileNewDialog::~ProjectFileNewDialog()
    {
    }

    //! @return Component name
    QString ProjectFileNewDialog::fileName() const
    {
        return(filename);
    }

    //! @return User choice
    Caneda::ProjectFileNewChoice ProjectFileNewDialog::userChoice() const
    {
        return(userchoice);
    }

    //! Checks the status of the print type dialog.
    void ProjectFileNewDialog::done(int r)
    {
        if (r == QDialog::Accepted) {
            if(ui.rbNewComponent->isChecked()) {
                userchoice = Caneda::NewComponent;
            }
            else if(ui.rbExistingComponent->isChecked()) {
                userchoice = Caneda::ExistingComponent;
            }
            else {
                userchoice = Caneda::ImportFromProject;
            }

            if(ui.rbNewComponent->isChecked()) {
                if(ui.editName->text().isEmpty()) {
                    QMessageBox::information(parentWidget(),
                                             tr("Component name missing"),
                                             tr("You must enter the name of the new component."));
                    return;
                }
                else {
                    filename = ui.editName->text();
                }
            }
        }

        QDialog::done(r);
    }

} // namespace Caneda
