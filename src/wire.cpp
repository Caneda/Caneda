/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "actionmanager.h"
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

        // Create ports
        m_ports << new Port(this);
        m_ports << new Port(this);
        movePort1(startPos);
        movePort2(endPos);
    }

    //! \brief Destructor.
    Wire::~Wire()
    {
        qDeleteAll(m_ports);
    }

    /*!
     * \brief Move port to a new scene position.
     *
     * This method moves the port to new scene position. This is normally used
     * while drawing a new wire, or changing the geometry of an existing one.
     *
     * The order of the steps involved is important, as if care is not taken
     * strange artifacts occur. On the other hand, the center or position of
     * the item must be updated to be positioned as close as possible to the
     * real center. If the position is not updated, although at first the wire
     * appears to be correctly drawn, when rotating the center of rotation
     * would be offset making it difficult to coordinate the rotations with the
     * rest of the scene's components.
     *
     * This method performs the following steps:
     * \li First saves the pos of the other port as it will be moved when a new
     * center is set.
     * \li Secondly, it moves the position of the wire to the calculated center
     * from the new position of this port and the saved position of the other
     * port. It is important to note that for odd number of grid lenghts the
     * center will be moved to the nearest grid. This results in rotations that
     * are slightly shifted to fit in the grid.
     * \li Lastly, it sets the new port position, and restores the other port
     * position with the new wire center.
     *
     * \param newScenePos Destination position of the port.
     *
     * sa smartNearingGridPoint()
     */
    void Wire::movePort1(const QPointF& newScenePos)
    {
        // Save the pos of the other port.
        QPointF otherPortPos = port2()->scenePos();

        // Move the position of the wire to the new calculated center.
        QPointF center = QRectF(otherPortPos, newScenePos).center();
        setPos(smartNearingGridPoint(center));

        // Set new port position, and restore the other port position with the new center.
        port1()->setPos(mapFromScene(newScenePos));
        port2()->setPos(mapFromScene(otherPortPos));
        updateGeometry();
    }

    //! \copydoc movePort1()
    void Wire::movePort2(const QPointF& newScenePos)
    {
        // Save the pos of the other port.
        QPointF otherPortPos = port1()->scenePos();

        // Move the position of the wire to the new calculated center.
        QPointF center = QRectF(otherPortPos, newScenePos).center();
        setPos(smartNearingGridPoint(center));

        // Set new port position, and restore the other port position with the new center.
        port2()->setPos(mapFromScene(newScenePos));
        port1()->setPos(mapFromScene(otherPortPos));
        updateGeometry();
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
    }

    /*!
     * \brief Saves current wire data to \a Caneda::XmlWriter.
     *
     * Saves current wire data to \a Caneda::XmlWriter. This method is the
     * virtual implementation of CGraphicsItem::saveData(), to allow for
     * wire copy/pasting. It is also used during
     * FormatXmlSchematic::saveWires() to save wires into a schematic file.
     *
     * \param writer The xmlwriter used to write xml data.
     *
     * \sa FormatXmlSchematic::saveWires(), CGraphicsItem::saveData(),
     * loadData()
     */
    void Wire::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("wire");

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

    //! \copydoc CGraphicsItem::contextMenuEvent()
    void Wire::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();

        // Launch context menu of item
        _menu->addAction(am->actionForName("editCut"));
        _menu->addAction(am->actionForName("editCopy"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->exec(event->screenPos());
    }

} // namespace Caneda
