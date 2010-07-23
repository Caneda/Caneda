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

#include "webdocument.h"

#include "documentviewmanager.h"
#include "webcontext.h"
#include "webpage.h"
#include "webview.h"

#include <QUrl>

namespace Caneda
{
    WebDocument::WebDocument()
    {
        m_webUrl = new QUrl;
    }

    WebDocument::~WebDocument()
    {
        delete m_webUrl;
    }

    IContext* WebDocument::context()
    {
        return WebContext::instance();
    }

    bool WebDocument::isModified() const
    {
        return false;
    }

    bool WebDocument::canUndo() const
    {
        return false;
    }

    bool WebDocument::canRedo() const
    {
        return false;
    }

    void WebDocument::undo()
    {
    }

    void WebDocument::redo()
    {
    }

    QUndoStack* WebDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
    }

    bool WebDocument::canCut() const
    {
        return false;
    }

    bool WebDocument::canCopy() const
    {
        return true;
    }

    bool WebDocument::canPaste() const
    {
        return false;
    }

    void WebDocument::cut()
    {
    }

    void WebDocument::copy()
    {
        WebPage *page = activeWebPage();
        if (!page) {
            return;
        }
        page->triggerPageAction(QWebPage::Copy);
    }

    void WebDocument::paste()
    {
    }

    void WebDocument::selectAll()
    {
    }

    bool WebDocument::printSupportsFitInPage() const
    {
        return false;
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

    void WebDocument::exportToPaintDevice(QPaintDevice *device,
            const QVariantMap &configuration)
    {
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

    bool WebDocument::save(QString *errorMessage)
    {
    }

    IView* WebDocument::createView()
    {
        return new WebView(this);
    }

    void WebDocument::updateSettingsChanges()
    {
    }

    QUrl* WebDocument::webUrl() const
    {
        return m_webUrl;
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
