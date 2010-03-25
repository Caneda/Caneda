/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "item.h"
#include "port.h"
#include "wireline.h"

#include <QList>

// Forward declarations
class Port;
class QGraphicsLineItem;
class SchematicScene;

typedef QList<WireLine> WireLines;

/*!
 * \brief This class represents a wire on schematic.
 */
class Wire : public QucsItem
{
public:
    enum { Type = QucsItem::WireType };

    //! A struct to store wire's details.
    struct Data {
        WireLines wLines;
        QPointF pos;
        QPointF port1Pos;
        QPointF port2Pos;
    };

    Wire(const QPointF &startPos, const QPointF &endPos, bool doConnect = true,
            SchematicScene *scene = 0);
    Wire(Port *startPort, Port *endPort, SchematicScene *scene = 0);
    ~Wire();

    //! Return's the wire's ports list.
    QList<Port*> ports() const { return m_ports; }
    //! Return's the list's first member.
    Port* port1() const { return m_ports[0]; }
    //! Returns the list's second member.
    Port* port2() const { return m_ports[1]; }

    void movePort(QList<Port*> *connections, const QPointF& scenePos);
    void movePort1(const QPointF& newLocalPos);
    void movePort2(const QPointF& newLocalPos);

    //! Wire identifier.
    int type() const { return Wire::Type; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget = 0);

    void beginGrabMode();
    void grabMoveBy(qreal dx, qreal dy);
    void endGrabMode();

    //! Returns the internal representation of wires as line's list.
    WireLines wireLines() const { return m_wLines; }

    //! Returns a reference of internal representaion of wire lines.
    WireLines& wireLinesRef() { return m_wLines; }

    void setWireLines(const WireLines& wirelines);

    void removeNullLines();

    void saveData(Qucs::XmlWriter *writer) const;
    void saveData(Qucs::XmlWriter *writer, int id) const;

    static Wire* loadWireData(Qucs::XmlReader *reader, SchematicScene *scene);
    void loadData(Qucs::XmlReader *reader);

    //! No rotate defined for wires.
    void rotate90(Qucs::AngleDirection dir = Qucs::AntiClockwise) {
        Q_UNUSED(dir);
    }

    //! No mirroring defined for wires.
    void mirrorAlong(Qt::Axis) {}

    void storeState();
    Data storedState() const;

    Data currentState() const;
    void setState(Data state);

    int checkAndConnect(Qucs::UndoOption opt);

    Wire* copy(SchematicScene *scene = 0) const;
    void copyDataTo(Wire *wire) const;

    bool isComponent() const { return false; }
    bool isWire() const { return true; }

    void updateGeometry();

    //! check if port 1 and 2 overlap
    bool overlap() const {
        return port1()->scenePos() == port2()->scenePos();
    }

    void tryConnectPorts();
    void tryConnectPort(Port * port);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:

    void initWireline();

    int indexForPos(const QPointF& pos) const;

    int m_grabbedIndex; //!< Represents the index of wireline being dragged.

    QList<Port*> m_ports;//!< The ports of wires (always contain only 2 elements).
    WireLines m_wLines;//!< Internal line representation of wires.
    Wire::Data store; //!< Stores the wire data when needed(undo/redo).

    QColor m_wireColor;
    qreal m_width;
};

namespace Qucs
{
    Wire::Data readWireData(Qucs::XmlReader *reader);
}

#endif //WIRE_H
