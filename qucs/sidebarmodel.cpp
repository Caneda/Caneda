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

#include <QtCore/QList>
#include <QtCore/QtAlgorithms>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtCore/QDebug>
#include <QtCore/QMimeData>


class CategoryItem
{
   public:
      CategoryItem(const QString& n, const QString& m="",const QString& f=QString(),CategoryItem *parent = 0,bool isc = true);
      ~CategoryItem();

      CategoryItem *parent() const;
      CategoryItem *child(int row) const;
      int childCount() const;
      void addChild(CategoryItem* c);
      void addChild(const QString& name,const QString& m,const QString& fname = QString(),bool isc = true);
      int row() const;
      QString name() const { return m_name; }
      QString model() const { return m_model; }
      QString fileName() const { return m_fileName; }
      bool isComponent() const { return m_isComponent; }

   private:
      QString m_name;
      QString m_fileName;
      QString m_model;
      bool m_isComponent;
      QList<CategoryItem*> m_childItems;
      CategoryItem *m_parentItem;

};

CategoryItem::CategoryItem(const QString& n, const QString& m,const QString& f, CategoryItem *parent, bool isc)
{
   m_name = n;
   m_model = m;
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

void CategoryItem::addChild(const QString& name,const QString& m,const QString& fname,bool isc)
{
   CategoryItem *n = new CategoryItem(name,m,fname,0,isc);
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
   CategoryItem *lumped = new CategoryItem(tr("Lumped Components"),"","",rootItem,false);
   CategoryItem *sources = new CategoryItem(tr("Sources"),"","",rootItem,false);
   CategoryItem *transmission = new CategoryItem(tr("Transmission lines"),"","",rootItem,false);
   CategoryItem *nonlinear = new CategoryItem(tr("Non linear components"),"","",rootItem,false);
   CategoryItem *digital = new CategoryItem(tr("Digital"),"","",rootItem,false);
   
   lumped->addChild( QObject::tr("Resistor"), "Resistor", "resistor.png");
   lumped->addChild( QObject::tr("Resistor"), "ResistorUS", "resistor_us.png");
   lumped->addChild( QObject::tr("Amplifier"), "Amp", "amplifier.png");
   lumped->addChild( QObject::tr("Attenuator"), "Attenuator", "attenuator.png");
   lumped->addChild( QObject::tr("Bias T"), "BiasT", "biast.png");
   lumped->addChild( QObject::tr("Circulator"), "Circulator", "circulator.png");
   lumped->addChild( QObject::tr("Coupler"), "Coupler", "coupler.png");
   lumped->addChild( QObject::tr("dc Block"), "DCBlock", "dcblock.png");
   lumped->addChild( QObject::tr("dc Feed"), "DCFeed", "dcfeed.png");
   lumped->addChild( QObject::tr("Ground"), "GND", "gnd.png");
   lumped->addChild( QObject::tr("Gyrator"), "Gyrator", "gyrator.png");
   lumped->addChild( QObject::tr("Inductor"), "L", "inductor.png");
   lumped->addChild( QObject::tr("Current Probe"), "IProbe", "iprobe.png");
   lumped->addChild( QObject::tr("Isolator"), "Isolator", "isolator.png");
   lumped->addChild( QObject::tr("3 Mutual Inductors"), "MUT2", "mutual2.png");
   lumped->addChild( QObject::tr("Mutual Inductors"), "MUT", "mutual.png");
   lumped->addChild( QObject::tr("Phase Shifter"), "PShift", "pshifter.png");
   lumped->addChild( QObject::tr("Relay"), "Relais", "relais.png");
   lumped->addChild( QObject::tr("symmetric Transformer"), "sTr", "symtrans.png");
   lumped->addChild( QObject::tr("Transformer"), "Tr", "transformer.png");
   lumped->addChild( QObject::tr("Voltage Probe"), "VProbe", "vprobe.png");


   sources->addChild( QObject::tr("AM modulated Source"), "AM_Mod", "am_mod.png");
   sources->addChild( QObject::tr("ac Current Source"), "Iac", "ac_current.png");
   sources->addChild( QObject::tr("dc Current Source"), "Idc", "dc_current.png");
   sources->addChild( QObject::tr("Noise Current Source"), "Inoise", "noise_current.png");
   sources->addChild( QObject::tr("Current Controlled Current Source"), "CCCS", "cccs.png");
   sources->addChild( QObject::tr("Current Controlled Voltage Source"), "CCVS", "ccvs.png");
   sources->addChild( QObject::tr("Current Pulse"), "Ipulse", "ipulse.png");
   sources->addChild( QObject::tr("Rectangle Current"), "Irect", "irect.png");
   sources->addChild( QObject::tr("Correlated Noise Sources"), "IInoise", "noise_ii.png");
   sources->addChild( QObject::tr("Correlated Noise Sources"), "IVnoise", "noise_iv.png");
   sources->addChild( QObject::tr("Correlated Noise Sources"), "VVnoise", "noise_vv.png");
   sources->addChild( QObject::tr("PM modulated Source"), "PM_Mod", "pm_mod.png");
   sources->addChild( QObject::tr("Power Source"), "Pac", "source.png");
   sources->addChild( QObject::tr("Voltage Controlled Current Source"), "VCCS", "vccs.png");
   sources->addChild( QObject::tr("Voltage Controlled Voltage Source"), "VCVS", "vcvs.png");
   sources->addChild( QObject::tr("ac Voltage Source"), "Vac", "ac_voltage.png");
   sources->addChild( QObject::tr("dc Voltage Source"), "Vdc", "dc_voltage.png");
   sources->addChild( QObject::tr("Noise Voltage Source"), "Vnoise", "noise_volt.png");
   sources->addChild( QObject::tr("Voltage Pulse"), "Vpulse", "vpulse.png");
   sources->addChild( QObject::tr("Rectangle Voltage"), "Vrect", "vrect.png");


   transmission->addChild( QObject::tr("Bond Wire"), "BOND", "bondwire.png");
   transmission->addChild( QObject::tr("Coaxial Line"), "COAX", "coaxial.png");
   transmission->addChild( QObject::tr("Coplanar Line"), "CLIN", "coplanar.png");
   transmission->addChild( QObject::tr("Coplanar Gap"), "CGAP", "cpwgap.png");
   transmission->addChild( QObject::tr("Coplanar Open"), "COPEN", "cpwopen.png");
   transmission->addChild( QObject::tr("Coplanar Short"), "CSHORT", "cpwshort.png");
   transmission->addChild( QObject::tr("Coplanar Step"), "CSTEP", "cpwstep.png");
   transmission->addChild( QObject::tr("Microstrip Corner"), "MCORN", "mscorner.png");
   transmission->addChild( QObject::tr("Coupled Microstrip Line"), "MCOUPLED", "mscoupled.png");
   transmission->addChild( QObject::tr("Microstrip Gap"), "MGAP", "msgap.png");
   transmission->addChild( QObject::tr("Microstrip Line"), "MLIN", "msline.png");
   transmission->addChild( QObject::tr("Microstrip Mitered Bend"), "MMBEND", "msmbend.png");
   transmission->addChild( QObject::tr("Microstrip Open"), "MOPEN", "msopen.png");
   transmission->addChild( QObject::tr("Microstrip Step"), "MSTEP", "msstep.png");
   transmission->addChild( QObject::tr("Microstrip Via"), "MVIA", "msvia.png");
   transmission->addChild( QObject::tr("Substrate"), "SUBST", "substrate.png");
   transmission->addChild( QObject::tr("4-Terminal Transmission Line"), "TLIN4P", "tline_4port.png");
   transmission->addChild( QObject::tr("Transmission Line"), "TLIN", "tline.png");
   transmission->addChild( QObject::tr("Twisted-Pair"), "TWIST", "twistedpair.png");


   nonlinear->addChild( QObject::tr("OpAmp"), "OpAmp", "opamp.png");


   digital->addChild( QObject::tr("D-FlipFlop"), "DFF", "dflipflop.png");
   digital->addChild( QObject::tr("digital source"), "DigiSource", "digi_source.png");
   digital->addChild( QObject::tr("JK-FlipFlop"), "JKFF", "jkflipflop.png");
   digital->addChild( QObject::tr("RS-FlipFlop"), "RSFF", "rsflipflop.png");
   
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
      return QVariant(QIcon(Qucs::bitmapDirectory() + item->fileName()));
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
      if (index.isValid())
      {
         CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
         //QString text = data(index, Qt::DisplayRole).toString();
         stream << item->model();
      }
   }

   mimeData->setData("application/qucs.sidebarItem", encodedData);
   return mimeData;
}
