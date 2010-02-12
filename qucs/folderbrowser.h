/***************************************************************************
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
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

#ifndef FOLDERBROWSER_H
#define FOLDERBROWSER_H

#include <QModelIndex>
#include <QWidget>

// Forward declarations
class QFileSystemModel;
class QListView;
class QToolButton;

class FolderBrowser : public QWidget
{
    Q_OBJECT;

public:
    FolderBrowser(QWidget *parent = 0);
    ~FolderBrowser() {}

signals:
    void itemDoubleClicked(const QString& filename);

private Q_SLOTS:
    void slotOnDoubleClicked(const QModelIndex& index);
    void slotUpFolder();
    void slotBackFolder();
    void slotForwardFolder();
    void slotHomeFolder();
    void slotNewFolder();
    void slotDeleteFile();

private:
    QFileSystemModel *m_fileModel;
    QListView *m_listView;
    QList<QModelIndex> previousPages;
    QList<QModelIndex> nextPages;

    QToolButton *buttonBack, *buttonForward;
};

#endif //FOLDERBROWSER_H
