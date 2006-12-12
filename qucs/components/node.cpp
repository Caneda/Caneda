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

#include "node.h"
#include "component.h"
#include "wire.h"
#include "schematicscene.h"
#include <QtGui/QPainter>

#include <cmath>

const qreal Node::Radius = 3.0;

inline qreal distance(const QPointF& p1, const QPointF& p2)
{
   qreal dx = p1.x() - p2.x();
   qreal dy = p1.y() - p2.y();
   return std::sqrt((dx*dx)+(dy*dy));
}

Node::Node(const QString& name,QGraphicsScene *scene) : QucsItem(0l,scene)
{
   setName(name);
   setFlags(0);
   setAcceptedMouseButtons(0);
   setZValue(1.0);
   m_controller = 0l;
}

Node::~Node()
{
}

void Node::addComponent(Component *comp)
{
   m_connectedComponents.insert(comp);
   update();
}

void Node::removeComponent(Component* comp)
{
   // call erase instead of remove to prevent rehashing
   QSet<Component*>::iterator it = m_connectedComponents.find(comp);
   if(it != m_connectedComponents.end())
   {
      m_connectedComponents.erase(it);
      update();
   }
}

const QSet<Component*>& Node::connectedComponents() const
{
   return m_connectedComponents;
}

bool Node::contains(const QPointF& pt) const
{
   qreal dist = distance(QPointF(0,0),pt);
   return (((dist * dist) - (Node::Radius*Node::Radius)) <= 0);
}

void Node::paint(QPainter* p,const QStyleOptionGraphicsItem *o, QWidget *w)
{
   Q_UNUSED(o);
   Q_UNUSED(w);
   p->setPen(Qt::darkBlue);
   if(!isOpen())
      p->setBrush(QBrush(Qt::cyan, Qt::SolidPattern));
   p->drawEllipse(QRectF(-Radius, -Radius, 2*Radius, 2*Radius));
}

QString Node::name() const
{
   return m_name;
}

void Node::setName(const QString& name)
{
   m_name = name;
}

QRectF Node::boundingRect() const
{
   return QRectF(-Radius, -Radius, 2*Radius, 2*Radius);
}

bool Node::collidesWith(const Node* port) const
{
   QPointF myCentre = scenePos();
   QPointF otherCentre = port->scenePos();
   qreal dist = distance(myCentre,otherCentre);

   if(dist > (2 * Node::Radius))
      return false;
   return true;
}

bool Node::isOpen() const
{
   return m_connectedComponents.size() == 1;
}

bool Node::areAllComponentsSelected() const
{
   foreach(Component *c, m_connectedComponents)
   {
      if(!(c->isSelected()))
	 return false;
   }
   return true;
}

QSet<Component*> Node::selectedComponents()
{
   QSet<Component*> selCom;
   foreach(Component *c, m_connectedComponents)
   {
      if(c->isSelected())
	 selCom.insert(c);
   }
   return selCom;
}

QSet<Component*> Node::unselectedComponents()
{
   QSet<Component*> unSelCom;
   foreach(Component *c, m_connectedComponents)
   {
      if(!(c->isSelected()))
	 unSelCom.insert(c);
   }
   return unSelCom;
}

void Node::setNewPos(const QPointF& np)
{
   m_newPos = np;
}

QPointF Node::newPos() const
{
   return m_newPos;
}

void Node::setController(Component *c)
{
   m_controller = c;
}

void Node::resetController()
{
   m_controller = 0l;
}

bool Node::isControllerSet() const
{
   return m_controller != 0l;
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant& val)
{
   if(change == ItemPositionChange)
   {
      QPointF newPos = val.toPointF();
      QPointF oldPos = scenePos();
      qreal dx = newPos.x() - oldPos.x();
      qreal dy = newPos.y()-oldPos.y();
      //This assert makes sure that the node is moved only by component/wire
      Q_ASSERT(m_controller);
      
      foreach(Component* c,selectedComponents())
      {
	 if(c != m_controller)
	    c->moveBy(dx,dy);
      }

      QSet<Component*> unselCom = unselectedComponents();
      if(selectedComponents().size() >= 1 && !unselCom.isEmpty())
      {
	 uint wireCount = 0;
	 Node *_new = 0l;
	 foreach(Component *c,unselectedComponents())
	 {
	    if(c->type() == QucsItem::WireType)
	    {
	       ++wireCount;
	       Wire *w = (Wire*)c;
	       QPointF s = w->componentPorts()[0]->node()->scenePos();
	       QPointF e = w->componentPorts()[1]->node()->scenePos();
	       if(s == oldPos)
		  s += QPointF(dx,dy);
	       else if(e == oldPos)
		  e += QPointF(dx,dy);
	       else
		  Q_ASSERT(0);
	       w->rebuild(s,e);
	       continue;
	    }
	    else if(_new == 0l)
	    {
	       _new = schematicScene()->createNode(scenePos());
	       removeComponent(c);
	       _new->addComponent(c);
	       new Wire(schematicScene(),this,_new);
	       ComponentPort *port = c->portWithNode(this);
	       Q_ASSERT(port != 0l);
	       port->m_node = _new;
	    }
	    else
	    {
	       removeComponent(c);
	       _new->addComponent(c);
	       ComponentPort *port = c->portWithNode(this);
	       Q_ASSERT(port != 0l);
	       port->m_node = _new;
	    }
	 }
      }
   
      return QVariant(newPos);
   }
   
   return QGraphicsItem::itemChange(change,val);
}

int Node::type() const
{
   return QucsItem::NodeType;
}
