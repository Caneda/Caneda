/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2014 by Pablo Daniel Pareja Obregon                  *
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

#include "property.h"

#include "global.h"
#include "graphicsscene.h"
#include "propertydialog.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*************************************************************************
     *                            PropertyData                               *
     *************************************************************************/
    //! \brief Constructor.
    PropertyData::PropertyData()
    {
        name = QString();
        value = QString();
        description = QString();
        visible = false;
    }

    //! \brief Copy constructor
    PropertyData::PropertyData(const PropertyData& p) : QSharedData(p)
    {
        name = p.name;
        value = p.value;
        description = p.description;
        visible = p.visible;
    }


    /*************************************************************************
     *                              Property                                 *
     *************************************************************************/
    /*!
     * \brief Constructs a property object
     *
     * \param _name Name of property object.
     * \param _value The default value of property.
     * \param _description Description of property.
     * \param _visible Visibility of property object(item)
     */
    Property::Property(const QString& _name,
                       const QString& _value,
                       const QString& _description,
                       bool _visible)
    {
        d = new PropertyData;
        d->name = _name;
        d->value = _value;
        d->description = _description;
        d->visible = _visible;
    }

    //! \brief Construct property from shared data.
    Property::Property(QSharedDataPointer<PropertyData> data) : d(data)
    {
    }

    /*!
     * \brief Method used to create a property from an xml file.
     *
     * \param reader XmlReader which is in use for parsing.
     */
    Property Property::loadProperty(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "property");
        QSharedDataPointer<PropertyData> data(new PropertyData);

        QXmlStreamAttributes attributes = reader->attributes();

        data->name = attributes.value("name").toString();
        if(data->name.isEmpty()) {
            reader->raiseError("Couldn't find name attribute in property description");
            return Property();
        }

        QString visible = attributes.value("visible").toString();
        data->visible = !(visible.isEmpty() || visible == "false");

        data->value = attributes.value("default").toString();

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "description") {
                    data->description = reader->readLocaleText(Caneda::localePrefix());
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }

        return Property(data);
    }

    /*!
     * \brief Method used to save a property to an xml file.
     *
     * \param reader XmlReader which is used for writing.
     */
    void Property::saveProperty(Caneda::XmlWriter *writer)
    {
        writer->writeStartElement("property");

        writer->writeAttribute("name", name());
        writer->writeAttribute("default", value());
        writer->writeAttribute("unit", "-");

        if(isVisible()) {
            writer->writeAttribute("visible", "true");
        }
        else {
            writer->writeAttribute("visible", "false");
        }

        writer->writeStartElement("description");
        writer->writeLocaleText(Caneda::localePrefix(), description());
        writer->writeEndElement(); // </description>

        writer->writeEndElement(); // </property>
    }


    /*************************************************************************
     *                            PropertyGroup                              *
     *************************************************************************/
    /*!
     * \brief Constructs a PropertyGroup from a given scene and PropertyMap.
     *
     * \param scene The graphics scene to which this property should belong.
     * \param propMap The PropertyMap to use on initialization.
     */
    PropertyGroup::PropertyGroup(GraphicsScene *scene, const PropertyMap &propMap)
    {
        m_propertyMap = propMap;
        m_userPropertiesEnabled = false;

        if(scene) {
            scene->addItem(this);
        }

        // Set items flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);
    }

    //! \brief Adds a new property to the PropertyMap.
    void PropertyGroup::addProperty(const QString& key, const Property &prop)
    {
        m_propertyMap.insert(key, prop);
        updatePropertyDisplay();  // This is necessary to update the properties display on a scene
    }

    //! \brief Sets property \a key to \a value in the PropertyMap.
    void PropertyGroup::setPropertyValue(const QString& key, const QString& value)
    {
        if(m_propertyMap.contains(key)) {
            m_propertyMap[key].setValue(value);
            updatePropertyDisplay();  // This is necessary to update the properties display on a scene
        }
    }

    /*!
     * \brief Set all the properties values through a PropertyMap.
     *
     * This method sets the properties values by updating the propertyMap
     * to \a propMap. After setting the propertyMap, this method also takes
     * care of updating the properties display on the scene.
     *
     * \param propMap The new property map to be set.
     *
     * \sa Property, PropertyMap, updatePropertyDisplay()
     */
    void PropertyGroup::setPropertyMap(const PropertyMap& propMap)
    {
        m_propertyMap = propMap;
        updatePropertyDisplay();  // This is necessary to update the properties display on a scene
    }

    /*!
     * \brief Sets the userPropertiesEnabled status.
     *
     * This method sets if the user is enabled to add or remove properties.
     * This is acomplished by telling the property dialog if the user should be
     * enabled to add or remove properties, and if the existing properties
     * should be allowed to change its key (property name)
     *
     * \sa m_userPropertiesEnabled, \sa userPropertiesEnabled()
     */
    void PropertyGroup::setUserPropertiesEnabled(const bool enable)
    {
        m_userPropertiesEnabled = enable;
    }

    /*!
     * \brief Updates the visual display of all the properties in the PropertyGroup.
     *
     * This method is key to alter the visual display text of given properties. It
     * should be called wherever a property changes.
     *
     * To update the visual display, it recreates all individual properties display
     * from the group and then adds them to the plaintext property of this item (if
     * the given property is visible). This method also updates the visible value of
     * the property.
     */
    void PropertyGroup::updatePropertyDisplay()
    {
        bool itemsVisible = false;

        // Determine if any item is visible.
        foreach(const Property property, m_propertyMap) {
            if(property.isVisible()) {
                 itemsVisible = true;
                 break;
            }
        }

        // Hide the display if none of the properties are visible.
        if(!itemsVisible) {
            hide();
            return;
        }

        // Else update its value, and make visible (show()).
        QString newValue;  // New value to set

        // Iterate through all properties to add its values
        foreach(const Property property, m_propertyMap) {
            if(property.isVisible()) {

                QString propertyText = "";  // Current property text

                // Add property name (except for the label property)
                if(!property.name().startsWith("label", Qt::CaseInsensitive)) {
                    propertyText = property.name() + " = ";
                }

                // Add property value
                propertyText.append(property.value());

                // Add the property to the group
                if(!newValue.isEmpty()) {
                    newValue.append("\n");  // If already has properties, add newline
                }
                newValue.append(propertyText);
            }
        }

        // Set new properties values
        setText(newValue);

        // Make item visible
        show();
    }

    /*!
     * \brief Draws the PropertyGroup to painter.
     *
     * This method draws the PropertyGroup contents on a scene. The pen color
     * changes according to the selection state, thus giving state feedback to
     * the user.
     *
     * The selection rectangle around all PropertyGroup contents is handled by
     * this method. Currently, no selection rectangle around property items is
     * drawn, although it could change in the future (acording to user's feedback).
     * In that case, this class bounding rect should be used. The selection state
     * is instead handled by changing the properties' pen color according to the
     * global selection pen.
     */
    void PropertyGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(isSelected()) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/foregroundColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        // Paint the property text
        painter->drawText(boundingRect(), text());

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \brief Helper method to write all properties in \a m_propertyMap to xml.
    void PropertyGroup::writeProperties(Caneda::XmlWriter *writer)
    {
        writer->writeStartElement("properties");
        writer->writePointAttribute(pos(), "pos");

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
        setPos(reader->readPointAttribute("pos"));

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

        updatePropertyDisplay();
    }

    //! \copydoc GraphicsItem::launchPropertiesDialog()
    int PropertyGroup::launchPropertiesDialog()
    {
        PropertyDialog *dia = new PropertyDialog(this);
        int status = dia->exec();
        delete dia;

        return status;
    }

    //! \brief On mouse click deselect selected items other than this.
    void PropertyGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(scene()) {
            foreach(QGraphicsItem *item, scene()->selectedItems()) {
                if(item != this) {
                    item->setSelected(false);
                }
            }
        }

        QGraphicsSimpleTextItem::mousePressEvent(event);
    }

    //! \brief Launches property dialog on double click.
    void PropertyGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        launchPropertiesDialog();
    }

} // namespace Caneda
