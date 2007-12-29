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
#include "wire.h"
#include "undocommands.h"
#include "qucs-tools/global.h"

#include <QtCore/QDebug>
#include <QtGui/QPainter>

namespace Qucs
{
   //! Constructs and initializes default empty component object.
   Component::Component(SchematicScene *scene) :
      SvgItem(scene),
      d(new ComponentData()), m_propertyGroup(0)
   {
      init();
   }

   //! Constructs a component from \a other data.
   Component::Component(const QSharedDataPointer<ComponentData>& other,
                        SchematicScene *scene) :
      SvgItem(scene),
      d(other), m_propertyGroup(0)
   {
      init();
   }

   //! Constructs a component from xml stream.
   Component::Component(Qucs::XmlReader *reader, const QString& path,
                        SchematicScene *scene) :
      SvgItem(scene),
      d(new ComponentData()), m_propertyGroup(0)
   {
      readXml(reader, path);
      init();
   }

   //! Destructor.
   Component::~Component()
   {
      delete m_propertyGroup;
   }

   void Component::init()
   {
      setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
      Property _label("label", tr("Label"), QVariant::String, true,
                      false);
      d->propertyMap.insert("label", _label);
   }

   /*!
    * \brief This method updates the property display on schematic.
    *
    * This also takes care of creating new property group if it didn't
    * exist before and also deletes it if none of property are visible.
    */
   void Component::updatePropertyGroup()
   {
      bool itemsVisible = false;
      PropertyMap::const_iterator it = propertyMap().constBegin(),
         end = propertyMap().constEnd();
      while (it != end) {
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

   //! Creates property group for the first time.
   void Component::createPropertyGroup()
   {
      //delete the old group if it exists.
      delete m_propertyGroup;
      m_propertyGroup = new PropertiesGroup(schematicScene());
      m_propertyGroup->setParentItem(this);
      m_propertyGroup->realignItems();
   }

   /*!
    * \brief Method used to set property's value.
    *
    * This also handles the property change of special properties such as
    * symbol, label and forwards the call to those methods on match.
    * It also updates the textual display of property on schematic.
    *
    * \param propName The property which is to be set.
    * \param value The new value to be set.
    * \return Returns true if successful, else returns false.
    */
   bool Component::setProperty(const QString& propName, const QVariant& value)
   {
      if(!propertyMap().contains(propName)) {
         qDebug() << "Component::setPropertyValue(): Property '" << propName
                  << "' doesn't exist!";
         return false;
      }
      if(propName == "symbol") {
         return setSymbol(value.toString());
      }
      if(propName == "label") {
         return setLabel(value.toString());
      }

      bool state = d->propertyMap[propName].setValue(value);
      if(state) {
         updatePropertyGroup();
      }
      return state;
   }

   //! Takes care of visibility of property text on schematic.
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

   /*
    *  \brief Sets the symbol of component to newSymbol if it exists.
    *
    * This method sets the symbol property's value and then takes care
    * of geometry changes as well.
    *
    * \param newSymbol The symbol to be set now
    * \return True on success, and false on failure.
    * \todo Take care of undo\redo as well as connection changes.
    */
   bool Component::setSymbol(const QString& newSymbol)
   {
      QString svgid = newSymbol;
      QString prefix(name());
      prefix.append('/');
      if(!propertyMap().contains("symbol")) {
         qDebug() << "Component::setSymbol() : 'symbol' property not found";
         return false;
      }
      if(!d->propertyMap["symbol"].setValue(svgid)) {
         return false;
      }

      qDeleteAll(m_ports);
      m_ports.clear();
      const QList<PortData*> portDatas = d.constData()->schematicPortMap[svgid];
      foreach(const PortData *data, portDatas) {
         m_ports << new Port(this, data->pos, data->name);
      }
      svgid.prepend(prefix);

      //now register the new connections for new symbol.
      registerConnections(svgid, schematicScene()->svgPainter());
      updatePropertyGroup();

      return true;
   }

   /*! \brief Sets the label of component.
    *
    * This also handles lable prefix and number suffix appropriately.
    * \param newLabel The label to be set now.
    * \return Returns true on success and false on failure.
    * \todo Yet to implement label prefix and number suffixing.
    */
   bool Component::setLabel(const QString& newLabel)
   {
      if(!propertyMap().contains("label")) {
         qDebug() << "Component::setLabel() : 'label property not found";
         return false;
      }
      //TODO: Yet to implement label prefix and number suffixing.
      bool state = d->propertyMap["label"].setValue(newLabel);
      if(state) {
         updatePropertyGroup();
      }
      return state;
   }

   //! Sets the component's activeStatus to \a status.
   void Component::setActiveStatus(ActiveStatus status)
   {
      if(status == Short && m_ports.size() <= 1) {
         qWarning() << "Cannot short components with <= 1 ports";
         return;
      }
      d->activeStatus = status;
      update();
   }

   /*! Toggles active status appropriately also taking care of special
    * condition where components with <= 1 port shouldn't be shorted.
    */
   void Component::toggleActiveStatus()
   {
      ActiveStatus status = (ActiveStatus)((d->activeStatus + 1) % 3);
      if(status == Short && m_ports.size() <= 1) {
         status = (ActiveStatus)((status + 1) % 3);
      }
      setActiveStatus(status);
   }

   //! \todo implement.
   void Component::loadData(Qucs::XmlReader *reader)
   {
      //TODO: Implement.
   }

   //! \todo implement.
   void Component::saveData(Qucs::XmlWriter *writer) const
   {
      //TODO: Implement.
   }

   //! Draw the compnent using svg painter. Also handle active status.
   void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *o,
                         QWidget *w)
   {
      SvgItem::paint(painter, o, w);
      drawPorts(m_ports, painter, o);

      if(activeStatus() != Qucs::Active) {
         painter->setPen(activeStatus() == Qucs::Short ? Qt::darkGreen :
                         Qt::darkRed);
         painter->drawRect(boundingRect());

         QPointF tl = boundingRect().topLeft();
         QPointF br = boundingRect().bottomRight();
         QPointF tr = boundingRect().topRight();
         QPointF bl = boundingRect().bottomLeft();

         painter->drawLine(tl, br);
         painter->drawLine(bl, tr);
      }
   }

   //! Check for connections and connect the coinciding ports.
   void Component::checkAndConnect(bool pushUndoCommands)
   {
      foreach(Port *port, m_ports) {
         Port *other = port->findCoincidingPort();
         if(other) {
            if(pushUndoCommands) {
               ConnectCmd *cmd = new ConnectCmd(port, other);
               schematicScene()->undoStack()->push(cmd);
            } else {
               port->connectTo(other);
            }
         }
      }
   }

   //! Returns the rect adjusted to accomodate ports too.
   QRectF Component::adjustedBoundRect(const QRectF& rect)
   {
      return portsRect(m_ports, rect);
   }

   //! React to change of item position.
   QVariant Component::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
   {
      return SvgItem::itemChange(change, value);
   }

   /*! This method reads component from xml stream
    * \param reader XmlReader pointing to component's xml.
    * \param path Absolute path of xml file being processed.
    */
   void Component::readXml(Qucs::XmlReader *reader, const QString& path)
   {
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
               d->displayText = reader->readLocaleText(Qucs::localePrefix());
               Q_ASSERT(reader->isEndElement() &&
                        reader->name() == "displaytext");
            }
            else if(reader->name() == "description") {
               d->description = reader->readLocaleText(Qucs::localePrefix());
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
   }

   void Component::readSchematics(Qucs::XmlReader *reader, const QString& svgPath)
   {
      Q_ASSERT(reader->isStartElement() && reader->name() == "schematics");

      QStringList parsedSymbols;

      QString defaultSchematic =
         reader->attributes().value("default").toString();
      Q_ASSERT(!defaultSchematic.isEmpty());

      while(!reader->atEnd()) {
         reader->readNext();

         if(reader->isEndElement()) break;

         if(reader->isStartElement()) {
            if(reader->name() == "schematic") {
               QString schName =
                  reader->attributes().value("name").toString();

               Q_ASSERT(!schName.isEmpty());

               parsedSymbols << schName;

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

      Q_ASSERT(parsedSymbols.contains(defaultSchematic));
      QString symbolDescription =
         tr("Represents the current symbol of component.");
      QVariant defValue(defaultSchematic);
      Q_ASSERT(defValue.convert(QVariant::String));
      Property symb("symbol", symbolDescription, QVariant::String, false,
                    false, defValue, parsedSymbols);
      d->propertyMap.insert("symbol", symb);
      setSymbol(defaultSchematic);
   }

   void Component::readSchematic(Qucs::XmlReader *reader, const QString& svgPath)
   {
      Q_ASSERT(reader->isStartElement() && reader->name() == "schematic");

      QString schName = reader->attributes().value("name").toString();
      QString schType = reader->attributes().value("href").toString();

      if(!schType.isEmpty()) {
         QFile svgFile(svgPath + "/" + schType);
         svgFile.open(QIODevice::ReadOnly | QIODevice::Text);

         QByteArray svgContent(svgFile.readAll());

         QString svgId = d.constData()->name + "/" + schName;
         schematicScene()->svgPainter()->registerSvg(svgId, svgContent);
      }

      while(!reader->atEnd()) {
         reader->readNext();

         if(reader->isEndElement())
            break;

         if(reader->isStartElement()) {
            if(reader->name() == "svg") {
               Q_ASSERT(schType.isEmpty());
               QByteArray svgContent = reader->readXmlFragment().toLocal8Bit();
               QString svgId = d.constData()->name + "/" + schName;
               schematicScene()->svgPainter()->registerSvg(svgId, svgContent);
            }
            else if(reader->name() == "port") {
               QXmlStreamAttributes attribs = reader->attributes();
               bool ok;
               qreal x = attribs.value("x").toString().toDouble(&ok);
               Q_ASSERT(ok);
               qreal y = attribs.value("y").toString().toDouble(&ok);
               Q_ASSERT(ok);
               QString portName = attribs.value("name").toString();
               d->schematicPortMap[schName] <<
                  new PortData(QPointF(x, y), portName);

               while(!reader->isEndElement())
                  reader->readNext();
            }
            else {
               reader->readUnknownElement();
            }
         }
      }
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
               Property prop = PropertyFactory::createProperty(reader);
               d->propertyMap.insert(prop.name(), prop);
            }
            else {
               reader->readUnknownElement();
            }
         }
      }
   }
}