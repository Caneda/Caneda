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

ComponentPort::ComponentPort(Component* owner,const QPointF& pos) : m_centrePos(pos)
{
   m_owner = owner;
   
   SchematicScene *s = owner->schematicScene();
   QPointF spos = owner->mapToScene(pos);
   
   if((m_node = s->nodeAt(spos)))
   {
      m_node->addComponent(owner);
      qreal dx = m_node->x() - pos.x();
      qreal dy = m_node->y() - pos.y();
      owner->moveBy(dx,dy);
   }
   else
   {
      m_node = s->createNode(spos);
      m_node->addComponent(m_owner);
   }
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
	 
      if(schematicScene()->isCommonMovingNode(port->node()))
      {
	 // Doesn't move now. Moves this later managed by SchematicScene
	 port->node()->setNewPos(spos);
	 continue;
      }
      if( simplyMove)
      {
	 port->node()->setPos(spos);
	 continue;
      }

      const QSet<Component*>& conCom = port->node()->connectedComponents();
      int size = conCom.size();

      // Size can never be zero here since atleast this component exists
      Q_ASSERT(size != 0);

      if(size == 1) // Or open
      {
	port->node()->setPos(spos);
	continue;
      }
      
      else if(size == 2)
      {
	 // The following four lines detect the index/pos of other component in node
	 QSet<Component*>::const_iterator it = conCom.begin();
	 if(*it == this)
	    ++it;
	 else
	    Q_ASSERT(*(it+1) == this);
	    
	 if((*it)->type() == QucsItem::WireType)
	 {
	    Wire *w = qgraphicsitem_cast<Wire*>(*it);//(Wire*)(*it);
	    port->node()->setPos(spos);
	    w->rebuild();
	    continue;
	 }
      }

      // Here size can be 2 when other component is not wire
      Node *n = schematicScene()->nodeAt(spos);
      if(!n)
	 n = schematicScene()->createNode(spos);
      if(port->node() != n)
      {
	 Node *pn = port->node();
	 port->setNode(n);
	 new Wire(scene(),pn,n);
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

SchematicScene* Component::schematicScene() const
{
   SchematicScene *s = qobject_cast<SchematicScene*>(this->scene());
   Q_ASSERT(s != 0l);
   return s;
}

const QList<ComponentPort*>& Component::componentPorts() const
{
   return m_ports;
}

int Component::type() const
{
   return QucsItem::ComponentType;
}
