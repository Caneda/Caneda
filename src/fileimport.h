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

#ifndef FILE_IMPORT_H
#define FILE_IMPORT_H

#include <QList>

// Forward declarations
class QFile;
class QString;
class QTextStream;

namespace Caneda
{
    // Forward declarations
    class SimulationDocument;
    class CSimulationPlotCurve;
    class CSimulationScene;

    /*!
     * \brief This class handles all the access to the raw spice simulation
     * documents file format.
     *
     * This class is in charge of saving and loading all raw spice simulation
     * related documents. This is the only class that knows about raw spice
     * simulation document formats, and has the access functions to return a
     * SimulationDocument, with all of its components.
     *
     * This class does not handle document saving, as waveform data saving will
     * not be supported at the moment (raw waveform data is only generated and
     * saved by the simulator).
     *
     * \sa \ref DocumentFormats
     */
    class FormatRawSimulation
    {
    public:
        FormatRawSimulation(SimulationDocument *doc = 0);

        bool load();

    private:
        void parseFile(QTextStream *file);
        bool parseAsciiData(QTextStream *file);
        bool parseBinaryData(QTextStream *file);

        CSimulationScene* cSimulationScene() const;
        QString fileName() const;

        SimulationDocument *m_simulationDocument;

        QList<CSimulationPlotCurve*> plotCurves;       // List of magnitude curves.
        QList<CSimulationPlotCurve*> plotCurvesPhase;  // List of phase curves.
    };

} // namespace Caneda

#endif //FILE_IMPORT_H
