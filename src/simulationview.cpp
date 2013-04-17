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

#include "simulationview.h"

#include "simulationcontext.h"
#include "simulationdocument.h"
#include "simulationscene.h"
#include "statehandler.h"

namespace Caneda
{
    SimulationView::SimulationView(SimulationDocument *document) :
        IView(document)
    {
        SimulationScene *m_simulationScene = simulationDocument()->simulationScene();
        connect(m_simulationScene, SIGNAL(focussedIn(SimulationScene*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_simulationScene, SIGNAL(focussedOut(SimulationScene*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_simulationScene, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    SimulationView::~SimulationView()
    {
    }

    SimulationDocument* SimulationView::simulationDocument() const
    {
        return qobject_cast<SimulationDocument*>(document());
    }

    QWidget* SimulationView::toWidget() const
    {
        return simulationDocument()->simulationScene();
    }

    IContext* SimulationView::context() const
    {
        return SimulationContext::instance();
    }

    void SimulationView::zoomIn()
    {
         simulationDocument()->simulationScene()->zoomIn();
    }

    void SimulationView::zoomOut()
    {
         simulationDocument()->simulationScene()->zoomOut();
    }

    void SimulationView::zoomFitInBest()
    {
        simulationDocument()->simulationScene()->zoomFitInBest();
    }

    void SimulationView::zoomOriginal()
    {
        simulationDocument()->simulationScene()->zoomOriginal();
    }

    IView* SimulationView::duplicate()
    {
        return document()->createView();
    }

    void SimulationView::updateSettingsChanges()
    {
        simulationDocument()->simulationScene()->repaint();
    }

    void SimulationView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void SimulationView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }

} // namespace Caneda
