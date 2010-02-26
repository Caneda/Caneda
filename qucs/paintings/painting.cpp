/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "paintings.h"
#include "schematicscene.h"
#include "styledialog.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

//! Constructs a painting item with default pen and default brush.
Painting::Painting(SchematicScene *scene) : QucsItem(0, scene),
    m_pen(defaultPaintingPen),
    m_brush(defaultPaintingBrush),
    m_resizeHandles(Qucs::NoHandle),
    m_activeHandle(Qucs::NoHandle)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
#if QT_VERSION >= 0x040600
    setFlag(ItemSendsGeometryChanges, true);
    setFlag(ItemSendsScenePositionChanges, true);
#endif
}

//! Destructor
Painting::~Painting()
{
}

/*!
 * \brief Sets the painting rect to \a rect.
 *
 * \copydoc Painting::m_paintingRect
 */
void Painting::setPaintingRect(const QRectF& rect)
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
QPainterPath Painting::shapeForRect(const QRectF& rect) const
{
    QPainterPath path;
    path.addRect(rect);
    return path;
}

//! Sets item's pen to \a _pen.
void Painting::setPen(const QPen& _pen)
{
    if(m_pen == _pen) {
        return;
    }

    prepareGeometryChange();
    m_pen = _pen;

    adjustGeometry();
}

//! Sets item's brush to \a _brush.
void Painting::setBrush(const QBrush& _brush)
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
void Painting::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
        QWidget *)
{
    if(option->state & QStyle::State_Selected) {
        Qucs::drawResizeHandles(m_resizeHandles, m_paintingRect, painter);
    }
}

//!\brief Indicate the resize handles to be shown.
void Painting::setResizeHandles(Qucs::ResizeHandles handles)
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
Painting* Painting::fromName(const QString& name)
{
    static QRectF rect(-30, -30, 90, 60);

    //check if name begins with capital letter and if so use the following.
    //This happens when painting is placed by selecting in sidebar.
    if(name.at(0).isUpper()) {
        if(name == QObject::tr("Line")) {
            return new GraphicLine(QLineF(rect.bottomLeft(), rect.topRight()));
        }
        else if(name == QObject::tr("Arrow")) {
            return new Arrow(QLineF(rect.bottomLeft(), rect.topRight()));
        }
        else if(name == QObject::tr("Ellipse")) {
            return new Ellipse(rect);
        }
        else if(name == QObject::tr("Rectangle")) {
            return new Rectangle(rect);
        }
        else if(name == QObject::tr("Elliptic Arc")) {
            return new EllipseArc(rect, 100, 300);
        }
        else if(name == QObject::tr("Text")) {
            return new GraphicText;
        }
    }

    // This is true usually when painting is being read from xml file.
    else {
        if(name == QLatin1String("line")) {
            return new GraphicLine(QLineF(rect.bottomLeft(), rect.topRight()));
        }
        else if(name == QLatin1String("arrow")) {
            return new Arrow(QLineF(rect.bottomLeft(), rect.topRight()));
        }
        else if(name == QLatin1String("ellipse")) {
            return new Ellipse(rect);
        }
        else if(name == QLatin1String("rectangle")) {
            return new Rectangle(rect);
        }
        else if(name == QLatin1String("ellipseArc")) {
            return new EllipseArc(rect, 100, 300);
        }
        else if(name == QLatin1String("text")) {
            return new GraphicText;
        }
    }
    return 0;
}

//! Reimplemented for convenience though it doesn't do actual work.
Painting* Painting::copy(SchematicScene *) const
{
    return 0;
}

//! \copydoc QucsItem::copyDataTo()
void Painting::copyDataTo(Painting *painting) const
{
    painting->setPen(pen());
    painting->setBrush(brush());
    QucsItem::copyDataTo(static_cast<QucsItem*>(painting));
}

/*!
 * \brief Loads and returns a pointer to new painting object as read
 * from \a reader. On failure returns null.
 */
Painting* Painting::loadPainting(Qucs::XmlReader *reader, SchematicScene *scene)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "painting");

    Painting *painting = Painting::fromName(reader->attributes().value("name").toString());
    if(painting) {
        painting->loadData(reader);
        if(scene) {
            scene->addItem(painting);
        }
    }
    return painting;
}

//! Adjust geometry of item to accommodate resize handles.
void Painting::adjustGeometry()
{
    QRectF boundRect = boundForRect(m_paintingRect);
    QPainterPath _shape = shapeForRect(m_paintingRect);

    // Now determine how to adjust bounding rect based on resize handles being used.

    if(m_resizeHandles.testFlag(Qucs::TopLeftHandle)) {
        QRectF rect = Qucs::handleRect.translated(m_paintingRect.topLeft());
        boundRect |= rect;
        _shape.addRect(rect);
    }

    if(m_resizeHandles.testFlag(Qucs::TopRightHandle)) {
        QRectF rect = Qucs::handleRect.translated(m_paintingRect.topRight());
        boundRect |= rect;
        _shape.addRect(rect);
    }

    if(m_resizeHandles.testFlag(Qucs::BottomLeftHandle)) {
        QRectF rect = Qucs::handleRect.translated(m_paintingRect.bottomLeft());
        boundRect |= rect;
        _shape.addRect(rect);

    }

    if(m_resizeHandles.testFlag(Qucs::BottomRightHandle)) {
        QRectF rect = Qucs::handleRect.translated(m_paintingRect.bottomRight());
        boundRect |= rect;
        _shape.addRect(rect);
    }

    setShapeAndBoundRect(_shape, boundRect, m_pen.widthF());
    update();
}

//! Takes care of handle resizing on mouse press event.
void Painting::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_activeHandle = Qucs::NoHandle;

    if(event->buttons().testFlag(Qt::LeftButton)) {
        m_activeHandle = handleHitTest(event->pos(), m_resizeHandles, m_paintingRect);
    }

    //call base method to get move behaviour as no handle is pressed
    if(m_activeHandle == Qucs::NoHandle) {
        QucsItem::mousePressEvent(event);
    }
    else {
        storePaintingRect();
    }
}

//! Takes care of handle resizing on mouse move event.
void Painting::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_activeHandle == Qucs::NoHandle) {
        QucsItem::mouseMoveEvent(event);
        Q_ASSERT(scene()->mouseGrabberItem() == this);
        return;
    }

    if(event->buttons().testFlag(Qt::LeftButton)) {
        QRectF rect = m_paintingRect;
        QPointF point = event->pos();

        switch(m_activeHandle) {
            case Qucs::TopLeftHandle:
                rect.setTopLeft(point);
                break;

            case Qucs::TopRightHandle:
                rect.setTopRight(point);
                break;

            case Qucs::BottomLeftHandle:
                rect.setBottomLeft(point);
                break;

            case Qucs::BottomRightHandle:
                rect.setBottomRight(point);
                break;

            case Qucs::NoHandle:
                break;
        }

        setPaintingRect(rect);
    }
}

//! Takes care of handle resizing on mouse release event.
void Painting::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QucsItem::mouseReleaseEvent(event);
    if(m_activeHandle != Qucs::NoHandle && m_paintingRect != m_store) {
        schematicScene()->undoStack()->push(
                new PaintingRectChangeCmd(this, storedPaintingRect(), m_paintingRect));
    }
    m_activeHandle = Qucs::NoHandle;
}
