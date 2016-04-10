/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2016 by Pablo Daniel Pareja Obregon                  *
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
#include "port.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QGraphicsSceneEvent>
#include <QMenu>

namespace Caneda
{
    /*!
     * \brief Constructs a new graphics item and adds it to a scene.
     *
     * \param parent Parent of the item.
     * \param scene CGraphicsScene where to add the item.
     */
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
     * \brief Constructs and returns a context menu with the actions
     * corresponding to the selected object.
     */
    void CGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();

        // Launch context menu of item
        _menu->addAction(am->actionForName("editCut"));
        _menu->addAction(am->actionForName("editCopy"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editRotate"));
        _menu->addAction(am->actionForName("editMirror"));
        _menu->addAction(am->actionForName("editMirrorY"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("propertiesDialog"));

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

        if(axis == Qt::XAxis) {
            setTransform(QTransform::fromScale(1.0, -1.0), true);
        }
        else {
            setTransform(QTransform::fromScale(-1.0, 1.0), true);
        }
    }

    //! \brief Rotate item by -90 degrees
    void CGraphicsItem::rotate90(Caneda::AngleDirection dir)
    {
        if(dir == Caneda::AntiClockwise) {
            setRotation(rotation() - 90.0);
        }
        else {
            setRotation(rotation() + 90.0);
        }
    }

    /*!
     * \fn CGraphicsItem::copy()
     *
     * \brief Returns a copy of the current item parented to \a scene.
     *
     * Subclasses should reimplement this method to return the appropriate
     * copy of the reimplemented item.
     */

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
        item->setRotation(rotation());
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
