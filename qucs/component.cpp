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
#include "library.h"

#include <QtCore/QDebug>
#include <QtGui/QPainter>


//! Constructs and initializes default empty component object.
Component::Component(SchematicScene *scene) :
   SvgItem(0, scene),
   d(new ComponentData()), m_propertyGroup(0)
{
   init();
}

//! Constructs a component from \a other data.
Component::Component(const QSharedDataPointer<ComponentData>& other,
                     SvgPainter *svgPainter_,
                     SchematicScene *scene) :
   SvgItem(svgPainter_, scene),
   d(other), m_propertyGroup(0)
{
   init();
}

//! Destructor.
Component::~Component()
{
   delete m_propertyGroup;
   qDeleteAll(m_ports);
}

//! Intialize the component.
void Component::init()
{
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   Property _label("label", tr("Label"), QVariant::String, true,
                   false, labelPrefix().append('1'));
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
   // determine if any item is visible.
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
   m_propertyGroup->setTransform(transform().inverted());
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
 * \brief Sets the symbol of component to newSymbol if it exists.
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

   registerConnections(svgid, svgPainter());

   updatePropertyGroup();

   return true;
}

/*!
 * \brief Sets the label of component.
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

   if(!newLabel.startsWith(labelPrefix()))
      return false;

   //TODO: Yet to implement label prefix and number suffixing.
   bool state = d->propertyMap["label"].setValue(newLabel);
   if(state) {
      updatePropertyGroup();
   }
   return state;
}

//! Sets the component's activeStatus to \a status.
void Component::setActiveStatus(Qucs::ActiveStatus status)
{
   if(status == Qucs::Short && m_ports.size() <= 1) {
      qWarning() << "Cannot short components with <= 1 ports";
      return;
   }
   d->activeStatus = status;
   update();
}

/*!
 * \brief Toggles active status appropriately.
 *
 * Also taking care of special condition where components with <= 1 port
 * shouldn't be shorted.
 */
void Component::toggleActiveStatus()
{
   Qucs::ActiveStatus status = (Qucs::ActiveStatus)((d->activeStatus + 1) % 3);
   if(status == Qucs::Short && m_ports.size() <= 1) {
      status = (Qucs::ActiveStatus)((status + 1) % 3);
   }
   setActiveStatus(status);
}

/*!
 * \brief Convenience static method to load component saved as xml.
 *
 * \param reader The xmlreader used to read xml data.
 * \param scene SchematicScene to which component should be parented to.
 * \return Returns new component pointer on success and null on failure.
 */
Component* Component::loadComponentData(Qucs::XmlReader *reader, SchematicScene *scene)
{
   Component *retVal = 0;
   Q_ASSERT(reader->isStartElement() && reader->name() == "component");

   QString compName = reader->attributes().value("name").toString();
   QString libName = reader->attributes().value("library").toString();

   Q_ASSERT(!compName.isEmpty());

   retVal = LibraryLoader::defaultInstance()->newComponent(compName, scene, libName);
   if(retVal) {
      retVal->loadData(reader);
   }
   else {
      //read upto end if component is not found in any of qucs identified libraries.
      reader->readUnknownElement();
   }
   return retVal;
}

//! \reimp
void Component::loadData(Qucs::XmlReader *reader)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "component");

   d->name = reader->attributes().value("name").toString();
   d->library = reader->attributes().value("library").toString();

   QString activeStr = reader->attributes().value("activeStatus").toString();
   if(activeStr.isEmpty())
      activeStr = "active";
   setActiveStatus(activeStr == "active" ? Qucs::Active :
                   activeStr == "open" ? Qucs::Open : Qucs::Short);

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement())
         break;

      if(reader->isStartElement()) {
         if(reader->name() == "pos") {
            setPos(reader->readPoint());
         }
         else if(reader->name() == "propertyPos") {
            QPointF point = reader->readPoint();
            if(m_propertyGroup) {
               m_propertyGroup->setPos(point);
            }
         }
         else if(reader->name() == "transform") {
            setTransform(reader->readTransform());
         }
         else if(reader->name() == "properties") {
            //note the usage as it expects reference of property map.
            readProperties(reader, d->propertyMap);
         }
         else {
            qWarning() << "Warning: Found unknown element" << reader->name().toString();
            reader->readUnknownElement();
         }
      }
   }
}

