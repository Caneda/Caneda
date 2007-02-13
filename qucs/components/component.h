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
      ComponentPort(Component* owner,const QPointF& pos);
            
      inline void setNode(Node *node);
      inline Node* node() const;
      inline Component* owner() const;

      inline const QPointF& centrePos() const;

   private:
      Node *m_node;
      Component* const m_owner;
      const QPointF m_centrePos; //pos of port w.r.t Component is a const
};

inline void ComponentPort::setNode(Node *node)
{
   m_node = node;
}

inline Node* ComponentPort::node() const
{
   return m_node;
}

inline Component* ComponentPort::owner() const
{
   return m_owner;
}

inline const QPointF& ComponentPort::centrePos() const
{
   return m_centrePos;
}

// This class encapsulates a component in general
class Component : public QucsItem
{
   public:
      enum {
         Type = QucsItem::ComponentType
      };
      
      inline explicit Component(QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
      virtual ~Component();
      
      virtual QString netlist() const;
      virtual QString shortNetlist() const;
      inline int type() const;

      inline const QList<ComponentPort*>& componentPorts() const;
      ComponentPort* portWithNode(Node *n) const;
      void replaceNode(Node *_old, Node *_new);

      static Component* componentFromName(const QString& str,SchematicScene *scene);

   public:
      QString name;
      QString model;
      QString description;
      
   protected:
      QVariant itemChange(GraphicsItemChange c,const QVariant& value);
      void mousePressEvent ( QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
      void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

      virtual void initComponentStrings() {}
      void initPainter(QPainter *p, const QStyleOptionGraphicsItem *o);
      void drawNodes(QPainter *p);
      
      QList<ComponentPort*> m_ports;
      QList<PropertyText*> m_properties;

   private:
      QVariant handlePositionChange(const QPointF& pos);
};

inline Component::Component(QGraphicsItem* parent, QGraphicsScene* scene) : QucsItem(parent,scene)
{
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
}

inline int Component::type() const
{
   return QucsItem::ComponentType;
}

inline const QList<ComponentPort*>& Component::componentPorts() const
{
   return m_ports;
}

#endif //__COMPONENT_H
