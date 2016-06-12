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

#ifndef QUICK_INSERT_H
#define QUICK_INSERT_H

#include <QMenu>

// Forward declarations.
class QLineEdit;
class QTreeView;
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class FilterProxyModel;
    class SidebarItemsModel;

    /*!
     * \brief Quick insert dialog to select and insert items.
     *
     * This dialog presents to the user the context available items allowing to
     * insert any item in an easy way.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while SidebarItemsModel class handles the
     * data interaction itself.
     *
     * \sa SidebarItemsModel, QSortFilterProxyModel
     */
    class QuickInsert : public QMenu
    {
        Q_OBJECT

    public:
        explicit QuickInsert(SidebarItemsModel *model, QWidget *parent = 0);

    signals:
        void itemClicked(const QString& item, const QString& category);

    protected:
        bool eventFilter(QObject *object, QEvent *event);

    private Q_SLOTS:
        void filterTextChanged();
        void insertItem();

    private:
        SidebarItemsModel *m_model;
        FilterProxyModel *m_proxyModel;
        QTreeView *m_treeView;

        QLineEdit *m_filterEdit;
    };

} // namespace Caneda

#endif //QUICK_INSERT_H
