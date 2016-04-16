/***************************************************************************
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "filenewdialog.h"

#include "actionmanager.h"

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * We use the actions instead of directly setting the buttons to mantain
     * coeherence with the rest of the application.
     */
    FileNewDialog::FileNewDialog(QWidget *parent) :
        QDialog(parent)
    {
        ui.setupUi(this);

        ActionManager* am = ActionManager::instance();

        ui.choiceSchematic->addAction(am->actionForName("fileNewSchematic"));
        ui.choiceSchematic->setIcon(am->actionForName("fileNewSchematic")->icon());
        ui.choiceSchematic->setText(am->actionForName("fileNewSchematic")->text());

        ui.choiceSymbol->addAction(am->actionForName("fileNewSymbol"));
        ui.choiceSymbol->setIcon(am->actionForName("fileNewSymbol")->icon());
        ui.choiceSymbol->setText(am->actionForName("fileNewSymbol")->text());

        ui.choiceLayout->addAction(am->actionForName("fileNewLayout"));
        ui.choiceLayout->setIcon(am->actionForName("fileNewLayout")->icon());
        ui.choiceLayout->setText(am->actionForName("fileNewLayout")->text());

        ui.choiceText->addAction(am->actionForName("fileNewText"));
        ui.choiceText->setIcon(am->actionForName("fileNewText")->icon());
        ui.choiceText->setText(am->actionForName("fileNewText")->text());
    }

    //! \brief Call the selected action and accept the dialog.
    void FileNewDialog::done(int r)
    {
        if (r == QDialog::Accepted) {

            if(ui.choiceSchematic->isChecked()) {
                ui.choiceSchematic->actions().first()->trigger();
            }
            else if(ui.choiceSymbol->isChecked()) {
                ui.choiceSymbol->actions().first()->trigger();
            }
            else if(ui.choiceLayout->isChecked()) {
                ui.choiceLayout->actions().first()->trigger();
            }
            else if(ui.choiceText->isChecked()) {
                ui.choiceText->actions().first()->trigger();
            }
        }

        QDialog::done(r);
    }

} // namespace Caneda
