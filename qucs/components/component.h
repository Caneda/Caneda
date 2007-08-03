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
#include <QtGui/QPen>

class Component;
class Node;
class SchematicScene;
class Shape;
class ComponentProperty;
class PropertyGroup;

/// Encapsulates a node for component to use it as a port
class ComponentPort
{
   public:
      ComponentPort(Component* owner,const QPointF& pos);

      void setNode(Node *node) { m_node = node; }
      Node* node() const { return m_node; }

      Component* owner() const { return m_owner; }

      const QPointF& centrePos() const { return m_centrePos; }

   private:
      Node *m_node;
      Component* const m_owner;
      //pos of port w.r.t Component is a const
      const QPointF m_centrePos;
};


// This class encapsulates a component in general
class Component : public QucsItem
{
   public:
      enum {
         Type = QucsItem::ComponentType,
      };

      enum ActiveStatus {
         Open = 0,
         Active = 1,
         Shorten = 2
      };

      explicit Component(SchematicScene* scene = 0);
      virtual ~Component();

      virtual QString netlist() const;
      virtual QString shortNetlist() const;

      QString saveString() const;
      bool loadFromString(QString str);

      void writeXml(Qucs::XmlWriter *writer);
      void readXml(Qucs::XmlReader *reader);

      int type() const { return QucsItem::ComponentType; }

      const QList<ComponentPort*>& componentPorts() const { return m_ports; }
      ComponentPort* portWithNode(Node *n) const;
      void replaceNode(Node *_old, Node *_new);

      void addProperty(QString _name,QString _initVal,QString _des,bool isVisible = false,const QStringList& options = QStringList());
      ComponentProperty* property(const QString& _name) const;
      QList<ComponentProperty*> properties() const;

      void addPort(const QPointF& pos);
      PropertyGroup* propertyGroup() const { return m_propertyGroup; }

      static Component* componentFromName(const QString& str,SchematicScene *scene);
      static Component* componentFromLine( QString line,SchematicScene *scene);
      static Component* componentFromModel(const QString& model, SchematicScene *scene);

      void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w = 0);
      QRectF boundingRect() const
      {
         qreal half_pw = 0.5;
         return m_boundingRect.adjusted(-half_pw, -half_pw, half_pw, half_pw);
      }

      void setInitialPropertyPosition();

      void rotate();
      void mirrorX();
      void mirrorY();

   public:
      QString name;
      QString model;
      QString description;
      bool showName;
      ActiveStatus activeStatus;
      static QMap<int,QPen> pens;
      static const QPen& getPen(QColor col = Qt::darkBlue,int pw = 0,Qt::PenStyle = Qt::SolidLine);

   protected:
      QVariant itemChange(GraphicsItemChange c,const QVariant& value);
      void mousePressEvent ( QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
      void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

      void drawNodes(QPainter *p);

      QList<ComponentPort*> m_ports;

      PropertyGroup *m_propertyGroup;
      QList<Shape*> m_shapes;
      QPen m_pen;
      QPointF m_textPos;
      QRectF m_boundingRect;

      unsigned int m_rotated;
      bool m_mirroredX;
      bool m_mirroredY;

   private:
      QVariant handlePositionChange(const QPointF& pos);
      friend class SchematicScene;
};



#endif //__COMPONENT_H
