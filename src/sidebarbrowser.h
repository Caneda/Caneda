/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013-2014 by Pablo Daniel Pareja Obregon                  *
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

#ifndef SIDEBAR_BROWSER_H
#define SIDEBAR_BROWSER_H

#include <QAbstractItemModel>
#include <QPair>
#include <QPixmap>
#include <QTreeView>

namespace Caneda
{
    // Forward declarations
    class CLineEdit;
    class FilterProxyModel;
    class SidebarModel;

    /*!
     * \brief The CategoryItem class implements the items to be used by the
     * SidebarModel class.
     *
     * This class implements a custom type of items to be inserted in a
     * QAbstractItemModel. It is used in Caneda by the SidebarModel class,
     * to insert items and have the ability to use QPixmaps associated with
     * each item inserted. Items can also correspond to categories
     * (or libraries), in which case the QPixmap is not necessary.
     *
     * \sa SidebarModel
     */
    class CategoryItem
    {
    public:
        CategoryItem(const QString& name, const QString& filename,
                const QPixmap &pixmap = QPixmap(), bool isLibrary = false,
                CategoryItem *parent = 0);
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
        bool m_isLibrary;
        QPixmap m_iconPixmap;
        QList<CategoryItem*> m_childItems;
        CategoryItem *m_parentItem;
    };


    /*!
     * \brief The SidebarModel class implements a custom QAbstractItemModel,
     * and provides the actual interface for item model classes.
     *
     * The QAbstractItemModel class defines the standard interface that item
     * models must use to be able to interoperate with other components in the
     * model/view architecture. It is not supposed to be instantiated directly,
     * and must be subclassed to create new models. The SidebarModel class is
     * the subclass implemented by Caneda.
     *
     * The SidebarModel class is one of the Model/View Classes and implements
     * the model part of Qt's model/view framework.
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
    class SidebarModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        enum {
            DragPixmapRole = Qt::UserRole + 1
        };

        SidebarModel(QObject *parent=0);

        int columnCount(const QModelIndex & parent = QModelIndex()) const {
            Q_UNUSED(parent);
            return 1;
        }

        int rowCount(const QModelIndex & parent) const;

        QVariant data(const QModelIndex & index, int role) const;
        QModelIndex index(int row, int column, const QModelIndex & parent) const;

        QModelIndex parent(const QModelIndex & index) const;

        Qt::ItemFlags flags(const QModelIndex& index) const;

        bool isLeaf(const QModelIndex& index) const;
        bool isLibrary(const QModelIndex& index) const;

        QStringList mimeTypes() const;
        QMimeData* mimeData(const QModelIndexList& indexes) const;

        void plugLibrary(const QString& libraryName, const QString& category);
        void unPlugLibrary(const QString& libraryName, const QString& category);

        int libraryRow(const QString& libraryName, const QString& category);

        void plugItem(QString itemName, const QPixmap& itemPixmap, QString category);
        void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);

    private:
        CategoryItem *rootItem;
    };

    /*!
     * \brief The TreeView class implements a custom QTreeView, and provides a
     *  model/view implementation of a tree view.
     *
     * The TreeView class implements a tree representation of items from a
     * model. This class is used to provide standard hierarchical lists, using
     * the flexible approach provided by Qt's model/view architecture.
     *
     * The TreeView class, derived from QTreeView is one of the Model/View
     * Classes defined by Qt, and is part of the model/view framework. TreeView
     * implements the interfaces defined by the QAbstractItemView class to
     * allow it to display data provided by models derived from the
     * QAbstractItemModel class.
     *
     * The model/view architecture ensures that the contents of the tree view
     * are updated as the model changes.
     *
     *  \sa SidebarModel
     */
    class TreeView : public QTreeView
    {
        Q_OBJECT

    public:
        TreeView(QWidget *parent = 0);

        void startDrag(Qt::DropActions supportedActions);

    signals:
        void invalidAreaClicked(const QModelIndex &index);

        protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

        bool invalidPressed;
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
     * \sa TreeView, SidebarModel
     */
    class SidebarBrowser : public QWidget
    {
        Q_OBJECT

    public:
        SidebarBrowser(QWidget *parent = 0);
        ~SidebarBrowser();

        void plugLibrary(QString libraryName, QString category);
        void unPlugLibrary(QString libraryName, QString category);

        void plugItem(QString itemName, const QPixmap& itemPixmap, QString category);
        void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);

        QString currentComponent();

    signals:
        void itemClicked(const QString& item, const QString& category);
        void itemDoubleClicked(const QString& item, const QString& category);

    private Q_SLOTS:
        void filterTextChanged();
        void slotOnClicked(const QModelIndex& index);
        void slotOnDoubleClicked(const QModelIndex& index);

    private:
        SidebarModel *m_model;
        FilterProxyModel *m_proxyModel;
        TreeView *m_treeView;

        CLineEdit *m_filterEdit;

        QString m_currentComponent;
    };

} // namespace Caneda

#endif //SIDEBAR_BROWSER_H
