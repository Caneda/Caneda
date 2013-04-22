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

#include "simulationdocument.h"

#include "formatrawsimulation.h"
#include "simulationcontext.h"
#include "csimulationscene.h"
#include "simulationview.h"

#include "dialogs/exportdialog.h"

#include <QFileInfo>
#include <QPrinter>
#include <QUndoStack>

namespace Caneda
{
    SimulationDocument::SimulationDocument()
    {
        m_cSimulationScene = new CSimulationScene();
    }

    IContext* SimulationDocument::context()
    {
        return SimulationContext::instance();
    }

    QUndoStack* SimulationDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
    }

    void SimulationDocument::distributeHorizontal()
    {
        /*!
         * \todo Implement this. This method should distribute the available
         * waveforms into several graphs, distributed horizontally.
         */
    }

    void SimulationDocument::distributeVertical()
    {
        /*!
         * \todo Implement this. This method should distribute the available
         * waveforms into several graphs, distributed vertically.
         */
    }

    void SimulationDocument::centerHorizontal()
    {
        //! \todo Implement this. Merge horizontally distributed waveforms.
    }

    void SimulationDocument::centerVertical()
    {
        //! \todo Implement this. Merge vertically distributed waveforms.
    }

    void SimulationDocument::print(QPrinter *printer, bool fitInView)
    {
        //! \todo Reimplement this
    }

    bool SimulationDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "raw") {
            FormatRawSimulation *format = new FormatRawSimulation(this);
            return format->load();
        }

        if (errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    void SimulationDocument::exportImage()
    {
        //! \todo Reimplement this
    }

    IView* SimulationDocument::createView()
    {
        return new SimulationView(this);
    }

} // namespace Caneda