//! \reimp
void Component::saveData(Qucs::XmlWriter *writer) const
{
   writer->writeStartElement("component");
   writer->writeAttribute("name", name());
   writer->writeAttribute("library", library());

   QLatin1String activeStr(d->activeStatus == Qucs::Active ? "active" :
                           d->activeStatus == Qucs::Open ? "open" : "short");
   writer->writeAttribute("activeStatus", activeStr);

   writer->writePoint(pos(), "pos");
   if(m_propertyGroup) {
      writer->writePoint(m_propertyGroup->pos(), "propertyPos");
   }

   writer->writeTransform(transform());

   writeProperties(writer, d.constData()->propertyMap);

   writer->writeEndElement();
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
void Component::checkAndConnect(Qucs::UndoOption opt)
{
   if(opt == Qucs::PushUndoCmd)
      schematicScene()->undoStack()->beginMacro(QString());
   foreach(Port *port, m_ports) {
      Port *other = port->findCoincidingPort();
      if(other) {
         QList<Wire*> wires = Port::wiresBetween(port, other);

         if(opt == Qucs::PushUndoCmd) {
            ConnectCmd *cmd = new ConnectCmd(port, other, wires, schematicScene());
            schematicScene()->undoStack()->push(cmd);
         }
         else {
            qDeleteAll(wires);
            port->connectTo(other);
         }
      }
   }

   if(opt == Qucs::PushUndoCmd)
      schematicScene()->undoStack()->endMacro();
}

//! Returns a copy of this component.
QucsItem* Component::copy(SchematicScene *scene) const
{
   Component *retVal = new Component(d, svgPainter(), scene);
   //no need for Component::copyDataTo() because the data is already copied from d pointer.
   QucsItem::copyDataTo(static_cast<QucsItem*>(retVal));
   retVal->setSymbol(symbol()); //to register svg connections
   retVal->updatePropertyGroup();
   return retVal;
}

//! Copies the data to \a component.
void Component::copyDataTo(Component *component) const
{
   QucsItem::copyDataTo(static_cast<QucsItem*>(component));
   component->d = d;
   component->updatePropertyGroup();
   component->update();
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
   if(change == ItemTransformHasChanged && m_propertyGroup) {
      //set the inverse of component's matrix to property group so that
      //it maintains identity when transformed.
      m_propertyGroup->setTransform(transform().inverted());
   }
   return SvgItem::itemChange(change, value);
}

namespace Qucs
{
   /*!
    * \brief Reads component data from component description xml file.
    *
    * \param reader XmlReader responsible for reading xml data.
    * \param path The path of the xml file being processed.
    * \param svgPainter The SvgPainter object to which the symbols should be exported to.
    * \param d (Output variable) The data ptr where data should be uploaded.
    */
   void readComponentData(Qucs::XmlReader *reader, const QString& path,
                          SvgPainter *svgPainter, QSharedDataPointer<ComponentData> &d)
   {
      Q_ASSERT(reader->isStartElement() && reader->name() == "component");
      QXmlStreamAttributes attributes = reader->attributes();

      //check version compatibility first.
      if(!Qucs::checkVersion(attributes.value("version").toString())) {
         reader->readUnknownElement(); //read till end tag to ignore this component.
         d = static_cast<ComponentData*>(0);
         return;
      }

      d->name = attributes.value("name").toString();
      Q_ASSERT(!d->name.isEmpty());
      d->labelPrefix = attributes.value("label").toString();
      Q_ASSERT(!d->labelPrefix.isEmpty());

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
               readSchematics(reader, path, svgPainter, d);
            }
            else if(reader->name() == "properties") {
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
            else {
               reader->readUnknownElement();
            }
         }
      }
      if(reader->hasError()) {
         //Force data to be null on error to avoid obvious errors.
         d = static_cast<ComponentData*>(0);
      }
   }

   //! Read schematics section of component description xml file.
   void readSchematics(Qucs::XmlReader *reader, const QString& svgPath, SvgPainter *svgPainter,
                       QSharedDataPointer<ComponentData> &d)
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

               readSchematic(reader, svgPath, svgPainter, d);
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
         QObject::tr("Represents the current symbol of component.");
      QVariant defValue(defaultSchematic);
      Q_ASSERT(defValue.convert(QVariant::String));
      Property symb("symbol", symbolDescription, QVariant::String, false,
                    false, defValue, parsedSymbols);
      d->propertyMap.insert("symbol", symb);
   }

   //! Reads the schematic tag of component description xml file.
   void readSchematic(Qucs::XmlReader *reader, const QString& svgPath, SvgPainter *svgPainter,
                      QSharedDataPointer<ComponentData> &d)
   {
      Q_ASSERT(reader->isStartElement() && reader->name() == "schematic");

      QString schName = reader->attributes().value("name").toString();
      QString schType = reader->attributes().value("href").toString();

      if(!schType.isEmpty()) {
         QFile svgFile(svgPath + "/" + schType);
         svgFile.open(QIODevice::ReadOnly | QIODevice::Text);

         QByteArray svgContent(svgFile.readAll());

         QString svgId = d.constData()->name + "/" + schName;
         svgPainter->registerSvg(svgId, svgContent);
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
               svgPainter->registerSvg(svgId, svgContent);
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
}
