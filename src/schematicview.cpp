/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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
#include "schematiccontext.h"
#include "schematicdocument.h"
#include "schematicstatehandler.h"
#include "schematicwidget.h"

#include <QToolBar>

namespace Caneda
{
    SchematicView::SchematicView(SchematicDocument *document) :
        IView(document)
    {
        m_schematicWidget = new SchematicWidget(document->schematicScene());
        SchematicStateHandler::instance()->registerWidget(m_schematicWidget);
        connect(m_schematicWidget, SIGNAL(focussedIn(SchematicWidget*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_schematicWidget, SIGNAL(focussedOut(SchematicWidget*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_schematicWidget, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    SchematicView::~SchematicView()
    {
        delete m_schematicWidget;
    }

    SchematicDocument* SchematicView::schematicDocument() const
    {
        return qobject_cast<SchematicDocument*>(document());
    }

    QWidget* SchematicView::toWidget() const
    {
        return m_schematicWidget;
    }

    IContext* SchematicView::context() const
    {
        return SchematicContext::instance();
    }

    void SchematicView::zoomIn()
    {
        m_schematicWidget->zoomIn();
    }

    void SchematicView::zoomOut()
    {
        m_schematicWidget->zoomOut();
    }

    void SchematicView::zoomFitInBest()
    {
        m_schematicWidget->zoomFitInBest();
    }

    void SchematicView::zoomOriginal()
    {
        m_schematicWidget->zoomOriginal();
    }

    qreal SchematicView::currentZoom()
    {
        return m_schematicWidget->currentZoom();
    }

    IView* SchematicView::duplicate()
    {
        return document()->createView();
    }

    void SchematicView::updateSettingsChanges()
    {
        m_schematicWidget->invalidateScene();
        m_schematicWidget->resetCachedContent();
    }

    void SchematicView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
        ActionManager *am = ActionManager::instance();
        Action *action = am->actionForName("snapToGrid");
        action->setChecked(m_schematicWidget->schematicScene()->gridSnap());
    }

    void SchematicView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }

} // namespace Caneda
