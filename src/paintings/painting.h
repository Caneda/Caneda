/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef PAINTING_H
#define PAINTING_H

#include "graphicsitem.h"

// Forward declarations
class QBrush;
class QPen;

namespace Caneda
{
    //! \brief Resize handles displayed while selecting a painting item.
    enum ResizeHandle {
        NoHandle = 0,
        TopLeftHandle = 1,     //0001
        TopRightHandle = 2,    //0010
        BottomRightHandle = 4, //0100
        BottomLeftHandle = 8   //1000
    };

    // Declare resize handles as flags to allow bitwise operations
    Q_DECLARE_FLAGS(ResizeHandles, ResizeHandle)
    Q_DECLARE_OPERATORS_FOR_FLAGS(Caneda::ResizeHandles)

    /*!
     * \brief The Painting class forms part of one of the GraphicsItem derived
     * classes available on Caneda. It is the base class for all painting
     * related items, like lines, rectangles, ellipses, etc.
     *
     * This class also takes care of resize handles. All the derived classes
     * will be passed a rectangle and they should use this rectangle as a hint
     * to draw. For example, a line can use the topleft and bottom right of the
     * rectangle to represent itself. The rectangle is set using
     * \a setPaintingRect(). The mouse functionalities corresponding to resize
     * handles are also handled by this class.
     *
     * \sa GraphicsItem
     */
    class Painting : public GraphicsItem
    {
    public:
        explicit Painting(GraphicsScene *scene = 0);

        static Painting* fromName(const QString& name);

        //! \copydoc GraphicsItem::Type
        enum { Type = GraphicsItem::PaintingType };
        //! \copydoc GraphicsItem::type()
        int type() const { return Type; }

        enum PaintingType {
            ArrowType = GraphicsItem::PaintingType + 1,
            EllipseType,
            EllipseArcType,
            GraphicLineType,
            GraphicTextType,
            RectangleType,
            LayerType
        };

        //! Returns paintingRect of this painting item.
        QRectF paintingRect() const { return m_paintingRect; }
        void setPaintingRect(const QRectF& rect);

        //! Returns the adjusted shape to a given \a rect.
        virtual QPainterPath shapeForRect(const QRectF& rect) const;

        //! Returns the pen with which the item is drawn.
        QPen pen() const { return m_pen; }
        virtual void setPen(const QPen& _pen);

        //! Returns the brush with which the item is drawn.
        QBrush brush() const { return m_brush; }
        virtual void setBrush(const QBrush& _brush);

        void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

        //! \copydoc GraphicsItem::copy()
        virtual Painting* copy(GraphicsScene *scene = 0) const = 0;
        virtual void copyDataTo(Painting *painting) const;

        //! \copydoc GraphicsItem::launchPropertiesDialog()
        virtual int launchPropertiesDialog() = 0;

    protected:
        /*!
         * \brief Subclasses should reimplement to do calculations this is
         * notified usually in call \a setPaintingRect.
         */
        virtual void geometryChange() {}

        void adjustGeometry();

        void setResizeHandles(Caneda::ResizeHandles handles);

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    private:
        // Resize handle related methods
        const QRectF handleRect() { return QRect(-5, -5, 10, 10); }

        void drawResizeHandle(const QPointF &centrePos, QPainter *painter);
        void drawResizeHandles(Caneda::ResizeHandles handles, const QRectF& rect, QPainter *painter);

        Caneda::ResizeHandle handleHitTest(const QPointF& point, Caneda::ResizeHandles handles,
                                           const QRectF& rect);

        //! \brief Returns the previously stored geometry required for undo/redo.
        QRectF storedPaintingRect() const { return m_store; }
        //! \brief Stores the item's geometry required for undo/redo
        void storePaintingRect() { m_store = m_paintingRect; }

        /*!
         * \brief Represents the rectangle containing the painting item.
         *
         * For example, a line can use its topleft and bottom right to conform a
         * rectangle to represent itself. This is not the same as the bounding rect,
         * as the latter also includes resizehandles.
         */
        QRectF m_paintingRect;
        QPen m_pen;
        QBrush m_brush;

        Caneda::ResizeHandles m_resizeHandles;
        Caneda::ResizeHandle m_activeHandle;

        QRectF m_store;
    };

} // namespace Caneda

#endif //PAINTING_H
