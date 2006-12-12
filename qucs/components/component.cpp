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

#include "schematicscene.h"
#include "propertytext.h"
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtCore/QDebug>

ComponentPort::ComponentPort(Component* owner,const QPointF& pos,Node *n) : m_centrePos(pos)
{
   m_owner = owner;

   SchematicScene *s = owner->schematicScene();
   QPointF spos = owner->mapToScene(pos);
   
   if(n != 0l)
      m_node = n;
   else if((m_node = s->nodeAt(spos)))
      ;
   else
      m_node = s->createNode(spos);
   m_node->addComponent(m_owner);
}

ComponentPort::~ComponentPort()
{
}

void ComponentPort::setNode(Node *node)
{
   Q_ASSERT(m_node);
   if(m_node == node)
      return;
   m_node->removeComponent(m_owner);
   
   if(m_node->connectedComponents().isEmpty())
      m_owner->schematicScene()->removeNode(m_node);

   m_node = node;
   
   if(!(m_node->connectedComponents().contains(m_owner)))
      m_node->addComponent(m_owner);
}

Node* ComponentPort::node() const
{
   return m_node;
}

Component* ComponentPort::owner() const
{
   return m_owner;
}

void ComponentPort::moveBy(qreal dx, qreal dy)
{
   m_node->moveBy(dx,dy);
}

const QPointF& ComponentPort::centrePos() const
{
   return m_centrePos;
}

Component::Component(QGraphicsItem* parent, QGraphicsScene* scene) : QucsItem(parent,scene)
{
   simplyMove = true;
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
}

void Component::determineHowToMove()
{
   simplyMove = true; //Reset simply move
   foreach(ComponentPort* port, m_ports)
   {
      foreach(Component* c, port->node()->connectedComponents())
      {
	 if(c != this && !(c->isSelected()))
	 {
	    simplyMove = false;
	    break;
	 }
      }
      if(!simplyMove)
	 break;
   }
}

void Component::resetSimplyMove()
{
   simplyMove = true;
}

QVariant Component::handlePositionChange(const QPointF& pos)
{
   QPointF oldPos = scenePos();
   qreal dx = pos.x() - oldPos.x();
   qreal dy = pos.y() - oldPos.y();
      
   foreach(ComponentPort* port, m_ports)
   {
      QPointF spos = mapToScene(port->centrePos());
      spos += QPointF(dx,dy);
	 
      if(!(port->node()->isControllerSet()))
      {
	 port->node()->setController(this);
	 port->node()->setPos(spos);
	 port->node()->resetController();
      }
   }
   
   foreach(PropertyText *text, m_properties)
      text->moveBy(dx,dy);

   return QVariant(pos);
}

QVariant Component::itemChange(GraphicsItemChange change,const QVariant& value)
{
   Q_ASSERT(scene());
   
   if (change == ItemPositionChange)
      return handlePositionChange(value.toPointF());

   else if(change == ItemMatrixChange)
   {
      QMatrix newMatrix = qVariantValue<QMatrix>(value);
      foreach(ComponentPort* port, m_ports)
      {
	 QPointF old = mapFromScene(port->node()->scenePos());
	 QPointF newP = newMatrix.map(old);
	 port->node()->setPos(sceneMatrix().map(newP));
      }
      return QVariant(newMatrix);
   }
   return QGraphicsItem::itemChange(change,value);
}

void Component::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
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

const QList<ComponentPort*>& Component::componentPorts() const
{
   return m_ports;
}

int Component::type() const
{
   return QucsItem::ComponentType;
}

ComponentPort* Component::portWithNode(Node *n) const
{
   foreach(ComponentPort *p,m_ports)
   {
      if(p->node() == n)
	 return p;
   }
   return 0l;
}

void Component::adjustPosAccordingTo(ComponentPort *port)
{
   Q_ASSERT(port && port->owner() == this);
   QPointF pos = mapFromScene(port->node()->scenePos());
   qreal dx = pos.x() - port->centrePos().x();
   qreal dy = pos.y() - port->centrePos().y();
   if(dx == 0 && dy == 0)
      return;
   simplyMove = true;
   moveBy(dx,dy);
   foreach(ComponentPort *p, componentPorts())
   {
      if(p == port)
	 continue;
      p->node()->setPos(mapToScene(p->centrePos()));
      foreach(Component *c, p->node()->connectedComponents())
      {
	 if(c == this)
	    continue;
	 ComponentPort *pt = c->portWithNode(p->node());
	 c->adjustPosAccordingTo(pt);
      }
   }
}
