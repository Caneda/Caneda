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

#ifndef __PROPERTYGROUP_H
#define __PROPERTYGROUP_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItemGroup>

// Forward declarations.
class PropertyItem;
class SchematicScene;
namespace Qucs {
   class Component;
}

/*! This class groups the properties of a item.
 * \details This takes care of creation and destruction of property items as
 * well. This stores a reference to the actual property map.
 * \sa Property, PropertyItem
 */
class PropertiesGroup : public QObject, public QGraphicsItemGroup
{
   Q_OBJECT;
   public:
      enum { PropertiesGroupType = UserType + 73 };
      enum { Type = PropertiesGroupType };

      PropertiesGroup(SchematicScene *scene = 0);

      int type() { return PropertiesGroupType; }

      void realignItems();
      void forceUpdate();

      SchematicScene* schematicScene() const;
      Qucs::Component* component() const;

   protected:
      void mousePressEvent(QGraphicsSceneMouseEvent *event);

   private:
      //! Internal storage of property items for book keeping.
      QMap<QString, PropertyItem*> m_propertyItemsMap;
};

#endif //__PROPERTYGROUP_H
