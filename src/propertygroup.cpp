/***************************************************************************
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#include "global.h"
#include "propertydisplay.h"
#include "xmlutilities.h"

#include <QDebug>

namespace Caneda
{
    //! \brief Construct PropertyGroup from given QMap.
    PropertyGroup::PropertyGroup(PropertyMap propertyMap) :
        m_propertyMap(propertyMap), m_propertyDisplay(0)
    {
    }

    //! \brief Destructor.
    PropertyGroup::~PropertyGroup()
    {
        delete m_propertyDisplay;
    }

    //! \brief Adds new property to PropertyMap.
    void PropertyGroup::addProperty(const QString& key, const Property &prop)
    {
        m_propertyMap.insert(key, prop);
        updatePropertyDisplay();  // This is neccessary to update the properties display on a scene
    }

    //! \brief Sets \a key property to \a value in the PropertyMap.
    void PropertyGroup::setProperty(const QString& key, const QString& value)
    {
        m_propertyMap[key].setValue(value);
        updatePropertyDisplay();  // This is neccessary to update the properties display on a scene
    }

    /*!
     * \brief Set the component's properties' values.
     *
     * This method sets the properties' values by updating the propertyMap
     * to \a propMap.
     *
     * After setting the propertyMap, this method takes care of updating
     * the PropertyDisplay (which is the class that displays properties on
     * a scene).
     *
     * \param propMap The new property map to be set.
     *
     * \sa Property, PropertyMap, updatePropertyDisplay(), PropertyDisplay
     */
    void PropertyGroup::setPropertyMap(const PropertyMap& propMap)
    {
        m_propertyMap = propMap;
        updatePropertyDisplay();  // This is neccessary to update the properties display on a scene
    }

    /*!
     * \brief Updates properties' display on a scene (schematic).
     *
     * This method updates properties' display on a scene. It also takes
     * care of creating a new PropertyDisplay if it didn't exist before
     * and deletes it if none of the properties are visible.
     *
     * \sa PropertyDisplay, PropertyDisplay::updateProperties()
     */
    void PropertyGroup::updatePropertyDisplay()
    {
        bool itemsVisible = false;

        // Determine if any item is visible.
        PropertyMap::const_iterator it = m_propertyMap.constBegin(),
            end = m_propertyMap.constEnd();
        while(it != end) {
            if(it->isVisible()) {
                itemsVisible = true;
                break;
            }
            ++it;
        }

        // Delete the group if none of the properties are visible.
        if(!itemsVisible) {
            delete m_propertyDisplay;
            m_propertyDisplay = 0;
            return;
        }

        // If m_propertyDisplay = 0 create a new PopertyDisplay, else
        // just update it calling PopertyDisplay::updateProperties()
        if(!m_propertyDisplay) {
            m_propertyDisplay = new PropertyDisplay();
//            m_propertyDisplay->setParentItem(this);
//            m_propertyDisplay->setTransform(transform().inverted());
        }

        m_propertyDisplay->updateProperties();
    }

    //! \brief Helper method to write all properties in \a m_propertyMap to xml.
    void PropertyGroup::writeProperties(Caneda::XmlWriter *writer)
    {
        writer->writeStartElement("properties");
        foreach(const Property p, m_propertyMap) {
            writer->writeEmptyElement("property");
            writer->writeAttribute("name", p.name());
            writer->writeAttribute("value", p.value());
            writer->writeAttribute("visible", Caneda::boolToString(p.isVisible()));
        }
        writer->writeEndElement(); // </properties>
    }

    //! \brief Helper method to read xml saved properties into \a m_propertyMap.
    void PropertyGroup::readProperties(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "properties");

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    QXmlStreamAttributes attribs(reader->attributes());
                    QString propName = attribs.value("name").toString();
                    if(!m_propertyMap.contains(propName)) {
                        qWarning() << "readProperties() : " << "Property " << propName
                                   << "not found in map!";
                    }
                    else {
                        Property &prop = m_propertyMap[propName];
                        prop.setValue(attribs.value("value").toString());
                        prop.setVisible(attribs.value("visible") == "true");
                    }
                    // Read till end element
                    reader->readUnknownElement();
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }
    }

    //! \brief Helper function to write all properties in \a propMap in xml.
    void writeProperties(Caneda::XmlWriter *writer, const PropertyMap& propMap)
    {
        writer->writeStartElement("properties");
        foreach(const Property p, propMap) {
            writer->writeEmptyElement("property");
            writer->writeAttribute("name", p.name());
            writer->writeAttribute("value", p.value());
            writer->writeAttribute("visible", Caneda::boolToString(p.isVisible()));
        }
        writer->writeEndElement(); // </properties>
    }

    //! \brief Helper function to read the saved properties into \a propMap.
    void readProperties(Caneda::XmlReader *reader, PropertyMap &propMap)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "properties");

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    QXmlStreamAttributes attribs(reader->attributes());
                    QString propName = attribs.value("name").toString();
                    if(!propMap.contains(propName)) {
                        qWarning() << "readProperties() : " << "Property " << propName
                                   << "not found in map!";
                    }
                    else {
                        Property &prop = propMap[propName];
                        prop.setValue(attribs.value("value").toString());
                        prop.setVisible(attribs.value("visible") == "true");
                    }
                    // Read till end element
                    reader->readUnknownElement();
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }
    }

} // namespace Caneda
