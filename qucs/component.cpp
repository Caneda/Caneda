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

#include "component.h"
#include "propertygroup.h"
#include "port.h"
#include "xmlutilities.h"
#include "schematicscene.h"
#include "qucs-tools/global.h"

#include <QtCore/QDebug>

namespace Qucs
{
   Component::Component(SchematicScene *scene) : SvgItem(scene)
   {
      d = new ComponentData();
      m_propertyGroup = 0;
      m_activeStatus = Active;
      setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   }

   Component::Component(Qucs::XmlReader *reader, const QString& path,
                        SchematicScene *scene) : SvgItem(scene)
   {
      d = new ComponentData();
      m_propertyGroup = 0;
      m_activeStatus = Active;
      setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
      readXml(reader, path);
   }

   Component::~Component()
   {
      delete m_propertyGroup;
   }

   void Component::updatePropertyGroup()
   {
      qDebug() << "Component::updatePropertyGroup() called";
      bool itemsVisible = false;
      PropertyMap::const_iterator it = propertyMap().constBegin();
      while (it != propertyMap().constEnd()) {
         if(it->isVisible()) {
            itemsVisible = true;
            break;
         }
         ++it;
      }
      // Delete the group if none of the properties are visible.
      if(!itemsVisible) {
         delete m_propertyGroup;
         m_propertyGroup = 0;
         return;
      }

      if(!m_propertyGroup) {
         createPropertyGroup();
      }
      else {
         m_propertyGroup->realignItems();
      }
   }

   void Component::createPropertyGroup()
   {
      qDebug() << "creating";
      m_propertyGroup = new PropertiesGroup(schematicScene());
      m_propertyGroup->setParentItem(this);
      m_propertyGroup->realignItems();
   }

   void Component::setProperty(const QString& propName, const QVariant& value)
   {
      if(!propertyMap().contains(propName)) {
         qWarning() << "Component::setPropertyValue(): Property " << propName
                    << " doesn't exist!";
         return;
      }
      d->propertyMap[propName].setValue(value);
      updatePropertyGroup();
   }

   void Component::setPropertyVisible(const QString& propName, bool visiblity)
   {
      if(!propertyMap().contains(propName)) {
         qWarning() << "Component::setPropertyVisible() : Property " << propName
                    << " doesn't exist!";
         return;
      }
      d->propertyMap[propName].setVisible(visiblity);
      updatePropertyGroup();
   }

   void Component::setActiveStatus(ActiveStatus status)
   {
      if(status == Short && m_ports.size() <= 1) {
         qWarning() << "Cannot short components with <= 1 ports";
         return;
      }
      m_activeStatus = status;
      update();
   }

   void Component::toggleActiveStatus()
   {
      ActiveStatus status = (ActiveStatus)((m_activeStatus + 1) % 3);
      if(status == Short && m_ports.size() <= 1) {
         status = (ActiveStatus)((status + 1) % 3);
      }
      setActiveStatus(status);
   }

   void Component::readSavedData(Qucs::XmlReader *reader)
   {
   }

   void Component::writeSaveData(Qucs::XmlWriter *writer) const
   {
   }

   void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
   {
      SvgItem::paint(painter, o, w);
      drawPorts(m_ports, painter, o);
   }

   QRectF Component::adjustedBoundRect(const QRectF& rect)
   {
      return portsRect(m_ports, rect);
   }

   void Component::readXml(Qucs::XmlReader *reader, const QString& path)
   {
      qDebug() << "Entered Component::readXml()";
      Q_ASSERT(reader->isStartElement() && reader->name() == "component");
      QXmlStreamAttributes attributes = reader->attributes();

      d->name = attributes.value("name").toString();
      Q_ASSERT(!d->name.isEmpty());

      Qucs::checkVersion(attributes.value("version").toString());

      while(!reader->atEnd()) {
         reader->readNext();

         if(reader->isEndElement())
            break;

         if(reader->isStartElement()) {
            if(reader->name() == "displaytext") {
               qDebug() << "Locale prefix = " << Qucs::localePrefix();
               d->displayText = reader->readLocaleText("uk");//Qucs::localePrefix());
               qDebug() << "reading displaytext = " << d->displayText;
               qDebug() << reader->name().toString();
               Q_ASSERT(reader->isEndElement() && reader->name() == "displaytext");
            }
            else if(reader->name() == "description") {
               d->description = reader->readLocaleText(Qucs::localePrefix());
               qDebug() << "reading description" << d->description;
               Q_ASSERT(reader->isEndElement());
            }
            else if(reader->name() == "schematics") {
               readSchematics(reader, path);
            }
            else if(reader->name() == "properties") {
               readProperties(reader);
            }
            else {
               reader->readUnknownElement();
            }
         }
      }
      qDebug() << "Leaving Component::readXml()";
   }

