/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "port.h"

#include "component.h"
#include "schematicscene.h"

#include <QtGui/QPainter>
#include <QtCore/QDebug>

static const QPen connectedPen(Qt::red, 0);
static const QBrush connectedBrush(Qt::cyan);
static const QPen unconnectedPen(Qt::darkRed, 0);
static const QBrush unconnectedBrush(Qt::NoBrush);
static const qreal portRadius(3.0);
static const QRectF portEllipse(-portRadius, -portRadius, 2*portRadius,
                                 2*portRadius);

bool circleIntersects(const QPointF& c1, const QPointF& c2, qreal radius)
{
   qreal x_sqr = (c1.x() - c2.x()) * (c1.x() * c2.x());
   qreal y_sqr = (c1.y() - c2.y()) * (c1.y() * c2.y());
   return x_sqr + y_sqr - (4 * radius * radius) <= 0;
}

PortOwner::PortOwner(Qucs::Wire *wire) : m_wire(wire), m_component(0)
{
   Q_ASSERT(m_wire != 0 || m_component != 0);
}

PortOwner::PortOwner(Qucs::Component *component) : m_wire(0), m_component(component)
{
   Q_ASSERT(m_wire != 0 || m_component != 0);
}

Port::Port(Qucs::Wire *owner, const QSharedDataPointer<PortData> &data) :
      d(data),
      m_owner(new PortOwner(owner)),
      m_connections(0)
{
}

Port::Port(Qucs::Wire *owner, QPointF _pos, QString portName) :
      d(new PortData(_pos, portName)),
      m_owner(new PortOwner(owner)),
      m_connections(0)
{
}

Port::Port(Qucs::Component *owner, const QSharedDataPointer<PortData> &data) :
      d(data),
      m_owner(new PortOwner(owner)),
      m_connections(0)
{
}

Port::Port(Qucs::Component *owner, QPointF _pos, QString portName) :
      d(new PortData(_pos, portName)),
      m_owner(new PortOwner(owner)),
      m_connections(0)
{
}

Port::~Port()
{
   if(m_connections) {
      Port *other = 0;
      foreach(Port *p, *m_connections) {
         if(p != this) {
            other = p;
            break;
         }
      }
      if(other) {
         Port::disconnect(this, other);
      }
      else {
         Q_ASSERT(m_connections->size() <= 1);
         delete m_connections;
      }
   }
   //deletes the container only, not the actual component or wire.
   delete m_owner;
}

QPointF Port::scenePos(bool *ok) const
{
   if(ok) {
      *ok = m_owner->item()->scene() != 0;
   }
   return m_owner->item()->mapToScene(d->pos);
}

void Port::connectTo(Port *other)
{
   Port::connect(this, other);
}

void Port::connect(Port *port1, Port *port2)
{
   bool ok1, ok2;

   if(port1 == port2 || !port1 || !port2)
      return;

   QPointF p1 = port1->scenePos(&ok1);
   QPointF p2 = port2->scenePos(&ok2);

   if(!ok1 || !ok2 ||
       port1->ownerItem()->scene() != port2->ownerItem()->scene()) {
      qWarning() << "Cannot connect nodes across different or null scenes";
      return;
   }

   if(p1 != p2) {
      qWarning() << "Cannot connect nodes as positions mismatch" << p1 << p2;
      return;
   }

   if(!port1->m_connections && !port2->m_connections) {
      port1->m_connections = port2->m_connections = new QList<Port*>;
      *(port1->m_connections) << port1 << port2;
   }
   else if(!port1->m_connections) {
      port1->m_connections = port2->m_connections;
      //Q_ASSERT(!m_connections->contains(m_port1));
      *(port1->m_connections) << port1;
   }
   else if(!port2->m_connections) {
      port2->m_connections = port1->m_connections;
      *(port2->m_connections) << port2;
   }
   else {
      if(port1->m_connections == port2->m_connections) {
         Q_ASSERT(port1->m_connections->contains(port1));
         Q_ASSERT(port1->m_connections->contains(port2));
      }
      else if(port1->m_connections->size() >= port2->m_connections->size()) {
         *(port1->m_connections) += *(port2->m_connections);
         QList<Port*> *save = port2->m_connections;
         foreach(Port *p, *(port2->m_connections)) {
            p->m_connections = port1->m_connections;
         }
         delete save;
      }
      else {
         *(port2->m_connections) += *(port1->m_connections);
         QList<Port*> *save = port1->m_connections;
         foreach(Port *p, *(port1->m_connections)) {
            p->m_connections = port2->m_connections;
         }
         delete save;
      }
   }

   foreach(Port *p, *(port1->m_connections))
      p->ownerItem()->update();
}

