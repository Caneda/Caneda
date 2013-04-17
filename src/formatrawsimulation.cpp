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

#include "formatrawsimulation.h"

#include "simulationdocument.h"
#include "simulationscene.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include <qwt_plot_curve.h>

namespace Caneda
{
    //! Constructor
    FormatRawSimulation::FormatRawSimulation(SimulationDocument *doc) :
        m_simulationDocument(doc)
    {
    }

    bool FormatRawSimulation::load()
    {
        SimulationScene *scene = simulationScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ")+fileName());
            return false;
        }

        QTextStream stream(&file);
        bool result = loadFromText(stream.readAll());
        file.close();

        scene->showAll();  //! \todo This should be removed from here when an interface to add waveforms to the plot is implemented.

        return result;
    }

    bool FormatRawSimulation::loadFromText(const QString& text)
    {
        SimulationScene *scene = simulationScene();
        if(!scene) {
            return false;
        }

        //! \todo Implement this
        // Read all the data from the file "filename() + ".raw"" (waveforms).

        /* Add curves, for example:
        QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");

        double *x1 = new double[100];
        double *y1 = new double[100];
        for(int i = 0; i < 100; i++) {
            x1[i] = 2.777*i;
            y1[i] = 0.888/(i+1);
        }

        // Copy the data into the curves
        curve1->setSamples(x1,y1,100);

        // Add the curve to the scene
        scene->addItem(curve1);
        */

        return true;
    }

    SimulationDocument* FormatRawSimulation::simulationDocument() const
    {
        return m_simulationDocument;
    }

    SimulationScene* FormatRawSimulation::simulationScene() const
    {
        return m_simulationDocument ? m_simulationDocument->simulationScene() : 0;
    }

    QString FormatRawSimulation::fileName() const
    {
        return m_simulationDocument ? m_simulationDocument->fileName() : QString();
    }

} // namespace Caneda