   void Component::readSchematics(Qucs::XmlReader *reader, const QString& svgPath)
   {
      qDebug() << "Entered Component::readSchematics()";
      Q_ASSERT(reader->isStartElement() && reader->name() == "schematics");

      QStringList parsedSymbols;

      QString defaultSchematic = reader->attributes().value("default").toString();
      Q_ASSERT(!defaultSchematic.isEmpty());
      int schematicsFound = 0;
      while(!reader->atEnd()) {
         reader->readNext();

         if(reader->isEndElement()) break;

         if(reader->isStartElement()) {
            if(reader->name() == "schematic") {
               QString schName = reader->attributes().value("name").toString();

               Q_ASSERT(!schName.isEmpty());

               parsedSymbols << schName.prepend(d.constData()->name + '/');

               ++schematicsFound;
               readSchematic(reader, svgPath);
            }
            else {
               reader->readUnknownElement();
            }
         }
      }
      if(reader->hasError()) {
         qWarning() << "Some error found.";
         return;
      }

      defaultSchematic.prepend(d.constData()->name + '/');
      Q_ASSERT(parsedSymbols.contains(defaultSchematic));
      QString symbolDescription = tr("Represents the current symbol of component.");
      QVariant defValue(defaultSchematic);
      Q_ASSERT(defValue.convert(QVariant::String));
      Property symb("symbol", symbolDescription, QVariant::String, false, false,
                    defValue, parsedSymbols);
      d->propertyMap.insert("symbol", symb);
      registerConnections(defaultSchematic, schematicScene()->svgPainter());
      qDebug() << "Leaving Component::readSchematics()";
   }

   void Component::readSchematic(Qucs::XmlReader *reader, const QString& svgPath)
   {
      qDebug() << "Entered Component::readSchematic()";
      Q_ASSERT(reader->isStartElement() && reader->name() == "schematic");

      QString schName = reader->attributes().value("name").toString();
      QString schType = reader->attributes().value("href").toString();

      if(!schType.isEmpty()) {
         QFile svgFile(svgPath + schType);
         svgFile.open(QIODevice::ReadOnly | QIODevice::Text);

         QByteArray svgContent(svgFile.readAll());

         QString svgId = d.constData()->name + "/" + schName;
         qDebug() << "Component::readSchematic() : Registering svg id " << svgId;
         schematicScene()->svgPainter()->registerSvg(svgId, svgContent);
      }

      while(!reader->atEnd()) {
         reader->readNext();

         if(reader->isEndElement())
            break;

         if(reader->isStartElement()) {
            if(reader->name() == "svg") {
               Q_ASSERT(schType.isEmpty());
               qDebug() << "Yet to implement internal svg support";
               reader->readUnknownElement();
            }
            else if(reader->name() == "port") {
               QXmlStreamAttributes attribs = reader->attributes();
               bool ok;
               qreal x = attribs.value("x").toString().toDouble(&ok);
               Q_ASSERT(ok);
               qreal y = attribs.value("y").toString().toDouble(&ok);
               Q_ASSERT(ok);
               QString portName = attribs.value("name").toString();

               m_ports << new Port(this, QPointF(x, y), portName);

               while(!reader->isEndElement())
                  reader->readNext();
            }
            else {
               reader->readUnknownElement();
            }
         }
      }
      qDebug() << "Leaving Component::readSchematic()";
   }

   void Component::readProperties(Qucs::XmlReader *reader)
   {
      Q_ASSERT(reader->isStartElement() && reader->name() == "properties");
      while(!reader->atEnd()) {
         reader->readNext();

         if(reader->isEndElement())
            break;

         else if(reader->isStartElement()) {
            if(reader->name() == "property") {
               Property prop(reader);
               d->propertyMap.insert(prop.name(), prop);
            }
            else {
               reader->readUnknownElement();
            }
         }
      }
   }
}
