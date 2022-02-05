/***************************************************************************
 * Copyright (C) 2013-2015 by Pablo Daniel Pareja Obregon                  *
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

#ifndef CHART_VIEW_H
#define CHART_VIEW_H

#include <QtPrintSupport/QPrinter>

#include <qwt_plot.h>
#include <qwt_plot_magnifier.h>

// Forward declations
class QwtLegend;
class QwtPlotCanvas;
class QwtPlotGrid;
class QwtPlotZoomer;

namespace Caneda
{
    // Forward declations
    class ChartScene;

    /*!
     * \brief Reimplementation of the QwtPlotMagnifier class to allow for the
     * implementation zoom in/zoom out actions.
     *
     * \sa QwtPlotMagnifier
     */
    class PlotMagnifier: public QwtPlotMagnifier
    {
        Q_OBJECT

    public:
        explicit PlotMagnifier(QWidget *canvas);

        void zoomIn();
        void zoomOut();

    private:
        double m_zoomFactor;
    };

    /*!
     * \brief This class provides a view for displaying ChartScenes
     * (several grouped simulation waveforms).
     *
     * This class implements the view class of Qt's Graphics View Architecture.
     * The view class provides the view widget, which visualizes the contents
     * of a scene (individual waveforms). You can attach several views to the
     * same scene, to provide different viewports into the same data set of the
     * document (for example, when using split views).
     *
     * In this class common view operations are implemented, for example
     * zooming and focus events. In this way, one single scene can have
     * multiple views associated to it, allowing the user to look at the scene
     * for example, with multiple zoom levels.
     *
     * \sa ChartScene
     */
    class ChartView : public QwtPlot
    {
        Q_OBJECT

    public:
        explicit ChartView(ChartScene *scene, QWidget *parent = 0);

        virtual void zoomIn();
        virtual void zoomOut();
        virtual void zoomFitInBest();
        virtual void zoomOriginal();

        void populate();
        void resetAxis();
        void setLogAxis(QwtPlot::Axis axis, bool logarithmic);
        bool isLogAxis(QwtPlot::Axis axis);

        void loadUserSettings();

        void print(QPrinter *printer, bool fitInView);
        void exportImage(QPaintDevice &device);

    public Q_SLOTS:
        void launchPropertiesDialog();
        void contextMenuEvent(const QPoint &pos);

    Q_SIGNALS:
        void cursorPositionChanged(const QString& newPos);

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent * event);

    private:
        ChartScene *m_chartScene;

        QwtPlotCanvas *m_canvas;
        QwtPlotGrid *m_grid;
        QwtLegend *m_legend;
        QwtPlotZoomer *m_zoomer;
        PlotMagnifier *m_magnifier;

        bool m_logXaxis, m_logYleftAxis, m_logYrightAxis;
    };

} // namespace Caneda

#endif //CHART_VIEW_H
