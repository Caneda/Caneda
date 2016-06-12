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

#include "graphicsview.h"

#include "graphicsscene.h"

#include <QMouseEvent>

namespace Caneda
{
    /*!
     * \brief Constructs a new graphics view.
     *
     * \param scene Scene to point this view to.
     */
    GraphicsView::GraphicsView(GraphicsScene *scene) :
        QGraphicsView(scene),
        m_zoomFactor(0.3),
        m_zoomRange(0.30, 10.0),
        m_currentZoom(1.0),
        panMode(false)
    {
        centerOn(QPointF(0, 0));

        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        setAcceptDrops(true);
        setRenderHints(Caneda::DefaulRenderHints);
        setViewportUpdateMode(SmartViewportUpdate);
        setCacheMode(CacheBackground);
        setTransformationAnchor(QGraphicsView::NoAnchor);
        setMouseTracking(true);
        setAttribute(Qt::WA_NoSystemBackground);

        connect(scene, SIGNAL(mouseActionChanged(Caneda::MouseAction)),
                this, SLOT(onMouseActionChanged(Caneda::MouseAction)));

        // Update current drag mode
        onMouseActionChanged(Caneda::Normal);
    }

    GraphicsScene* GraphicsView::graphicsScene() const
    {
        GraphicsScene* s = qobject_cast<GraphicsScene*>(scene());
        return s;
    }

    void GraphicsView::zoomIn()
    {
        qreal newZoom = m_currentZoom * (1 + m_zoomFactor);
        setZoomLevel(qMin(newZoom, m_zoomRange.max));
    }

    void GraphicsView::zoomOut()
    {
        qreal newZoom = m_currentZoom / (1 + m_zoomFactor);
        setZoomLevel(qMax(newZoom, m_zoomRange.min));
    }

    void GraphicsView::zoomFitInBest()
    {
        if(scene()) {
            zoomFitRect(scene()->itemsBoundingRect());
        }
    }

    void GraphicsView::zoomOriginal()
    {
        setZoomLevel(1.0);
    }

    void GraphicsView::zoomFitRect(const QRectF &rect)
    {
        if(rect.isEmpty()) {
            return;
        }

        // Find the best scale radio to fit in the view.
        const qreal xratio = this->rect().width() / rect.width();
        const qreal yratio = this->rect().height() / rect.height();
        const qreal minRatio = qMin(xratio, yratio);

        // Save the position to center on after the zoom operation.
        QPointF center = rect.center();

        // Now set that zoom level and center the result.
        setZoomLevel(minRatio);
        centerOn(center);
    }

    void GraphicsView::mousePressEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::MiddleButton) {
            panMode = true;
            panStartPosition = mapToScene(event->pos());

            setCursor(Qt::ClosedHandCursor);

            event->accept();
            return;
        }

        QGraphicsView::mousePressEvent(event);
    }

    void GraphicsView::mouseMoveEvent(QMouseEvent *event)
    {
        if(panMode) {
            QPointF d = mapToScene(event->pos()) - panStartPosition;
            translate(d.x(), d.y());
            panStartPosition = mapToScene(event->pos());
        }

        QPoint newCursorPos = mapToScene(event->pos()).toPoint();
        QString str = QString("%1 : %2")
            .arg(newCursorPos.x())
            .arg(newCursorPos.y());
        emit cursorPositionChanged(str);

        QGraphicsView::mouseMoveEvent(event);
    }

    void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::MiddleButton) {
            panMode = false;

            setCursor(Qt::ArrowCursor);
        }

        QGraphicsView::mouseReleaseEvent(event);
    }

    void GraphicsView::focusInEvent(QFocusEvent *event)
    {
        QGraphicsView::focusInEvent(event);
        if(hasFocus()) {
            emit focussedIn(this);
        }
    }

    void GraphicsView::focusOutEvent(QFocusEvent *event)
    {
        QGraphicsView::focusOutEvent(event);
        if(!hasFocus()) {
            emit focussedOut(this);
        }
    }

    /*!
     * \brief Update the mouse action mode.
     *
     * This method updates the mouse action mode, taking care of setting the
     * correct mouse drag mode. That enables the RubberBandDrag mouse mode
     * during mouse selection, and disables it on every other mouse action.
     */
    void GraphicsView::onMouseActionChanged(MouseAction mouseAction)
    {
        if(mouseAction == Caneda::Normal) {
            setDragMode(QGraphicsView::RubberBandDrag);
        }
        else {
            setDragMode(QGraphicsView::NoDrag);
        }
    }

    void GraphicsView::setZoomLevel(qreal zoomLevel)
    {
        if(!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        // If zoom is perform with any method but the mouse
        // (QGraphicsView::AnchorUnderMouse), set anchor to the center
        if(transformationAnchor() != QGraphicsView::AnchorUnderMouse) {
            setTransformationAnchor(QGraphicsView::AnchorViewCenter);  // Set graphicsview anchor to the center
        }

        // Scale in proportion to current zoom level and set new currentZoom
        scale(zoomLevel/m_currentZoom, zoomLevel/m_currentZoom);
        m_currentZoom = zoomLevel;

        setTransformationAnchor(QGraphicsView::NoAnchor);  // Restore graphicsview anchor to be able to move afterwards
    }

} // namespace Caneda
