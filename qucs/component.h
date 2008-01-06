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

#ifndef __QCOMPONENT_H
#define __QCOMPONENT_H

#include "svgitem.h"
#include "property.h"

class PropertiesGroup;
class Port;
class PortData;
class QUndoCommand;


//! Represents status of component - short, open or just active.
namespace Qucs
{
   enum ActiveStatus {
      Open=0, Active, Short
   };
}

/*!
 * \brief Represents the shareable data of component.
 *
 * This class inherits \a QSharedData which enables implicit sharing
 * of this class.
 */
struct ComponentData : public QSharedData
{
   public:
      ComponentData(Qucs::ActiveStatus a = Qucs::Active) : activeStatus(a) {}
      QString name;
      QString displayText;
      QString description;
      PropertyMap propertyMap;
      Qucs::ActiveStatus activeStatus;
      QMap<QString, QList<PortData*> > schematicPortMap;
};

/*!
 * \brief Represents the component on schematic.
 *
 * This component can either be directly loaded from an xml doc or
 * manually set data if it requires.
 */
class Component : public SvgItem
{
      Q_OBJECT;
   public:
      enum { Type = QucsItem::ComponentType };

      Component(SchematicScene *scene = 0);
      Component(const QSharedDataPointer<ComponentData>& other,
                SvgPainter *svgPainter,
                SchematicScene *scene = 0);
      ~Component();

      //! Returns a list of ports of the component.
      QList<Port*> ports() const { return m_ports; }
      //! Used for component identification at runtime.
      int type() const { return Component::Type; }
      //! Returns name of the component.
      QString name() const { return d->name; }
      //! Represents model of component, which is infact a property.
      QString model() const { return property("model").toString(); }
      //! Returns string to be displayed in sidebar, toolbar or ..
      QString displayText() const { return d->displayText; }
      //! Returns a helpful text corresponding to component.
      QString description() const { return d->description; }
      //! Returns the property map (actually copy of property map)
      PropertyMap propertyMap() const { return d->propertyMap; }
      //! Returns property group of the component.
      PropertiesGroup* propertyGroup() const { return m_propertyGroup; }

      void updatePropertyGroup();
      void createPropertyGroup();

      bool setProperty(const QString& propName, const QVariant& value);
      /*! \brief Method to obtain property's value.
       * \param propName The name of property.
       * \return Returns corresponding property if it exists otherwise
       * returns empty QVariant().
       */
      QVariant property(const QString& propName) const {
         return d->propertyMap.contains(propName) ? d->propertyMap[propName].value() :
            QVariant();
      }

      void setPropertyVisible(const QString& propName, bool visibility);

      /*! Returns the label of the component which is of form
       * {label_prefix}{number_suffix}
       */
      QString label() const { return property("label").toString(); }
      bool setLabel(const QString& _label);

      //! Returns current symbol of component.
      QString symbol() const { return property("symbol").toString(); }
      bool setSymbol(const QString& newSymbol);

      //! Returns the active status of the component.
      Qucs::ActiveStatus activeStatus() const { return d->activeStatus; }
      void setActiveStatus(Qucs::ActiveStatus status);
      void toggleActiveStatus();

      void loadData(Qucs::XmlReader *reader);
      void saveData(Qucs::XmlWriter *writer) const;

      void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *);

      void checkAndConnect(bool pushUndoCommands = true, QUndoCommand *parent = 0);

      virtual void rotate90(Qucs::AngleDirection dir = Qucs::AntiClockwise);
      virtual void mirrorAlong(Qt::Axis axis);

   protected:
      QRectF adjustedBoundRect(const QRectF& rect);
      QVariant itemChange(GraphicsItemChange change, const QVariant &value);

   private:
      void init();

      QSharedDataPointer<ComponentData> d;
      PropertiesGroup *m_propertyGroup;
      QList<Port*> m_ports;
};

namespace Qucs
{
   void readComponentData(Qucs::XmlReader *reader, const QString& path,
                          SvgPainter *svgPainter, QSharedDataPointer<ComponentData> &d);
   void readSchematics(Qucs::XmlReader *reader,
                       const QString& path,
                       SvgPainter *svgPainter,
                       QSharedDataPointer<ComponentData> &d);
   void readSchematic(Qucs::XmlReader *reader,
                      const QString& path,
                      SvgPainter *svgPainter,
                      QSharedDataPointer<ComponentData> &d);
}

#endif //__COMPONENT_H
