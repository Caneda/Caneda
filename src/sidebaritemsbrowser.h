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

#ifndef SIDEBAR_ITEMS_BROWSER_H
#define SIDEBAR_ITEMS_BROWSER_H

#include <QPair>
#include <QSortFilterProxyModel>
#include <QWidget>

// Forward declaration
class QLineEdit;
class QPixmap;
class QStandardItemModel;
class QTreeView;

namespace Caneda
{
    /*!
     * \brief The FilterProxyModel class helps in filtering a sidebar model
     * corresponding to a QLineEdit.
     *
     * This class is used to be able to filter the items present in any model
     * column (categories are in the first columns of the tree, and the items
     * are in succesive columns). The QSortFilterProxyModel doesn't allow
     * multicolumn filtering, hence it must be subclassed for those cases where
     * it's needed.
     */
    class FilterProxyModel : public QSortFilterProxyModel
    {
    public:
        explicit FilterProxyModel(QObject *parent = 0);

        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    };

    /*!
     * \brief This class implements the sidebar dockwidget with components to
     * be inserted in graphic documents.
     *
     * This class implements the sidebar dockwidget corresponding to the
     * LayoutContext, SchematicContext, and SymbolContext classes. It allows
     * previously generated components to be inserted in these type of
     * documents.
     *
     * The components depend on the final context class. In the LayoutContext,
     * for example, components are layout layers; in the SchematicContext,
     * components are electronic components; and in the SymbolContext,
     * components are painting items. All these components are inserted into
     * the currently opened document upon user double click.
     *
     * \sa LayoutContext, SchematicContext, SymbolContext, SidebarTextBrowser
     * \sa QStandardItemModel
     */
    class SidebarItemsBrowser : public QWidget
    {
        Q_OBJECT

    public:
        explicit SidebarItemsBrowser(QWidget *parent = 0);
        ~SidebarItemsBrowser();

        void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);
        void plugLibrary(QString libraryName, QString category);
        void unPlugLibrary(QString libraryName, QString category);

        void focusFilter();

    signals:
        void itemClicked(const QString& item, const QString& category);
        void itemDoubleClicked(const QString& item, const QString& category);

    protected:
        bool eventFilter(QObject *object, QEvent *event);

    private Q_SLOTS:
        void filterTextChanged();
        void itemClicked(const QModelIndex& index);

    private:
        QStandardItemModel *m_model;
        FilterProxyModel *m_proxyModel;
        QTreeView *m_treeView;

        QLineEdit *m_filterEdit;
    };

} // namespace Caneda

#endif //SIDEBAR_ITEMS_BROWSER_H
