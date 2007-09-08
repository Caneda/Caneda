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
#include <QtCore/QTimer>

PropertyGroup::PropertyGroup(Component *comp,SchematicScene *scene) :
   QGraphicsItemGroup(0, scene),
   m_component(comp)
{
   firstTime = true;
   QTimer::singleShot(30, this, SLOT(init()));
}

PropertyGroup::~PropertyGroup()
{
   qDeleteAll(m_properties);
}

void PropertyGroup::copyTo(PropertyGroup *grp)
{
   foreach(ComponentProperty *property, m_properties) {
      ComponentProperty *other = grp->m_component->property(property->name());
      if(other)
         other->setValue(property->value());
   }
}

void PropertyGroup::init()
{
   QFontMetricsF fm(Qucs::font());
   fontHeight = fm.height()+4;
   itemLeft = 2.0;
   lastChildPos = QPointF(boundingRect().left(), fontHeight);
   forceUpdate();
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
      np.rx() -= property->item()->boundingRect().left();
      property->setPos(mapToScene(np));

      addToGroup(property->item());
      update();
      lastChildPos.ry() += fontHeight;
   }
}

void PropertyGroup::realignItems()
{
   init();
   foreach( ComponentProperty *property, m_properties) {
      if(property->isVisible()) {
         //Q_ASSERT(property->item()->group() == this);
         QPointF np = lastChildPos;
         // This makes the leftmost part of item to coincide with leftmost of item
         np.rx() -= property->item()->boundingRect().left();
         property->setPos(np);
         lastChildPos.ry() += fontHeight;
      }
   }
   forceUpdate();
}

//HACK: This adds and removes a dummy item from group to update its geometry
void PropertyGroup::forceUpdate()
{
   QGraphicsLineItem *l = new QGraphicsLineItem(-5,-5,5,5);
   addToGroup(l);
   removeFromGroup(l);
   delete l;
}

QRectF PropertyGroup::boundingRect() const
{
   if(!QGraphicsItemGroup::children().isEmpty())
      return QGraphicsItemGroup::boundingRect();
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
   if(isVisible())
      delete m_item;
}

void ComponentProperty::show()
{
   if(isVisible()) {
      Q_ASSERT(m_item->isVisible());
      return;
   }

   m_item = new PropertyText(this,m_component->schematicScene());

   Q_ASSERT(group());
   group()->addToGroup(m_item);
   group()->realignItems();
}

void ComponentProperty::hide()
{
   if(!isVisible()) {
      Q_ASSERT(m_item == 0);
      return;
   }

   updateValueFromItem();

   Q_ASSERT(group());
   group()->removeFromGroup(m_item);
   group()->realignItems();

   delete m_item;
   m_item = 0;
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
