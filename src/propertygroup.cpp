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

#include "cgraphicsscene.h"
#include "component.h"
#include "global.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Construct PropertyGroup from given scene and PropertyMap.
     *
     * \param scene The graphics scene to which this property should belong.
     */
    PropertyGroup::PropertyGroup(CGraphicsScene *scene)
    {
        m_propertyMap = PropertyMap();

        if(scene) {
            scene->addItem(this);
        }
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
     * \brief Set all properties values through a PropertyMap.
     *
     * This method sets the properties values by updating the propertyMap
     * to \a propMap.
     *
     * After setting the propertyMap, this method takes care of updating
     * the properties display on a scene.
     *
     * \param propMap The new property map to be set.
     *
     * \sa Property, PropertyMap, updatePropertyDisplay()
     */
    void PropertyGroup::setPropertyMap(const PropertyMap& propMap)
    {
        m_propertyMap = propMap;
        updatePropertyDisplay();  // This is neccessary to update the properties display on a scene
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
        PropertyMap::const_iterator it = m_propertyMap.constBegin(),
            end = m_propertyMap.constEnd();
        while(it != end) {
            if(it->isVisible()) {
                itemsVisible = true;
                break;
            }
            ++it;
        }

        // Hide the display if none of the properties are visible.
        if(!itemsVisible) {
            hide();
            return;
        }

        // Update parent item and transform
//        setParentItem(this->parent());
        setTransform(transform().inverted());

        // If created for the first time (boundingRect == null), set text position
        if(boundingRect().isNull()) {
            setPos(parentItem()->boundingRect().bottomLeft());
        }

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

        if(text().isEmpty()) {
            // Disables moving, selection and focussing of empty PropertyGroups
            setFlags(0);
            setFlag(ItemSendsGeometryChanges, true);
            setFlag(ItemSendsScenePositionChanges, true);
        }
        else {
            setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
            setFlag(ItemSendsGeometryChanges, true);
            setFlag(ItemSendsScenePositionChanges, true);
        }

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
        Component *comp = static_cast<Component*>(parentItem());
        comp->launchPropertyDialog(Caneda::PushUndoCmd);
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
