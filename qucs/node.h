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

#ifndef __NODE_H
#define __NODE_H

#include "item.h"
#include <QtCore/QSet>

class Component;
class Wire;

class Node : public QucsItem
{
   public:
      enum {
         Type = QucsItem::NodeType
      };

      static const qreal Radius;

      Node(const QString& name = QString(),QGraphicsScene *scene = 0);
      ~Node() {}

      void addComponent(Component *comp);
      void removeComponent(Component *comp);
      inline const QSet<Component*>& connectedComponents();
      inline bool isOpen() const;
      bool areAllComponentsSelected() const;
      void addAllComponentsFrom(Node *other);
      
      void addWire(Wire *w);
      void removeWire(Wire *w);
      void addAllWiresFrom(Node *other);
      inline QSet<Wire*> wires() const;

      void paint(QPainter *p,const QStyleOptionGraphicsItem *o, QWidget *w = 0);
      inline bool contains(const QPointF& pt) const;
      inline QRectF boundingRect() const;
      bool collidesWithItem(QGraphicsItem *other) const;
      QPainterPath shape() const;

      inline QString name() const;
      inline void setName(const QString& name);

      inline void setController(Component *c);
      inline void resetController();
      inline bool isControllerSet() const;
      inline Component* controller() const;

      inline int type() const;

   private:
      QSet<Component*> m_connectedComponents;
      QSet<Wire*> m_wires;
      QString m_name;
      Component *m_controller;
};

#include <cmath>

inline qreal distance(const QPointF& p1, const QPointF& p2)
{
   qreal dx = p1.x() - p2.x();
   qreal dy = p1.y() - p2.y();
   return std::sqrt((dx*dx)+(dy*dy));
}

inline const QSet<Component*>& Node::connectedComponents()
{
   return m_connectedComponents;
}

inline bool Node::isOpen() const
{
   return m_connectedComponents.size() <= 1 && m_wires.isEmpty();
}

inline QSet<Wire*> Node::wires() const
{
   return m_wires;
}

inline bool Node::contains(const QPointF& pt) const
{
   qreal dist = distance(QPointF(0,0),pt);
   return (((dist * dist) - (Node::Radius*Node::Radius)) <= 0);
}

inline QRectF Node::boundingRect() const
{
   return QRectF(-Radius, -Radius, 2*Radius, 2*Radius);
}

inline QString Node::name() const
{
   return m_name;
}

inline void Node::setName(const QString& name)
{
   m_name = name;
}

inline void Node::setController(Component *c)
{
   m_controller = c;
}

inline void Node::resetController()
{
   m_controller = 0;
}

inline bool Node::isControllerSet() const
{
   return m_controller != 0;
}

inline Component* Node::controller() const
{
   return m_controller;
}

inline int Node::type() const
{
   return QucsItem::NodeType;
}

#endif //__NODE_H
