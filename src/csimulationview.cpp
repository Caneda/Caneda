/***************************************************************************
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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

#include "csimulationview.h"

#include "csimulationscene.h"
#include "settings.h"

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_renderer.h>

namespace Caneda
{
    CSimulationView::CSimulationView(CSimulationScene *scene, QWidget *parent) :
        QwtPlot(parent),
        m_csimulationScene(scene)
    {
        m_grid = new QwtPlotGrid();
        m_legend = new QwtLegend();

        loadUserSettings();
    }

    CSimulationView::~CSimulationView()
    {
    }

    void CSimulationView::zoomIn()
    {
        //! \todo Implement this
    }

    void CSimulationView::zoomOut()
    {
        //! \todo Implement this
    }

    void CSimulationView::zoomFitInBest()
    {
        //! \todo Implement this
    }

    void CSimulationView::zoomOriginal()
    {
        //! \todo Implement this
    }

    //! \brief Displays all items available in the scene, in the plot widget.
    void CSimulationView::showAll()
    {
        QList<QwtPlotCurve*> m_items = m_csimulationScene->items();

        QColor color = QColor(0, 0, 0);
        int colorIndex= 0;
        int valueIndex = 255;

        // Attach the items to the plot
        foreach(QwtPlotCurve *item, m_items) {
            // Recreate the curve to be able to attach
            // the same curve to different views
            QwtPlotCurve *newCurve = new QwtPlotCurve();
            newCurve->setData(item->data());
            newCurve->attach(this);
            newCurve->setTitle(item->title());

            newCurve->setRenderHint(QwtPlotCurve::RenderAntialiased);

            // Select the color of the new curve
            color.setHsv(colorIndex , 200, valueIndex);
            newCurve->setPen(QPen(color));

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

        // Refresh the plot
        replot();
    }

    void CSimulationView::loadUserSettings()
    {
        Settings *settings = Settings::instance();
        QColor foregroundColor = settings->currentValue("gui/foregroundColor").value<QColor>();
        QColor backgroundColor = settings->currentValue("gui/simulationBackgroundColor").value<QColor>();

        setCanvasBackground(backgroundColor);

        if(Settings::instance()->currentValue("gui/gridVisible").value<bool>()) {
            m_grid->enableXMin(true);
            m_grid->setMajPen(QPen(foregroundColor, 1, Qt::DashLine));
            m_grid->setMinPen(QPen(foregroundColor, 0 , Qt::DotLine));
            m_grid->attach(this);
        }
        else {
            m_grid->detach();
        }

        m_legend->setItemMode(QwtLegend::ClickableItem);
        this->insertLegend(m_legend, QwtPlot::TopLegend);
    }

    void CSimulationView::print(QPrinter *printer, bool fitInView)
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardNone, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);

        renderer.renderTo(this, *printer);
    }

    void CSimulationView::exportImage(QPaintDevice &device, qreal width, qreal height,
                                      Qt::AspectRatioMode aspectRatioMode)
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardNone, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);

        renderer.renderTo(this, device);
    }

} // namespace Caneda
