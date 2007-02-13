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
#include "components/component.h"
#include "wire.h"

#include "schematicscene.h"
#include "undocommands.h"
#include <QtGui/QPainter>
#include <QtCore/QDebug>

const qreal Node::Radius = 3.0;

Node::Node(const QString& name,QGraphicsScene *scene) : QucsItem(0,scene)
{
   setName(name);
   setFlags(0);
   setAcceptedMouseButtons(0);
   setZValue(1.0);
   m_controller = 0;
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

bool Node::areAllComponentsSelected() const
{
   QSet<Component*>::const_iterator it = m_connectedComponents.constBegin();
   const QSet<Component*>::const_iterator end = m_connectedComponents.constEnd();
   for( ; it != end; ++it)
   {
      if(!((*it)->isSelected()))
         return false;
   }
   return true;
}

void Node::addAllComponentsFrom(Node *n)
{
   m_connectedComponents.unite(n->m_connectedComponents);
   update();
}

void Node::addWire(Wire *w)
{
   m_wires.insert(w);
   update();
}

void Node::removeWire(Wire *w)
{
   m_wires.remove(w);
   update();
}

void Node::addAllWiresFrom(Node *n)
{
   m_wires.unite(n->m_wires);
   update();
}

void Node::paint(QPainter* p,const QStyleOptionGraphicsItem *o, QWidget *w)
{
   Q_UNUSED(o);
   Q_UNUSED(w);
   p->setPen(Qt::darkRed);
   if(!isOpen())
      p->setBrush(QBrush(Qt::cyan, Qt::SolidPattern));
   p->drawEllipse(boundingRect());
}

bool Node::collidesWithItem(QGraphicsItem *other) const
{
   Node *port = qgraphicsitem_cast<Node*>(other);
   if(!port)
      return QGraphicsItem::collidesWithItem(other);
   qreal dist = distance(pos(),port->pos());

   return (dist <= (2 * Node::Radius));
}

QPainterPath Node::shape() const
{
   QPainterPath path;
   path.addEllipse(boundingRect());
   return path;
}
