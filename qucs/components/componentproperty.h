/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __COMPONENTPROPERTY_H
#define __COMPONENTPROPERTY_H

#include "propertytext.h"

#include <QtCore/QStringList>
#include <QtCore/QPointF>
#include <QtCore/QtDebug>

#include <QtGui/QGraphicsItemGroup>

class Component;

class PropertyGroup : public QGraphicsItemGroup
{
   public:
      PropertyGroup(Component *comp,SchematicScene *scene = 0);
      ~PropertyGroup() {}

      void addProperty(ComponentProperty *child);

      void realignItems();
      QRectF boundingRect() const;

      QList<ComponentProperty*> properties() const { return m_properties; }

   protected:
      void mousePressEvent(QGraphicsSceneMouseEvent *event);

   private:
      void initVariables();

      Component *const m_component;
      qreal fontHeight;
      qreal itemLeft;
      QList<ComponentProperty*> m_properties;
      QPointF lastChildPos;
      bool firstTime;
};

//An abstraction to the propoerty of the component
class ComponentProperty
{
   public:
      ComponentProperty(Component *c,const QString& name, const QString& value,
                        const QString& description, bool visible = false,
                        const QStringList& options = QStringList());
      ~ComponentProperty();

      void show();
      void hide();
      void setVisible(bool visible);

      void setPos(const QPointF& pos);
      QPointF pos() const { return isVisible() ? m_item->pos() : m_pos; }
      void setValue(const QString& value);

      PropertyText* item() const { return m_item; }
      Component* component() const { return m_component; }
      bool isVisible() const { return m_item != 0; }
      void updateValueFromItem();
      PropertyGroup* group() const;

      QString name() const { return m_name; }
      QString value() const { return m_value; }
      QString description() const { return m_description; }
      QStringList options() const { return m_options; }

   private:
      Component *const m_component;
      const QString m_name;
      QString m_value;
      const QString m_description;
      const QStringList m_options;
      QPointF m_pos;
      PropertyText *m_item;
};





#endif //__COMPONENTPROPERTY_H

