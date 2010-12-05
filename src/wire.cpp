/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "wire.h"

#include "cgraphicsitem.h"
#include "cgraphicsscene.h"
#include "global.h"

#include "xmlutilities/xmlutilities.h"

#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    //! \todo Make these constants settings.
    static const QColor unselectedWire(Qt::blue);
    static const qreal wirewidth = 1.0;

    /*!
     * \brief Constructs a wire between \a startPos and \a endPos.
     *
     * Also connects the wire's port's to coinciding ports.
     */
    Wire::Wire(const QPointF& startPos, const QPointF& endPos,
            CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        /* set position */
        setPos((startPos + endPos)/2);

        /* set flags */
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        /* create port */
        QPointF localStartPos = mapFromScene(startPos);
        QPointF localEndPos = mapFromScene(endPos);

        m_ports << new Port(this, localStartPos);
        m_ports << new Port(this, localEndPos);

        m_wLine = WireLine(port1()->pos(), port2()->pos());
    }

    //! Destructor.
    Wire::~Wire()
    {
        qDeleteAll(m_ports);
    }

    //! \brief Moves port matching connection \a connections to scene pos \a scenePos.
    void Wire::movePort(QList<Port*> *connections, const QPointF& scenePos)
    {
        if(port1()->connections() == connections) {
            movePort1(mapFromScene(scenePos));
        }
        else if(port2()->connections() == connections) {
            movePort2(mapFromScene(scenePos));
        }
    }

    //! \brief Moves port1 to \a newLocalPos and adjust's the wire's lines.
    void Wire::movePort1(const QPointF& newLocalPos)
    {
        port1()->setPos(newLocalPos);
        m_wLine.setP1(newLocalPos);

        updateGeometry();
    }

    //! \brief Moves port2 to \a newLocalPos and adjust's the wire's lines.
    void Wire::movePort2(const QPointF& newLocalPos)
    {
        port2()->setPos(newLocalPos);
        m_wLine.setP2(newLocalPos);

        updateGeometry();
    }

    //! \brief Draw wire.
    void Wire::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
    {
        Q_UNUSED(widget);
        QPen savedPen;

        /* save painter */
        savedPen = painter->pen();

        if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(Caneda::invertcolor(unselectedWire), wirewidth));
        }
        else {
            painter->setPen(QPen(unselectedWire, wirewidth));
        }

        painter->drawLine(m_wLine);

        /* restore pen */
        painter->setPen(savedPen);
        drawPorts(m_ports, painter, option);
    }

    //! \brief Moves wire by (\a dx, \a dy).
    void Wire::grabMoveBy(qreal dx, qreal dy)
    {
        m_wLine.translate(QPointF(dx, dy));
        movePort1(port1()->pos() + QPointF(dx, dy));
        movePort2(port2()->pos() + QPointF(dx, dy));

        // Now update the wires.
        updateGeometry();
    }

    /*!
     * \brief Check for connections and connect the coinciding ports.
     *
     * \return Returns the number of connections made.
     */
    int Wire::checkAndConnect(Caneda::UndoOption opt)
    {
        int num_of_connections = 0;

        if(opt == Caneda::PushUndoCmd) {
            cGraphicsScene()->undoStack()->beginMacro(QString());
        }

        foreach(Port *port, m_ports) {
            Port *other = port->findCoincidingPort();
            if(other) {
                if(opt == Caneda::PushUndoCmd) {
                    QList<Wire*> wires = Port::wiresBetween(port, other);
                    ConnectCmd *cmd = new ConnectCmd(port, other, wires, cGraphicsScene());
                    cGraphicsScene()->undoStack()->push(cmd);
                }
                else {
                    port->connectTo(other);
                }
                ++num_of_connections;
            }
        }

        if(opt == Caneda::PushUndoCmd) {
            cGraphicsScene()->undoStack()->endMacro();
        }

        return num_of_connections;
    }

    //! \brief Updates the wire's geometry and caches it.
    void Wire::updateGeometry()
    {
        QRectF rect;
        QPainterPath path;

        rect = m_wLine.boundingRect();
        path.addRect(m_wLine.boundingRect());

        addPortEllipses(m_ports, path);
        rect = portsRect(m_ports, rect);
        CGraphicsItem::setShapeAndBoundRect(path, path.boundingRect());
    }

    Wire* Wire::copy(CGraphicsScene *scene) const
     {
         Wire *wire = new Wire(QPointF(1,1), QPointF(5,5), scene);
         Wire::copyDataTo(wire);
         return wire;
     }

     void Wire::copyDataTo(Wire *wire) const
     {
         CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(wire));
         Wire::Data _data;

         _data.wLine = m_wLine;
         _data.pos = pos();
         _data.port1Pos = port1()->pos();
         _data.port2Pos = port2()->pos();

         wire->setState(_data);
     }

    //! \brief Returns the wire's stored status. Required for undo/redo.
    Wire::Data Wire::storedState() const
    {
        return store;
    }

    //! \brief Stores wire's status. Required for undo/redo.
    void Wire::storeState()
    {
        store.wLine = m_wLine;
        store.pos = pos();
        store.port1Pos = port1()->pos();
        store.port2Pos = port2()->pos();
    }

    //! \brief Set's the wire status to \a state. Required for undo/redo.
    void Wire::setState(Wire::Data state)
    {
        setPos(state.pos);
        port1()->setPos(state.port1Pos);
        port2()->setPos(state.port2Pos);
        m_wLine = state.wLine;
        updateGeometry();
    }

    //! \brief Returns current wire status. Required for undo/redo.
    Wire::Data Wire::currentState() const
    {
        Wire::Data retVal;
        retVal.wLine = m_wLine;
        retVal.pos = pos();
        retVal.port1Pos = port1()->pos();
        retVal.port2Pos = port2()->pos();
        return retVal;
    }

    void Wire::saveData(Caneda::XmlWriter *writer, int id) const
    {
        writer->writeStartElement("wire");

        if(id != -1){
            writer->writeAttribute("id", QString::number(id));
        }

        QPointF p1 = port1()->scenePos();
        QPointF p2 = port2()->scenePos();

        QString start = QString("%1,%2").arg(p1.x()).arg(p1.y());
        QString end = QString("%1,%2").arg(p2.x()).arg(p2.y());
        QString pos_str = QString("%1,%2").arg(pos().x()).arg(pos().y());

        writer->writeAttribute("start", start);
        writer->writeAttribute("end", end);
        writer->writeAttribute("pos", pos_str);

        //write the lines
        writer->writeEmptyElement("line");

        writer->writeAttribute("x1", QString::number(m_wLine.x1()));
        writer->writeAttribute("y1", QString::number(m_wLine.y1()));
        writer->writeAttribute("x2", QString::number(m_wLine.x2()));
        writer->writeAttribute("y2", QString::number(m_wLine.y2()));

        writer->writeEndElement();
    }

    Wire* Wire::loadWireData(Caneda::XmlReader *reader, CGraphicsScene *scene)
    {
        Wire *retVal = new Wire(QPointF(10, 10), QPointF(50,50), scene);
        retVal->loadData(reader);

        return retVal;
    }

    void Wire::loadData(Caneda::XmlReader *reader)
    {
        Wire::Data data = readWireData(reader);
        setPos(data.pos);
        port1()->setPos(mapFromScene(data.port1Pos));
        port2()->setPos(mapFromScene(data.port2Pos));
        m_wLine = data.wLine;
        updateGeometry();
    }

    //! \brief Mouse press event. Prepare for grab mode.
    void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        scene()->clearSelection();
        scene()->clearFocus();
        CGraphicsItem::mousePressEvent(event);
        Q_ASSERT(mapFromScene(event->scenePos()) == event->pos());
    }

    //! \brief Hide's on first call and then loses focus. Then updated through scene.
    void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        event->ignore();
    }

    //! \todo Not called at all!
    void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        CGraphicsItem::mouseReleaseEvent(event);
    }

    Wire::Data readWireData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "wire");

        Wire::Data data;

        data.port1Pos = reader->readPointAttribute("start");
        data.port2Pos = reader->readPointAttribute("end");
        data.pos = reader->readPointAttribute("pos");

        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "line") {
                    qreal x1 = reader->readDoubleAttribute("x1");
                    qreal y1 = reader->readDoubleAttribute("y1");
                    qreal x2 = reader->readDoubleAttribute("x2");
                    qreal y2 = reader->readDoubleAttribute("y2");
                    data.wLine = WireLine(x1, y1, x2, y2);

                    //read till end now of line tag now
                    reader->readUnknownElement();
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }
        return data;
    }

} // namespace Caneda
