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

#include "filenewdialog.h"

#include "documentviewmanager.h"
#include "global.h"
#include "layoutcontext.h"
#include "schematiccontext.h"
#include "symbolcontext.h"
#include "textcontext.h"

namespace Caneda
{
    FileNewDialog::FileNewDialog(QWidget *parent) :
        QDialog(parent)
    {
        ui.setupUi(this);

        ui.choiceSchematic->setIcon(Caneda::icon("document-new"));
        ui.choiceSymbol->setIcon(Caneda::icon("document-properties"));
        ui.choiceLayout->setIcon(Caneda::icon("view-grid"));
        ui.choiceText->setIcon(Caneda::icon("text-plain"));
    }

    //! Destructor
    FileNewDialog::~FileNewDialog()
    {
    }

    void FileNewDialog::done(int r)
    {
        if (r == QDialog::Accepted) {

            DocumentViewManager *manager = DocumentViewManager::instance();

            if(ui.choiceSchematic->isChecked()) {
                manager->newDocument(SchematicContext::instance());
            }
            else if(ui.choiceSymbol->isChecked()) {
                manager->newDocument(SymbolContext::instance());
            }
            else if(ui.choiceLayout->isChecked()) {
                manager->newDocument(LayoutContext::instance());
            }
            else if(ui.choiceText->isChecked()) {
                manager->newDocument(TextContext::instance());
            }
        }

        QDialog::done(r);
    }

} // namespace Caneda
