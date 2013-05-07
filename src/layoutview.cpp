/***************************************************************************
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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

#include "layoutview.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "layoutcontext.h"
#include "layoutdocument.h"
#include "statehandler.h"

#include <QToolBar>

namespace Caneda
{
    //! \brief Constructor.
    LayoutView::LayoutView(LayoutDocument *document) :
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

    //! \brief Destructor.
    LayoutView::~LayoutView()
    {
        delete m_cGraphicsView;
    }

    LayoutDocument* LayoutView::layoutDocument() const
    {
        return qobject_cast<LayoutDocument*>(document());
    }

    QWidget* LayoutView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* LayoutView::context() const
    {
        return LayoutContext::instance();
    }

    void LayoutView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void LayoutView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void LayoutView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void LayoutView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    IView* LayoutView::duplicate()
    {
        return document()->createView();
    }

    void LayoutView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
    }

    void LayoutView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void LayoutView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }

} // namespace Caneda
