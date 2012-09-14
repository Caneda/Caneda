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

#include "cgraphicsitem.h"

#include "actionmanager.h"
#include "cgraphicsscene.h"
#include "cgraphicsview.h"
#include "port.h"
#include "xmlutilities.h"

#include <QGraphicsSceneEvent>
#include <QMenu>
#include <QStyleOptionGraphicsItem>
#include <QXmlStreamWriter>

namespace Caneda
{
    /*!
     * \brief Utility function to draw resize handle for painting and other items.
     *
     * \param centrePos The point which will be center for the handle rectangle
     *                  drawn.
     * \param painter   The painter used to draw handle rectangle.
     */
    void drawResizeHandle(const QPointF &centrePos, QPainter *painter)
    {
        QPen savedPen = painter->pen();
        QBrush savedBrush = painter->brush();

        painter->setPen(Caneda::handlePen);
        painter->setBrush(Caneda::handleBrush);

        // handleRect is defined as QRectF(-w/2, -h/2, w, h)
        painter->drawRect(Caneda::handleRect.translated(centrePos));

        painter->setPen(savedPen);
        painter->setBrush(savedBrush);
    }

    /*!
     * \brief Utility method to draw four resize handles along four corners of rectangle passed.
     *
     * \param handles  The handle areas where handle rectangles neeed to be drawn.
     * \param rect     The rectangle around which resize handles are to be drawn.
     * \param painter  The painter used to draw resize handles.
     */
    void drawResizeHandles(ResizeHandles handles, const QRectF& rect, QPainter *painter)
    {
        if(handles.testFlag(Caneda::TopLeftHandle)) {
            drawResizeHandle(rect.topLeft(), painter);
        }

        if(handles.testFlag(Caneda::TopRightHandle)) {
            drawResizeHandle(rect.topRight(), painter);
        }

        if(handles.testFlag(Caneda::BottomLeftHandle)) {
            drawResizeHandle(rect.bottomLeft(), painter);
        }

        if(handles.testFlag(Caneda::BottomRightHandle)) {
            drawResizeHandle(rect.bottomRight(), painter);
        }
    }

    /*!
     * \brief Utility method that returns the resize handle area around rect for the point passed.
     *
     * \param point   The point to be tested for collision with handle rectangle around rect.
     * \param handles The bitmask indicating the handle areas to be tested.
     * \param rect    The rectangle around which resize handles are to be tested.
     */
    ResizeHandle handleHitTest(const QPointF& point, ResizeHandles handles,
            const QRectF& rect)
    {
        if(handles == Caneda::NoHandle) {
            return Caneda::NoHandle;
        }

        if(handles.testFlag(Caneda::TopLeftHandle)) {
            if(Caneda::handleRect.translated(rect.topLeft()).contains(point)) {
                return Caneda::TopLeftHandle;
            }
        }

        if(handles.testFlag(Caneda::TopRightHandle)) {
            if(Caneda::handleRect.translated(rect.topRight()).contains(point)) {
                return Caneda::TopRightHandle;
            }
        }

        if(handles.testFlag(Caneda::BottomLeftHandle)){
            if(Caneda::handleRect.translated(rect.bottomLeft()).contains(point)) {
                return Caneda::BottomLeftHandle;
            }
        }

        if(handles.testFlag(Caneda::BottomRightHandle)){
            if(Caneda::handleRect.translated(rect.bottomRight()).contains(point)) {
                return Caneda::BottomRightHandle;
            }
        }

        return Caneda::NoHandle;
    }

    //! Constructor
    //! \brief Create a new item and add to scene.
    CGraphicsItem::CGraphicsItem(QGraphicsItem* parent, CGraphicsScene* scene) :
        QGraphicsItem(parent),
        m_boundingRect(0, 0, 0, 0)
    {
        m_shape.addRect(m_boundingRect);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        if(scene) {
            scene->addItem(this);
        }
    }

