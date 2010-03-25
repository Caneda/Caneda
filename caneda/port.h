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

#ifndef PORT_H
#define PORT_H

#include "item.h"

#include <QGraphicsItem>
#include <QList>
#include <QPainter>
#include <QPointF>
#include <QSharedData>
#include <QString>

//Forward declarations.
class Wire;
class Component;
class SchematicScene;

//! \todo Make these constants settings.
static const QPen connectedPen(Qt::blue, 0);
//! \todo Make these constants settings.
static const QBrush connectedBrush(Qt::blue);
//! \todo Make these constants settings.
static const QPen unconnectedPen(Qt::darkRed, 0);
//! \todo Make these constants settings.
static const QBrush unconnectedBrush(Qt::NoBrush);
//! \todo Make these constants settings.
static const qreal portRadius(3.0);
//! \todo Make these constants settings.
static const QRectF portEllipse(-portRadius, -portRadius, 2*portRadius,
        2*portRadius);
//! \todo Make these constants settings.
static const qreal portConnectedRadius(2.0);
//! \todo Make these constants settings.
static const QRectF portEllipseConnected(-portConnectedRadius, -portConnectedRadius,
        2*portConnectedRadius,
        2*portConnectedRadius);


//! Thin class used to abstract owner of port.
class PortOwner
{
public:
    PortOwner(QucsItem * item);

    //! return type of owner
    int type() const { return m_item->type(); }

    Wire* wire() const;
    //! Return the component if stored, or null otherwise.
    Component* component() const;
    //! Returns the owner item as graphicsitem.
    QGraphicsItem* item() const;

    //! return weather item is a wire
    bool isWire() const { return m_item->isWire(); }
    //! return weather item is a component
    bool isComponent() const { return m_item->isComponent(); }

private:
    //! owner of the port
    QucsItem *const m_item;
    //! Disable copy
    PortOwner(const PortOwner& other);
};

//! Class used to represend sharable port's data.
struct PortData : public QSharedData
{
    PortData(QPointF _pos, QString _name) : pos(_pos), name(_name) {}

    QPointF pos;
    QString name;
};

class Port
{
public:
    Port(QucsItem  *owner, const QSharedDataPointer<PortData> &data);
    Port(QucsItem  *owner, QPointF _pos, QString portName = QString());

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

    //! Returns a pointer to list of connected ports(null if unconnected).
    QList<Port*> * connections() const { return m_connections; }

    SchematicScene* schematicScene() const;

    static void connect(Port* port1, Port *port2);
    static void disconnect(Port *port, Port *from);

    static bool isConnected(Port *port1, Port* port2);

    bool hasConnection() const;

    void connectTo(Port *other);
    /*!
     * Shorthand for Port::disconnect(this, from)
     * \note from == NULL is allowed
     */
    void disconnectFrom(Port *from) { Port::disconnect(this, from); }

    Port* getAnyConnectedPort();
    void removeConnections();

    static QList<Wire*> wiresBetween(Port* port1, Port* port2);

    Port* findIntersectingPort() const;
    Port* findCoincidingPort() const;

    bool areAllOwnersSelected() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem* option);

private:
    Port* findIntersectingPort(const QList<Port*> &ports) const;
    Port* findCoincidingPort(const QList<Port*> &ports) const;

    void setPos(const QPointF& newPos);

    friend class Wire;

    QSharedDataPointer<PortData> d;
    PortOwner *m_owner;
    QList<Port*> *m_connections;
    /*!
     * This represents the connection name.
     * \todo Implement this feature.
     */
    QString *m_nodeName;
};

void drawPorts(const QList<Port*> & ports, QPainter *painter,
        const QStyleOptionGraphicsItem* option);

QRectF portsRect(const QList<Port*> &ports, const QRectF& rect);

void addPortEllipses(const QList<Port*> &ports, QPainterPath &path);

#endif //PORT_H
