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

#include "fileimport.h"

#include "csimulationitem.h"
#include "csimulationscene.h"
#include "idocument.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include <math.h>

namespace Caneda
{
    //! \brief Constructor.
    FormatRawSimulation::FormatRawSimulation(SimulationDocument *doc) :
        m_simulationDocument(doc)
    {
    }

    //! \brief Load the waveform file indicated by \a filename.
    bool FormatRawSimulation::load(const QString filename)
    {
        CSimulationScene *scene = cSimulationScene();
        if(!scene) {
            return false;
        }

        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ") + filename);
            return false;
        }

        QTextStream in(&file);
        parseFile(&in);  // Parse the raw file
        file.close();

        return true;
    }

    /*!
     * \brief Parse the raw file
     *
     * Parse the raw file. First check the header of the file and then call
     * the parseAsciiData() or parseBinaryData() method depending on the
     * type of file.
     *
     * \todo There can be more than one plot set. This should be considered.
     */
    void FormatRawSimulation::parseFile(QTextStream *file)
    {
        int nvars = 0;     // Number of variables
        int npoints = 0;   // Number of points in the simulation
        bool real = true;  // Transient/AC simulation: real = transient / false = ac (complex numbers)

        QString line = file->readLine();

        while(!line.isNull()) {

            line = line.toLower();  // Don't care the case of the entry
            QStringList tok = line.split(":");
            QString keyword = tok.at(0);

            // Ignore the following keywords: title, date, plotname
            if( keyword == "flags" ) {
                if(tok.at(1) == " real") {
                    real = true;
                }
                else if(tok.at(1) == " complex") {
                    real = false;
                }
                else {
                    qDebug() << "Warning: unknown flag: " + tok.at(1);
                }
            }
            else if( keyword == "no. variables") {
                nvars = tok.at(1).toInt();
            }
            else if( keyword == "no. points") {
                npoints = tok.at(1).toInt();
            }
            else if( keyword == "variables") {

                for(int i = 0; i < nvars; i++) {
                    line = file->readLine();

                    tok = line.split("\t", QString::SkipEmptyParts);
                    if(tok.size() >= 3){
                        // Number property not used: number = tok.at(0)

                        // Create a new curve, and add it to the list
                        if(real) {
                            // If dealing with real numbers, create an array only for the magnitude and use the provided curve types
                            CSimulationPlotCurve *curve = new CSimulationPlotCurve(tok.at(1));  // tok.at(1) = name
                            curve->setType(tok.at(2));  // tok.at(2) = type of curve (voltage, current, etc)
                            plotCurves.append(curve);   // Append new curve to the list
                        }
                        else {
                            // If dealing with complex numbers, create an array for the magnitude and another one for the phase
                            CSimulationPlotCurve *curve = new CSimulationPlotCurve("Mag(" + tok.at(1) + ")");       // tok.at(1) = name
                            CSimulationPlotCurve *curvePhase = new CSimulationPlotCurve("Phase(" + tok.at(1) + ")");  // tok.at(1) = name
                            curve->setType("magnitude");         // type of curve (magnitude, phase, etc)
                            curvePhase->setType("phase");        // type of curve (magnitude, phase, etc)
                            plotCurves.append(curve);            // Append new curve to the list
                            plotCurvesPhase.append(curvePhase);  // Append new curve to the list
                        }

                    }
                    else {
                        qDebug() << "List of variables too short.";
                    }
                }
            }
            else if( keyword == "values" ) {
                parseAsciiData(file, nvars, npoints, real);  // Read the data itself
            }
            else if( keyword == "binary") {
                parseAsciiData(file, nvars, npoints, real);  // Read the data itself
            }

            // Read the next line
            line = file->readLine();
        }
    }

    //! \brief Read the data in Ascii format implementation
    void FormatRawSimulation::parseAsciiData(QTextStream *file, const int nvars, const int npoints, const bool real)
    {
        // Create the arrays to deal with the data
        QList<double*> dataSamples;               // List of curve's magnitude data. Once filled, used to set data in plotCurves.
        QList<double*> dataSamplesPhase;          // List of curve's phase data. Used for complex numbers. Once filled, used to set data in plotCurves.

        for(int i = 0; i < nvars; i++) {
            if(real) {
                // If dealing with real numbers, create an array only for the magnitude and use the provided curve types
                double *data = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
            }
            else {
                // If dealing with complex numbers, create an array for the magnitude and another one for the phase
                double *data = new double[npoints];  // Create new dataset
                double *dataPhase = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
                dataSamplesPhase.append(dataPhase);  // Append new data set to the list
            }
        }

        // Read the data
        if(real) {
            // The data is of type real
            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    QString line = file->readLine();
                    dataSamples[j][i] = line.split("\t").last().toDouble();
                }
            }

            // Avoid the first var, as it is the time base for the rest
            // of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                // Add the curve to the scene
                cSimulationScene()->addItem(plotCurves[i]);
            }
        }
        else {
            // The data is of type complex
            double real = 0;
            double imaginary = 0;
            double magnitude = 0;
            double phase = 0;

            // Read the data values, converting the complex data into
            // magnitude and phase data.
            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    QString line = file->readLine();
                    QStringList tok = line.split("\t");
                    line = tok.last();  // Get the complex numeric data
                    tok = line.split(",");  // Split real and imaginary part

                    real = tok.first().toDouble();  // Get the real part
                    imaginary = tok.last().toDouble();  // Get the imaginary part

                    magnitude = sqrt(real*real + imaginary*imaginary);  // Calculate the magnitude part
                    phase = atan(imaginary/real) * 180/M_PI;  // Calculate the phase part

                    dataSamples[j][i] = magnitude;
                    dataSamplesPhase[j][i] = phase;
                }
            }

            // Convert the magnitude values into dB ( dB = 20*log10(V) ).
            // Avoid the first var (var=0), as it is the frequency base
            // for the rest of the curves.
            for(int i = 0; i < npoints; i++){
                for(int j = 1; j < nvars; j++){
                    dataSamples[j][i] = 20*log10(dataSamples[j][i]);
                }
            }

            // Avoid the first var, as it is the frequency base for the
            // rest of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                plotCurvesPhase[i]->setSamples(dataSamples[0], dataSamplesPhase[i], npoints);
                // Add the curve to the scene
                cSimulationScene()->addItem(plotCurves[i]);
                cSimulationScene()->addItem(plotCurvesPhase[i]);
            }
        }
    }

    /*!
     * \brief Read the data in Binary format implementation
     *
     * \todo Fix the binary raw file read implementation
     */
    void FormatRawSimulation::parseBinaryData(QTextStream *file, int nvars, int npoints, bool real)
    {
        // Create the arrays to deal with the data
        QList<double*> dataSamples;               // List of curve's magnitude data. Once filled, used to set data in plotCurves.
        QList<double*> dataSamplesPhase;          // List of curve's phase data. Used for complex numbers. Once filled, used to set data in plotCurves.

        for(int i = 0; i < nvars; i++) {
            if(real) {
                // If dealing with real numbers, create an array only for the magnitude and use the provided curve types
                double *data = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
            }
            else {
                // If dealing with complex numbers, create an array for the magnitude and another one for the phase
                double *data = new double[npoints];  // Create new dataset
                double *dataPhase = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
                dataSamplesPhase.append(dataPhase);  // Append new data set to the list
            }
        }

        // Read the data
        if(real) {
            // The data is of type real
            QDataStream out(file->device());
            out.setByteOrder(QDataStream::LittleEndian);

            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    out >> dataSamples[j][i];
                }
            }

            // Avoid the first var, as it is the time/frequency base
            // for the rest of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                // Add the curve to the scene
                cSimulationScene()->addItem(plotCurves[i]);
            }
        }
        else {
            // The data is of type complex
            QDataStream out(file->device());
            out.setByteOrder(QDataStream::LittleEndian);

            double real = 0;
            double imaginary = 0;

            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    out >> real;  // Get the real part
                    out >> imaginary;  // Get the imaginary part

                    dataSamples[2*j][i] = sqrt(real*real + imaginary*imaginary);
                    dataSamples[2*j+1][i] = atan(imaginary/real) * 180/M_PI;
                }
            }

            // Avoid the first var, as it is the time/frequency base
            // for the rest of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[2*i-1]->setSamples(dataSamples[0], dataSamples[2*i], npoints);
                plotCurves[2*i]->setSamples(dataSamples[0], dataSamples[2*i+1], npoints);
                // Add the curve to the scene
                cSimulationScene()->addItem(plotCurves[2*i-1]);
                cSimulationScene()->addItem(plotCurves[2*i]);
            }
        }
    }

    CSimulationScene* FormatRawSimulation::cSimulationScene() const
    {
        return m_simulationDocument ? m_simulationDocument->cSimulationScene() : 0;
    }

} // namespace Caneda
