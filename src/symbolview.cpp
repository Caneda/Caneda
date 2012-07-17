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

#include "symbolview.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "symbolcontext.h"
#include "symboldocument.h"
#include "statehandler.h"

#include <QToolBar>

namespace Caneda
{
    SymbolView::SymbolView(SymbolDocument *document) :
        IView(document)
    {
        m_cGraphicsView = new CGraphicsView(document->cGraphicsScene());
        StateHandler::instance()->registerWidget(m_cGraphicsView);
        connect(m_cGraphicsView, SIGNAL(focussedIn(CGraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_cGraphicsView, SIGNAL(focussedOut(CGraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_cGraphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    SymbolView::~SymbolView()
    {
        delete m_cGraphicsView;
    }

    SymbolDocument* SymbolView::symbolDocument() const
    {
        return qobject_cast<SymbolDocument*>(document());
    }

    QWidget* SymbolView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* SymbolView::context() const
    {
        return SymbolContext::instance();
    }

    void SymbolView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void SymbolView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void SymbolView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void SymbolView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    qreal SymbolView::currentZoom()
    {
        return m_cGraphicsView->currentZoom();
    }

    void SymbolView::setZoom(int percentage)
    {
        m_cGraphicsView->setZoom(percentage);
    }

    IView* SymbolView::duplicate()
    {
        return document()->createView();
    }

    void SymbolView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
    }

    void SymbolView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void SymbolView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }

} // namespace Caneda
