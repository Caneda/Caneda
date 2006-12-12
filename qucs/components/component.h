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

#ifndef __COMPONENT_H
#define __COMPONENT_H

#include "item.h"
#include "propertytext.h"

class Component;
class Node;
class SchematicScene;

/// Encapsulates a node for component to use it as a port
class ComponentPort
{
   public:
      ComponentPort(Component* owner,const QPointF& pos,Node *n = 0l);
      ComponentPort(Component *owner,Node *n);
      ~ComponentPort();

      void setNode(Node *node);
      Node* node() const;
      Component* owner() const;
      
      void moveBy(qreal dx, qreal dy);
      const QPointF& centrePos() const;

      friend class SchematicScene;
      friend class Node;
   private:
      Node *m_node;
      Component* m_owner;
      const QPointF m_centrePos; //pos of port w.r.t Component is a const
};

// This class encapsulates a component in general
class Component : public QucsItem
{
   public:

      Component(QGraphicsItem* parent = 0l, QGraphicsScene* scene = 0l);
      virtual ~Component() {};

      virtual QString name() const = 0;
      virtual QString model() const = 0;
      virtual QString text() const = 0;
      virtual QString netlist() const = 0;
      
      const QList<ComponentPort*>& componentPorts() const;
      
      int type() const;

      void determineHowToMove();
      void resetSimplyMove();

      ComponentPort* portWithNode(Node *n) const;
      void adjustPosAccordingTo(ComponentPort *p);
      
   protected:
      QVariant itemChange(GraphicsItemChange c,const QVariant& value);
      void mousePressEvent ( QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
      void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
      
      QList<ComponentPort*> m_ports;
      QList<PropertyText*> m_properties;
                  
   private:
      QVariant handlePositionChange(const QPointF& pos);
      bool simplyMove;
};

#endif //__COMPONENT_H
