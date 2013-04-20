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

#ifndef FORMAT_RAW_SIMULATION_H
#define FORMAT_RAW_SIMULATION_H

// Forward declarations
class QFile;
class QString;

namespace Caneda
{
    // Forward declarations
    class SimulationDocument;
    class SimulationScene;

    class FormatRawSimulation
    {
    public:
        FormatRawSimulation(SimulationDocument *doc = 0);
        ~FormatRawSimulation() {}

        bool load();

        SimulationDocument* simulationDocument() const;
        SimulationScene* simulationScene() const;
        QString fileName() const;

    private:
        SimulationDocument *m_simulationDocument;
    };

} // namespace Caneda

#endif //FORMAT_RAW_SIMULATION_H
