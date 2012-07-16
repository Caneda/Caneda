/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef WIRE_H
#define WIRE_H

#include "port.h"

#include <QList>

namespace Caneda
{
    // Forward declarations
    class CGraphicsItem;
    class CGraphicsScene;

    //! \brief This class represents a wire on schematic.
    class Wire : public CGraphicsItem
    {
    public:
        enum { Type = CGraphicsItem::WireType };

        //! A struct to store wire's details.
        struct Data {
            QPointF port1Pos;
            QPointF port2Pos;
        };

        Wire(const QPointF &startPos, const QPointF &endPos,
                CGraphicsScene *scene = 0);
        ~Wire();

        //! Wire identifier.
        int type() const { return Wire::Type; }

        //! Return's the list's first member.
        Port* port1() const { return m_ports[0]; }
        //! Returns the list's second member.
        Port* port2() const { return m_ports[1]; }

        void movePort(QList<Port*> *connections, const QPointF& scenePos);
        void movePort1(const QPointF& newLocalPos);
        void movePort2(const QPointF& newLocalPos);

        //! Return true if wire is horizontal
        bool isHorizontal() const { return port1()->pos().y() == port2()->pos().y(); }
        //! Return true if wire is vertical
        bool isVertical() const { return port1()->pos().x() == port2()->pos().x(); }
        //! Check if port 1 and 2 overlap
        bool isNull() const { return port1()->scenePos() == port2()->scenePos(); }

        int checkAndConnect(Caneda::UndoOption opt);
        bool splitAndCreateNodes(CGraphicsScene *scene = 0);
        void updateGeometry();
        QRectF boundingRect () const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget = 0);

        //! \todo Implement this
        void rotate90(Caneda::AngleDirection dir = Caneda::AntiClockwise) { Q_UNUSED(dir); }
        //! \todo Implement this
        void mirrorAlong(Qt::Axis) {}

        Wire* copy(CGraphicsScene *scene = 0) const;
        void copyDataTo(Wire *wire) const;

        bool isComponent() const { return false; }
        bool isWire() const { return true; }

        Wire::Data storedState() const;
        void storeState();
        void setState(Data state);
        Data currentState() const;

        void saveData(Caneda::XmlWriter *writer, int id=-1) const;
        static Wire* loadWireData(Caneda::XmlReader *reader, CGraphicsScene *scene);
        void loadData(Caneda::XmlReader *reader);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);

    private:
        Wire::Data store; //!< Stores the wire data when needed(undo/redo).
    };

} // namespace Caneda

#endif //WIRE_H
