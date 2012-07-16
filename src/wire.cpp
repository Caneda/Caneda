/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2011 by Pablo Daniel Pareja Obregon                  *
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

#include "cgraphicsscene.h"
#include "global.h"
#include "settings.h"

#include "xmlutilities/xmlutilities.h"

#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a wire between \a startPos and \a endPos.
     *
     * Also connects the wire's port's to coinciding ports.
     */
    Wire::Wire(const QPointF& startPos, const QPointF& endPos,
            CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        // Set position
        setPos((startPos + endPos)/2);

        // Set flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Create ports
        m_ports << new Port(this, mapFromScene(startPos));
        m_ports << new Port(this, mapFromScene(endPos));

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
        foreach(Port *port, m_ports) {
            port->paint(painter, option);
        }

        // Save painter
        QPen savedPen = painter->pen();

        Settings *settings = Settings::instance();

        if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        painter->drawLine(m_wLine);

        // Restore pen
        painter->setPen(savedPen);
    }

    /*!
     * \brief Check for connections and connect the coinciding ports.
     *
     * \return Returns the number of connections made.
     */
    int Wire::checkAndConnect(Caneda::UndoOption opt)
    {
        int num_of_connections = CGraphicsItem::checkAndConnect(opt);

        splitAndCreateNodes(cGraphicsScene());

        return num_of_connections;
    }

    /*!
     * \brief If a collision is present, splits a wire in two and creates
     * a new node.
     *
     * \return Returns true if new node was created.
     */
    bool Wire::splitAndCreateNodes(CGraphicsScene *scene)
    {
        bool nodeCreated = false;

        // Check both ends for collisions
        foreach(Port *port, m_ports) {
            QList<QGraphicsItem*> collisions =
                    collidingItems(Qt::IntersectsItemBoundingRect);

            // Filter colliding wires only
            foreach(QGraphicsItem *item, collisions) {

                Wire* _collidingItem = canedaitem_cast<Wire*>(item);
                if(_collidingItem) {
                    // If wires are connected, the collision is the result of the connection.
                    // Otherwise, there is a potential new node.
                    bool wiresAreConnected = Port::isConnected(port1(), _collidingItem->port1()) ||
                                             Port::isConnected(port1(), _collidingItem->port2()) ||
                                             Port::isConnected(port2(), _collidingItem->port1()) ||
                                             Port::isConnected(port2(), _collidingItem->port2());

                    if(!wiresAreConnected){

                        // Check if the collision is in the extremes of the wire (ports). Otherwise,
                        // they intersect, but no node should be created.
                        bool verticalCollision = _collidingItem->m_wLine.isHorizontal() &&
                                                 port->pos().y() + pos().y() == _collidingItem->pos().y();
                        bool horizontalCollision = _collidingItem->m_wLine.isVertical() &&
                                                   port->pos().x() + pos().x() == _collidingItem->pos().x();

                        if( verticalCollision || horizontalCollision ) {

                            QPointF startPoint = _collidingItem->port1()->pos() + _collidingItem->pos();
                            QPointF middlePoint = port->pos() + pos();
                            QPointF endPoint =  _collidingItem->port2()->pos() + _collidingItem->pos();

                            // Create two new wires
                            Wire *wire1 = new Wire(startPoint, middlePoint, scene);
                            Wire *wire2 = new Wire(middlePoint, endPoint, scene);

                            // Delete old wire
                            delete _collidingItem;

                            // Create new node (connections to the colliding wire)
                            port->connectTo(wire1->port2());
                            port->connectTo(wire1->port1());

                            wire1->updateGeometry();
                            wire2->updateGeometry();

                            // Restore old wire connections
                            wire1->checkAndConnect(Caneda::DontPushUndoCmd);
                            wire2->checkAndConnect(Caneda::DontPushUndoCmd);

                            nodeCreated = true;

                        }
                    }
                }
            }
        }

        return nodeCreated;
    }

    //! \brief Updates the wire's wireline, geometry and caches it.
    void Wire::updateGeometry()
    {
        m_wLine.setP1(port1()->pos());
        m_wLine.setP2(port2()->pos());

        QRectF rect;
        rect = m_wLine.boundingRect();
        foreach(Port *port, m_ports) {
            rect |= portEllipse.translated(port->pos());
        }

        QPainterPath path;
        path.addRect(rect);

        CGraphicsItem::setShapeAndBoundRect(path, rect);
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
        store.port1Pos = port1()->pos();
        store.port2Pos = port2()->pos();
    }

    //! \brief Set's the wire status to \a state. Required for undo/redo.
    void Wire::setState(Wire::Data state)
    {
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

        writer->writeAttribute("start", start);
        writer->writeAttribute("end", end);

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
        Q_ASSERT(reader->isStartElement() && reader->name() == "wire");

        QPointF port1Pos = reader->readPointAttribute("start");
        QPointF port2Pos = reader->readPointAttribute("end");

        port1()->setPos(mapFromScene(port1Pos));
        port2()->setPos(mapFromScene(port2Pos));
        m_wLine = WireLine(port1Pos, port2Pos);

        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isEndElement()) {
                break;
            }
        }

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

} // namespace Caneda
