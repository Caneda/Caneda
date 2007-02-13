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

#include "capacitor.h"
#include "node.h"
#include "propertytext.h"
#include "schematicscene.h"

#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionGraphicsItem>

Capacitor::Capacitor(QGraphicsScene *s) : Component(0,s)
{
   m_ports.append(new ComponentPort(this,QPointF(-30.0,0.0)));
   m_ports.append(new ComponentPort(this,QPointF(30.0,0.0)));
   
   PropertyText *t1 = new PropertyText("C","0.1mF","Simple capacitor",0,scene());
   m_properties.append(t1);
   if(s)
      t1->setPos(pos() + QPointF(0,-35));
}

QString Capacitor::netlist() const
{
   QString s = model+":"+name;

   // output all node names
   foreach(ComponentPort *port, m_ports)
      s += ' ' + port->node()->name(); // node names
   
   // output all properties
   foreach(PropertyText *prop, m_properties)
   {
      if(prop->name() != "Symbol")
         s += ' ' + prop->name() + "'=\"" + prop->value() + "\"";
   }
   return s;
}

void Capacitor::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
   Q_UNUSED(w);
   initPainter(p,o);
   
   QPen pen = p->pen();
   
   pen.setWidth(4);
   p->setPen(pen);
   p->drawLine(QLineF( -4,-11, -4, 11));
   p->drawLine(QLineF(  4,-11,  4, 11));
   pen.setWidth(1);
   p->setPen(pen);
   p->drawLine(QLineF(-27,  0, -4,  0));
   p->drawLine(QLineF(  4,  0, 27,  0));

   if(o->state & QStyle::State_Open)
      drawNodes(p);
}


