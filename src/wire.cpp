/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "wire.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a wire between \a startPos and \a endPos.
     *
     * During creation, this class also connects the wire's port's to every
     * other coinciding ports in the scene.
     *
     * \param startPos Starting position of the wire.
     * \param endPos Ending position of the wire.
     * \param scene CGraphicsScene where to add this wire.
     */
    Wire::Wire(const QPointF& startPos, const QPointF& endPos,
            CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        // Set flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Set the initial position of the item
        setPos(startPos);

        // Create ports
        m_ports << new Port(this, mapFromScene(startPos));
        m_ports << new Port(this, mapFromScene(endPos));
    }

    //! \brief Destructor.
    Wire::~Wire()
    {
        qDeleteAll(m_ports);
    }

    //! \brief Moves port1 to \a newScenePos.
    void Wire::movePort1(const QPointF& newScenePos)
    {
        port1()->setPos(mapFromScene(newScenePos));
        updateGeometry();
    }

    //! \brief Moves port2 to \a newScenePos.
    void Wire::movePort2(const QPointF& newScenePos)
    {
        port2()->setPos(mapFromScene(newScenePos));
        updateGeometry();
    }

    /*!
     * \todo Implement a list of components and wires connections and
     * return directly that list, simplifying this procedure and
     * avoiding the recursion.
     */
    void Wire::getConnectedWires(QList<Wire*> &connectedWires)
    {
        if(connectedWires.contains(this)) {
            return;
        }

        connectedWires << this;

        foreach(Port *port, *(port1()->connections())) {
            if(port->parentItem()->type() == CGraphicsItem::WireType) {
                Wire *_wire = static_cast<Wire*>(port->parentItem());
                _wire->getConnectedWires(connectedWires);
            }
        }

        foreach(Port *port, *(port2()->connections())) {
            if(port->parentItem()->type() == CGraphicsItem::WireType) {
                Wire *_wire = static_cast<Wire*>(port->parentItem());
                _wire->getConnectedWires(connectedWires);
            }
        }
    }

    //! \brief Draw wire.
    void Wire::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        // Draw the wire
        painter->drawLine(port1()->pos(), port2()->pos());

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

        // List of wires to delete after collision and creation of new wires
        QList<Wire*> markedForDeletion;

        // Check for collisions in each port, otherwise the items intersect
        // but no node should be created.
        foreach(Port *port, m_ports) {

            // Detect all colliding items
            QList<QGraphicsItem*> collisions = port->collidingItems(Qt::IntersectsItemBoundingRect);

            // Filter colliding wires only
            foreach(QGraphicsItem *item, collisions) {
                Wire* _collidingItem = canedaitem_cast<Wire*>(item);
                if(_collidingItem) {

                    // If already connected, the collision is the result of the connection,
                    // otherwise there is a potential new node.
                    bool alreadyConnected = false;
                    foreach(Port *portIterator, m_ports) {
                        alreadyConnected |=
                                portIterator->isConnectedTo(_collidingItem->port1()) ||
                                portIterator->isConnectedTo(_collidingItem->port2());
                    }

                    if(!alreadyConnected){
                        // Calculate the start, middle and end points. As the ports are mapped in the parent's
                        // coordinate system, we must calculate the positions (via the mapToScene method) in
                        // the global (scene) coordinate system.
                        QPointF startPoint  = _collidingItem->port1()->scenePos();
                        QPointF middlePoint = port->scenePos();
                        QPointF endPoint    = _collidingItem->port2()->scenePos();

                        // Mark old wire for deletion. The deletion is performed in a second
                        // stage to avoid referencing null pointers inside the foreach loop.
                        markedForDeletion << _collidingItem;

                        // Create two new wires
                        Wire *wire1 = new Wire(startPoint, middlePoint, scene);
                        Wire *wire2 = new Wire(middlePoint, endPoint, scene);

                        // Create new node (connections to the colliding wire)
                        port->connectTo(wire1->port2());
                        port->connectTo(wire2->port1());

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

        // Delete all wires marked for deletion. The deletion is performed
        // in a second stage to avoid referencing null pointers inside the
        // foreach loop.
        foreach(Wire *w, markedForDeletion) {
            delete w;
        }

        return nodeCreated;
    }

    //! \brief Updates the wire's geometry and caches it.
    void Wire::updateGeometry()
    {
        // Create a path for the shape using a QPainterPathStroker to allow for
        // a thick selection band. Otherwise, if we use only a line, it is very
        // difficult to make a wire selection. The last alternative is to use a
        // QRect as a shape, but that would extend far off limits the selection
        // of diagonal wires.
        QPainterPath path;
        path.moveTo(port1()->pos());
        path.lineTo(port2()->pos());

        QPainterPathStroker stroker;
        stroker.setWidth(2*portRadius);
        path = stroker.createStroke(path);

        CGraphicsItem::setShapeAndBoundRect(path, boundingRect());
    }

    //! \brief Returns bounding rectangle arround the wire
    QRectF Wire::boundingRect() const
    {
        // Check that ports have been previously created
        // This is used to avoid a crash in the case a wire is created
        // and its position set before any ports added. That specially
        // happens during wire loading in a file open operation.
        if(ports().isEmpty()) {
            return QRectF(0,0,0,0);
        }

        // If everything's OK, set the bounding rect of the wire
        QRectF rect;
        rect.setTopLeft(port1()->pos());
        rect.setBottomRight(port2()->pos());
        rect = rect.normalized();
        rect.adjust(-portRadius, -portRadius, +portRadius, +portRadius);
        return rect;
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

         WireData _data;
         _data.port1Pos = port1()->pos();
         _data.port2Pos = port2()->pos();

         wire->setState(_data);
     }

    //! \brief Returns the wire's stored status. Required for undo/redo.
    WireData Wire::storedState() const
    {
        return store;
    }

    //! \brief Stores wire's status. Required for undo/redo.
    void Wire::storeState()
    {
        store.port1Pos = port1()->pos();
        store.port2Pos = port2()->pos();
    }

    //! \brief Set's the wire status to \a state. Required for undo/redo.
    void Wire::setState(WireData state)
    {
        port1()->setPos(state.port1Pos);
        port2()->setPos(state.port2Pos);
        updateGeometry();
    }

    //! \brief Returns current wire status. Required for undo/redo.
    WireData Wire::currentState() const
    {
        WireData retVal;
        retVal.port1Pos = port1()->pos();
        retVal.port2Pos = port2()->pos();
        return retVal;
    }

    /*!
     * \brief Saves current wire data to \a Caneda::XmlWriter.
     *
     * Saves current wire data to \a Caneda::XmlWriter. This method is the
     * virtual implementation of CGraphicsItem::saveData(), to allow for
     * wire copy/pasting. Do not confuse with
     * saveData(Caneda::XmlWriter *writer, int id) const which is used by
     * this method with a constant id, and is used during
     * FormatXmlSchematic::saveWires() to save wires with different ids.
     *
     * \warning Do not delete this method (replacing it by
     * saveData(Caneda::XmlWriter *writer, int id) const), otherwise
     * copy/pasting of wires will stop functioning.
     *
     * \param writer The xmlwriter used to write xml data.
     *
     * \sa saveData(Caneda::XmlWriter *writer, int id) const,
     * CGraphicsItem::saveData(), loadData()
     */
    void Wire::saveData(Caneda::XmlWriter *writer) const
    {
        saveData(writer, -1);
    }

    /*!
     * \brief Saves current wire data to \a Caneda::XmlWriter.
     *
     * Saves current wire data to \a Caneda::XmlWriter. Do not confuse this
     * method with saveData(Caneda::XmlWriter *writer) const which is the
     * virtual implementation of CGraphicsItem::saveData().
     *
     * This method is used during FormatXmlSchematic::saveWires() to save wires
     * with different ids.
     *
     * \param writer The xmlwriter used to write xml data.
     * \param id The wire id inside the schematic.
     *
     * \sa saveData(Caneda::XmlWriter *writer) const,
     * FormatXmlSchematic::saveWires(), loadData()
     */
    void Wire::saveData(Caneda::XmlWriter *writer, int id) const
    {
        writer->writeStartElement("wire");

        writer->writeAttribute("id", QString::number(id));

        QPointF p1 = port1()->scenePos();
        QPointF p2 = port2()->scenePos();

        QString start = QString("%1,%2").arg(p1.x()).arg(p1.y());
        QString end = QString("%1,%2").arg(p2.x()).arg(p2.y());

        writer->writeAttribute("start", start);
        writer->writeAttribute("end", end);

        writer->writeEndElement();
    }

    /*!
     * \brief Convenience static method to load a wire saved as xml.
     *
     * This method loads a wire saved as xml. Once the wire is
     * created, its data is filled using the loadData() method.
     *
     * \param reader The xmlreader used to read xml data.
     * \param scene CGraphicsScene to which the wire should be parented to.
     * \return Returns new wire pointer on success and null on failure.
     *
     * \sa loadData()
     */
    Wire* Wire::loadWire(Caneda::XmlReader *reader, CGraphicsScene *scene)
    {
        Wire *retVal = new Wire(QPointF(10,10), QPointF(50,50), scene);
        retVal->loadData(reader);

        return retVal;
    }

    void Wire::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "wire");

        QPointF port1Pos = reader->readPointAttribute("start");
        QPointF port2Pos = reader->readPointAttribute("end");
        movePort1(port1Pos);
        movePort2(port2Pos);

        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isEndElement()) {
                break;
            }
        }

        updateGeometry();
    }

} // namespace Caneda
