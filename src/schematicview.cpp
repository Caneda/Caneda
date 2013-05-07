/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "schematicview.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "schematiccontext.h"
#include "schematicdocument.h"
#include "statehandler.h"

#include <QToolBar>

namespace Caneda
{
    //! \brief Constructor.
    SchematicView::SchematicView(SchematicDocument *document) :
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
    SchematicView::~SchematicView()
    {
        delete m_cGraphicsView;
    }

    SchematicDocument* SchematicView::schematicDocument() const
    {
        return qobject_cast<SchematicDocument*>(document());
    }

    QWidget* SchematicView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* SchematicView::context() const
    {
        return SchematicContext::instance();
    }

    void SchematicView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void SchematicView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void SchematicView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void SchematicView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    IView* SchematicView::duplicate()
    {
        return document()->createView();
    }

    void SchematicView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
    }

    void SchematicView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void SchematicView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }

} // namespace Caneda
