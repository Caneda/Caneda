/***************************************************************************
 * Copyright (C) 2013-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "chartview.h"

#include "actionmanager.h"
#include "chartsdialog.h"
#include "chartscene.h"
#include "settings.h"

#include <QMenu>
#include <QMouseEvent>

#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_engine.h>

namespace Caneda
{
    /*************************************************************************
     *                           CPlotMagnifier                              *
     *************************************************************************/
    //! \brief Constructor
    PlotMagnifier::PlotMagnifier(QWidget *canvas): QwtPlotMagnifier(canvas)
    {
        m_zoomFactor = 1.1;
    }

    //! \brief Zoom in the plot
    void PlotMagnifier::zoomIn()
    {
        rescale(1.0 / m_zoomFactor);
    }

    //! \brief Zoom out the plot
    void PlotMagnifier::zoomOut()
    {
        rescale(m_zoomFactor);
    }


    /*************************************************************************
     *                              ChartView                                *
     *************************************************************************/
    /*!
     * \brief Constructs a new simulation view.
     *
     * \param sv Simulation scene to point this view to.
     * \param parent Parent of this object.
     */
    ChartView::ChartView(ChartScene *scene, QWidget *parent) :
        QwtPlot(parent),
        m_chartScene(scene),
        m_logXaxis(false),
        m_logYleftAxis(false),
        m_logYrightAxis(false)
    {
        // Canvas
        m_canvas = new QwtPlotCanvas();
        m_canvas->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
        setCanvas(m_canvas);

        // Panning with the middle mouse button
        QwtPlotPanner *panner = new QwtPlotPanner(m_canvas);
        panner->setMouseButton(Qt::MidButton);

        // Zoom in/out with the wheel
        m_magnifier = new PlotMagnifier(m_canvas);
        m_magnifier->setMouseButton(Qt::NoButton);  // Disable default left button action
        m_magnifier->setWheelFactor(1/m_magnifier->wheelFactor());  // Invert the wheel direction

        // Box zoom with left button and position label
        m_zoomer = new QwtPlotZoomer(m_canvas);
        m_zoomer->setTrackerMode(QwtPicker::AlwaysOn);
        m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::NoButton); // Disable default right button action
        m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::NoButton); // Disable default middle button action

        // Grid
        m_grid = new QwtPlotGrid();
        m_grid->enableXMin(true);
        m_grid->enableYMin(true);

        // Legend
        m_legend = new QwtLegend();
        insertLegend(m_legend, QwtPlot::TopLegend);

        // Enable mouse tracking for status bar position display
        setMouseTracking(true);

        // Load user settings, for example canvas color
        loadUserSettings();

        // Context menu event
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuEvent(const QPoint &)));
    }

    void ChartView::zoomIn()
    {
        m_magnifier->zoomIn();
    }

    void ChartView::zoomOut()
    {
        m_magnifier->zoomOut();
    }

    void ChartView::zoomFitInBest()
    {
        m_zoomer->zoom(0);
    }

    void ChartView::zoomOriginal()
    {
        m_zoomer->zoom(0);
    }

    //! \brief Adds all items available in the scene to the plot widget.
    void ChartView::populate()
    {
        QList<ChartSeries*> m_items = m_chartScene->items();

        QColor color = QColor(0, 0, 0);
        int colorIndex= 0;
        int valueIndex = 255;

        // Attach the items to the plot
        foreach(ChartSeries *item, m_items) {
            // Recreate the curve to be able to attach
            // the same curve to different views
            ChartSeries *newCurve = new ChartSeries();
            newCurve->setData(item->data());
            newCurve->setTitle(item->title());
            newCurve->attach(this);

            // Set the correct axis depending on the curve magnitude
            if(item->type() == "current" || item->type() == "phase") {
                newCurve->setYAxis(yRight);
            }

            // Select the style and color of the new curve
            newCurve->setRenderHint(ChartSeries::RenderAntialiased);
            color.setHsv(colorIndex , 200, valueIndex);
            newCurve->setPen(QPen(color));

            // If the curve is of type magnitude, avoid updating the color
            // for the next curve (that should be the phase).
            // If the curve is of type phase, use a dashed line.
            if(item->type() == "magnitude") {
                continue;
            }
            else if(item->type() == "phase") {
                QPen m_pen = QPen(color);
                m_pen.setStyle(Qt::DashLine);
                newCurve->setPen(m_pen);
            }

            // Set the next color to be used (to change colors
            // from curve to curve.
            if(colorIndex < 300) {  // Avoid 360, as it equals 0
                colorIndex += 60;
            }
            else {
                colorIndex = 0;
                if(valueIndex == 255) {
                    valueIndex = 100;
                }
                else {
                    valueIndex = 255;
                }
            }
        }

        // Set different axis titles depending on the type of simulation,
        // ie. time for transient; frequency for ac simulation
        if(m_items.first()->type() == "voltage" || m_items.first()->type() == "current") {
            setAxisTitle(xBottom, QwtText(tr("Time [s]")));
            setAxisTitle(yLeft, QwtText(tr("Voltage [V]")));
            setAxisTitle(yRight, QwtText(tr("Current [A]")));
        }
        else {
            setAxisTitle(xBottom, QwtText(tr("Frequency [Hz]")));
            setAxisTitle(yLeft, QwtText(tr("Magnitude [dB]")));
            setAxisTitle(yRight, QwtText(tr("Phase [º]")));
            setLogAxis(QwtPlot::xBottom, true);
        }

        enableAxis(yRight);  // Always enable the y axis

        // Refresh the plot
        replot();

        // Set the zoom base to the current (autoscale) value.
        // This value is later used to be able to return to the base zoom,
        // displaying all waveforms contents.
        m_zoomer->setZoomBase();
    }

    /*!
     * \brief Set axis scale logarithmic state.
     *
     * \param axis Axis to set the logarithmic scale state.
     * \param logarithmic True if logaritmich scale is wanted, false otherwise.
     *
     * \sa isLogAxis()
     */
    void ChartView::setLogAxis(QwtPlot::Axis axis, bool logarithmic)
    {
        // Set logarithmic the corresponding axis
        if(logarithmic) {
            setAxisScaleEngine(axis, new QwtLogScaleEngine);
        }
        else {
            setAxisScaleEngine(axis, new QwtLinearScaleEngine);
        }

        // Set m_logXaxis/m_logYaxis logarithmic to track the state
        if(axis == QwtPlot::xBottom || axis == QwtPlot::xTop) {
            m_logXaxis = logarithmic;
        }
        else if(axis == QwtPlot::yLeft) {
            m_logYleftAxis = logarithmic;
        }
        else {
            m_logYrightAxis = logarithmic;
        }
    }

    /*!
     * \brief Get axis scale logarithmic state.
     *
     * \param axis Axis which logarithmic scale state is checked.
     * \return true if axis scale is logarithmic, false otherwise.
     *
     * \sa setLogAxis()
     */
    bool ChartView::isLogAxis(QwtPlot::Axis axis)
    {
        if(axis == QwtPlot::xBottom || axis == QwtPlot::xTop) {
            return m_logXaxis;
        }
        else if(axis == QwtPlot::yLeft) {
            return m_logYleftAxis;
        }
        else {
            return m_logYrightAxis;
        }
    }

    //! \brief Loads the saved user settings, updating the values on the canvas.
    void ChartView::loadUserSettings()
    {
        // Load settings
        Settings *settings = Settings::instance();
        QColor foregroundColor = settings->currentValue("gui/foregroundColor").value<QColor>();
        QColor backgroundColor = settings->currentValue("gui/simulationBackgroundColor").value<QColor>();

        // Canvas
        QPalette canvasPalette(backgroundColor);
        m_canvas->setPalette(canvasPalette);

        // Grid
        if(Settings::instance()->currentValue("gui/gridVisible").value<bool>()) {
            m_grid->setMajorPen(QPen(foregroundColor, 1, Qt::DashLine));
            m_grid->setMinorPen(QPen(foregroundColor, 0 , Qt::DotLine));
            m_grid->attach(this);
        }
        else {
            m_grid->detach();
        }
    }

    void ChartView::print(QPrinter *printer, bool fitInView)
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardNone, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);

        renderer.renderTo(this, *printer);
    }

    void ChartView::exportImage(QPaintDevice &device)
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardNone, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);

        renderer.renderTo(this, device);
    }

    //! \copydoc CGraphicsItem::launchPropertiesDialog()
    int ChartView::launchPropertiesDialog()
    {
        ChartsDialog *dia = new ChartsDialog(this);
        int status = dia->exec();
        delete dia;

        return status;
    }

    //! \brief Context menu.
    void ChartView::contextMenuEvent(const QPoint &pos)
    {
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();

        //launch context menu of item
        _menu->addAction(am->actionForName("zoomFitInBest"));
        _menu->addAction(am->actionForName("zoomOriginal"));
        _menu->addAction(am->actionForName("zoomIn"));
        _menu->addAction(am->actionForName("zoomOut"));

        _menu->addSeparator();
        _menu->addAction(am->actionForName("schEdit"));
        _menu->addAction(am->actionForName("propertiesDialog"));

        _menu->exec(QCursor::pos());
    }

    //! \brief Update the status bar with the plot coordinates on move event.
    void ChartView::mouseMoveEvent(QMouseEvent *event)
    {
        QPoint newCursorPos = m_zoomer->trackerPosition();
        double x = invTransform(xBottom, newCursorPos.x());
        double y1 = invTransform(yLeft, newCursorPos.y());
        double y2 = invTransform(yRight, newCursorPos.y());

        QString str = QString("%1 : %2 : %3").arg(x).arg(y1).arg(y2);
        emit cursorPositionChanged(str);

        QwtPlot::mouseMoveEvent(event);
    }

    //! \brief Show plot properties dialog upon mouse double click.
    void ChartView::mouseDoubleClickEvent(QMouseEvent *event)
    {
        launchPropertiesDialog();
    }

} // namespace Caneda
