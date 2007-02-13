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

#include "global.h"
#include "sidebarmodel.h"

#include <QtCore/QList>
#include <QtCore/QtAlgorithms>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtCore/QDebug>
#include <QtCore/QMimeData>


class CategoryItem
{
   public:
      CategoryItem(const QString& n, const QString& f=QString(),CategoryItem *parent = 0,bool isc = true);
      ~CategoryItem();

      CategoryItem *parent() const;
      CategoryItem *child(int row) const;
      int childCount() const;
      void addChild(CategoryItem* c);
      void addChild(const QString& name,const QString& fname = QString(),bool isc = true);
      int row() const;
      QString name() const { return m_name; }
      QString fileName() const { return m_fileName; }
      bool isComponent() const { return m_isComponent; }

   private:
      QString m_name;
      QString m_fileName;
      bool m_isComponent;
      QList<CategoryItem*> m_childItems;
      CategoryItem *m_parentItem;

};

CategoryItem::CategoryItem(const QString& n, const QString& f, CategoryItem *parent, bool isc)
{
   m_name = n;
   m_fileName = f;
   m_isComponent = isc;
   m_parentItem = parent;
   if(parent)
      parent->addChild(this);
}

CategoryItem::~CategoryItem()
{
   qDeleteAll(m_childItems);
}

CategoryItem* CategoryItem::parent() const
{
   return m_parentItem;
}

CategoryItem* CategoryItem::child(int row) const
{
   if(m_childItems.isEmpty())
      return 0;
   if(row < m_childItems.size())
      return m_childItems.value(row);
   return m_childItems.value(0);
}

int CategoryItem::childCount() const
{
   return m_childItems.size();
}

void CategoryItem::addChild(CategoryItem *c)
{
   c->m_parentItem = const_cast<CategoryItem*>(this);
   m_childItems << c;
}

void CategoryItem::addChild(const QString& name,const QString& fname,bool isc)
{
   CategoryItem *n = new CategoryItem(name,fname,0,isc);
   n->m_parentItem = const_cast<CategoryItem*>(this);
   m_childItems << n;
}

int CategoryItem::row() const
{
   if(m_parentItem)
      return m_parentItem->m_childItems.indexOf(const_cast<CategoryItem*>(this));
   return 0;
}


SidebarModel::SidebarModel(QObject *parent) : QAbstractItemModel(parent)
{
   rootItem = new CategoryItem("Root");
   fillData();
}

void SidebarModel::fillData()
{
   CategoryItem *lumped = new CategoryItem(tr("Lumped Components"),QString(),rootItem,false);
   CategoryItem *sources = new CategoryItem(tr("Sources"),QString(),rootItem,false);


   lumped->addChild(tr("Resistor"),QString("resistor.png"));
   lumped->addChild(tr("ResistorUS"),QString("resistor_us.png"));
   lumped->addChild(tr("Capacitor"),QString("capacitor.png"));
   lumped->addChild(tr("Inductor"),QString("inductor.png"));
   lumped->addChild(tr("Ground"),QString("ground.png"));
   lumped->addChild(tr("Subcircuit Port"),QString("subport.png"));
   lumped->addChild(tr("Transformer"),QString("transformer.png"));
   lumped->addChild(tr("Symmetric Transformer"),QString("symtrans.png"));
   lumped->addChild(tr("DC Block"),QString("dcblock.png"));
   lumped->addChild(tr("DC Feed"),QString("dcfeed.png"));
   lumped->addChild(tr("Bias T"),QString("biast.png"));
   lumped->addChild(tr("Attenuator"),QString("attenuator.png"));
   lumped->addChild(tr("Amplifier"),QString("amplifier.png"));
   lumped->addChild(tr("Isolator"),QString("isolator.png"));
   lumped->addChild(tr("Circulator"),QString("circulator.png"));
   lumped->addChild(tr("Gyrator"),QString("gyrator.png"));
   lumped->addChild(tr("Phase Shifter"),QString("pshifter.png"));
   lumped->addChild(tr("Coupler"),QString("coupler.png"));
   lumped->addChild(tr("Current Probe"),QString("iprobe.png"));
   lumped->addChild(tr("Voltage Probe"),QString("vprobe.png"));
   lumped->addChild(tr("Mutual Inductors"),QString(".png"));
   lumped->addChild(tr("3 Mutual Inductors"),QString(".png"));
   lumped->addChild(tr("Switch"),QString("switch.png"));
   lumped->addChild(tr("Relay"),QString("relay.png"));

   sources->addChild(tr("DC Voltage Source"),QString("dc_voltage.png"));
   sources->addChild(tr("DC Current Source"),QString("dc_current.png"));
   sources->addChild(tr("AC Voltage Source"),QString("ac_voltage.png"));
   sources->addChild(tr("AC Current Source"),QString("ac_current.png"));
   sources->addChild(tr("Power Source"),QString("source.png"));
   sources->addChild(tr("Noise Voltage Source"),QString("noise_volt.png"));
   sources->addChild(tr("Noise Current Source"),QString("noise_current.png"));
   sources->addChild(tr("Voltage Controlled Current Source"),QString("vccs.png"));
   sources->addChild(tr("Current Controlled Current Source"),QString("cccs.png"));
   sources->addChild(tr("Voltage Controlled Voltage Source"),QString("vcvs.png"));
   sources->addChild(tr("Current Controlled Voltage Source"),QString("ccvs.png"));
   sources->addChild(tr("Voltage Pulse"),QString("vpulse.png"));
   sources->addChild(tr("Current Pulse"),QString("ipulse.png"));
   sources->addChild(tr("Rectangle Voltage"),QString("vrect.png"));
   sources->addChild(tr("Rectangle Current"),QString("irect.png"));
   sources->addChild(tr("Correlated Noise Sources"),QString("noise_ii.png"));
   sources->addChild(tr("Correlated Noise Sources"),QString("noise_vv.png"));
   sources->addChild(tr("Correlated Noise Sources"),QString("noise_iv.png"));
   sources->addChild(tr("AM Modulated Source"),QString("am_mod.png"));
   sources->addChild(tr("PM Modulated Source"),QString("pm_mod.png"));

}

QModelIndex SidebarModel::index ( int row, int column, const QModelIndex & parent ) const
{
   if(column != 0)
      return QModelIndex();
   CategoryItem *parentItem;

   if (!parent.isValid())
      parentItem = rootItem;
   else
      parentItem = static_cast<CategoryItem*>(parent.internalPointer());

   CategoryItem *childItem = parentItem->child(row);
   if (childItem)
      return createIndex(row, 0, childItem);
   else
      return QModelIndex();
}

int SidebarModel::rowCount ( const QModelIndex & parent) const
{
   CategoryItem *parentItem;

   if (!parent.isValid())
      parentItem = rootItem;
   else
      parentItem = static_cast<CategoryItem*>(parent.internalPointer());

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
   else if(role == Qt::DecorationRole && item->isComponent())
      return QVariant(QIcon(imageDirectory() + item->fileName()));
   return QVariant();
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

QMimeData* SidebarModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *mimeData = new QMimeData();
   QByteArray encodedData;

   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   foreach (QModelIndex index, indexes) {
      if (index.isValid()) {
         QString text = data(index, Qt::DisplayRole).toString();
         stream << text;
      }
   }

   mimeData->setData("application/qucs.sidebarItem", encodedData);
   return mimeData;
}
