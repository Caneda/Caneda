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

#include "component.h"
#include "node.h"
#include "wire.h"
#include "components.h"

#include "schematicscene.h"
#include "propertytext.h"
#include "undocommands.h"
#include <QtGui/QUndoStack>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtCore/QDebug>
#include <QtGui/QPainter>

ComponentPort::ComponentPort(Component* owner,const QPointF& pos) : m_owner(owner), m_centrePos(pos)
{
   SchematicScene *s = owner->schematicScene();
   if(s)
   {
      QPointF spos = m_owner->mapToScene(pos);

      m_node = s->nodeAt(spos);
      if(!m_node)
         m_node = s->createNode(spos);
   }
   else
      m_node = new Node(); // To avoid crashes
   m_node->addComponent(m_owner);
}

Component::~Component()
{
   foreach(ComponentPort *port, m_ports)
      port->node()->removeComponent(this);
}

QString Component::netlist() const
{
   QString s = model + ":" + name;

   // output all node names
   foreach(ComponentPort *port, m_ports)
      s += ' ' + port->node()->name(); // node names
   
   // output all properties
   foreach(PropertyText *prop, m_properties)
      s += ' ' + prop->name() + "'=\"" + prop->value() + "\"";
   return s;
}

QString Component::shortNetlist() const
{
   int z=0;
   QString s;
   QString Node1 = m_ports.first()->node()->name();
   foreach(ComponentPort *port, m_ports)
   {
      if( z == 0) continue;
      s += "R:" + name + "." + QString::number(z++) + ' ' +
         Node1 + ' ' + port->node()->name() + " R=\"0\"\n";
   }
   return s;
}

ComponentPort* Component::portWithNode(Node *n) const
{
   QList<ComponentPort*>::const_iterator it = m_ports.constBegin();
   const QList<ComponentPort*>::const_iterator end = m_ports.constEnd();
   for(; it != end; ++it)
   {
      if((*it)->node() == n)
         return *it;
   }
   return 0;
}

void Component::replaceNode(Node *_old, Node *_new)
{
   ComponentPort *p = portWithNode(_old);
   Q_ASSERT(p);
   p->setNode(_new);
}

QVariant Component::handlePositionChange(const QPointF& hpos)
{
   QPointF oldPos = pos();
   qreal dx = hpos.x() - oldPos.x();
   qreal dy = hpos.y() - oldPos.y();

   QList<PropertyText*>::iterator it = m_properties.begin();
   const QList<PropertyText*>::iterator end = m_properties.end();
   for(; it != end; ++it)
      (*it)->moveBy(dx,dy);

   if(schematicScene()->areItemsMoving() == false)
   {
      QList<ComponentPort*>::iterator _it = m_ports.begin();
      const QList<ComponentPort*>::iterator _end = m_ports.end();
      for(; _it != _end; ++_it)
      {
         ComponentPort *port = *_it;
         if(port->node()->isControllerSet() && port->node()->controller() != this)
            continue;
         port->node()->setController(this);
         port->node()->moveBy(dx,dy);
         port->node()->resetController();
      }
   }
   return QVariant(hpos);
}

QVariant Component::itemChange(GraphicsItemChange change,const QVariant& value)
{
   Q_ASSERT(scene());

   if (change == ItemPositionChange)
      return handlePositionChange(value.toPointF());

   else if(change == ItemMatrixChange)
   {
      QMatrix newMatrix = qVariantValue<QMatrix>(value);
      QList<ComponentPort*>::iterator it = m_ports.begin();
      const QList<ComponentPort*>::iterator end = m_ports.end();
      for(; it != end; ++it)
      {
         ComponentPort *port = *it;
         QMatrix newSceneMatrix = newMatrix * QMatrix().translate(pos().x(),pos().y());
         QPointF newP = newSceneMatrix.map(port->centrePos());

         port->node()->setController(this);
         port->node()->setPos(newP);
         port->node()->resetController();
      }
      return QVariant(newMatrix);
   }
   return QGraphicsItem::itemChange(change,value);
}

void Component::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
   if(event->buttons() & Qt::RightButton)
   {
      rotate(-45);
      foreach(QGraphicsItem *item, scene()->selectedItems())
      {
         if(item != this)
            item->setSelected(false);
         else
            setSelected(true);
      }
   }
   else
      QucsItem::mousePressEvent(event);
}

void Component::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
   QucsItem::mouseMoveEvent(event);
}

void Component::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
   QucsItem::mouseReleaseEvent(event);
}

Component* Component::componentFromName(const QString& comp,SchematicScene *scene)
{
   if(comp == "Resistor")
      return new Resistor(scene);
   else if(comp == "ResistorUS")
      return new ResistorUS(scene);
   else if(comp == "Capacitor")
      return new Capacitor(scene);
   else if(comp == "Coupler")
      return new Coupler(scene);
   return 0;
}

void Component::initPainter(QPainter *p,const QStyleOptionGraphicsItem *o)
{
   if(!(o->state & QStyle::State_Open))
      p->setPen(Qt::darkBlue);
   if(o->state & QStyle::State_Selected)
      p->setPen(QPen(Qt::darkGray,1));
}

void Component::drawNodes(QPainter *p)
{
   QList<ComponentPort*>::const_iterator it = m_ports.constBegin();
   const QList<ComponentPort*>::const_iterator end = m_ports.constEnd();
   p->setPen(QPen(Qt::red));
   for(; it != end; ++it)
   {
      QRectF rect = (*it)->node()->boundingRect().translated((*it)->centrePos());
      p->drawEllipse(rect);
   }
}
