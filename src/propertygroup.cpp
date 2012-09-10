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

#include "propertygroup.h"

#include "cgraphicsscene.h"
#include "component.h"
#include "settings.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    //*************************************************************
    //********************* PropertyItem **************************
    //*************************************************************

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

    //*************************************************************
    //******************** PropertiesGroup ************************
    //*************************************************************

    /*!
     * Constructor
     *
     * \param scene The graphics scene to which this property should belong.
     */
    PropertiesGroup::PropertiesGroup(CGraphicsScene *scene)
    {
        if(scene) {
            scene->addItem(this);
        }
    }

    /*!
     * \brief Realigns all the child items of this group (and deletes hidden items).
     *
     * This is quite expensive. It removes all property items
     * from the group and then freshly adds them to the group again if it is
     * visible. If the property is hidden, then the corresponding property items
     * are removed from group and deleted. This also updates the visible value of
     * property.
     */
    void PropertiesGroup::realignItems()
    {
        // Nothing to do if scene doesn't exist.
        if(!scene()) {
            return;
        }

        QPointF savePos;
        if(boundingRect().isNull()) {
            savePos = parentItem()->sceneBoundingRect().bottomLeft();
        }
        else {
            savePos = sceneBoundingRect().topLeft();
        }

        // Remove items from group and make them top level items.
        foreach(PropertyItem *item, m_propertyItemsMap) {
            item->updateValue();
            removeFromGroup(item);
            item->setParentItem(0);
        }

        int visibleItemsCount = 0;

        foreach(const Property property, component()->propertyMap()) {
            if(property.isVisible()) {

                bool newlyCreated = false;
                // Create new property item if it doesn't exist.
                if(!m_propertyItemsMap.contains(property.name())) {
                    PropertyItem *item = new PropertyItem(property.name());

                    m_propertyItemsMap.insert(property.name(), item);
                    newlyCreated = true;
                }

                PropertyItem *item = m_propertyItemsMap[property.name()];
                visibleItemsCount++;

                QList<QGraphicsItem*> _children = QGraphicsItemGroup::children();
                if(!_children.isEmpty()) {
                    // Place the new item at bottom, properly aligned with group.
                    QPointF itemPos = mapToScene(boundingRect().bottomLeft());
                    itemPos.rx() -= item->boundingRect().left();
                    item->setPos(itemPos);
                }
                else {
                    savePos.rx() -= item->boundingRect().left();
                    item->setPos(savePos);
                }

                addToGroup(item);
                if(newlyCreated) {
                    item->updateValue();
                }

            }
            else {
                // Delete item if it existed before as it is being hidden now.
                if(m_propertyItemsMap.contains(property.name())) {
                    PropertyItem *item = m_propertyItemsMap[property.name()];
                    m_propertyItemsMap.remove(property.name());
                    delete item;
                }
            }

        }

        if(visibleItemsCount > 0) {
            setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
            setFlag(ItemSendsGeometryChanges, true);
            setFlag(ItemSendsScenePositionChanges, true);
        }
        else {
            // Disables moving, selection and focussing of empty groups.
            setFlags(0);
            setFlag(ItemSendsGeometryChanges, true);
            setFlag(ItemSendsScenePositionChanges, true);
        }

        updateGeometry();
    }

    //! \brief Forces the update of the geometry of the property group.
    void PropertiesGroup::updateGeometry()
    {
        // HACK: This adds and removes a dummy item from group to update its geometry
        static const QLineF line(-5, -5, 5, 5);
        QGraphicsLineItem *dummy = new QGraphicsLineItem(line);
        addToGroup(dummy);
        removeFromGroup(dummy);
        delete dummy;
    }

    //! \brief Returns the associated component.
    Component* PropertiesGroup::component() const
    {
        return static_cast<Component*>(parentItem());
    }

    /*!
     * \brief Draws the propertiesGroup bounding rect to painter.
     *
     * This method is empty to avoid drawing a selection rectangle around
     * property items in the scene. The selection state is instead
     * handled by PropertyItem::paint method, changing the properties'
     * color according to the global selection pen.
     *
     * \sa PropertyItem::paint()
     */
    void PropertiesGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
    {
    }

    /*!
     * \brief Deselect other items on mouse click.
     *
     * Deselects selected items other than this, and also clears focus of children items
     */
    void PropertiesGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(scene()) {
            foreach(QGraphicsItem *item, scene()->selectedItems()) {
                if(item != this) {
                    item->setSelected(false);
                }
            }
        }

        foreach(PropertyItem *p, m_propertyItemsMap) {
            if(p->hasFocus()) {
                p->clearFocus();
            }
        }

        QGraphicsItemGroup::mousePressEvent(event);
    }

    //! \brief Launches property dialog on double click.
    void PropertiesGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        component()->launchPropertyDialog(Caneda::PushUndoCmd);
    }

} // namespace Caneda
