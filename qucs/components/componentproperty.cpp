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

#include "componentproperty.h"
#include "propertytext.h"
#include "component.h"
#include "schematicscene.h"
#include "qucs-tools/global.h"

#include <QtGui/QFontMetricsF>
#include <QtCore/QtDebug>

PropertyGroup::PropertyGroup(Component *comp,SchematicScene *scene) :
   QGraphicsItemGroup(0, scene),
   m_component(comp)
{
   firstTime = true;
   //initVariables();
   firstTime = false;
   if(scene)
      setPos(mapToScene(lastChildPos));
}

void PropertyGroup::initVariables()
{
   lastChildPos = QPointF(5,5);
   if(scene() && !firstTime)
      lastChildPos = m_component->sceneBoundingRect().bottomLeft();
   QFontMetricsF fm(Qucs::font());
   fontHeight = fm.height()+4;
   itemLeft = 2.0;
   lastChildPos += QPointF(0.0, fontHeight);
   if(scene() && !firstTime)
      lastChildPos = mapFromScene(lastChildPos);
}

void PropertyGroup::addProperty(ComponentProperty *property)
{
   if(m_properties.contains(property))
      return;
   //Calling this here prevents setting flags for empty group
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   m_properties << property;

   if(property->isVisible()) {
      QPointF np = lastChildPos;
      // This makes the leftmost part of item to coincide with leftmost of item
      //np.rx() -= property->item()->boundingRect().left();
      property->setPos(mapToScene(np));
      addToGroup(property->item());
      lastChildPos.ry() += fontHeight;
   }
}

void PropertyGroup::realignItems()
{
   initVariables();
   foreach( ComponentProperty *property, m_properties) {
      if(property->isVisible()) {
         Q_ASSERT(property->item()->group() == this);
         QPointF np = lastChildPos;
         // This makes the leftmost part of item to coincide with leftmost of item
         np.rx() -= property->item()->boundingRect().left();
         property->setPos(np);
         lastChildPos.ry() += fontHeight;
      }
   }
}

QRectF PropertyGroup::boundingRect() const
{
   if(!children().isEmpty()) return QGraphicsItemGroup::boundingRect();
   //HACK: An empty group makes item to move very slow. The below
   //     line fixes it.
   return QRectF(-2.0,-2.0,4.0,4.0);
}

void PropertyGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if(scene()) {
      foreach(QGraphicsItem *item, scene()->selectedItems()) {
         if(item != this)
            item->setSelected(false);
      }
   }

   foreach(ComponentProperty *p, m_properties) {
      if(p->item() && p->item()->hasFocus())
         p->item()->clearFocus();
   }

   QGraphicsItemGroup::mousePressEvent(event);
}


ComponentProperty::ComponentProperty( Component *c,const QString& name, const QString& value,
                                      const QString& description, bool visible,const QStringList& options)
   : m_component(c),
     m_name(name),
     m_value(value),
     m_description(description),
     m_options(options),
     m_item(0)
{
   if(visible)
      show();
}

ComponentProperty::~ComponentProperty()
{
   hide();
}

void ComponentProperty::show()
{
   if(isVisible()) {
      qDebug("Item visible when calling ComponentProperty::show()");
      m_item->setPos(m_pos);
      m_item->show();
      return;
   }

   m_item = new PropertyText(this,m_component->schematicScene());
   if(group()) {
      group()->addToGroup(m_item);
      group()->realignItems();
   }
}

void ComponentProperty::hide()
{
   if(isVisible()) {
      m_pos = m_item->pos();
      if(group()) {
         group()->removeFromGroup(m_item);
         group()->realignItems();
      }
      delete m_item;
      m_item = 0;
   }
   else
      qDebug("Item already hidden while calling ComponentProperty::hide()");
}

void ComponentProperty::setVisible(bool visible)
{
   if(visible)
      show();
   else
      hide();
}

void ComponentProperty::setPos(const QPointF& pos)
{
   if(isVisible())
      m_item->setPos(pos);
   else
      m_pos = pos;
}

void ComponentProperty::setValue(const QString& value)
{
   if(m_options.isEmpty() || m_options.contains(value))
      m_value = value;
   else {
      qDebug() << "Trying to set value out of given options" ;
      qDebug() << m_name << m_options << "Given val is" << value;
   }
   if(isVisible())
      m_item->updateValue();
}

void ComponentProperty::updateValueFromItem()
{
   if(isVisible())
      m_value = m_item->toPlainText();
}

PropertyGroup* ComponentProperty::group() const
{
   return m_component->propertyGroup();
}
