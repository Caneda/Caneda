/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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
#include "cgraphicsscene.h"
#include "component.h"
#include "wire.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>

namespace Caneda
{
    /*************************************************************
     *
     *  PortOwner
     *
     *************************************************************/

    //! \brief Construct portowner with wire as owner.
    PortOwner::PortOwner(CGraphicsItem * item) : m_item(item)
    {
        Q_ASSERT(isWire() || isComponent());
    }

    /*!
     * \brief Return port owner as  QGraphicsItem
     * \todo check why is it needed
     */
    QGraphicsItem* PortOwner::item() const
    {
        QGraphicsItem *item = static_cast<QGraphicsItem*>(m_item);
        //Cast should always succeed as both are graphics items.
        Q_ASSERT(item);
        return item;
    }

    //! \brief Return the wire if stored, or null otherwise.
    Wire* PortOwner::wire() const
    {
        if(m_item->type() == CGraphicsItem::WireType) {
            return static_cast<Wire*>(m_item);
        }
        else {
            return NULL;
        }
    }

    //! \brief Return the wire if stored, or null otherwise.
    Component* PortOwner::component() const
    {
        if(m_item->type() == CGraphicsItem::ComponentType) {
            return static_cast<Component*>(m_item);
        }
        else {
            return NULL;
        }
    }

    /**************************************************************************
     *
     *  Port
     *
     ***************************************************************************/

    /*! Constructor
     *  \brief Construct port with canedaitem as owner and shared data \data.
     */
    Port::Port(CGraphicsItem *owner, const QSharedDataPointer<PortData> &data) :
        d(data),
        m_owner(new PortOwner(owner)),
        m_connections(0)
    {
    }

    /*! Constructor
      \brief Construct port with canedaitem as owner, position \pos and port's name \portName.
      */
    Port::Port(CGraphicsItem *owner, QPointF _pos, QString portName) :
        d(new PortData(_pos, portName)),
        m_owner(new PortOwner(owner)),
        m_connections(0)
    {
    }

    /*!
     * \brief Remove all connections from this node before destruction.
     * \bug GPK: see comment
     */
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
            /* GPK: this test is strange */
            if(other) {
                disconnectFrom(other);
            }
            else {
                Q_ASSERT(m_connections->size() <= 1);
                delete m_connections;
            }
        }
        //deletes the container only, not the actual component or wire.
        delete m_owner;
    }

    /*!
     * \brief Returns position mapped to scene.
     *
     * \param ok It is set to success state if non null.
     */
    QPointF Port::scenePos(bool *ok) const
    {
        if(ok) {
            *ok = m_owner->item()->scene() != 0;
        }
        return m_owner->item()->mapToScene(d->pos);
    }

    /*!
     * \brief This is private method needed only for wires as components do not
     * change relative port position.
     */
    void Port::setPos(const QPointF& newPos)
    {
        d->pos = newPos;
    }

    CGraphicsScene* Port::cGraphicsScene() const
    {
        if(owner()->wire()) {
            return owner()->wire()->cGraphicsScene();
        }
        else {
            return owner()->component()->cGraphicsScene();
        }
    }


    //! \brief Shorhand for Port::connect(this, other)
    void Port::connectTo(Port *other)
    {
        Port::connect(this, other);
    }

    //! \brief Connect the ports \a port1 and \a port2.
    void Port::connect(Port *port1, Port *port2)
    {
        bool ok1, ok2;

        if(port1 == port2 || !port1 || !port2) {
            return;
        }

        if(port1->ownerItem() == port2->ownerItem()) {
            qWarning() << "Cannot connect nodes of same component/wire";
            return;
        }

        QPointF p1 = port1->scenePos(&ok1);
        QPointF p2 = port2->scenePos(&ok2);

        if(!ok1 || !ok2 ||
                port1->ownerItem()->scene() != port2->ownerItem()->scene()) {
            qWarning() << "Cannot connect nodes across different or null scenes";
            return;
        }

        if(p1 != p2) {
            qWarning() << "Cannot connect nodes as positions mismatch" << p1 << p2;
            return;
        }

        //create new connection list if both the ports are not at all connected.
        if(!port1->m_connections && !port2->m_connections) {
            port1->m_connections = port2->m_connections = new QList<Port*>;
            *(port1->m_connections) << port1 << port2;
        }
        //use port2->m_connections if port1->m_connections is null
        else if(!port1->m_connections) {
            port1->m_connections = port2->m_connections;
            //Q_ASSERT(!m_connections->contains(m_port1));
            *(port1->m_connections) << port1;
        }
        //use port1->m_connections if port2->m_connections is null
        else if(!port2->m_connections) {
            port2->m_connections = port1->m_connections;
            *(port2->m_connections) << port2;
        }
        // else both the m_connections exist.
        else {
            // the connections are same indicates they are already connected.
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
            p->ownerItem()->update();
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
        port->ownerItem()->update();
        from->ownerItem()->update();
        if(from->m_connections) {
            foreach(Port *p, *(from->m_connections)) {
                p->ownerItem()->update();
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

    //! \brief Finds a coinciding port in a given list of ports.
    Port* Port::findCoincidingPort(const QList<Port*> &ports) const
    {
        foreach(Port *p, ports) {
            if(p->scenePos() == scenePos() && p->owner() != owner() &&
                    (!m_connections || !m_connections->contains(p))) {
                return p;
            }
        }
        return 0;
    }

    //! \brief Finds a coinciding port on schematic.
    Port* Port::findCoincidingPort() const
    {
        CGraphicsScene *scene =
            qobject_cast<CGraphicsScene*>(ownerItem()->scene());
        if(!scene) {
            return 0;
        }
        QList<QGraphicsItem*> collisions =
            ownerItem()->collidingItems(Qt::IntersectsItemBoundingRect);
        QList<Port*> ports;
        foreach(QGraphicsItem *item, collisions) {
            if(canedaitem_cast<Component*>(item)) {
                ports = canedaitem_cast<Component*>(item)->ports();
            }
            else if(canedaitem_cast<Wire*>(item)) {
                ports = canedaitem_cast<Wire*>(item)->ports();
            }
            else {
                continue;
            }

            Port *p = findCoincidingPort(ports);
            if(p) {
                return p;
            }
        }
        return 0;
    }

    //! \brief Returns true only if all the connected components are selected.
    bool Port::areAllOwnersSelected() const
    {
        if(!m_connections) {
            return ownerItem()->isSelected();
        }

        foreach(Port *p, *m_connections) {
            if(!p->ownerItem()->isSelected()) {
                return false;
            }
        }
        return true;
    }

    /*!
     * \brief Draws the port based on the current connection status.
     *
     *  Port are drawn only if:
     *         - port is not connected
     *         - port they are more than two connection to port
     */
    void Port::paint(QPainter *painter, const QStyleOptionGraphicsItem* option)
    {
        Q_UNUSED(option);

        /* save pen */
        QPen savedPen = painter->pen();

        if(m_connections == NULL) {
            painter->setPen(unconnectedPen);
            painter->setBrush(unconnectedBrush);
            painter->drawEllipse(portEllipse.translated(pos()));
        } else if(m_connections->size() > 2) {
            painter->setPen(connectedPen);
            painter->setBrush(connectedBrush);
            painter->drawEllipse(portEllipseConnected.translated(pos()));
        }

        /* restore pen */
        painter->setPen(savedPen);
    }

} // namespace Caneda
