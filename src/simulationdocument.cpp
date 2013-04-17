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
#include "simulationscene.h"
#include "simulationview.h"
#include "statehandler.h"
#include "xmlsimulation.h"

#include "dialogs/exportdialog.h"

#include <QFileInfo>
#include <QPainter>
#include <QPrinter>
#include <QUndoStack>

#include <cmath>

namespace Caneda
{
    SimulationDocument::SimulationDocument()
    {
        m_simulationScene = new SimulationScene();
        connect(m_simulationScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
    }

    IContext* SimulationDocument::context()
    {
        return SimulationContext::instance();
    }

    bool SimulationDocument::isModified() const
    {
        return m_simulationScene->isModified();
    }

    bool SimulationDocument::canUndo() const
    {
        return m_simulationScene->undoStack()->canUndo();
    }

    bool SimulationDocument::canRedo() const
    {
        return m_simulationScene->undoStack()->canRedo();
    }

    void SimulationDocument::undo()
    {
        m_simulationScene->undoStack()->undo();
    }

    void SimulationDocument::redo()
    {
        m_simulationScene->undoStack()->redo();
    }

    QUndoStack* SimulationDocument::undoStack()
    {
        return m_simulationScene->undoStack();
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

        if(info.suffix() == "xdat") {
            XmlSimulation *format = new XmlSimulation(this);
            return format->load();
        }
        else if(info.suffix() == "raw") {
            FormatRawSimulation *format = new FormatRawSimulation(this);
            return format->load();
        }

        if (errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    bool SimulationDocument::save(QString *errorMessage)
    {
        if(fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty file name");
            }
            return false;
        }

        QFileInfo info(fileName());

        if(info.suffix() == "xdat") {
            XmlSimulation *format = new XmlSimulation(this);
            if(!format->save()) {
                return false;
            }

            m_simulationScene->undoStack()->clear();
            return true;
        }

        if(errorMessage) {
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
