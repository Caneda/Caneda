/***************************************************************************
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

        // Create the new plot
        m_plot = new QwtPlot(this);

        connect(undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(setModified(bool)));

        QVBoxLayout *vlayout = new QVBoxLayout();
        vlayout->addWidget(m_plot);
        setLayout(vlayout);
    }

    SimulationScene::~SimulationScene()
    {
        delete m_undoStack;
    }

    /*!
     * \brief Adds or moves the item and all its childen to this scene. This
     * scene takes ownership of the item.
     */
    void SimulationScene::addItem(QwtPlotItem *item)
    {
        m_items.append(item);
    }

    //! \brief Displays all items available in the scene, in the plot widget.
    void SimulationScene::showAll()
    {
        // Attach the items to the plot
        foreach(QwtPlotItem *item, m_items) {
            item->attach(m_plot);
        }

        // Refresh the plot
        m_plot->replot();
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
        //! \todo Reimplement this
        // zoomFitRect(itemsBoundingRect());
    }

    void SimulationScene::zoomOriginal()
    {
        setZoomLevel(1.0);
    }

    void SimulationScene::zoomFitRect(const QRectF &rect)
    {
        //! \todo Reimplement this
//        if (rect.isEmpty()) {
//            return;
//        }

//        // Find the best scale radio to fit in the view.
//        const qreal xratio = this->rect().width() / rect.width();
//        const qreal yratio = this->rect().height() / rect.height();
//        const qreal minRatio = qMin(xratio, yratio);

//        // Save the position to center on after the zoom operation.
//        QPointF center = rect.center();

//        // Now set that zoom level and center the result.
//        setZoomLevel(minRatio);
//        centerOn(center);
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
        //! \todo Rapaint each plot via myPlot->replot();
        QWidget::repaint();
    }

    void SimulationScene::mouseMoveEvent(QMouseEvent *event)
    {
        //! \todo Get cursor position in current plot to display correct coordinates
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

    void SimulationScene::setZoomLevel(qreal zoomLevel)
    {
        //! \todo Reimplement this
//        if (!m_zoomRange.contains(zoomLevel)) {
//            return;
//        }

//        // Scale in proportion to current zoom level and set new currentZoom
//        scale(zoomLevel/m_currentZoom, zoomLevel/m_currentZoom);
//        m_currentZoom = zoomLevel;
    }

} // namespace Caneda
