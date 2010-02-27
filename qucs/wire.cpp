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

#include "wire.h"

#include "port.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "undocommands.h"

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QGraphicsSceneEvent>
#include <QGraphicsView>
#include <QList>
#include <QRubberBand>
#include <QStyleOptionGraphicsItem>
#include <QtAlgorithms>
#include <QVariant>


//! \todo Make these constants settings.
static const QColor unselectedWire(Qt::blue);
static const qreal wirewidth = 1.0;

/*!
 * \brief Invert a color
 * \todo Put in a header
 */
static QColor invertcolor(const QColor & color)
{
    QColor inverted;
    inverted.setRgbF(1.0 - color.redF(), 1.0 - color.greenF(), 1.0 - color.blueF(),1.0);
    return inverted;
}

/*!
 * \brief Try to connect a port
 *
 * \param port: port to connect
 * \todo handle more than one coinciding port
 * \bug the todo is a bug...
 */
void Wire::tryConnectPort(Port * port)
{
    Port * p = port->findCoincidingPort();
    if(p) {
        p->connectTo(p);
    }
}

//! \brief Try to connect ports
void Wire::tryConnectPorts()
{
    tryConnectPort(port1());
    tryConnectPort(port2());
}


/*!
 * \brief Initialize wire line used on wire creation
 * \note assert m_wLines.isEmpty()
 * \todo BR->GPK document inter
 */
void Wire::initWireline()
{
    Q_ASSERT(m_wLines.isEmpty());

    QPointF inter = QPointF(port1()->pos().x(), port2()->pos().y());
    /* create two wire line */
    m_wLines << WireLine(port1()->pos(), inter);
    m_wLines << WireLine(inter, port2()->pos());

    /* update wire */
    updateGeometry();
}


/*!
 * \brief Constructs a wire between \a startPos and \a endPos.
 *
 * Also connects the wire's port's to coinciding ports.
 * \todo try to remove doconnect and do another operation for do connect
 */
Wire::Wire(const QPointF& startPos, const QPointF& endPos, bool doConnect,
        SchematicScene *scene) : QucsItem(0, scene), m_grabbedIndex(-1)
{
    /* set position */
    setPos((startPos + endPos)/2);

    /* set flags */
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
#if QT_VERSION >= 0x040600
    setFlag(ItemSendsGeometryChanges, true);
    setFlag(ItemSendsScenePositionChanges, true);
#endif

    /* create port */
    QPointF localStartPos = mapFromScene(startPos);
    QPointF localEndPos = mapFromScene(endPos);

    m_ports << new Port(this, localStartPos);
    m_ports << new Port(this, localEndPos);

    initWireline();

    /* show in scene */
    if(scene) {
        removeNullLines();
    }

    if(doConnect) {
        tryConnectPorts();
    }
}

/*!
 * \brief Constructs a wire between \a startPort and \a endPort.
 *
 * Also connects the wire's port's to coinciding ports.
 * \bug br->gpk: does the new port is a purpose??
 */
Wire::Wire(Port *startPort, Port *endPort, SchematicScene *scene) :
QucsItem(0, scene), m_grabbedIndex(-1)
{
    QPointF startPos = startPort->scenePos();
    QPointF endPos = endPort->scenePos();

    /* set position */
    setPos((startPos + endPos)/2);

    /* set flags */
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
#if QT_VERSION >= 0x040600
    setFlag(ItemSendsGeometryChanges, true);
    setFlag(ItemSendsScenePositionChanges, true);
#endif

    /* create port
       BR->GPK: Why not add startport and endport
       */
    QPointF localStartPos = mapFromScene(startPos);
    QPointF localEndPos = mapFromScene(endPos);

    m_ports << new Port(this, localStartPos);
    m_ports << new Port(this, localEndPos);

    initWireline();

    if(scene) {
        removeNullLines();
    }

    port1()->connectTo(startPort);
    port2()->connectTo(endPort);
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
    else {
        qWarning() << "Wire::movePort() : Port not found";
    }
}

