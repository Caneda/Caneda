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

#include "webdocument.h"

#include "documentviewmanager.h"
#include "webcontext.h"
#include "webpage.h"
#include "webview.h"

#include <QUndoStack>
#include <QUrl>

namespace Caneda
{
    //! \brief Constructor.
    WebDocument::WebDocument()
    {
        m_webUrl = new QUrl;
    }

    IContext* WebDocument::context()
    {
        return WebContext::instance();
    }

    QUndoStack* WebDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
    }

    void WebDocument::copy()
    {
        WebPage *page = activeWebPage();
        if (!page) {
            return;
        }
        page->copy();
    }

    void WebDocument::print(QPrinter *printer, bool fitInView)
    {
        Q_UNUSED(fitInView);

        WebPage *page = activeWebPage();
        if (!page) {
            return;
        }

        page->print(printer);
    }

    QSizeF WebDocument::documentSize()
    {
        // Return 0, as this method is only used for graphic documents.
        QSizeF size(0, 0);
        return size;
    }

    bool WebDocument::load(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty filename");
            }
            return false;
        }

        m_webUrl->setUrl(fileName());
        return true;
    }

    IView* WebDocument::createView()
    {
        return new WebView(this);
    }

    WebPage* WebDocument::activeWebPage()
    {
        IView *view = DocumentViewManager::instance()->currentView();
        WebView *active = qobject_cast<WebView*>(view);

        if (active) {
            return active->webPage();
        }

        return 0;
    }

} // namespace Caneda
