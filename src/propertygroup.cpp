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
#include "propertyitem.h"
#include "settings.h"

#include <QApplication>
#include <QDebug>
#include <QFontMetricsF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * Constructor
     * \param propMap A reference to PropertyMap of a component.
     * \param scene The graphics scene to which this property should belong.
     */
    PropertiesGroup::PropertiesGroup(CGraphicsScene *scene) :
        m_pointSize(qApp->font().pointSize())
    {
        if(scene) {
            scene->addItem(this);
        }
    }

    Component* PropertiesGroup::component() const
    {
        return static_cast<Component*>(parentItem());
    }

    void PropertiesGroup::setFontSize(int pointSize)
    {
        if(m_pointSize == pointSize) {
            return;
        }
        qDeleteAll(m_propertyItemsMap.values());
        m_propertyItemsMap.clear();
        m_pointSize = pointSize;
        realignItems();
    }

    //! \brief Draws the propertiesGroup bounding rect to painter.
    void PropertiesGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        // Draw bounding rect
        QGraphicsItemGroup::paint(painter, option, widget);

        // Restore pen
        painter->setPen(savedPen);
    }

    /*!
     * \brief Realigns all the child items of this group (deletes hidden items).
     * This is quite expensive. It removes all property items
     * from the group and then freshly adds them to the group again if it is
     * visible. If the property is hidden, then the corresponding property items
     * are removed from group and deleted. This also updates the visible value of
     * property.
     */
    void PropertiesGroup::realignItems()
    {
        //Nothing to do if scene doesn't exist.
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

        //Remove items from group and make them top level items.
        foreach(PropertyItem *item, m_propertyItemsMap) {
            item->updateValue();
            removeFromGroup(item);
            item->setParentItem(0);
        }
        int visibleItemsCount = 0;

        foreach(const Property property, component()->propertyMap()) {
            if(property.isVisible()) {
                bool newlyCreated = false;
                //Create new property item if it doesn't exist.
                if(!m_propertyItemsMap.contains(property.name())) {
                    PropertyItem *item = new PropertyItem(property.name(),
                            cGraphicsScene());
                    QFont font = item->font();
                    if(font.pointSize() != m_pointSize) {
                        font.setPointSize(m_pointSize);
                        item->setFont(font);
                    }

                    m_propertyItemsMap.insert(property.name(), item);
                    newlyCreated = true;

                }

                PropertyItem *item = m_propertyItemsMap[property.name()];
                visibleItemsCount++;

                QList<QGraphicsItem*> _children = QGraphicsItemGroup::children();
                if(!_children.isEmpty()) {

                    //Place the new item at bottom, properly aligned with group.
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
            //Disables moving , selection and focussing of empty groups.
            setFlags(0);
            setFlag(ItemSendsGeometryChanges, true);
            setFlag(ItemSendsScenePositionChanges, true);
        }
        forceUpdate();
    }

    //! \brief Forces the updation of the geometry of the property group.
    void PropertiesGroup::forceUpdate()
    {
        static const QLineF line(-5, -5, 5, 5);
        //HACK:This adds and removes a dummy item from group to update its geometry
        QGraphicsLineItem *dummy = new QGraphicsLineItem(line);
        addToGroup(dummy);
        removeFromGroup(dummy);
        delete dummy;
    }

    //! \brief Returns the graphics scene associated.
    CGraphicsScene* PropertiesGroup::cGraphicsScene() const
    {
        return qobject_cast<CGraphicsScene*>(scene());
    }

    /*!
     * \brief Deselect other items on mouse click.
     * Deselects selected item and also clears focus of childeren items
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

    void PropertiesGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        component()->launchPropertyDialog(Caneda::PushUndoCmd);
    }

} // namespace Caneda
