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

#include <cmath>
const qreal Node::Radius = 3.0;

inline qreal distance(const QPointF& p1, const QPointF& p2)
{
   qreal dx = p1.x() - p2.x();
   qreal dy = p1.y() - p2.y();
   return std::sqrt((dx*dx)+(dy*dy));
}

Node::Node(const QString& name,SchematicScene *scene) : QucsItem(0,scene)
{
   setName(name);
   setFlags(0);
   setAcceptedMouseButtons(0);
   setZValue(1.0);
   m_controller = 0;
}

Node::~Node()
{
}

void Node::addComponent(Component *comp)
{
   if(!m_components.contains(comp)) {
      m_components << comp;
      update();
   }
}

void Node::removeComponent(Component* comp)
{
   int index =  m_components.indexOf(comp);
   if(index != -1) {
      m_components.removeAt(index);
      update();
   }
}

QList<Component*> Node::selectedComponents() const
{
   QList<Component*> selCom;
   foreach(Component *c, m_components) {
      if(c->isSelected())
         selCom << c;
   }
   return selCom;
}

bool Node::areAllComponentsSelected() const
{
   QList<Component*>::const_iterator it = m_components.constBegin();
   const QList<Component*>::const_iterator end = m_components.constEnd();
   for( ; it != end; ++it) {
      if(!((*it)->isSelected()))
         return false;
   }
   return true;
}

void Node::addAllComponentsFrom(Node *n)
{
   foreach(Component *c, n->m_components) {
      if(!m_components.contains(c))
         m_components << c;
   }
   update();
}

void Node::addWire(Wire *w)
{
   if(!m_wires.contains(w)) {
      m_wires << w;
      update();
   }
}

void Node::removeWire(Wire *w)
{
   int index = m_wires.indexOf(w);
   if(index != -1) {
      m_wires.removeAt(index);
      update();
   }
}

QList<Wire*> Node::selectedWires() const
{
   QList<Wire*> selWires;
   foreach(Wire *w, m_wires) {
      if(w->isSelected())
         selWires << w;
   }
   return selWires;
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

bool Node::contains(const QPointF& pt) const
{
   qreal dist = distance(QPointF(0,0),pt);
   return (((dist * dist) - (Node::Radius*Node::Radius)) <= 0);
}

bool Node::collidesWithItem(QGraphicsItem *other) const
{
   Node *port = qucsitem_cast<Node*>(other);
   if(!port)
      return QGraphicsSvgItem::collidesWithItem(other);
   qreal dist = distance(pos(),port->pos());

   return (dist <= (2. * Node::Radius));
}

QPainterPath Node::shape() const
{
   QPainterPath path;
   path.addEllipse(boundingRect());
   return path;
}
