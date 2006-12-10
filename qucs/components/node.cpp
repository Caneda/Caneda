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
