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

#include "components/component.h"
#include <QtCore/QList>

class Wire;

class Node : public QucsItem
{
   public:
      enum {
         Type = QucsItem::NodeType
      };

      static const qreal Radius;

      Node(const QString& name = QString(),SchematicScene *scene = 0);
      ~Node();

      void addComponent(Component *comp);
      void removeComponent(Component *comp);

      QList<Component*> components() { return m_components; }
      QList<Component*> selectedComponents() const;

      bool areAllComponentsSelected() const;
      void addAllComponentsFrom(Node *other);

      void addWire(Wire *w);
      void removeWire(Wire *w);

      QList<Wire*> wires() const { return m_wires; }
      QList<Wire*> selectedWires() const;

      void addAllWiresFrom(Node *other);

      //Only one component or one wire is taken by node.
      bool isOpen() const { return totalConnections() == 1; }
      bool isEmpty() const { return m_components.isEmpty() && m_wires.isEmpty(); }

      int totalConnections() const { return m_components.size() + m_wires.size(); }

      void paint(QPainter *p,const QStyleOptionGraphicsItem *o, QWidget *w = 0);

      bool contains(const QPointF& pt) const;
      bool collidesWithItem(QGraphicsItem *other) const;

      QRectF boundingRect() const { return QRectF(-Radius, -Radius, 2*Radius, 2*Radius); }
      QPainterPath shape() const;

      QString name() const { return m_name; }
      void setName(const QString& name) { m_name = name; }

      void setController(Component *c) { m_controller = c; }
      Component* controller() const { return m_controller; }

      int type() const { return QucsItem::NodeType; }

      // Reimplemented virtuals to not to react
      void rotate() {}
      void mirrorX() {}
      void mirrorY() {}

   private:
      QList<Component*> m_components;
      QList<Wire*> m_wires;
      QString m_name;
      Component *m_controller;
};

#endif //__NODE_H
