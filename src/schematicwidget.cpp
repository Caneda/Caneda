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

#include "schematicwidget.h"

#include "schematicdocument.h"
#include "schematicscene.h"
#include "schematicview.h"

#include <QWheelEvent>

namespace Caneda
{
    const qreal SchematicWidget::zoomFactor = 0.3;

    //! Constructor
    SchematicWidget::SchematicWidget(SchematicView *sv) :
        QGraphicsView(sv ? sv->schematicDocument()->schematicScene() : 0),
        m_schematicView(sv),
        m_zoomRange(0.30, 10.0),
        m_currentZoom(1.0)
    {
        centerOn(QPointF(0, 0));

        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        setAcceptDrops(true);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        setViewportUpdateMode(SmartViewportUpdate);
        setCacheMode(CacheBackground);
        setAlignment(static_cast<Qt::AlignmentFlag>(Qt::AlignLeft | Qt::AlignTop));
        setTransformationAnchor(QGraphicsView::NoAnchor);

        viewport()->setMouseTracking(true);
        viewport()->setAttribute(Qt::WA_NoSystemBackground);

        connect(schematicScene(), SIGNAL(mouseActionChanged()), this,
                SLOT(onMouseActionChanged()));

        // Update current drag mode
        onMouseActionChanged();
    }

    //! Destructor
    SchematicWidget::~SchematicWidget()
    {
    }

    SchematicView* SchematicWidget::schematicView() const
    {
        return m_schematicView;
    }

    SchematicScene* SchematicWidget::schematicScene() const
    {
        SchematicScene* s = qobject_cast<SchematicScene*>(scene());
        Q_ASSERT(s);// This should never fail!
        return s;
    }

    void SchematicWidget::zoomIn()
    {
        qreal newZoom = m_currentZoom * (1 + zoomFactor);
        setZoomLevel(qMin(newZoom, m_zoomRange.max));
    }

    void SchematicWidget::zoomOut()
    {
        qreal newZoom = m_currentZoom * (1 - zoomFactor);
        setZoomLevel(qMax(newZoom, m_zoomRange.min));
    }

    void SchematicWidget::zoomFitInBest()
    {
        if (scene()) {
            zoomFitRect(scene()->itemsBoundingRect());
        }
    }

    void SchematicWidget::zoomOriginal()
    {
        setZoomLevel(1.0);
    }

    void SchematicWidget::zoomFitRect(const QRectF &rect)
    {
        if (rect.isEmpty()) {
            return;
        }

        // Find the ideal x / y scaling ratio to fit \a rect in the view.
        QRectF viewRect = viewport()->rect();
        viewRect = transform().mapRect(viewRect);
        if (viewRect.isEmpty()) {
            return;
        }

        QRectF sceneRect = transform().mapRect(rect);
        if (sceneRect.isEmpty()) {
            return;
        }

        const qreal xratio = viewRect.width() / sceneRect.width();
        const qreal yratio = viewRect.height() / sceneRect.height();

        // Qt::KeepAspecRatio
        const qreal minRatio = qMin(xratio, yratio);

        // Also compute where the the view should be centered
        QPointF center = rect.center();

        // Now set that zoom level.
        setZoomLevel(minRatio, &center);
    }

    void SchematicWidget::mouseMoveEvent(QMouseEvent *event)
    {
        QPoint newCursorPos = mapToScene(event->pos()).toPoint();
        QString str = QString("%1 : %2")
            .arg(newCursorPos.x())
            .arg(newCursorPos.y());
        emit cursorPositionChanged(str);
        QGraphicsView::mouseMoveEvent(event);
    }

    void SchematicWidget::focusInEvent(QFocusEvent *event)
    {
        QGraphicsView::focusInEvent(event);
        if (hasFocus()) {
            emit focussedIn(this);
        }
    }

    void SchematicWidget::focusOutEvent(QFocusEvent *event)
    {
        QGraphicsView::focusOutEvent(event);
        if (!hasFocus()) {
            emit focussedOut(this);
        }
    }

    void SchematicWidget::onMouseActionChanged()
    {
        if (schematicScene()->mouseAction() == SchematicScene::Normal) {
            setDragMode(QGraphicsView::RubberBandDrag);
        } else {
            setDragMode(QGraphicsView::NoDrag);
        }
    }

    void SchematicWidget::setZoomLevel(qreal zoomLevel, QPointF *toCenter)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        QPointF currentCenter;
        if (toCenter) {
            currentCenter = *toCenter;
        } else {
            currentCenter = mapToScene(viewport()->rect().center());
        }

        m_currentZoom = zoomLevel;

        QTransform transform;
        transform.scale(m_currentZoom, m_currentZoom);
        setTransform(transform);

        centerOn(currentCenter);
    }

} // namespace Caneda
