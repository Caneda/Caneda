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

#ifndef C_SIMULATION_VIEW_H
#define C_SIMULATION_VIEW_H

#include <qwt_plot.h>

// Forward declations
class QwtPlotGrid;

namespace Caneda
{
    // Forward declations
    class CSimulationScene;

    class CSimulationView : public QwtPlot
    {
        Q_OBJECT

    public:
        CSimulationView(CSimulationScene *scene, QWidget *parent = 0);

        virtual void zoomIn();
        virtual void zoomOut();
        virtual void zoomFitInBest();
        virtual void zoomOriginal();

        void showAll();

        void loadUserSettings();

        void print(QPrinter *printer, bool fitInView);
        void exportImage(QPaintDevice &device);

    private:
        CSimulationScene *m_csimulationScene;

        QwtPlotGrid *m_grid;
        QwtLegend *m_legend;
    };

} // namespace Caneda

#endif //C_SIMULATION_VIEW_H
