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

#include "item.h"
#include "schematicdocument.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "xmlformat.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QWheelEvent>

namespace Caneda
{
    const qreal SchematicWidget::zoomFactor = 1.2f;

    //! Constructor
    SchematicWidget::SchematicWidget(SchematicView *sv) :
        QGraphicsView(sv ? sv->schematicDocument()->schematicScene() : 0),
        m_schematicView(sv),
        m_horizontalScroll(0),
        m_verticalScroll(0)
    {
        setAcceptDrops(true);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        setViewportUpdateMode(SmartViewportUpdate);
        setCacheMode(CacheBackground);

        viewport()->setMouseTracking(true);
        viewport()->setAttribute(Qt::WA_NoSystemBackground);
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

    void SchematicWidget::saveScrollState()
    {
        m_horizontalScroll = horizontalScrollBar()->value();
        m_verticalScroll  = verticalScrollBar()->value();
    }

    void SchematicWidget::restoreScrollState()
    {
        horizontalScrollBar()->setValue(m_horizontalScroll);
        verticalScrollBar()->setValue(m_verticalScroll);
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
            emit focussed(this);
        }
    }

    qreal SchematicWidget::fit(const QRectF &rect)
    {
        if (!scene() || !m_schematicView || rect.isNull()) {
            return 1.0;
        }

        qreal savedZoom = m_schematicView->currentZoom();

        // Reset the view scale to 1:1.
        QRectF unity = transform().mapRect(QRectF(0, 0, 1, 1));
        if (unity.isEmpty())
            return 1.0;
        scale(1 / unity.width(), 1 / unity.height());

        // Find the ideal x / y scaling ratio to fit \a rect in the view.
        int margin = 2;
        QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
        if (viewRect.isEmpty()) {
            m_schematicView->setZoomLevel(savedZoom);
            return 1.0;
        }
        QRectF sceneRect = transform().mapRect(rect);
        if (sceneRect.isEmpty()) {
            m_schematicView->setZoomLevel(savedZoom);
            return 1.0;
        }
        qreal xratio = viewRect.width() / sceneRect.width();
        qreal yratio = viewRect.height() / sceneRect.height();

        // Qt::KeepAspecRatio
        xratio = yratio = qMin(xratio, yratio);
        m_schematicView->setZoomLevel(savedZoom);

        return xratio;
    }

} // namespace Caneda
