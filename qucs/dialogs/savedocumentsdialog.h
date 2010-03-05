/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include <QSet>

class FileBrowserLineEditPrivate;
class QFileInfo;
class QucsView;
class SaveDocumentsDialogPrivate;

class FileBrowserLineEdit : public QWidget
{
    Q_OBJECT
public:
    FileBrowserLineEdit(QTreeWidgetItem *item, const QFileInfo& fileInfo,
            QWidget *parent = 0);
    ~FileBrowserLineEdit();

    QFileInfo fileInfo() const;
    void updateTexts();

private Q_SLOTS:
    void browseButtonClicked();

private:
    FileBrowserLineEditPrivate *d;
};

class SaveDocumentsDialog : public QDialog
{
    Q_OBJECT
public:
    enum ResultType {
        SaveSelected = QDialogButtonBox::AcceptRole,
        DoNotSave = QDialogButtonBox::DestructiveRole,
        AbortClosing = QDialogButtonBox::RejectRole
    };

    SaveDocumentsDialog(const QSet<QPair<QucsView*, int> > &modifiedViews, QWidget *parent = 0);
    ~SaveDocumentsDialog();

    QSet<QPair<QucsView*, QString> > newFilePaths() const;

public Q_SLOTS:
    void slotButtonClicked(QAbstractButton *button);
    void slotHandleClick(const QModelIndex& index);
    void reject();

private:
    void populateItems();

    Ui::SaveDocumentsDialog ui;
    SaveDocumentsDialogPrivate *d;
};

#endif