//! \brief Moves port1 to \a newLocalPos and adjust's the wire's lines.
void Wire::movePort1(const QPointF& newLocalPos)
{
    port1()->setPos(newLocalPos);

    // This is true when wire is created
    if(m_wLines.isEmpty()) {
        return initWireline();
    }

    QPointF referencePos = port1()->pos();

    if(m_wLines.size() == 1) {
        m_wLines.append(WireLine(port2()->pos(), port2()->pos()));
    }

    WireLine &first = m_wLines[0];
    WireLine &second = m_wLines[1];

    Q_ASSERT(!first.isOblique());
    Q_ASSERT(!second.isOblique());

    {
        bool should_set_x = ((first.isNull() &&
                    (second.isNull() || second.isHorizontal()))
                ||
                (!first.isNull() && first.isVertical()));

        if(should_set_x) {
            first.setX(referencePos.x());
        } else {
            first.setY(referencePos.y());
        }

        first.setP1(referencePos);
    }

    referencePos = first.p2();

    {
        bool should_set_y = ((second.isNull() &&
                    (first.isNull() || first.isVertical()))
                ||
                (!second.isNull() && second.isHorizontal()));

        if(should_set_y) {
            second.setY(referencePos.y());
        } else {
            second.setX(referencePos.x());
        }

        second.setP1(referencePos);
    }

    updateGeometry();
}

//! \brief Moves port2 to \a newLocalPos and adjust's the wire's lines.
void Wire::movePort2(const QPointF& newLocalPos)
{
    port2()->setPos(newLocalPos);

    if(m_wLines.isEmpty()) {
        return initWireline();
    }

    QPointF referencePos = port2()->pos();

    if(m_wLines.size() == 1) {
        m_wLines.prepend(WireLine(port1()->pos(), port1()->pos()));
    }

    WireLine &last = m_wLines.last();
    WireLine &last_but_one = m_wLines[m_wLines.size() - 2];

    Q_ASSERT(!last.isOblique());
    Q_ASSERT(!last_but_one.isOblique());


    {
        bool should_set_x = ((last.isNull() &&
                    (last_but_one.isNull() || last_but_one.isHorizontal()))
                ||
                (!last.isNull() && last.isVertical()));

        if(should_set_x) {
            last.setX(referencePos.x());
        } else {
            last.setY(referencePos.y());
        }

        last.setP2(referencePos);
    }

    referencePos = last.p1();

    {
        bool should_set_y = ((last_but_one.isNull() &&
                    (last.isNull() || last.isVertical()))
                ||
                (!last_but_one.isNull() && last_but_one.isHorizontal()));

        if(should_set_y) {
            last_but_one.setY(referencePos.y());
        } else {
            last_but_one.setX(referencePos.x());
        }

        last_but_one.setP2(referencePos);
    }

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
        painter->setPen(QPen(invertcolor(unselectedWire), wirewidth));
    }
    else {
        painter->setPen(QPen(unselectedWire, wirewidth));
    }

    QList<WireLine>::const_iterator it = m_wLines.constBegin();
    QList<WireLine>::const_iterator end = m_wLines.constEnd();

    while(it != end) {
        painter->drawLine(*it);
        ++it;
    }
    /*
       {
    //debugging purpose only.
    if(0) {
    painter->setPen(Qt::darkGreen);
    painter->drawPath(shape());
    }
    }*/

    /* restore pen */
    painter->setPen(savedPen);
    drawPorts(m_ports, painter, option);

}

/*!
 * \brief Starts grab mode, that is mouse pressed and dragged.
 *
 * This actually is also used when wire is selected but not grabbed. The
 * wiring adjustments remain same in that case, so this function simply
 * sets the grabbed index to middle of wire's lines.
 */