void Port::disconnectFrom(Port *from)
{
   Port::disconnect(this, from);
}

void Port::disconnect(Port *port, Port *from)
{
   if(port == from || !port || !from) {
      return;
   }
   if(port->m_connections != from->m_connections || !port->m_connections) {
      qWarning() << "Cannot disconnect already disconnected ports or null list";
      return;
   }
   port->m_connections->removeAll(port);
   port->m_connections = 0;
   if(from->m_connections->size() <= 1) {
      if(from->m_connections->size() == 1) {
         Q_ASSERT(from->m_connections->first() == from);
      }
      delete from->m_connections;
      from->m_connections = 0;
   }
   port->ownerItem()->update();
   from->ownerItem()->update();
   if(from->m_connections) {
      foreach(Port *p, *(from->m_connections))
         p->ownerItem()->update();
   }
}

bool Port::isConnected(Port *port1, Port *port2)
{
   bool retVal = port1->m_connections == port2->m_connections &&
         port1->m_connections != 0 && port1->m_connections->contains(port1) &&
         port1->m_connections->contains(port2);
   if(retVal) {
      bool ok1, ok2;
      Q_ASSERT(port1->scenePos(&ok1) == port2->scenePos(&ok2));
      Q_ASSERT(ok1 && ok2);
   }
   return retVal;
}

Port* Port::findIntersectingPort(const QList<Port*> &ports) const
{
   foreach(Port *p, ports) {
      if(circleIntersects(p->scenePos(), scenePos(), portRadius) &&
         (!m_connections || !m_connections->contains(p))) {
         return p;
      }
   }
   return 0;
}

Port* Port::findIntersectingPort() const
{
   SchematicScene *scene =
         qobject_cast<SchematicScene*>(ownerItem()->scene());
   if(!scene) {
      return 0;
   }
   QList<QGraphicsItem*> collisions =
         ownerItem()->collidingItems(Qt::IntersectsItemBoundingRect);
   QList<Port*> ports;
   foreach(QGraphicsItem *item, collisions) {
      if(qucsitem_cast<Qucs::Component*>(item)) {
         ports = qucsitem_cast<Qucs::Component*>(item)->ports();
      }
      else if(qucsitem_cast<Qucs::Wire*>(item)) {
         ports = qucsitem_cast<Qucs::Wire*>(item)->ports();
      }
      else {
         continue;
      }

      Port *p = findIntersectingPort(ports);
      if(p) {
         return p;
      }
   }
   return 0;
}

Port* Port::findCoincidingPort(const QList<Port*> &ports) const
{
   foreach(Port *p, ports) {
      if(p->scenePos() == scenePos() &&
         (!m_connections || !m_connections->contains(p))) {
         return p;
         }
   }
   return 0;
}

Port* Port::findCoincidingPort() const
{
   SchematicScene *scene =
         qobject_cast<SchematicScene*>(ownerItem()->scene());
   if(!scene) {
      return 0;
   }
   QList<QGraphicsItem*> collisions =
         ownerItem()->collidingItems(Qt::IntersectsItemBoundingRect);
   QList<Port*> ports;
   foreach(QGraphicsItem *item, collisions) {
      if(qucsitem_cast<Qucs::Component*>(item)) {
         ports = qucsitem_cast<Qucs::Component*>(item)->ports();
      }
      else if(qucsitem_cast<Qucs::Wire*>(item)) {
         ports = qucsitem_cast<Qucs::Wire*>(item)->ports();
      }
      else {
         continue;
      }

      Port *p = findCoincidingPort(ports);
      if(p) {
         return p;
      }
   }
   return 0;
}

void Port::paint(QPainter *painter, const QStyleOptionGraphicsItem* option)
{
   Q_UNUSED(option);

   if(m_connections) {
      painter->setPen(connectedPen);
      painter->setBrush(connectedBrush);
   } else {
      painter->setPen(unconnectedPen);
      painter->setBrush(unconnectedBrush);
   }
   painter->drawEllipse(portEllipse.translated(pos()));
}

void drawPorts(const QList<Port*> &ports, QPainter *painter, const QStyleOptionGraphicsItem* option)
{
   foreach(Port *port, ports) {
      port->paint(painter, option);
   }
}

QRectF portsRect(const QList<Port*> &ports, const QRectF& pRect)
{
   QRectF rect = pRect;
   foreach(Port *port, ports) {
      rect |= portEllipse.translated(port->pos());
   }
   return rect;
}
