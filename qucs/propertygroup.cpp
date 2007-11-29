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

#include "propertygroup.h"
#include "propertyitem.h"
#include "schematicscene.h"

#include <QtGui/QFontMetricsF>

/*! Constructor
 * \param propMap A reference to PropertyMap of a component.
 * \param scene The schematic scene to which this property should belong.
 */
PropertiesGroup::PropertiesGroup(PropertyMap &propMap,SchematicScene *scene)
   : m_propertyMapRef(propMap)
{
   if(scene) {
      scene->addItem(this);
   }
}

//! Destructor
PropertiesGroup::~PropertiesGroup()
{
   //delete only non child items, The child items
   //are deleted by destructor of base.
   foreach(PropertyItem *item, m_propertyItemsMap) {
      if(item->group() != this && !item->group()) {
         delete item;
      }
   }
}

/*! \brief Realigns all the child items of this group (deletes hidden items).
 * \details This is quite expensive . It removes all property items
 * from the group and then freshly adds them to the group again if it is
 * visible. If the property is hidden, then the corresponding property items
 * are removed from group and deleted. This also updates the visible value of
 * property.
 */
void PropertiesGroup::realignItems()
{
   // Nothing to do if scene doesn't exist.
   if(!scene()) {
      return;
   }
   // Remove all property items from group.
   foreach(PropertyItem *item, m_propertyItemsMap) {
      removeFromGroup(item);
      //Also update the property's value.
      item->updateValue();
   }
   int visibleItemsCount = 0;
   // Navigate through all properties and decide.
   foreach(const Property property, m_propertyMapRef) {
      if(property.isVisible()) {
         // Create new property item if the item is not yet created.
         // That is , if item was made visible now.
         if(!m_propertyItemsMap.contains(property.name())) {
            PropertyItem *item =
                  new PropertyItem(property.name(), m_propertyMapRef, schematicScene());
            m_propertyItemsMap.insert(property.name(), item);
         }
         PropertyItem *item = m_propertyItemsMap[property.name()];
         visibleItemsCount++;
         // If child property items exist already, this just puts the new item
         // below them.
         if(!QGraphicsItemGroup::children().isEmpty()) {
            //Calculate bottom left of group in terms of scene coords.
            QPointF itemPos = this->mapToScene(boundingRect().bottomLeft());
            // Add some offset correspondign to font.
            itemPos.ry() += QFontMetricsF(item->font()).height();
            // Make the item's left coord to conincide with that of group
            itemPos.rx() -= item->boundingRect().left();
            // Set items new position.
            item->setPos(itemPos);
         }
         addToGroup(item);
      }
      else {
         // Delete item if it existed before as it is being hidden now.
         if(m_propertyItemsMap.contains(property.name())) {
            PropertyItem *item = m_propertyItemsMap[property.name()];
            m_propertyItemsMap.remove(property.name());
            delete item;
         }
      }
   }
   if(visibleItemsCount > 0) {
      setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   }
   else {
      //Disables moving , selection and focussing of empty groups.
      setFlags(0);
   }
}

//! Forces the updation of the geometry of the property group.
void PropertiesGroup::forceUpdate()
{
   static const QLineF line(-5, -5, 5, 5);
   //HACK:This adds and removes a dummy item from group to update its geometry
   QGraphicsLineItem *dummy = new QGraphicsLineItem(-5,-5,5,5);
   addToGroup(dummy);
   removeFromGroup(dummy);
   delete dummy;
}

//! Returns the schematic scene associated.
SchematicScene* PropertiesGroup::schematicScene() const
{
   return qobject_cast<SchematicScene*>(scene());
}

/*! \brief Deselect other items on mouse click.
 * \details Deselects selected item and also clears focus of childeren items
 */
void PropertiesGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if(scene()) {
      foreach(QGraphicsItem *item, scene()->selectedItems()) {
         if(item != this) {
            item->setSelected(false);
         }
      }
   }

   foreach(PropertyItem *p, m_propertyItemsMap) {
      if(p->hasFocus()) {
         p->clearFocus();
      }
   }

   QGraphicsItemGroup::mousePressEvent(event);
}
