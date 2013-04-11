/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "webview.h"

#include "webcontext.h"
#include "webdocument.h"
#include "webpage.h"

#include <QApplication>
#include <QFontInfo>

namespace Caneda
{
    WebView::WebView(WebDocument *document) :
        IView(document),
        m_zoomRange(0.4, 10.0),
        m_originalZoom(QFontInfo(qApp->font()).pointSizeF()/10)
    {
        m_currentZoom = m_originalZoom;
        m_webPage = new WebPage(document->webUrl());

        connect(m_webPage, SIGNAL(focussed()), this,
                SLOT(onFocussed()));
    }

    WebView::~WebView()
    {
        delete m_webPage;;
    }

    WebPage* WebView::webPage() const
    {
        return m_webPage;
    }

    QWidget* WebView::toWidget() const
    {
        return m_webPage;
    }

    IContext* WebView::context() const
    {
        return WebContext::instance();
    }

    void WebView::zoomIn()
    {
        setZoomLevel(m_currentZoom + 0.1);
    }

    void WebView::zoomOut()
    {
        setZoomLevel(m_currentZoom - 0.1);
    }

    void WebView::zoomFitInBest()
    {
        setZoomLevel(2);
    }

    void WebView::zoomOriginal()
    {
        setZoomLevel(m_originalZoom);
    }

    qreal WebView::currentZoom()
    {
        return m_currentZoom;
    }

    IView* WebView::duplicate()
    {
        return document()->createView();
    }

    void WebView::updateSettingsChanges()
    {
    }

    void WebView::onFocussed()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void WebView::setZoomLevel(qreal zoomLevel)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        if (qFuzzyCompare(zoomLevel, m_currentZoom)) {
            return;
        }

        m_currentZoom = zoomLevel;

        m_webPage->setPointSize(m_currentZoom);
    }

} // namespace Caneda
