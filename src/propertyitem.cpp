/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "propertyitem.h"

#include "component.h"
#include "settings.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param propName The name of property.
     */
    PropertyItem::PropertyItem(const QString &propName) :
        m_propertyName(propName)
    {
        // This is necessary to allow correct position update
        setTextInteractionFlags(Qt::TextEditorInteraction);
    }

    /*!
     * \brief Updates visual display of property value.
     *
     * \note This method is key method to alter the visual text of property. Call
     * it wherever the property changes.
     */
    void PropertyItem::updateValue()
    {
        Component* component = static_cast<PropertiesGroup*>(group())->component();
        if(!component) {
            qDebug() << "PropertyItem::updateValue() : Component is null!";
            return;
        }

        QString newValue;

        // Add property name
        if(m_propertyName.startsWith("label", Qt::CaseInsensitive)) {
            newValue = "";  // Label is displayed without "label" tag
        }
        else {
            newValue = m_propertyName + " = ";
        }

        // Add property value
        newValue.append(component->property(m_propertyName));

        setPlainText(newValue);
    }

    /*!
     * \brief Draws the propertyItem to painter.
     *
     * This method draws the propertyItem on a scene. The pen color changes
     * according to the selection state, thus giving state feedback to the
     * user.
     *
     * The selection rectangle around all propertyItems is handled by the
     * propertyGroup::paint method. An empty method in propertyGroup::paint
     * will avoid drawing a selection rectangle around property items in the
     * scene.
     *
     * \sa PropertiesGroup::paint()
     */
    void PropertyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
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
        painter->drawText(boundingRect(), toPlainText());

        // Restore pen
        painter->setPen(savedPen);
    }

} // namespace Caneda
