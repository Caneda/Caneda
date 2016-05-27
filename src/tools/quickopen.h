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

#ifndef QUICK_OPEN_H
#define QUICK_OPEN_H

#include <QMenu>

// Forward declarations
class QFileSystemModel;
class QLineEdit;
class QListView;
class QToolButton;

namespace Caneda
{
    // Forward declarations.
    class FilterProxyModel;

    /*!
     * \brief QuickOpen dialog to select and open files.
     *
     * This class implements a simple folder browser dialog to be used as a
     * tool for easy access to the file system.
     *
     * This class handles user interaction to allow direct opening of files, as
     * well as basic file operations (as deletion). This also handles the mouse
     * and keyboad events, and sends, when appropiate, the file names to be
     * opened by the parent.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while QFileSystemModel class handles
     * the data interaction itself.
     *
     * \sa QFileSystemModel
     */
    class QuickOpen : public QMenu
    {
        Q_OBJECT

    public:
        explicit QuickOpen(QWidget *parent = 0);

        void setCurrentFolder(const QString& path);

    signals:
        void itemSelected(const QString& filename);

    protected:
        bool eventFilter(QObject *object, QEvent *event);

    private Q_SLOTS:
        void filterTextChanged();
        void filterFileTypes();
        void itemSelected();

        void slotUpFolder();
        void slotBackFolder();
        void slotForwardFolder();
        void slotHomeFolder();

    private:
        QFileSystemModel *m_model;
        FilterProxyModel *m_proxyModel;
        QListView *m_listView;

        QLineEdit *m_filterEdit;

        QList<QModelIndex> previousPages;
        QList<QModelIndex> nextPages;

        QAction *filterNone, *filterSchematics, *filterSymbols,
        *filterLayouts, *filterText;
        QToolButton *buttonBack, *buttonForward;
    };

} // namespace Caneda

#endif //QUICK_OPEN_H
