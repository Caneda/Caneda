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

namespace Caneda
{
    SchematicView::SchematicView(SchematicDocument *document) :
        IView(document),
        m_zoomRange(0.10, 8.0),
        m_currentZoom(1.0)
    {
        m_schematicWidget = new SchematicWidget(this);
        SchematicStateHandler::instance()->registerWidget(m_schematicWidget);
        connect(m_schematicWidget, SIGNAL(focussed(SchematicWidget*)), this,
                SLOT(onWidgetFocussed()));
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

    qreal SchematicView::currentZoom() const
    {
        return m_currentZoom;
    }

    void SchematicView::setZoomLevel(qreal zoomLevel)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }
        m_currentZoom = zoomLevel;
        QTransform transform;
        transform.scale(m_currentZoom, m_currentZoom);
        m_schematicWidget->setTransform(transform);
    }

    void SchematicView::zoomIn()
    {
        qreal newZoom = m_currentZoom * 1.2;
        setZoomLevel(qMin(newZoom, m_zoomRange.max));
    }

    void SchematicView::zoomOut()
    {
        qreal newZoom = m_currentZoom * 0.8;
        setZoomLevel(qMax(newZoom, m_zoomRange.min));
    }

    void SchematicView::zoomFitInBest()
    {
        SchematicScene *scene = m_schematicWidget->schematicScene();
        if (!scene) {
            return;
        }

        QRectF rect = scene->itemsBoundingRect();
        setZoomLevel(m_schematicWidget->fit(rect));
        m_schematicWidget->centerOn(rect.center());
    }

    void SchematicView::zoomOriginal()
    {
        setZoomLevel(1.0);
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

    void SchematicView::onWidgetFocussed()
    {
        emit focussedIn(static_cast<IView*>(this));
        ActionManager *am = ActionManager::instance();
        Action *action = am->actionForName("snapToGrid");
        action->setChecked(m_schematicWidget->schematicScene()->gridSnap());
    }

} // namespace Caneda
