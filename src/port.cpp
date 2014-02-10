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
        QGraphicsItem(parent),
        d(new PortData(pos, portName)),
        m_connections(0)
    {
    }

    //! \brief Destroys the port object, removing all connections from the item
    Port::~Port()
    {
        if(m_connections) {
            Port *other = 0;
            foreach(Port *p, *m_connections) {
                if(p != this) {
                    other = p;
                    break;
                }
            }
            //! \bug GPK: this test is strange
            if(other) {
                disconnectFrom(other);
            }
            else {
                Q_ASSERT(m_connections->size() <= 1);
                delete m_connections;
            }
        }
    }

    //! \brief Returns position mapped to scene.
    QPointF Port::scenePos() const
    {
        return parentItem()->mapToScene(d->pos);
    }

    //! \brief Set a new port position
    void Port::setPos(const QPointF& newPos)
    {
        d->pos = newPos;
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

    //! \brief Shorhand for Port::connect(this, other)
    void Port::connectTo(Port *other)
    {
        Port::connect(this, other);
    }

    //! \brief Connect the ports \a port1 and \a port2.
    void Port::connect(Port *port1, Port *port2)
    {
        if(port1 == port2 || !port1 || !port2) {
            return;
        }

        if(port1->parentItem() == port2->parentItem()) {
            qWarning() << "Cannot connect nodes of same component/wire";
            return;
        }

        if(!port1->scene() || !port2->scene() ||
                port1->scene() != port2->scene()) {
            qWarning() << "Cannot connect nodes across different or null scenes";
            return;
        }

        QPointF p1 = port1->scenePos();
        QPointF p2 = port2->scenePos();

        if(p1 != p2) {
            qWarning() << "Cannot connect nodes as positions mismatch" << p1 << p2;
            return;
        }

        // Create new connection list if both the ports are not at all connected.
        if(!port1->m_connections && !port2->m_connections) {
            port1->m_connections = port2->m_connections = new QList<Port*>;
            *(port1->m_connections) << port1 << port2;
        }
        // Use port2->m_connections if port1->m_connections is null
        else if(!port1->m_connections) {
            port1->m_connections = port2->m_connections;
            //Q_ASSERT(!m_connections->contains(m_port1));
            *(port1->m_connections) << port1;
        }
        // Use port1->m_connections if port2->m_connections is null
        else if(!port2->m_connections) {
            port2->m_connections = port1->m_connections;
            *(port2->m_connections) << port2;
        }
        // else both the m_connections exist.
        else {
            // The connections are same indicates they are already connected.
            if(port1->m_connections == port2->m_connections) {
                Q_ASSERT(port1->m_connections->contains(port1));
                Q_ASSERT(port1->m_connections->contains(port2));
                qWarning() << "Port::connect() : The ports are already connected";
            }
            // else use the biggest list to hold all others..
            else if(port1->m_connections->size() >= port2->m_connections->size()) {
                *(port1->m_connections) += *(port2->m_connections);
                QList<Port*> *save = port2->m_connections;
                foreach(Port *p, *(port2->m_connections)) {
                    p->m_connections = port1->m_connections;
                }
                delete save;
            }
            else {
                *(port2->m_connections) += *(port1->m_connections);
                QList<Port*> *save = port1->m_connections;
                foreach(Port *p, *(port1->m_connections)) {
                    p->m_connections = port2->m_connections;
                }
                delete save;
            }
        }

        // Update all ports owner.
        foreach(Port *p, *(port1->m_connections)) {
            p->parentItem()->update();
        }
    }

    Port* Port::getAnyConnectedPort()
    {
        if(!m_connections) {
            return 0;
        }

        if((*m_connections).size() <= 1) {
            qDebug() << "Connections size <= 1 detected. Might be a bug";
            delete m_connections;
            m_connections = 0;
            return 0;
        }

        Port *other = 0;
        foreach(Port *port, *m_connections) {
            if(port != this) {
                other = port;
                break;
            }
        }
        Q_ASSERT(other);
        return other;
    }

    void Port::removeConnections()
    {
        Port *other = getAnyConnectedPort();
        disconnectFrom(other);
    }

    /*!
     * \brief Disconnect two ports
     *
     * \param port The port to be disconnected.
     * \param from The port from which \a port will be disconnected.
     * \note port == from , port == NULL, from == NULL are allowed
     */
    void Port::disconnect(Port *port, Port *from)
    {
        if(port == from || !port || !from) {
            return;
        }
        if(port->m_connections != from->m_connections || !port->m_connections) {
            qWarning() << "Cannot disconnect already disconnected ports or null list";
            return;
        }
        //Initially remove 'port' from the list.
        port->m_connections->removeAll(port);
        port->m_connections = 0;
        if(from->m_connections->size() <= 1) {
            if(from->m_connections->size() == 1) {
                Q_ASSERT(from->m_connections->first() == from);
            }
            delete from->m_connections;
            from->m_connections = 0;
        }
        port->parentItem()->update();
        from->parentItem()->update();
        if(from->m_connections) {
            foreach(Port *p, *(from->m_connections)) {
                p->parentItem()->update();
            }
        }
    }

    //! \brief Check whether two ports are connected or not.
    bool Port::isConnected(Port *port1, Port *port2)
    {
        bool retVal = port1->m_connections == port2->m_connections &&
            port1->m_connections != 0 && port1->m_connections->contains(port1) &&
            port1->m_connections->contains(port2);
        if(retVal) {
            bool ok1, ok2;
            Q_ASSERT(port1->scenePos(&ok1) == port2->scenePos(&ok2));
            Q_ASSERT(ok1 && ok2);
        }
        return retVal;
    }

    //! \brief Returns true if this port is connected to any other port
    bool Port::hasConnection() const
    {
        bool retVal = (m_connections != 0);
        if(retVal) {
            Q_ASSERT(m_connections->size() > 1);
        }
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
                if(p->scenePos() == scenePos() && p->parentItem() != parentItem() &&
                        (!m_connections || !m_connections->contains(p))) {
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
        if(m_connections == NULL) {
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
        else if(m_connections->size() > 2) {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
            painter->setBrush(QBrush(settings->currentValue("gui/lineColor").value<QColor>()));
            painter->drawEllipse(portEllipse.translated(pos()).adjusted(1,1,-1,-1));  // Adjust the ellipse to be just a little smaller than the open port
        }

        // Restore pen
        painter->setPen(savedPen);

    }

} // namespace Caneda
