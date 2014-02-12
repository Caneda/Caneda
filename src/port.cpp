/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2014 by Pablo Daniel Pareja Obregon                  *
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

#include "port.h"

#include "cgraphicsitem.h"
#include "component.h"
#include "portsymbol.h"
#include "settings.h"
#include "wire.h"

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a Port item with a CGraphicsItem as \a parent, position
     * \a pos and port's name \a portName.
     */
    Port::Port(CGraphicsItem* parent, QPointF pos, QString portName) :
        QGraphicsItem(parent)
    {
        // Set component flags
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        d = new PortData(pos, portName);
        m_connections.append(this);
    }

    //! \brief Destroys the port object, removing all connections from the item
    Port::~Port()
    {
        disconnect();
    }

    //! \brief Set a new port position
    void Port::setPos(const QPointF& newPos)
    {
        d->pos = newPos;
    }

    //! \brief Returns position mapped to scene.
    QPointF Port::scenePos() const
    {
        return parentItem()->mapToScene(d->pos);
    }

    /*!
     *  \brief Returns a pointer to this item's parent item. If this item does
     *  not have a parent, 0 is returned.
     */
    CGraphicsItem* Port::parentItem() const
    {
        CGraphicsItem *item = canedaitem_cast<CGraphicsItem*>(QGraphicsItem::parentItem());
        if(item) {
            return item;
        }
        return 0;
    }

    //! \brief Connect this port to \a other.
    void Port::connectTo(Port *other)
    {
        if(this == other || !other) {
            qWarning() << "Cannot connect to itself or null port";
            return;
        }

        if(parentItem() == other->parentItem()) {
            qWarning() << "Cannot connect nodes of same component/wire";
            return;
        }

        if(!scene() || !other->scene() || scene() != other->scene()) {
            qWarning() << "Cannot connect nodes across different or null scenes";
            return;
        }

        QPointF p1 = scenePos();
        QPointF p2 = other->scenePos();

        if(p1 != p2) {
            qWarning() << "Cannot connect nodes as positions mismatch" << p1 << p2;
            return;
        }

        // If the connections are same, they are already connected.
        if(m_connections == other->m_connections) {
            qWarning() << "Port::connectTo() : The ports are already connected";
        }
        else {
            // Create a list of connections by merging both lists of connections
            // and replicating the list in all connected ports.
            m_connections += other->m_connections;
            foreach(Port *p, other->m_connections) {
                p->m_connections = m_connections;
            }
            other->m_connections = m_connections;
        }

        // Update all ports parents.
        foreach(Port *p, m_connections) {
            p->parentItem()->update();
        }
    }

    /*!
     * \brief Disconnect a port
     *
     * A disconnect operation must remove this port from every other port's
     * list of connections (effectively disconnecting all ports currently
     * connected), thus avoiding false or erroneous connections to remain as
     * valid.
     */
    void Port::disconnect()
    {
        // Check if there is any connection
        if(m_connections.size() <= 1) {
            return;
        }

        // Disconnect this port from every connected port
        foreach(Port *p, m_connections) {
            p->m_connections.removeAll(this);
            p->parentItem()->update();
        }

        m_connections.clear();
        m_connections.append(this);

        // Update parent item.
        parentItem()->update();
    }

    //! \brief Check whether two ports are connected or not.
    bool Port::isConnected(Port *port1, Port *port2)
    {
        bool retVal = port1->m_connections == port2->m_connections &&
            port1->m_connections.contains(port2);
        return retVal;
    }

    //! \brief Returns true if this port is connected to any other port
    bool Port::hasConnection() const
    {
        bool retVal = (m_connections.size() > 1);
        return retVal;
    }

    //! \brief Finds a coinciding port on schematic.
    Port* Port::findCoincidingPort() const
    {
        if(!scene()) {
            return 0;
        }

        QList<QGraphicsItem*> collisions =
            parentItem()->collidingItems(Qt::IntersectsItemBoundingRect);

        QList<Port*> ports;
        foreach(QGraphicsItem *item, collisions) {
            if(canedaitem_cast<Component*>(item)) {
                ports = canedaitem_cast<Component*>(item)->ports();
            }
            else if(canedaitem_cast<Wire*>(item)) {
                ports = canedaitem_cast<Wire*>(item)->ports();
            }
            else if(canedaitem_cast<PortSymbol*>(item)) {
                ports = canedaitem_cast<PortSymbol*>(item)->ports();
            }
            else {
                continue;
            }

            foreach(Port *p, ports) {
                if(p->scenePos() == scenePos() &&
                        p->parentItem() != parentItem() &&
                        !m_connections.contains(p)) {
                    return p;
                }
            }
        }
        return 0;

    }

    /*!
     * \brief Draws the port based on the current connection status.
     *
     *  Port are drawn only if:
     *         - port is not connected
     *         - port they are more than two connection to port
     */
    void Port::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget*)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(m_connections.size() <= 1) {
            painter->setPen(QPen(Qt::darkRed));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(portEllipse.translated(pos()));
        }
        else if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
            painter->setBrush(QBrush(settings->currentValue("gui/selectionColor").value<QColor>()));
            painter->drawEllipse(portEllipse.translated(pos()).adjusted(1,1,-1,-1));  // Adjust the ellipse to be just a little smaller than the open port
        }
        else if(m_connections.size() > 2) {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
            painter->setBrush(QBrush(settings->currentValue("gui/lineColor").value<QColor>()));
            painter->drawEllipse(portEllipse.translated(pos()).adjusted(1,1,-1,-1));  // Adjust the ellipse to be just a little smaller than the open port
        }

        // Restore pen
        painter->setPen(savedPen);

    }

} // namespace Caneda
