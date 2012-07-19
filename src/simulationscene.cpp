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
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace Caneda
{
    SimulationScene::SimulationScene(QWidget *parent) :
        QWidget(parent)
    {
        // Setup undo stack
        m_undoStack = new QUndoStack(this);

        QwtPlot *myPlot = new QwtPlot(this);

        // Add curves
        // QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
        // QwtPlotCurve *curve2 = new QwtPlotCurve("Curve 2");

        // Copy the data into the curves
        // curve1->setData(...);
        // curve2->setData(...);

        // curve1->attach(myPlot);
        // curve2->attach(myPlot);

        // Finally, refresh the plot
        myPlot->replot();


        connect(undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(setModified(bool)));
    }

    SimulationScene::~SimulationScene()
    {
        delete m_undoStack;
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

} // namespace Caneda
