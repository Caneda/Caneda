/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2015 by Pablo Daniel Pareja Obregon                  *
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

#include "settings.h"
#include "wire.h"

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a Port item with a GraphicsItem as \a parent and
     * port's name \a portName.
     */
    Port::Port(GraphicsItem *parent) : QGraphicsItem(parent)
    {
        // Set component flags
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        m_name = QString();
        m_connections.append(this);
    }

    //! \brief Destroys the port object, removing all connections from the item
    Port::~Port()
    {
        disconnect();
    }

    /*!
     *  \brief Returns a pointer to this item's parent item. If this item does
     *  not have a parent, 0 is returned.
     */
    GraphicsItem* Port::parentItem() const
    {
        GraphicsItem *item = canedaitem_cast<GraphicsItem*>(QGraphicsItem::parentItem());
        if(item) {
            return item;
        }
        return 0;
    }

    /*!
     *  \brief Returns the list of connected ports including those connected by
     *  wires
     *
     *  Returns the list of equipotential connected ports, that is all
     *  connected ports including those connected by wires. This conforms the
     *  net or node in the electrical sense. This method is to be used during
     *  netlist creation to determine each unique net and group all components
     *  connected ports under only one name.
     *
     *  This method works in a recursive way, filling a list with the port
     *  direct connections (contained in m_connections) and searching for
     *  the connections of those ports connected to this one by a wire.
     *
     *  \param connectedPorts List to fill with the connections of this port.
     *
     *  \sa connections()
     */
    void Port::getEquipotentialPorts(QList<Port*> &connectedPorts)
    {
        if(connectedPorts.contains(this)) {
            return;
        }

        connectedPorts << m_connections;

        foreach(Port *port, connectedPorts) {
            if(port->parentItem()->type() == GraphicsItem::WireType) {
                Wire *_wire = static_cast<Wire*>(port->parentItem());
                _wire->port1()->getEquipotentialPorts(connectedPorts);
                _wire->port2()->getEquipotentialPorts(connectedPorts);
            }
        }
    }

    //! \brief Connect this port to \a other.
    void Port::connectTo(Port *other)
    {
        if(this == other || !other) {
            qWarning() << "Cannot connect to itself or null port";
            return;
        }

        if(parentItem() == other->parentItem()) {
            qWarning() << "Cannot connect ports of same component/wire";
            return;
        }

        if(!scene() || !other->scene() || scene() != other->scene()) {
            qWarning() << "Cannot connect ports across different or null scenes";
            return;
        }

        QPointF p1 = scenePos();
        QPointF p2 = other->scenePos();

        if(p1 != p2) {
            qWarning() << "Cannot connect ports as positions mismatch" << p1 << p2;
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
            foreach(Port *p, m_connections) {
                p->m_connections = m_connections;
            }
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

    //! \brief Check if port \a other is connected to this port.
    bool Port::isConnectedTo(Port *other)
    {
        bool retVal = m_connections == other->m_connections &&
            m_connections.contains(other);
        return retVal;
    }

    //! \brief Returns true if this port is connected to any other port
    bool Port::hasAnyConnection() const
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

        foreach(QGraphicsItem *item, collisions) {
            if(canedaitem_cast<GraphicsItem*>(item)) {

                QList<Port*> ports = canedaitem_cast<GraphicsItem*>(item)->ports();
                foreach(Port *p, ports) {
                    if(p->scenePos() == scenePos() &&
                            p->parentItem() != parentItem() &&
                            !m_connections.contains(p)) {
                        return p;
                    }
                }

            }
        }

        return 0;
    }

    /*!
     * \brief Draws the port based on the current connection status.
     *
     *  Ports are drawn only if:
     *    \li the port is not connected
     *    \li there are more than two connections to the port
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
            painter->drawEllipse(portEllipse);
        }
        else if(m_connections.size() > 2 && parentItem()->isSelected()) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
            painter->setBrush(QBrush(settings->currentValue("gui/selectionColor").value<QColor>()));
            painter->drawEllipse(portEllipse.adjusted(1,1,-1,-1));  // Adjust the ellipse to be just a little smaller than the open port
        }
        else if(m_connections.size() > 2) {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
            painter->setBrush(QBrush(settings->currentValue("gui/lineColor").value<QColor>()));
            painter->drawEllipse(portEllipse.adjusted(1,1,-1,-1));  // Adjust the ellipse to be just a little smaller than the open port
        }

        // Restore pen
        painter->setPen(savedPen);

    }

} // namespace Caneda