void Wire::beginGrabMode()
{
    if(m_grabbedIndex == -1) {
        m_grabbedIndex = m_wLines.size()/2;
    }

    /* Calculate the number of wires to be inserted at start and end.
       This is done to make sure there are enough wire lines to easily
       support flexible wire movement.
       */
    int num_of_wires_in_beginning = -(m_grabbedIndex-2);
    int num_of_wires_in_end = -(m_wLines.size()-1-m_grabbedIndex-2);

    QPointF p1 = m_wLines.first().p1();
    QPointF p2 = m_wLines.last().p2();

    // insert zero sized wire-lines at both ends now as calculated.
    for(int i=0; i < num_of_wires_in_beginning; ++i) {
        m_wLines.prepend(WireLine(p1, p1));
    }
    for(int i=0; i < num_of_wires_in_end; ++i) {
        m_wLines.append(WireLine(p2, p2));
    }

    // adjust grabbed index to point where it was before.
    if(num_of_wires_in_beginning > 0) {
        m_grabbedIndex += num_of_wires_in_beginning;
    }
}

/*!
 * \brief Moves wire by (\a dx, \a dy).
 * \sa beginGrabMode
 */
void Wire::grabMoveBy(qreal dx, qreal dy)
{
    // assure atleast two wirelines presence at both ends.
    Q_ASSERT(m_grabbedIndex >= 2 &&
            (m_wLines.size()-1-m_grabbedIndex) >= 2);

    /* now calculate the constant positions in the beginning and end.
       This position wont change when wire is moved.
       */
    QPointF refPos1 = m_grabbedIndex == 2 ? port1()->pos() :
        m_wLines[m_grabbedIndex-3].p2();

    QPointF refPos2 = m_wLines.size()-1-m_grabbedIndex == 2 ?
        port2()->pos() : m_wLines[m_grabbedIndex+3].p1();

    QPointF inter1, inter2;

    // Move the grabbed line first and asserts its nature.
    WireLine& grabbedLine = m_wLines[m_grabbedIndex];
    grabbedLine.translate(QPointF(dx, dy));
    Q_ASSERT(grabbedLine.isHorizontal() || grabbedLine.isVertical());

    // Calculate the intermediate points of the wirelines.
    if(grabbedLine.isHorizontal()) {
        inter1 = QPointF(grabbedLine.p1().x(), refPos1.y());
        inter2 = QPointF(grabbedLine.p2().x(), refPos2.y());
    }
    else {
        inter1 = QPointF(refPos1.x(), grabbedLine.p1().y());
        inter2 = QPointF(refPos2.x(), grabbedLine.p2().y());
    }

    // Modify wirelines to accomodate the changes.
    m_wLines[m_grabbedIndex-1] = WireLine(inter1, grabbedLine.p1());
    m_wLines[m_grabbedIndex-2] = WireLine(refPos1, inter1);

    m_wLines[m_grabbedIndex+1] = WireLine(grabbedLine.p2(), inter2);
    m_wLines[m_grabbedIndex+2] = WireLine(inter2, refPos2);

    // Now update the wires.
    updateGeometry();
}

/*!
 * \brief Ends grab mode.
 * \sa beginGrabMode
 */
void Wire::endGrabMode()
{
    m_grabbedIndex = -1; //reset index

    removeNullLines();
    updateGeometry();
}

//! \brief Sets wire's internal representation to \a wlines
void Wire::setWireLines(const WireLines& wlines)
{
    m_wLines = wlines;
    updateGeometry();
}

/*!
 * \brief Removes zero length lines and optimizes multiple straight lines
 * to one big straight line.
 * \todo rename to optimize
 */
