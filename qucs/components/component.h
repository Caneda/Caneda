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
#include "components/componentproperty.h"

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

      int type() const { return QucsItem::ComponentType; }

      explicit Component(SchematicScene* scene = 0);
      virtual ~Component();

      Component* newOne() const;
      void copyTo(Component *component) const;

      // methods to assist in saving and loading from file, clipboard..
      QString saveString() const;
      bool loadFromString(QString str);

      void writeXml(Qucs::XmlWriter *writer);
      void readXml(Qucs::XmlReader *reader);

      QString getNetlist() const;
      QString getVHDLCode(int numOfPorts) const;
      QString getVerilogCode(int numOfPorts) const;

      const QList<ComponentPort*>& componentPorts() const { return m_ports; }
      void addPort(const QPointF& pos);

      void replaceNode(Node *_old, Node *_new);
      ComponentPort* portWithNode(Node *n) const;
      void checkForConnections(bool exactCentreMatch = true);

      void addProperty(QString _name,QString _initVal,QString _des,
                       bool isVisible = false,
                       const QStringList& options = QStringList());
      ComponentProperty* property(const QString& _name) const;
      QList<ComponentProperty*> properties() const { return m_propertyGroup->properties(); }
      PropertyGroup* propertyGroup() const { return m_propertyGroup; }
      void setInitialPropertyPosition();

      void drawNodes(QPainter *p);
      void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w = 0);

      static Component* componentFromModel(const QString& model, SchematicScene *scene);
      static QPen getPen(QColor col = Qt::darkBlue, qreal pw = 0, Qt::PenStyle ps = Qt::SolidLine)
      {
         return QPen(col, pw, ps);
      }

      QString name;
      QString model;
      QString description;
      bool showName;
      ActiveStatus activeStatus;

   protected:
      virtual QString netlist() const;
      virtual QString vhdlCode(int numOfPorts) const;
      virtual QString verilogCode(int numOfPorts) const;

      QVariant itemChange(GraphicsItemChange c,const QVariant& value);
      QList<Shape*> m_shapes;

   private:
      QList<ComponentPort*> m_ports;
      PropertyGroup *m_propertyGroup;

      QVariant handlePositionChange(const QPointF& pos);
      friend class SchematicScene;
};

#endif //__COMPONENT_H
