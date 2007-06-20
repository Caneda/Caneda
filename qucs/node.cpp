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
#include "wire.h"

#include "schematicscene.h"
#include "undocommands.h"
#include <QtGui/QPainter>
#include <QtCore/QDebug>

const qreal Node::Radius = 3.0;

Node::Node(const QString& name,SchematicScene *scene) : QucsItem(0,scene)
{
   setName(name);
   setFlags(0);
   setAcceptedMouseButtons(0);
   setZValue(1.0);
   m_controller = 0;
}

void Node::addComponent(Component *comp)
{
   if(!m_connectedComponents.contains(comp)) {
      m_connectedComponents << comp;
      update();
   }
}

void Node::removeComponent(Component* comp)
{
   int index =  m_connectedComponents.indexOf(comp);
   if(index != -1) {
      m_connectedComponents.removeAt(index);
      update();
   }
}

bool Node::areAllComponentsSelected() const
{
   QList<Component*>::const_iterator it = m_connectedComponents.constBegin();
   const QList<Component*>::const_iterator end = m_connectedComponents.constEnd();
   for( ; it != end; ++it) {
      if(!((*it)->isSelected()))
         return false;
   }
   return true;
}

void Node::addAllComponentsFrom(Node *n)
{
   foreach(Component *c, n->m_connectedComponents) {
      if(!m_connectedComponents.contains(c))
         m_connectedComponents << c;
   }
   update();
}

void Node::addWire(Wire *w)
{
   if(!m_wires.contains(w))
      m_wires << w;
   update();
}

void Node::removeWire(Wire *w)
{
   int index = m_wires.indexOf(w);
   if(index != -1) {
      m_wires.removeAt(index);
      update();
   }
}

void Node::addAllWiresFrom(Node *n)
{
   foreach(Wire *w, n->m_wires) {
      if(!m_wires.contains(w))
         m_wires << w;
   }
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
   Node *port = qucsitem_cast<Node*>(other);
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
