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

#ifndef PAINTING_H
#define PAINTING_H

#include "item.h"

#include <QBrush>
#include <QPen>

static const QPen defaultPaintingPen(Qt::black);
static const QBrush defaultPaintingBrush(Qt::NoBrush);

/*!
 * \brief This class is base for painting items like lines, rectangles..
 *
 * This class also takes care of resize handles. All the derived classes will be
 * passed a rectangle and they should use this rectangle as a hint to draw.
 * For example, a line can use the topleft and bottom right of the rectangle to
 * represent itself. The rectangle is set using \a setPaintingRect.
 * The mouse functionalities corresponding to resize handles are also handled
 * by this class.
 */
class Painting : public QucsItem
{
public:
    enum {
        NoPaintingType = 0,
        Type = QucsItem::PaintingType
    };

    enum PaintingType {
        ArrowType = QucsItem::PaintingType + 1,
        EllipseType,
        EllipseArcType,
        GraphicLineType,
        GraphicTextType,
        IdTextType,
        PortSymbolType,
        RectangleType
    };

    Painting(SchematicScene *scene = 0);
    ~Painting();

    //! \copydoc QucsItem::type()
    int type() const { return Type; }

    /*!
     * \brief Returns paintingRect of this painting item.
     *
     * \copydoc Painting::m_paintingRect
     */
    QRectF paintingRect() const { return m_paintingRect; }
    void setPaintingRect(const QRectF& rect);

    virtual QPainterPath shapeForRect(const QRectF& rect) const;

    //! Returns the adjusted painting bound rect for paintingrect \a rect.
    virtual QRectF boundForRect(const QRectF& rect) const {
        return rect;
    }

    //! Returns the pen with which the item is drawn.
    QPen pen() const { return m_pen; }
    virtual void setPen(const QPen& _pen);

    //! Returns the brush with which the item is drawn.
    QBrush brush() const { return m_brush; }
    virtual void setBrush(const QBrush& _brush);

    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

    //! Returns an OR representation of used resize handles.
    Qucs::ResizeHandles resizeHandles() const { return m_resizeHandles; }
    void setResizeHandles(Qucs::ResizeHandles handles);

    //! Returns the active handle i.e the one with mouse focus.
    Qucs::ResizeHandle activeHandle() const { return m_activeHandle; }

    Painting* copy(SchematicScene *scene = 0) const;

    virtual void copyDataTo(Painting *painting) const;

    static Painting* fromName(const QString& name);
    static Painting* loadPainting(Qucs::XmlReader *reader, SchematicScene *scene = 0);

    QRectF storedPaintingRect() const { return m_store; }
    void storePaintingRect() { m_store = paintingRect(); }

protected:
    /*!
     * Subclasses should reimplement to do calculations this is notified
     * usually in call \a setPaintingRect.
     */
    virtual void geometryChange() {}

    void adjustGeometry();

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    /*!
     * Represents the rectangle withing which painting should be drawn.
     *
     * For eg. GraphicLine can use topleft and bottom right of painting
     * rectangles to represent itself.
     * \note paintingRect is not same as bounding rect. The latter includes
     * resizehandles also.
     */
    QRectF m_paintingRect;
    QPen m_pen;
    QBrush m_brush;
    Qucs::ResizeHandles m_resizeHandles;
    Qucs::ResizeHandle m_activeHandle;
    QRectF m_store;
};

#endif //PAINTING_H