    /*!
     * \brief Check for connections and connect the coinciding ports.
     *
     * \return Returns the number of connections made.
     */
    int CGraphicsItem::checkAndConnect(Caneda::UndoOption opt)
    {
        int num_of_connections = 0;

        // Find existing intersecting ports and connect
        if(opt == Caneda::PushUndoCmd) {
            cGraphicsScene()->undoStack()->beginMacro(QString());
        }

        foreach(Port *port, m_ports) {
            Port *other = port->findCoincidingPort();
            if(other) {
                if(opt == Caneda::PushUndoCmd) {
                    ConnectCmd *cmd = new ConnectCmd(port, other, cGraphicsScene());
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

    /*!
     * \brief Context menu
     */
    void CGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();

        //launch context menu of item
        _menu->addAction(am->actionForName("editCut"));
        _menu->addAction(am->actionForName("editCopy"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editRotate"));
        _menu->addAction(am->actionForName("editMirror"));
        _menu->addAction(am->actionForName("editMirrorY"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editDelete"));

        _menu->exec(event->screenPos());
    }

    void CGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->buttons().testFlag(Qt::LeftButton)) {
            launchPropertyDialog(Caneda::PushUndoCmd);
        }
    }

    /*!
     * \brief Sets the shape cache as well as boundbox cache
     *
     * This method abstracts the method of changing the geometry with support for
     * cache as well.
     *
     * \param path The path to be cached. If empty, bound rect is added.
     * \param rect The bound rect to be cached.
     * \param pw Pen width of pen used to paint outer stroke of item.
     */
    void CGraphicsItem::setShapeAndBoundRect(const QPainterPath& path,
            const QRectF& rect, qreal pw)
    {
        // Inform scene about change in geometry.
        prepareGeometryChange();

        // Adjust the bounding rect by pen width as required by graphicsview.
        m_boundingRect = rect;
        m_boundingRect.adjust(-pw, -pw, pw, pw);

        m_shape = path;
        if(m_shape.isEmpty()) {
            // If path is empty just add the bounding rect to the path.
            m_shape.addRect(m_boundingRect);
        }
    }

    //! \brief returns a pointer to the graphics scene to which the item belongs.
    CGraphicsScene* CGraphicsItem::cGraphicsScene() const
    {
        return qobject_cast<CGraphicsScene*>(scene());
    }

    /*!
     * \brief Convenience method to get the saved text as string.
     *
     * Though this is simple, this method shouldn't be used in too many places as
     * there will be unnecessary creation of xml writer and reader instances which
     * will render the program inefficient.
     */
    QString CGraphicsItem::saveDataText() const
    {
        QString retVal;
        Caneda::XmlWriter writer(&retVal);
        saveData(&writer);
        return retVal;
    }

    /*!
     * \brief Convenience method to just load data from string.
     *
     * Though this is simple, this method shouldn't be used in too many places as
     * there will be unnecessary creation of xml writer and reader instances which
     * will render the program inefficient.
     */
    void CGraphicsItem::loadDataFromText(const QString &text)
    {
        Caneda::XmlReader reader(text.toUtf8());
        while(!reader.atEnd()) {
            // skip until end element is found.
            reader.readNext();

            if(reader.isEndElement()) {
                break;
            }

            if(reader.isStartElement()) {
                loadData(&reader);
            }
        }
    }

    /*!
     * \brief Graphically mirror item according to x axis
     * \note Items can be mirrored only along x and y axis.
     */
    void CGraphicsItem::mirrorAlong(Qt::Axis axis)
    {
        update();

        Q_ASSERT(axis == Qt::XAxis || axis == Qt::YAxis);
        if(axis == Qt::XAxis) {
            scale(1.0, -1.0);
        }
        else /*axis = Qt::YAxis*/ {
            scale(-1.0, 1.0);
        }
    }

    //! \brief Rotate item by -90 degrees
    void CGraphicsItem::rotate90(Caneda::AngleDirection dir)
    {
        rotate(dir == Caneda::AntiClockwise ? -90.0 : 90.0);
    }

    /*!
     * \brief This returns a copy of the current item parented to \a scene.
     *
     * Now it returns null but subclasses should reimplement this to return the
     * appropriate copy of that reimplemented item.
     */
    CGraphicsItem* CGraphicsItem::copy(CGraphicsScene *) const
    {
        return 0;
    }

    /*!
     * \brief Copies data of current-item to \a item.
     *
     * Sublasses should reimplement it to copy their data.
     */
    void CGraphicsItem::copyDataTo(CGraphicsItem *item) const
    {
        item->setTransform(transform());
        item->prepareGeometryChange();
        item->m_boundingRect = m_boundingRect;
        item->m_shape = m_shape;
        item->setPos(pos());
    }

    /*!
     * \brief Stores the item's current position in data field of item.
     *
     * This method is required for handling undo/redo.
     */
    void storePos(QGraphicsItem *item, const QPointF &pos)
    {
        item->setData(PointKey, QVariant(pos));
    }

    /*!
     * \brief Returns the stored point by fetching from item's data field.
     *
     * This method is required for handling undo/redo.
     */
    QPointF storedPos(QGraphicsItem *item)
    {
        return item->data(PointKey).toPointF();
    }

} // namespace Caneda
