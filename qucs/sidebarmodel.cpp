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

#include "qucs-tools/global.h"
#include "sidebarmodel.h"
#include "library.h"

#include <QtCore/QList>
#include <QtCore/QtAlgorithms>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtGui/QPainter>

class CategoryItem
{
   public:
      CategoryItem(const QString& name, const QPixmap &pixmap = QPixmap(),
                   CategoryItem *parent = 0);
      ~CategoryItem();

      CategoryItem *parent() const { return m_parentItem; }

      CategoryItem *child(int row) const;
      int childCount() const { return m_childItems.size(); }

      int row() const;
      QString name() const { return m_name; }

      QPixmap iconPixmap() const { return m_iconPixmap; }
      bool isLeaf() const { return m_childItems.isEmpty(); }

   private:
      void addChild(CategoryItem* c);

      QString m_name;
      QPixmap m_iconPixmap;
      QList<CategoryItem*> m_childItems;
      CategoryItem *m_parentItem;
};

CategoryItem::CategoryItem(const QString& name, const QPixmap& pixmap, CategoryItem *parent) :
   m_name(name), m_iconPixmap(pixmap), m_parentItem(parent)
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
   if(m_childItems.isEmpty() || row >= m_childItems.size())
      return 0;
   return m_childItems.value(row);
}

void CategoryItem::addChild(CategoryItem *c)
{
   c->m_parentItem = const_cast<CategoryItem*>(this);
   m_childItems << c;
}

int CategoryItem::row() const
{
   if(m_parentItem) {
      return m_parentItem->m_childItems.indexOf(const_cast<CategoryItem*>(this));
   }
   return 0;
}

SidebarModel::SidebarModel(QObject *parent) : QAbstractItemModel(parent)
{
   rootItem = new CategoryItem("Root");
   libComp = new CategoryItem(QObject::tr("Components"),
                              QPixmap(), rootItem);
}

void SidebarModel::plugLibrary(const QString& libraryName)
{
   const Library *libItem = LibraryLoader::defaultInstance()->library(libraryName);
   if(!libItem) return;

   CategoryItem *libRoot = new CategoryItem(libraryName, QPixmap(), libComp);
   QList<ComponentDataPtr> components = libItem->components().values();

   foreach(const ComponentDataPtr data, components) {
      new CategoryItem(data->name, libItem->renderedPixmap(data->name), libRoot);
   }
   reset();
}

void SidebarModel::plugItem(QString itemName, const QPixmap& itemPixmap, QString category)
{
   CategoryItem *catItem = 0;

   for(int i = 0; i < rootItem->childCount(); i++) {
      if(rootItem->child(i)->name() == category) {
         catItem = rootItem->child(i);
         break;
      }
   }

   if(!catItem) {
      catItem = new CategoryItem(category, QPixmap(), rootItem);
   }

   new CategoryItem(itemName, itemPixmap, catItem);
   reset();
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
      catItem = new CategoryItem(category, QPixmap(), rootItem);
   }

   QList<QPair<QString, QPixmap> >::const_iterator it = items.begin(),
      end = items.end();

   while(it != end) {
      new CategoryItem(it->first, it->second, catItem);
      ++it;
   }
   reset();
}

QModelIndex SidebarModel::index(int row, int column, const QModelIndex & parent) const
{
   if(column != 0)
      return QModelIndex();
   CategoryItem *parentItem;
   parentItem = !parent.isValid() ? rootItem : static_cast<CategoryItem*>(parent.internalPointer());
   CategoryItem *childItem = parentItem->child(row);
   return childItem ? createIndex(row, 0, childItem) : QModelIndex();
}

int SidebarModel::rowCount(const QModelIndex & parent) const
{
   CategoryItem *parentItem;
   parentItem = !parent.isValid() ? rootItem : static_cast<CategoryItem*>(parent.internalPointer());
   return parentItem->childCount();
}

QModelIndex SidebarModel::parent(const QModelIndex & index) const
{
   if (!index.isValid())
      return QModelIndex();

   CategoryItem *childItem = static_cast<CategoryItem*>(index.internalPointer());
   CategoryItem *parentItem = childItem->parent();

   if (parentItem == rootItem)
      return QModelIndex();

   return createIndex(parentItem->row(), 0, parentItem);
}

QVariant SidebarModel::data(const QModelIndex & index, int role) const
{
   if (!index.isValid())
      return QVariant();

   CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());

   if (role == Qt::DisplayRole) {
      return QVariant(item->name());
   }

   if(item->isLeaf()) {
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

Qt::ItemFlags SidebarModel::flags(const QModelIndex& index) const
{
   if(isLeaf(index)) {
      return (Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
   }
   return Qt::ItemIsEnabled;
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

   foreach (QModelIndex index, indexes) {
      if (index.isValid()) {
         CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
         if(item->isLeaf()) {
            QString category = item->parent()->name();
            stream << item->name() << category;
         }
      }
   }

   mimeData->setData("application/qucs.sidebarItem", encodedData);
   return mimeData;
}
