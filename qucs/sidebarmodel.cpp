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

#include "library.h"
#include "sidebarmodel.h"

#include "qucs-tools/global.h"

#include <QDebug>
#include <QIcon>
#include <QList>
#include <QMimeData>
#include <QPainter>
#include <QtAlgorithms>
#include <QVariant>

CategoryItem::CategoryItem(const QString& name, const QString& filename,
        const QPixmap& pixmap, bool isLibrary, CategoryItem *parent) :
    m_name(name),
    m_filename(filename),
    m_iconPixmap(pixmap),
    m_isLibrary(isLibrary),
    m_parentItem(parent)
{
    if(m_parentItem) {
        m_parentItem->addChild(this);
    }
}

CategoryItem::~CategoryItem()
{
    qDeleteAll(m_childItems);
}

CategoryItem* CategoryItem::child(int row) const
{
    if(m_childItems.isEmpty() || row >= m_childItems.size()) {
        return 0;
    }
    return m_childItems.value(row);
}

void CategoryItem::addChild(CategoryItem *c)
{
    c->m_parentItem = const_cast<CategoryItem*>(this);
    m_childItems << c;
}

void CategoryItem::removeChild(int c)
{
    m_childItems.removeAt(c);
}

int CategoryItem::row() const
{
    if(m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<CategoryItem*>(this));
    }
    return 0;
}

//! Constructor
SidebarModel::SidebarModel(QObject *parent) : QAbstractItemModel(parent)
{
    rootItem = new CategoryItem("Root", "");
}

/*!
 * \brief Adding a library to the sidebar
 *
 * @param QString &libraryName     Library name
 * @param QString &category        Category to place the library
 */
void SidebarModel::plugLibrary(const QString& libraryName, const QString& category)
{
    const Library *libItem = LibraryLoader::instance()->library(libraryName);
    if(!libItem) {
        return;
    }

    CategoryItem *libRoot;
    if(category == "root") {
        libRoot = new CategoryItem(libItem->libraryName(), libItem->libraryFileName(),
                QPixmap(), true, rootItem);
    }
    else {
        for(int i = 0; i < rootItem->childCount(); i++) {
            if(rootItem->child(i)->name() == category) {
                libRoot = new CategoryItem(libItem->libraryName(), libItem->libraryFileName(),
                        QPixmap(), true, rootItem->child(i));
                break;
            }
        }
    }

    QList<ComponentDataPtr> components = libItem->components().values();
    foreach(const ComponentDataPtr data, components) {
        new CategoryItem(data->name, data->filename,
                libItem->renderedPixmap(data->name), false, libRoot);
    }
    reset();
}

void SidebarModel::unPlugLibrary(const QString& libraryName, const QString& category)
{
    const Library *libItem = LibraryLoader::instance()->library(libraryName);
    if(!libItem) {
        return;
    }

    if(category == "root") {
        for(int i = 0; i < rootItem->childCount(); i++) {
            if(rootItem->child(i)->filename() == libItem->libraryFileName()) {
                rootItem->removeChild(i);
                break;
            }
        }
    }
    else {
        for(int i = 0; i < rootItem->childCount(); i++) {
            if(rootItem->child(i)->name() == category) {
                for(int j = 0; j < rootItem->child(i)->childCount(); j++) {
                    if(rootItem->child(i)->child(j)->filename() ==
                            libItem->libraryFileName()) {
                        rootItem->child(i)->removeChild(j);
                        break;
                    }
                }
            }
        }
    }

    reset();
}

void SidebarModel::plugItem(QString itemName, const QPixmap& itemPixmap, QString category)
{
    CategoryItem *catItem = 0;

    if(category == "root") {
        catItem = new CategoryItem(itemName, "",
                QPixmap(), true, rootItem);
    }
    else {
        for(int i = 0; i < rootItem->childCount(); i++) {
            if(rootItem->child(i)->name() == category) {
                catItem = rootItem->child(i);
                break;
            }
        }

        if(!catItem) {
            catItem = new CategoryItem(category, category, QPixmap(), false, rootItem);
        }

        new CategoryItem(itemName, category, itemPixmap, false, catItem);
        reset();
    }
}

void SidebarModel::plugItems(const QList<QPair<QString, QPixmap> > &items,
        QString category)
{
    CategoryItem *catItem = 0;

    for(int i = 0; i < rootItem->childCount(); i++) {
        if(rootItem->child(i)->name() == category) {
            catItem = rootItem->child(i);
            break;
        }
    }

    if(!catItem) {
        catItem = new CategoryItem(category, category, QPixmap(), false, rootItem);
    }

    QList<QPair<QString, QPixmap> >::const_iterator it = items.begin(),
        end = items.end();

    while(it != end) {
        new CategoryItem(it->first, category, it->second, false, catItem);
        ++it;
    }
    reset();
}

QModelIndex SidebarModel::index(int row, int column, const QModelIndex & parent) const
{
    if(column != 0) {
        return QModelIndex();
    }
    CategoryItem *parentItem;
    parentItem = !parent.isValid() ? rootItem : static_cast<CategoryItem*>(parent.internalPointer());
    CategoryItem *childItem = parentItem->child(row);
    return childItem ? createIndex(row, 0, childItem) : QModelIndex();
}

int SidebarModel::rowCount(const QModelIndex & parent) const
{
    CategoryItem *parentItem;
    parentItem = !parent.isValid() ? rootItem :
        static_cast<CategoryItem*>(parent.internalPointer());
    return parentItem->childCount();
}

QModelIndex SidebarModel::parent(const QModelIndex & index) const
{
    if(!index.isValid()) {
        return QModelIndex();
    }

    CategoryItem *childItem = static_cast<CategoryItem*>(index.internalPointer());
    CategoryItem *parentItem = childItem->parent();

    if(parentItem == rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant SidebarModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());

    if(role == Qt::DisplayRole) {
        return QVariant(item->name());
    }

    if(item->isLeaf() && !item->isLibrary()) {
        switch(role) {
            case Qt::DecorationRole :
                return QVariant(QIcon(item->iconPixmap()));

            case DragPixmapRole:
                return QVariant(QPixmap(item->iconPixmap()));
        }
    }

    if(role == Qt::SizeHintRole) {
        return QSize(150, 32);
    }
    return QVariant();
}

bool SidebarModel::isLeaf(const QModelIndex& index) const
{
    return (index.isValid() &&
            static_cast<CategoryItem*>(index.internalPointer())->isLeaf());
}

bool SidebarModel::isLibrary(const QModelIndex& index) const
{
    return (index.isValid() &&
            static_cast<CategoryItem*>(index.internalPointer())->isLibrary());
}

Qt::ItemFlags SidebarModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flag = Qt::ItemIsEnabled;
    if(isLeaf(index) && !isLibrary(index)) {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
    return flag;
}

QStringList SidebarModel::mimeTypes() const
{
    return QStringList() << "application/qucs.sidebarItem";
}

QMimeData* SidebarModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach(QModelIndex index, indexes) {
        if(index.isValid()) {
            CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
            if(item->isLeaf() && !item->isLibrary()) {
                QString category = item->parent()->name();
                stream << item->name() << category;
            }
        }
    }

    mimeData->setData("application/qucs.sidebarItem", encodedData);
    return mimeData;
}
