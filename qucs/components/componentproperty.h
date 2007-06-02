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

      void addChild(ComponentProperty *child);
      void hideChild(ComponentProperty *child);
      void showChild(ComponentProperty *child);
      void realignItems( int fromIndex = 0 );

      QRectF boundingRect() const;

   protected:
      void mousePressEvent(QGraphicsSceneMouseEvent *event);

   private:
      Component *m_component;
      qreal fontHeight;
      qreal itemLeft;
      QList<ComponentProperty*> m_children;
      QPointF lastChildPos;
};

inline QRectF PropertyGroup::boundingRect() const
{
   if(children().size()) return QGraphicsItemGroup::boundingRect();
   //HACK: An empty group makes item to move very slow. The below
   //     line fixes it.
   return QRectF(-2.0,-2.0,4.0,4.0);
}

class ComponentProperty
{
   public:
      ComponentProperty(Component *c,const QString& name, const QString& value,
                        const QString& description, bool visible = false,
                        const QStringList& options = QStringList());
      ~ComponentProperty();

      void show();
      void hide();
      inline void setPos(const QPointF& pos);
      inline void moveBy(qreal dx, qreal dy);
      inline QPointF pos() const;
      void operator=(const QString& value);

      inline PropertyText* item() const;
      inline bool isVisible() const;
      void update();

      inline QString name() const;
      inline QString value() const;
      inline QString description() const;
      inline QStringList options() const;

   private:
      Component *m_component;
      const QString m_name;
      QString m_value;
      const QString m_description;
      const QStringList m_options;
      QPointF m_pos;
      PropertyText *m_item;
};

inline PropertyText* ComponentProperty::item() const
{
   return m_item;
}

inline bool ComponentProperty::isVisible() const
{
   return m_item != 0;
}

inline void ComponentProperty::setPos(const QPointF& pos)
{
   if(m_item)
      m_item->setPos(pos);
   else
      m_pos = pos;
}

inline void ComponentProperty::moveBy(qreal dx, qreal dy)
{
   if(m_item)
      m_item->moveBy(dx,dy);
   else
      m_pos += QPointF(dx,dy);
}

inline QPointF ComponentProperty::pos() const
{
   if(m_item)
      return m_item->pos();
   else
      return m_pos;
}

inline QString ComponentProperty::name() const
{
   return m_name;
}

inline QString ComponentProperty::value() const
{
   return m_value;
}

inline QString ComponentProperty::description() const
{
   return m_description;
}

inline QStringList ComponentProperty::options() const
{
   return m_options;
}

#endif //__COMPONENTPROPERTY_H

