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
     * During creation, the initial position of the wire must be set to avoid
     * strange artifacts during copy/paste operations.
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

        // Set initial position
        setPos(startPos);

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
     * strange artifacts occur. On the other hand, the position of the wire is
     * set at the constructor, fixed in a point. When rotating the wire, the
     * center of rotation is offset around that point, but as the rotations are
     * performed around pivot points, this fact is hidded behind the
     * implementation.
     *
     * \param newScenePos Destination position of the port.
     */
    void Wire::movePort1(const QPointF& newScenePos)
    {
        port1()->setPos(mapFromScene(newScenePos));
        updateGeometry();
    }

    //! \copydoc movePort1()
    void Wire::movePort2(const QPointF& newScenePos)
    {
        port2()->setPos(mapFromScene(newScenePos));
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

    //! \copydoc CGraphicsItem::copy()
    Wire* Wire::copy(CGraphicsScene *scene) const
    {
        QPointF startPos = port1()->scenePos();
        QPointF endPos = port2()->scenePos();

        Wire *wire = new Wire(startPos, endPos, scene);
        return wire;
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