void Wire::removeNullLines()
{
    QList<WireLine>::iterator it = m_wLines.begin(), it1;

    /* erase null line */
    while(it != m_wLines.end()) {
        if(it->isNull()) {
            it = m_wLines.erase(it);
        }
        else {
            ++it;
        }
    }

    /* do not do further optimization ifonly one segment */
    if(m_wLines.size() <= 1) {
        return;
    }

    /* optimize multiple straight line */
    it = m_wLines.begin() + 1;
    while(it != m_wLines.end()) {
        it1 = it - 1;

        /* horizontal straight line */
        if(it->isHorizontal() && it1->isHorizontal()) {
            Q_ASSERT(it1->p2() == it->p1());
            it1->setP2(it->p2());
            it = m_wLines.erase(it);
        }
        /* horizontal case */
        else if(it->isVertical() && it1->isVertical()) {
            Q_ASSERT(it1->p2() == it->p1());
            it1->setP2(it->p2());
            it = m_wLines.erase(it);
        }
        /* other */
        else {
            ++it;
        }
    }
    updateGeometry();
}

void Wire::saveData(Qucs::XmlWriter *writer) const
{
    writer->writeStartElement("wire");

    QPointF p1 = port1()->scenePos();
    QPointF p2 = port2()->scenePos();

    QString start = QString("%1,%2").arg(p1.x()).arg(p1.y());
    QString end = QString("%1,%2").arg(p2.x()).arg(p2.y());
    QString pos_str = QString("%1,%2").arg(pos().x()).arg(pos().y());

    writer->writeAttribute("start", start);
    writer->writeAttribute("end", end);
    writer->writeAttribute("pos", pos_str);

    //write the lines
    foreach(WireLine line, m_wLines) {
        writer->writeEmptyElement("line");

        writer->writeAttribute("x1", QString::number(line.x1()));
        writer->writeAttribute("y1", QString::number(line.y1()));
        writer->writeAttribute("x2", QString::number(line.x2()));
        writer->writeAttribute("y2", QString::number(line.y2()));
    }

    writer->writeEndElement();
}

void Wire::saveData(Qucs::XmlWriter *writer, int id) const
{
    writer->writeStartElement("wire");

    writer->writeAttribute("id", QString::number(id));

    QPointF p1 = port1()->scenePos();
    QPointF p2 = port2()->scenePos();

    QString start = QString("%1,%2").arg(p1.x()).arg(p1.y());
    QString end = QString("%1,%2").arg(p2.x()).arg(p2.y());
    QString pos_str = QString("%1,%2").arg(pos().x()).arg(pos().y());

    writer->writeAttribute("start", start);
    writer->writeAttribute("end", end);
    writer->writeAttribute("pos", pos_str);

    //write the lines
    foreach(WireLine line, m_wLines) {
        writer->writeEmptyElement("line");

        writer->writeAttribute("x1", QString::number(line.x1()));
        writer->writeAttribute("y1", QString::number(line.y1()));
        writer->writeAttribute("x2", QString::number(line.x2()));
        writer->writeAttribute("y2", QString::number(line.y2()));
    }

    writer->writeEndElement();
}

Wire* Wire::loadWireData(Qucs::XmlReader *reader, SchematicScene *scene)
{
    Wire *retVal = new Wire(QPointF(10, 10), QPointF(50,50), false, scene);
    retVal->loadData(reader);

    return retVal;
}

void Wire::loadData(Qucs::XmlReader *reader)
{
    Wire::Data data = readWireData(reader);
    setPos(data.pos);
    port1()->setPos(mapFromScene(data.port1Pos));
    port2()->setPos(mapFromScene(data.port2Pos));
    m_wLines = data.wLines;
    updateGeometry();
}

//! \brief Stores wire's status. Required for undo/redo.
void Wire::storeState()
{
    store.wLines = m_wLines;
    store.pos = pos();
    store.port1Pos = port1()->pos();
    store.port2Pos = port2()->pos();
}

//! \brief Returns the wire's stored status. Required for undo/redo.
Wire::Data Wire::storedState() const
{
    return store;
}

