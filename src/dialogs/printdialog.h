/***************************************************************************
 * Copyright (C) 2009 by Pablo Daniel Pareja Obregon                       *
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

#ifndef PRINT_DIALOG_H
#define PRINT_DIALOG_H

#include "ui_printdialog.h"

#include <QDialog>

namespace Caneda
{
    class IDocument;

    class PrintDialog : public QDialog
    {
        Q_OBJECT

    public:
        PrintDialog(IDocument *document, QWidget *parent = 0);

    public Q_SLOTS:
        virtual void done(int r);

    private Q_SLOTS:
        void onChoiceToggled();
        void onBrowseButtonClicked();

    private:
        QPrinter *m_printer;
        IDocument *m_document;

        Ui::PrintDialog ui;
    };

} // namespace Caneda

#endif //PRINT_DIALOG_H
