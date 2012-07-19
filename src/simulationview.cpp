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

#include "simulationview.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "simulationcontext.h"
#include "simulationdocument.h"
#include "statehandler.h"

#include <QToolBar>

namespace Caneda
{
    SimulationView::SimulationView(SimulationDocument *document) :
        IView(document)
    {
        // TODO: Create own class for view and scene derived from
        // QwtPlot + QGraphicsScene
        m_cGraphicsView = new CGraphicsView(document->cGraphicsScene());
        StateHandler::instance()->registerWidget(m_cGraphicsView);
        connect(m_cGraphicsView, SIGNAL(focussedIn(CGraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_cGraphicsView, SIGNAL(focussedOut(CGraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_cGraphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    SimulationView::~SimulationView()
    {
        delete m_cGraphicsView;
    }

    SimulationDocument* SimulationView::simulationDocument() const
    {
        return qobject_cast<SimulationDocument*>(document());
    }

    QWidget* SimulationView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* SimulationView::context() const
    {
        return SimulationContext::instance();
    }

    void SimulationView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void SimulationView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void SimulationView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void SimulationView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    qreal SimulationView::currentZoom()
    {
        return m_cGraphicsView->currentZoom();
    }

    void SimulationView::setZoom(int percentage)
    {
        m_cGraphicsView->setZoom(percentage);
    }

    IView* SimulationView::duplicate()
    {
        return document()->createView();
    }

    void SimulationView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
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