//! \brief Returns current wire status. Required for undo/redo.
Wire::Data Wire::currentState() const
{
    Wire::Data retVal;
    retVal.wLines = m_wLines;
    retVal.pos = pos();
    retVal.port1Pos = port1()->pos();
    retVal.port2Pos = port2()->pos();
    return retVal;
}

//! \brief Set's the wire status to \a state. Required for undo/redo.
void Wire::setState(Wire::Data state)
{
    setPos(state.pos);
    port1()->setPos(state.port1Pos);
    port2()->setPos(state.port2Pos);
    m_wLines = state.wLines;
    updateGeometry();
}

/*!
 * \brief Check for connections and connect the coinciding ports.
 *
 * \return Returns the number of connections made.
 */
int Wire::checkAndConnect(Qucs::UndoOption opt)
{
    int num_of_connections = 0;

    if(opt == Qucs::PushUndoCmd) {
        schematicScene()->undoStack()->beginMacro(QString());
    }

    foreach(Port *port, m_ports) {
        Port *other = port->findCoincidingPort();
        if(other) {
            if(opt == Qucs::PushUndoCmd) {
                QList<Wire*> wires = Port::wiresBetween(port, other);
                ConnectCmd *cmd = new ConnectCmd(port, other, wires, schematicScene());
                schematicScene()->undoStack()->push(cmd);
            }
            else {
                port->connectTo(other);
            }
            ++num_of_connections;
        }
    }

    if(opt == Qucs::PushUndoCmd) {
        schematicScene()->undoStack()->endMacro();
    }

    return num_of_connections;
}

Wire* Wire::copy(SchematicScene *scene) const
{
    Wire *wire = new Wire(QPointF(1,1), QPointF(5,5), false, scene);
    Wire::copyDataTo(wire);
    return wire;
}

void Wire::copyDataTo(Wire *wire) const
{
    QucsItem::copyDataTo(static_cast<QucsItem*>(wire));
    Wire::Data _data;

    _data.wLines = m_wLines;
    _data.pos = pos();
    _data.port1Pos = port1()->pos();
    _data.port2Pos = port2()->pos();

    wire->setState(_data);
}

//! \brief Mouse press event. Prepare for grab mode.
void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_grabbedIndex = -1;
    scene()->clearSelection();
    scene()->clearFocus();
    QucsItem::mousePressEvent(event);
    Q_ASSERT(mapFromScene(event->scenePos()) == event->pos());
    m_grabbedIndex = indexForPos(event->pos());
}

//! \brief Hide's on first call and then loses focus. Then updated through scene.
void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_ASSERT(m_grabbedIndex != -1);
    event->ignore();
    beginGrabMode();
}

//! \todo Not called at all!
void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QucsItem::mouseReleaseEvent(event);
}

//! \brief Updates the wire's geometry and caches it.
void Wire::updateGeometry()
{
    QRectF rect;
    QPainterPath path;

    QList<WireLine>::const_iterator it = m_wLines.constBegin();
    QList<WireLine>::const_iterator end = m_wLines.constEnd();
    for(; it != end; ++it) {
        rect |= it->boundingRect();
        path.addRect(it->boundingRect());
    }

    addPortEllipses(m_ports, path);
    rect = portsRect(m_ports, rect);
    QucsItem::setShapeAndBoundRect(path, path.boundingRect());
}

//! \brief Returns index corresponding to position \a pos.
int Wire::indexForPos(const QPointF& pos) const
{
    int retVal = 0;
    QList<WireLine>::const_iterator it = m_wLines.begin();
    const QList<WireLine>::const_iterator end = m_wLines.end();

    for(; it != end; ++it , ++retVal) {
        if(it->boundingRect().contains(pos)) {
            return retVal;
        }
    }

    return -1;
}

namespace Qucs
{
    Wire::Data readWireData(Qucs::XmlReader *reader)
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
                    data.wLines << WireLine(x1, y1, x2, y2);

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
}
