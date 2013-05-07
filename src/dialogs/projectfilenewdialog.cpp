/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "global.h"

#include <QMessageBox>

namespace Caneda
{
    //! \brief Constructor.
    ProjectFileNewDialog::ProjectFileNewDialog(QWidget *parent) :
            QDialog(parent)
    {
        ui.setupUi(this);
        ui.rbNewComponent->setIcon(Caneda::icon("document-new"));
        ui.rbExistingComponent->setIcon(Caneda::icon("document-open"));
        ui.rbImportFromProject->setIcon(Caneda::icon("project-new"));

        connect(ui.rbNewComponent, SIGNAL(toggled(bool)), ui.editName, SLOT(setEnabled(bool)));

        m_userchoice = Caneda::NewComponent;
    }

    //! \brief Destructor.
    ProjectFileNewDialog::~ProjectFileNewDialog()
    {
    }

    //! \brief Checks the status of the print type dialog.
    void ProjectFileNewDialog::done(int r)
    {
        if (r == QDialog::Accepted) {
            if(ui.rbNewComponent->isChecked()) {
                m_userchoice = Caneda::NewComponent;
            }
            else if(ui.rbExistingComponent->isChecked()) {
                m_userchoice = Caneda::ExistingComponent;
            }
            else {
                m_userchoice = Caneda::ImportFromProject;
            }

            if(ui.rbNewComponent->isChecked()) {
                if(ui.editName->text().isEmpty()) {
                    QMessageBox::information(parentWidget(),
                                             tr("Component name missing"),
                                             tr("You must enter the name of the new component."));
                    return;
                }
                else {
                    m_filename = ui.editName->text();
                }
            }
        }

        QDialog::done(r);
    }

} // namespace Caneda
