/***************************************************************************
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

#include "layer.h"

#include "rectangle.h"
#include "schematicscene.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    //! Constructs a painting item with default pen and default brush.
    Layer::Layer(SchematicScene *scene) : SchematicItem(0, scene),
    m_pen(defaultLayerPen),
    m_brush(defaultLayerBrush),
    m_resizeHandles(Caneda::NoHandle),
    m_activeHandle(Caneda::NoHandle)
    {
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);
    }

    //! Destructor
    Layer::~Layer()
    {
    }

    /*!
     * \brief Sets the painting rect to \a rect.
     *
     * \copydoc Layer::m_paintingRect
     */
    void Layer::setPaintingRect(const QRectF& rect)
    {
        if(rect == m_paintingRect) {
            return;
        }

        prepareGeometryChange();

        m_paintingRect = rect;
        geometryChange();

        adjustGeometry();
    }

    /*!
     * \brief Returns shape of item for given painting rect.
     *
     * Subclasses can use this to customize their shapes.
     */
    QPainterPath Layer::shapeForRect(const QRectF& rect) const
    {
        QPainterPath path;
        path.addRect(rect);
        return path;
    }

    //! Sets item's pen to \a _pen.
    void Layer::setPen(const QPen& _pen)
    {
        if(m_pen == _pen) {
            return;
        }

        prepareGeometryChange();
        m_pen = _pen;

        adjustGeometry();
    }

    //! Sets item's brush to \a _brush.
    void Layer::setBrush(const QBrush& _brush)
    {
        if(m_brush == _brush) {
            return;
        }

        prepareGeometryChange();
        m_brush = _brush;

        //no need to adjust geometry as brush doesn't alter geometry
        update();
    }

    /*!
     * \brief Draws the handles if the item is selected.
     *
     * Subclasses should reimplement to draw itself with using paintingRect
     * as hint to draw. The subclassed paint method should also call this
     * base method in the end to get the resize handles drawn.
     */
    void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *)
    {
        if(option->state & QStyle::State_Selected) {
            Caneda::drawResizeHandles(m_resizeHandles, m_paintingRect, painter);
        }
    }

    //!\brief Indicate the resize handles to be shown.
    void Layer::setResizeHandles(Caneda::ResizeHandles handles)
    {
        if(m_resizeHandles == handles) {
            return;
        }

        prepareGeometryChange();
        m_resizeHandles = handles;

        adjustGeometry();
    }

    /*!
     * \brief Returns a pointer to new Painting object created appropriately
     * according to name.
     */
    Layer* Layer::fromName(const QString& name)
    {
        static QRectF rect(-30, -30, 90, 60);

        //check if name begins with capital letter and if so use the following.
        //This happens when painting is placed by selecting in sidebar.
        if(name.at(0).isUpper()) {
            if(name == QObject::tr("Rectangle")) {
                return new Rectangle(rect);
            }
        }

        // This is true usually when painting is being read from xml file.
        else {
            if(name == QLatin1String("rectangle")) {
                return new Rectangle(rect);
            }
        }
        return 0;
    }

    //! Reimplemented for convenience though it doesn't do actual work.
    Layer* Layer::copy(SchematicScene *) const
    {
        return 0;
    }

    //! \copydoc SchematicItem::copyDataTo()
    void Layer::copyDataTo(Layer *layer) const
    {
        layer->setPen(pen());
        layer->setBrush(brush());
        SchematicItem::copyDataTo(static_cast<SchematicItem*>(layer));
    }

    /*!
     * \brief Loads and returns a pointer to new painting object as read
     * from \a reader. On failure returns null.
     */
    Layer* Layer::loadLayer(Caneda::XmlReader *reader, SchematicScene *scene)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "layer");

        Layer *layer = Layer::fromName(reader->attributes().value("name").toString());
        if(layer) {
            layer->loadData(reader);
            if(scene) {
                scene->addItem(layer);
            }
        }
        return layer;
    }

    //! Adjust geometry of item to accommodate resize handles.
    void Layer::adjustGeometry()
    {
        QRectF boundRect = boundForRect(m_paintingRect);
        QPainterPath _shape = shapeForRect(m_paintingRect);

        // Now determine how to adjust bounding rect based on resize handles being used.

        if(m_resizeHandles.testFlag(Caneda::TopLeftHandle)) {
            QRectF rect = Caneda::handleRect.translated(m_paintingRect.topLeft());
            boundRect |= rect;
            _shape.addRect(rect);
        }

        if(m_resizeHandles.testFlag(Caneda::TopRightHandle)) {
            QRectF rect = Caneda::handleRect.translated(m_paintingRect.topRight());
            boundRect |= rect;
            _shape.addRect(rect);
        }

        if(m_resizeHandles.testFlag(Caneda::BottomLeftHandle)) {
            QRectF rect = Caneda::handleRect.translated(m_paintingRect.bottomLeft());
            boundRect |= rect;
            _shape.addRect(rect);

        }

        if(m_resizeHandles.testFlag(Caneda::BottomRightHandle)) {
            QRectF rect = Caneda::handleRect.translated(m_paintingRect.bottomRight());
            boundRect |= rect;
            _shape.addRect(rect);
        }

        setShapeAndBoundRect(_shape, boundRect, m_pen.widthF());
        update();
    }

    //! Takes care of handle resizing on mouse press event.
    void Layer::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        m_activeHandle = Caneda::NoHandle;

        if(event->buttons().testFlag(Qt::LeftButton)) {
            m_activeHandle = handleHitTest(event->pos(), m_resizeHandles, m_paintingRect);
        }

        //call base method to get move behaviour as no handle is pressed
        if(m_activeHandle == Caneda::NoHandle) {
            SchematicItem::mousePressEvent(event);
        }
        else {
            storePaintingRect();
        }
    }

    //! Takes care of handle resizing on mouse move event.
    void Layer::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        if(m_activeHandle == Caneda::NoHandle) {
            SchematicItem::mouseMoveEvent(event);
            Q_ASSERT(scene()->mouseGrabberItem() == this);
            return;
        }

        if(event->buttons().testFlag(Qt::LeftButton)) {
            QRectF rect = m_paintingRect;
            QPointF point = event->pos();

            switch(m_activeHandle) {
            case Caneda::TopLeftHandle:
                rect.setTopLeft(point);
                break;

            case Caneda::TopRightHandle:
                rect.setTopRight(point);
                break;

            case Caneda::BottomLeftHandle:
                rect.setBottomLeft(point);
                break;

            case Caneda::BottomRightHandle:
                rect.setBottomRight(point);
                break;

            case Caneda::NoHandle:
                break;
            }

            setPaintingRect(rect);
        }
    }

    //! Takes care of handle resizing on mouse release event.
    void Layer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        SchematicItem::mouseReleaseEvent(event);
        if(m_activeHandle != Caneda::NoHandle && m_paintingRect != m_store) {
            schematicScene()->undoStack()->push(
                    new LayerRectChangeCmd(this, storedPaintingRect(), m_paintingRect));
        }
        m_activeHandle = Caneda::NoHandle;
    }

} // namespace Caneda
