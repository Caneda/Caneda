/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef SAVEDOCUMENTSDIALOG_H
#define SAVEDOCUMENTSDIALOG_H

#include "ui_savedocumentsdialog.h"

#include <QDialog>
#include <QFileInfo>

// Forward declarations.
class QToolButton;

namespace Caneda
{
    // Forward declarations.
    class IDocument;

    class FileBrowserLineEdit : public QWidget
    {
        Q_OBJECT

    public:
        explicit FileBrowserLineEdit(QTreeWidgetItem *item,
                                     const QFileInfo& fileInfo,
                                     QWidget *parent = 0);

        QFileInfo fileInfo() const;

    private Q_SLOTS:
        void browseButtonClicked();
        void updateTexts();

    private:
        QTreeWidgetItem *m_item;
        QFileInfo m_fileInfo;

        QLineEdit *m_lineEdit;
        QToolButton *m_browseButton;
    };

    class SaveDocumentsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SaveDocumentsDialog(const QList<IDocument*> &modifiedDocuments,
                                     QWidget *parent = 0);

    public Q_SLOTS:
        void buttonClicked(QAbstractButton *button);

    private:
        Ui::SaveDocumentsDialog ui;

        QList<IDocument*> m_modifiedDocuments;
    };

} // namespace Caneda

#endif //SAVEDOCUMENTSDIALOG_H
