/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef SIDEBARMODEL_H
#define SIDEBARMODEL_H

#include <QAbstractItemModel>
#include <QPair>
#include <QPixmap>

class LibraryLoader;

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
    Q_OBJECT;
public:
    enum {
        DragPixmapRole = Qt::UserRole + 1,
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

    void plugItem(QString itemName, const QPixmap& itemPixmap, QString category);
    void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);

private:
    CategoryItem *rootItem;
};

#endif //SIDEBARMODEL_H
