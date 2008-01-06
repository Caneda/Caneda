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
      CategoryItem(const QString& name, bool _isComponent, CategoryItem *parent = 0);
      ~CategoryItem();

      CategoryItem *parent() const { return m_parentItem; }

      CategoryItem *child(int row) const;
      int childCount() const { return m_childItems.size(); }

      int row() const;
      QString name() const { return m_name; }

      QPixmap iconPixmap() const { return m_iconPixmap; }
      bool isComponent() const { return m_isComponent; }

   private:
      void addChild(CategoryItem* c);

      QString m_name;
      bool m_isComponent;
      QPixmap m_iconPixmap;
      QList<CategoryItem*> m_childItems;
      CategoryItem *m_parentItem;
};

CategoryItem::CategoryItem(const QString& name, bool _isComponent, CategoryItem *parent) :
   m_name(name), m_isComponent(_isComponent), m_parentItem(parent)
{
   if(m_parentItem) {
      m_parentItem->addChild(this);
      if(_isComponent) {
         const Library *libItem =
            LibraryLoader::defaultInstance()->library(m_parentItem->name());
         if(libItem) {
            m_iconPixmap = libItem->renderedPixmap(m_name);
         }
      }
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
   return m_childItems.value(row < m_childItems.size() ? row : 0);
}

void CategoryItem::addChild(CategoryItem *c)
{
   c->m_parentItem = const_cast<CategoryItem*>(this);
   m_childItems << c;
}

int CategoryItem::row() const
{
   if(m_parentItem)
      return m_parentItem->m_childItems.indexOf(const_cast<CategoryItem*>(this));
   return 0;
}


SidebarModel::SidebarModel(QObject *parent) : QAbstractItemModel(parent)
{
   rootItem = new CategoryItem("Root", false);
}

void SidebarModel::plugLibrary(const QString& libraryName)
{
   const Library *libItem = LibraryLoader::defaultInstance()->library(libraryName);
   if(!libItem) return;

   CategoryItem *libRoot = new CategoryItem(libraryName, false, rootItem);
   QList<ComponentDataPtr> components = libItem->components().values();
   foreach(const ComponentDataPtr data, components) {
      new CategoryItem(data->name, true, libRoot);
   }
   reset();
}

QModelIndex SidebarModel::index ( int row, int column, const QModelIndex & parent ) const
{
   if(column != 0)
      return QModelIndex();
   CategoryItem *parentItem;
   parentItem = !parent.isValid() ? rootItem : static_cast<CategoryItem*>(parent.internalPointer());
   CategoryItem *childItem = parentItem->child(row);
   return childItem ? createIndex(row, 0, childItem) : QModelIndex();
}

int SidebarModel::rowCount ( const QModelIndex & parent) const
{
   CategoryItem *parentItem;
   parentItem = !parent.isValid() ? rootItem : static_cast<CategoryItem*>(parent.internalPointer());
   return parentItem->childCount();
}

QModelIndex SidebarModel::parent ( const QModelIndex & index ) const
{
   if (!index.isValid())
      return QModelIndex();

   CategoryItem *childItem = static_cast<CategoryItem*>(index.internalPointer());
   CategoryItem *parentItem = childItem->parent();

   if (parentItem == rootItem)
      return QModelIndex();

   return createIndex(parentItem->row(), 0, parentItem);
}

QVariant SidebarModel::data ( const QModelIndex & index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
   if (role == Qt::DisplayRole)
      return QVariant(item->name());
   else if(role == Qt::DecorationRole && item->isComponent()) {
      return QVariant(QIcon(item->iconPixmap()));
   }
   else if(role == Qt::EditRole && item->isComponent()) {
      //HACK: Using unsed role for sending topleft of item.
      const Library *libItem =
         LibraryLoader::defaultInstance()->library(item->parent()->name());
      if(libItem) {
         const ComponentDataPtr data = libItem->componentDataPtr(item->name());
         if(data.constData()) {
            const QString symbol = data->propertyMap["symbol"].value().toString();
            const QString svgId = item->name() + '/' + symbol;

            QPointF translateHint = SvgPainter::defaultInstance()->boundingRect(svgId).topLeft();
            translateHint *= -1;
            return QVariant(translateHint);
         }
      }
   }
   else if(role == Qt::SizeHintRole) {
      QSize sz(150, 32);
      return sz;
   }
   return QVariant();
}

bool SidebarModel::isComponent(const QModelIndex& index) const
{
   return (index.isValid() &&
           static_cast<CategoryItem*>(index.internalPointer())->isComponent());
}

Qt::ItemFlags SidebarModel::flags(const QModelIndex& index) const
{
   if(!index.isValid())
      return QAbstractItemModel::flags(index);
   CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
   if(item->isComponent())
      return (Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
   return Qt::ItemIsEnabled;
}

QStringList SidebarModel::mimeTypes() const
{
   QStringList l;
   l << "application/qucs.sidebarItem";
   return l;
}

QPixmap SidebarModel::pixmap(const QModelIndex& index) const
{
   if (!index.isValid()) {
      return QPixmap();
   }

   CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
   return item->iconPixmap();
}

QMimeData* SidebarModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *mimeData = new QMimeData();
   QByteArray encodedData;

   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   foreach (QModelIndex index, indexes) {
      if (index.isValid())
      {
         CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
         stream << item->name();
      }
   }

   mimeData->setData("application/qucs.sidebarItem", encodedData);
   return mimeData;
}
