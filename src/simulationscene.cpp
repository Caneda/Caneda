/***************************************************************************
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

#include "simulationscene.h"

#include <QUndoStack>
#include <QVBoxLayout>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace Caneda
{
    SimulationScene::SimulationScene(QWidget *parent) :
        QWidget(parent),
        m_zoomRange(0.30, 10.0),
        m_zoomFactor(0.3),
        m_currentZoom(1.0)

    {
        // Setup undo stack
        m_undoStack = new QUndoStack(this);

        QwtPlot *waveform = new QwtPlot(this);

        // Add curves
        // QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
        // QwtPlotCurve *curve2 = new QwtPlotCurve("Curve 2");

        // Copy the data into the curves
        // curve1->setData();
        // curve2->setData();

        // curve1->attach(waveform);
        // curve2->attach(waveform);

        // Finally, refresh the plot
        waveform->replot();

        connect(undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(setModified(bool)));

        QVBoxLayout *vlayout = new QVBoxLayout();
        vlayout->addWidget(waveform);
        setLayout(vlayout);
    }

    SimulationScene::~SimulationScene()
    {
        delete m_undoStack;
    }

    void SimulationScene::zoomIn()
    {
        qreal newZoom = m_currentZoom * (1 + m_zoomFactor);
        setZoomLevel(qMin(newZoom, m_zoomRange.max));
    }

    void SimulationScene::zoomOut()
    {
        qreal newZoom = m_currentZoom / (1 + m_zoomFactor);
        setZoomLevel(qMax(newZoom, m_zoomRange.min));
    }

    void SimulationScene::zoomFitInBest()
    {
        // TODO: Reimplement this
        // zoomFitRect(itemsBoundingRect());
    }

    void SimulationScene::zoomOriginal()
    {
        setZoomLevel(1.0);
    }

    void SimulationScene::zoomFitRect(const QRectF &rect)
    {
        // TODO: Reimplement this
//        if (rect.isEmpty()) {
//            return;
//        }

//        // Find the ideal x / y scaling ratio to fit \a rect in the view.
//        QRectF viewRect = viewport()->rect();
//        viewRect = transform().mapRect(viewRect);
//        if (viewRect.isEmpty()) {
//            return;
//        }

//        QRectF sceneRect = transform().mapRect(rect);
//        if (sceneRect.isEmpty()) {
//            return;
//        }

//        const qreal xratio = viewRect.width() / sceneRect.width();
//        const qreal yratio = viewRect.height() / sceneRect.height();

//        // Qt::KeepAspecRatio
//        const qreal minRatio = qMin(xratio, yratio);

//        // Also compute where the the view should be centered
//        QPointF center = rect.center();

//        // Now set that zoom level.
//        setZoomLevel(minRatio, &center);
    }

    /*!
     * \brief Set whether this scene is modified or not
     *
     * This method emits the signal changed(bool)
     *
     * \param m True/false to set it to unmodified/modified.
     */
    void SimulationScene::setModified(const bool m)
    {
        if(m_modified != !m) {
            m_modified = !m;
            emit changed();
        }
    }

    void SimulationScene::repaint()
    {
        // TODO: rapaint each plot via myPlot->replot();
        QWidget::repaint();
    }

    void SimulationScene::mouseMoveEvent(QMouseEvent *event)
    {
        // TODO: Get cursor position in current plot to display correct coordinates
        QPoint newCursorPos = QPoint(0, 0);

        QString str = QString("%1 : %2")
            .arg(newCursorPos.x())
            .arg(newCursorPos.y());
        emit cursorPositionChanged(str);
        QWidget::mouseMoveEvent(event);
    }

    void SimulationScene::focusInEvent(QFocusEvent *event)
    {
        QWidget::focusInEvent(event);
        if (hasFocus()) {
            emit focussedIn(this);
        }
    }

    void SimulationScene::focusOutEvent(QFocusEvent *event)
    {
        QWidget::focusOutEvent(event);
        if (!hasFocus()) {
            emit focussedOut(this);
        }
    }

    void SimulationScene::setZoomLevel(qreal zoomLevel, QPointF *toCenter)
    {
        // TODO: Reimplement this
//        if (!m_zoomRange.contains(zoomLevel)) {
//            return;
//        }

//        QPointF currentCenter;
//        if (toCenter) {
//            currentCenter = *toCenter;
//        } else {
//            currentCenter = mapToScene(viewport()->rect().center());
//        }

//        m_currentZoom = zoomLevel;

//        QTransform transform;
//        transform.scale(m_currentZoom, m_currentZoom);
//        setTransform(transform);

//        centerOn(currentCenter);
    }

} // namespace Caneda
