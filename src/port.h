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

#ifndef PORT_H
#define PORT_H

#include "cgraphicsitem.h"

#include <QList>
#include <QSharedData>

namespace Caneda
{
    // Style constants definitions
    static const qreal portRadius(3.0);
    static const QRectF portEllipse(-portRadius, -portRadius, 2*portRadius,
            2*portRadius);

    //! \brief Sharable port's data.
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
    class Port : public QGraphicsItem
    {
    public:
        Port(CGraphicsItem* parent, QPointF pos, QString portName = QString());
        ~Port();

        enum { Type = CGraphicsItem::PortType };
        int type() const { return Port::Type; }

        //! Returns the port's name.
        QString name() const { return d->name; }

        //! Returns the position relative to owner - usually constant.
        QPointF pos() const { return d->pos; }
        void setPos(const QPointF& newPos);
        QPointF scenePos() const;

        CGraphicsItem* parentItem() const;

        //! Returns a pointer to list of connected ports
        QList<Port*> *connections() { return &(m_connections); }

        void connectTo(Port *other);
        void disconnect();

        static bool isConnected(Port *port1, Port* port2);
        bool hasAnyConnection() const;

        Port* findCoincidingPort() const;

        //! Return bounding box
        QRectF boundingRect() const { return portEllipse; }
        void paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget*);

    private:
        QSharedDataPointer<PortData> d;
        QList<Port*> m_connections;
    };

} // namespace Caneda

#endif //PORT_H
