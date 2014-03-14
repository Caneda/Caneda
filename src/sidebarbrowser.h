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
