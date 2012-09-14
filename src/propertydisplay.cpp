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

#include "propertydisplay.h"

#include "cgraphicsscene.h"
#include "component.h"
#include "settings.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * Constructor
     *
     * \param scene The graphics scene to which this property should belong.
     */
    PropertyDisplay::PropertyDisplay(CGraphicsScene *scene)
    {
        if(scene) {
            scene->addItem(this);
        }
    }

    /*!
     * \brief Updates the visual display of all the properties in a PropertyGroup.
     *
     * This method is key to alter the visual display text of given properties. It
     * should be called wherever a property changes.
     *
     * To update the visual display, it recreates all individual properties' display
     * from the group and then adds them to the plaintext property of this item (if
     * the given property is visible). This method also updates the visible value of
     * the property.
     */
    void PropertyDisplay::updateProperties()
    {
        if(!component()) {
            qDebug() << "PropertyDisplay::update() : Component is null!";
            return;
        }

        // If created for the first time (boundingRect == null), set text position
        if(boundingRect().isNull()) {
            setPos(parentItem()->boundingRect().bottomLeft());
        }

        QString newValue;  // New value to set

        // Iterate through all properties to add its values
        foreach(const Property property, component()->propertyMap()) {
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
            // Disables moving, selection and focussing of empty PropertyDisplays
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
     * \brief Draws the PropertyDisplay to painter.
     *
     * This method draws the PropertyDisplay contents on a scene. The pen color
     * changes according to the selection state, thus giving state feedback to
     * the user.
     *
     * The selection rectangle around all PropertyDisplay contents is handled by
     * this method. Currently, no selection rectangle around property items is
     * drawn, although it could change in the future (acording to user's feedback).
     * In that case, this class bounding rect should be used. The selection state
     * is instead handled by changing the properties' pen color according to the
     * global selection pen.
     */
    void PropertyDisplay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
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

    //! \brief Returns the associated component.
    Component* PropertyDisplay::component() const
    {
        return static_cast<Component*>(parentItem());
    }

    //! \brief On mouse click deselect selected items other than this.
    void PropertyDisplay::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
    void PropertyDisplay::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        component()->launchPropertyDialog(Caneda::PushUndoCmd);
    }

} // namespace Caneda
