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

#ifndef __COMPONENT_H
#define __COMPONENT_H

#include "svgitem.h"
#include "property.h"

class Port;
class PropertiesGroup;

namespace Qucs
{
   struct ComponentData : public QSharedData
   {
         QString name;
         QString model;
         QString displayText;
         QString description;
         PropertyMap propertyMap;
   };

   class Component : public SvgItem
   {
         Q_OBJECT;
      public:
         enum ActiveStatus {
            Open, Active, Short
         };

         Component(SchematicScene *scene = 0);
         Component(Qucs::XmlReader *reader, const QString& path,
                   SchematicScene *scene);
         ~Component();

         QList<Port*> ports() const { return m_ports; }

         QString name() const { return d->name; }
         QString model() const { return d->model; }
         QString displayText() const { return d->displayText; }
         QString description() const { return d->description; }

         PropertyMap propertyMap() const { return d->propertyMap; }
         PropertyMap& propertyMapRef() { return d->propertyMap; }

         PropertiesGroup* propertyGroup() const { return m_propertyGroup; }
         void updatePropertyGroup();
         void createPropertyGroup();

         void setProperty(const QString& propName, const QVariant& value);
         QVariant property(const QString& propName) const {
            return d->propertyMap[propName].value();
         }

         void setPropertyVisible(const QString& propName, bool visibility);

         QString label() const { return property("label").toString(); }
         void setLabel(const QString& _label) {
            setProperty("label",QVariant(_label));
         }

         ActiveStatus activeStatus() const { return m_activeStatus; }
         void setActiveStatus(ActiveStatus status);
         void toggleActiveStatus();

         void readSavedData(Qucs::XmlReader *reader);
         void writeSaveData(Qucs::XmlWriter *writer) const;

         void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *);

      protected:
         QRectF adjustedBoundRect(const QRectF& rect);

      private:
         void readXml(Qucs::XmlReader *reader, const QString& path);
         void readXml(Qucs::XmlReader *reader) {}
         void writeXml(Qucs::XmlWriter *writer) {}
         void invokePropertiesDialog() {}
         void readSchematic(Qucs::XmlReader *reader, const QString& path);
         void readSchematics(Qucs::XmlReader *reader, const QString& path);

         void readProperties(Qucs::XmlReader *reader);

         QSharedDataPointer<ComponentData> d;
         PropertiesGroup *m_propertyGroup;
         ActiveStatus m_activeStatus;
         QList<Port*> m_ports;
   };

   class Wire : public QucsItem
   {
      public:
         QList<Port*> ports() const { return QList<Port*>(); }
   };
}

#endif //__COMPONENT_H
