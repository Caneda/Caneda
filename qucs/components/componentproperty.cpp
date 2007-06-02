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

PropertyGroup::PropertyGroup(Component *comp,SchematicScene *scene) :
   QGraphicsItemGroup(0,static_cast<QGraphicsScene*>(scene)), m_component(comp)
{
   Q_ASSERT(scene);

   QFontMetricsF fm(Qucs::font());
   lastChildPos = comp->mapToScene(comp->boundingRect().bottomLeft());
   fontHeight = fm.height()+4;
   itemLeft = 2.0;
   lastChildPos += QPointF(0.0, fontHeight);
   setPos(lastChildPos);
   lastChildPos = mapFromScene(lastChildPos);
   //lastChildPos += QPointF(itemLeft,fontHeight);
}

void PropertyGroup::addChild(ComponentProperty *child)
{
   if(m_children.count(child)) return;
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   m_children << child;
   if(child->isVisible()) {
      QPointF np = lastChildPos;
      // This makes the leftmost part of item to coincide with leftmost of item
      //np.rx() -= child->item()->boundingRect().left();
      child->setPos(mapToScene(np));
      addToGroup(child->item());
      lastChildPos.ry() += fontHeight;
   }
}

void PropertyGroup::hideChild(ComponentProperty *child)
{
   int index = m_children.indexOf(child);
   Q_ASSERT(index != -1);
   child->hide();
   if(index + 1 < m_children.size())
      realignItems(index+1);
}

void PropertyGroup::showChild(ComponentProperty *child)
{
   int index = m_children.indexOf(child);
   Q_ASSERT(index != -1);
   child->show();
   addToGroup(child->item());
   realignItems(index);
}

void PropertyGroup::realignItems( int fromIndex )
{
   Q_ASSERT(fromIndex < m_children.size());
   QPointF lastVisibleItemPos = pos() + QPointF(itemLeft, 0.0);
   if(fromIndex != 0)
      for(int i = fromIndex - 1; i > -1; --i)
         if(m_children.at(i)->isVisible()) {
            lastVisibleItemPos = m_children.at(i)->item()->pos();
            break;
         }

   qreal y = lastVisibleItemPos.y() + fontHeight;

   for(int i = fromIndex; i < m_children.size(); ++i) {
      ComponentProperty *p = m_children[i];
      if(p->isVisible()) {
         p->item()->setPos(itemLeft - p->item()->boundingRect().left(),y);
         y += fontHeight;
      }
   }
}

void PropertyGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if(scene())
      scene()->clearSelection();
   qDebug("CA");
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
   if(m_item)
   {
      m_item->setPos(m_pos);
      m_item->show();
      return;
   }

   m_item = new PropertyText(this,m_component->schematicScene());
}

void ComponentProperty::hide()
{
   if(m_item)
      m_pos = m_item->pos();
   delete m_item;
   m_item = 0;
}

void ComponentProperty::operator=(const QString& value)
{
   if(m_options.isEmpty())
      m_value = value;
   else if(m_options.contains(value))
   {
      m_value = value;
   }
   else
   {
      qDebug() << "Trying to set value out of given options" ;
      qDebug() << m_name << m_options << "Given val is" << value;
   }
   update();
}

void ComponentProperty::update()
{
   if(m_item)
      m_item->updateValue();
}
