/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2012 by Pablo Daniel Pareja Obregon                  *
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

#ifndef PORT_H
#define PORT_H

#include "cgraphicsitem.h"

#include <QList>
#include <QSharedData>

namespace Caneda
{
    // Forward declarations
    class Wire;
    class Component;
    class CGraphicsScene;

    // Style constants definitions
    static const qreal portRadius(3.0);
    static const QRectF portEllipse(-portRadius, -portRadius, 2*portRadius,
            2*portRadius);


    //! Thin class used to abstract owner of port.
    class PortOwner
    {
    public:
        PortOwner(CGraphicsItem * item);

        //! Return type of owner
        int type() const { return m_item->type(); }

        Wire* wire() const;
        //! Return the component if stored, or null otherwise.
        Component* component() const;
        //! Returns the owner item as graphicsitem.
        QGraphicsItem* item() const;

        //! Return weather item is a wire
        bool isWire() const { return m_item->isWire(); }
        //! Return weather item is a component
        bool isComponent() const { return m_item->isComponent(); }

    private:
        //! Owner of the port
        CGraphicsItem *const m_item;
        //! Disable copy
        PortOwner(const PortOwner& other);
    };

    //! Sharable port's data.
    struct PortData : public QSharedData
    {
        PortData(QPointF _pos, QString _name) : pos(_pos), name(_name) {}

        QPointF pos;
        QString name;
    };

    /*!
     * \brief The Port class is an electric port graphical representation, that
     * allows components to be connected together through the use of wires.
     *
     * This class always has a parent item (CGraphicsItem) and cannot be moved
     * on its own. The port position on a scene is determined from the parent's
     * position, moving along with it. A Port class has only one parent, but
     * can be connected to multiple ports, thus allowing interconnection of
     * electric components such as wires, pasive and active components, etc.
     *
     * A disconnected port (that only has its parent) is represented by a
     * hollow circle, while a connected port (with only one connection) is not
     * drawn. When multiple connections are made into one port, a filled circle
     * with the foreground color is drawn.
     *
     * \sa Component, Wire, Node
     */
    class Port
    {
    public:
        Port(CGraphicsItem  *owner, const QSharedDataPointer<PortData> &data);
        Port(CGraphicsItem  *owner, QPointF _pos, QString portName = QString());

        ~Port();

        //! Returns the position relative to owner - usually constant.
        QPointF pos() const { return d->pos; }

        QPointF scenePos(bool *ok = 0) const;

        //! Returns the port's name.
        QString name() const { return d->name; }

        //! Returns the owner.
        PortOwner* owner() const { return m_owner; }

        //! Shorthand method for owner->item()
        QGraphicsItem* ownerItem() const { return m_owner->item(); }

        //! Returns a pointer to list of connected ports (null if unconnected).
        QList<Port*> *connections() const { return m_connections; }

        CGraphicsScene* cGraphicsScene() const;

        static void connect(Port* port1, Port *port2);
        static void disconnect(Port *port, Port *from);
        void connectTo(Port *other);
        void disconnectFrom(Port* from) { Port::disconnect(this, from); }

        void removeConnections();

        static bool isConnected(Port *port1, Port* port2);
        bool hasConnection() const;

        Port* getAnyConnectedPort();
        Port* findCoincidingPort() const;

        bool areAllOwnersSelected() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem* option);

    private:
        Port* findCoincidingPort(const QList<Port*> &ports) const;

        friend class Wire;
        void setPos(const QPointF& newPos); // Only needed for wires

        QSharedDataPointer<PortData> d;
        PortOwner *m_owner;
        QList<Port*> *m_connections;
    };

} // namespace Caneda

#endif //PORT_H
