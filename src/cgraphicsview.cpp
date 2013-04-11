/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#include "cgraphicsview.h"

#include "cgraphicsscene.h"

#include <QMouseEvent>

namespace Caneda
{
    //! Constructor
    CGraphicsView::CGraphicsView(CGraphicsScene *sv) :
        QGraphicsView(sv ? sv : 0),
        m_zoomRange(0.30, 10.0),
        m_zoomFactor(0.3),
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
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        setMouseTracking(true);
        setAttribute(Qt::WA_NoSystemBackground);

        connect(cGraphicsScene(), SIGNAL(mouseActionChanged()), this,
                SLOT(onMouseActionChanged()));

        // Update current drag mode
        onMouseActionChanged();
    }

    //! Destructor
    CGraphicsView::~CGraphicsView()
    {
    }

    CGraphicsScene* CGraphicsView::cGraphicsScene() const
    {
        CGraphicsScene* s = qobject_cast<CGraphicsScene*>(scene());
        Q_ASSERT(s);// This should never fail!
        return s;
    }

    void CGraphicsView::zoomIn()
    {
        qreal newZoom = m_currentZoom * (1 + m_zoomFactor);
        setZoomLevel(qMin(newZoom, m_zoomRange.max));
    }

    void CGraphicsView::zoomOut()
    {
        qreal newZoom = m_currentZoom / (1 + m_zoomFactor);
        setZoomLevel(qMax(newZoom, m_zoomRange.min));
    }

    void CGraphicsView::zoomFitInBest()
    {
        if (scene()) {
            zoomFitRect(scene()->itemsBoundingRect());
        }
    }

    void CGraphicsView::zoomOriginal()
    {
        setZoomLevel(1.0);
    }

    void CGraphicsView::zoomFitRect(const QRectF &rect)
    {
        if (rect.isEmpty()) {
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

    void CGraphicsView::mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::MiddleButton) {
            panMode = true;
            panStartPosition = mapToScene(event->pos());

            setCursor(Qt::ClosedHandCursor);

            event->accept();
            return;
        }

        QGraphicsView::mousePressEvent(event);
    }

    void CGraphicsView::mouseMoveEvent(QMouseEvent *event)
    {
        if (panMode) {
            setTransformationAnchor(QGraphicsView::NoAnchor);  // Remove temporarily the anchor to be able to move

            QPointF d = mapToScene(event->pos()) - panStartPosition;
            translate(d.x(), d.y());
            panStartPosition = mapToScene(event->pos());

            setTransformationAnchor(QGraphicsView::AnchorViewCenter);  // Restore graphicsview anchor to the center
        }

        QPoint newCursorPos = mapToScene(event->pos()).toPoint();
        QString str = QString("%1 : %2")
            .arg(newCursorPos.x())
            .arg(newCursorPos.y());
        emit cursorPositionChanged(str);

        QGraphicsView::mouseMoveEvent(event);
    }

    void CGraphicsView::mouseReleaseEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::MiddleButton) {
            panMode = false;

            setCursor(Qt::ArrowCursor);

            event->accept();
            return;
        }

        QGraphicsView::mouseReleaseEvent(event);
    }

    void CGraphicsView::focusInEvent(QFocusEvent *event)
    {
        QGraphicsView::focusInEvent(event);
        if (hasFocus()) {
            emit focussedIn(this);
        }
    }

    void CGraphicsView::focusOutEvent(QFocusEvent *event)
    {
        QGraphicsView::focusOutEvent(event);
        if (!hasFocus()) {
            emit focussedOut(this);
        }
    }

    void CGraphicsView::onMouseActionChanged()
    {
        if (cGraphicsScene()->mouseAction() == Caneda::Normal) {
            setDragMode(QGraphicsView::RubberBandDrag);
        } else {
            setDragMode(QGraphicsView::NoDrag);
        }
    }

    void CGraphicsView::setZoomLevel(qreal zoomLevel)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        // Scale in proportion to current zoom level and set new currentZoom
        scale(zoomLevel/m_currentZoom, zoomLevel/m_currentZoom);
        m_currentZoom = zoomLevel;
    }

} // namespace Caneda
