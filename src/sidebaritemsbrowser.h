/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013-2016 by Pablo Daniel Pareja Obregon                  *
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

#include <QAbstractItemModel>
#include <QPair>
#include <QSortFilterProxyModel>
#include <QWidget>

// Forward declaration
class QLineEdit;
class QPixmap;
class QTreeView;

namespace Caneda
{
    /*!
     * \brief The CategoryItem class implements the items to be used by the
     * SidebarItemsModel class.
     *
     * This class implements a custom type of items to be inserted in a
     * QAbstractItemModel. It is used in Caneda by the SidebarItemsModel
     * class, to insert items and have the ability to use QPixmaps associated
     * with each item inserted. Items can also correspond to categories (or
     * libraries), in which case the QPixmap is not necessary.
     *
     * \sa SidebarItemsModel
     */
    class CategoryItem : public QObject
    {
        Q_OBJECT

    public:
        explicit CategoryItem(const QString& name,
                              const QString& filename,
                              const QPixmap &pixmap = QPixmap(),
                              bool isLibrary = false,
                              CategoryItem *parentItem = 0,
                              QObject *parent = 0);
        ~CategoryItem();

        CategoryItem *parent() const { return m_parentItem; }

        CategoryItem *child(int row) const;
        int childCount() const { return m_childItems.size(); }

        int row() const;
        QString name() const { return m_name; }
        QString filename() const { return m_filename; }
        QPixmap iconPixmap() const { return m_iconPixmap; }

        bool isLeaf() const { return m_childItems.isEmpty(); }
        bool isLibrary() const { return m_isLibrary; }

        void addChild(CategoryItem* c);
        void removeChild(int c);

    private:
        QString m_name;
        QString m_filename;
        QPixmap m_iconPixmap;

        bool m_isLibrary;
        QList<CategoryItem*> m_childItems;
        CategoryItem *m_parentItem;
    };


    /*!
     * \brief The SidebarItemsModel class implements a custom
     * QAbstractItemModel, and provides the actual interface for item model
     * classes.
     *
     * The QAbstractItemModel class defines the standard interface that item
     * models must use to be able to interoperate with other components in the
     * model/view architecture. It is not supposed to be instantiated directly,
     * and must be subclassed to create new models. The SidebarItemsModel
     * class is the subclass implemented by Caneda.
     *
     * The SidebarItemsModel class is one of the Model/View Classes and
     * implements the model part of Qt's model/view framework.
     *
     * The underlying data model is exposed to views and delegates as a
     * hierarchy of tables. Each item has a unique index specified by a
     * QModelIndex. Every item of data that can be accessed via a model has an
     * associated model index. You can obtain this model index using the
     * index() function. Each index may have a sibling() index; child items
     * have a parent() index. Each item has a number of data elements
     * associated with it and they can be retrieved by specifying a role (see
     * Qt::ItemDataRole) to the model's data() function.
     *
     * \sa TreeView, CategoryItem
     */
    class SidebarItemsModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit SidebarItemsModel(QObject *parent = 0);

        int columnCount(const QModelIndex & = QModelIndex()) const { return 1; }
        int rowCount(const QModelIndex & parent) const;

        QVariant data(const QModelIndex & index, int role) const;

        Qt::ItemFlags flags(const QModelIndex & index) const;

        QModelIndex index(int row, int column, const QModelIndex & parent) const;
        QModelIndex parent(const QModelIndex & index) const;

        QMimeData* mimeData(const QModelIndexList & indexes) const;

    private:
        friend class SidebarItemsBrowser;

        CategoryItem *rootItem;
    };

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
     * \sa TreeView, SidebarItemsModel
     */
    class SidebarItemsBrowser : public QWidget
    {
        Q_OBJECT

    public:
        explicit SidebarItemsBrowser(QWidget *parent = 0);
        ~SidebarItemsBrowser();

        void plugLibrary(QString libraryName, QString category);
        void unPlugLibrary(QString libraryName, QString category);

        void plugItem(QString itemName, const QPixmap& itemPixmap, QString category);
        void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);

        QString currentComponent();
        void filterItems();

    signals:
        void itemClicked(const QString& item, const QString& category);
        void itemDoubleClicked(const QString& item, const QString& category);

    protected:
        bool eventFilter(QObject *object, QEvent *event);

    private Q_SLOTS:
        void filterTextChanged();
        void slotOnClicked(const QModelIndex& index);

    private:
        SidebarItemsModel *m_model;
        FilterProxyModel *m_proxyModel;
        QTreeView *m_treeView;

        QLineEdit *m_filterEdit;

        QString m_currentComponent;
    };

} // namespace Caneda

#endif //SIDEBAR_ITEMS_BROWSER_H
