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

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "global.h"
#include "iview.h"

namespace Caneda
{
    // Forward declrations
    class WebDocument;
    class WebPage;

    /*!
     * \brief This class represents the web browser view interface
     * implementation.
     *
     * This class represents the view for a web document, in a manner
     * similar to Qt's Graphics View Architecture, and provides the view
     * widget, which visualizes the contents of the document. The view is
     * included as a pointer to WebPage, that contains all the view specific
     * methods. You can attach several views to the same document, to provide
     * different viewports into the same data set of the document (for example,
     * when using split views).
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa WebContext, WebDocument
     */
    class WebView : public IView
    {
        Q_OBJECT

    public:
        WebView(WebDocument *document);
        virtual ~WebView();

        WebPage* webPage() const;

        // IView interface methods
        virtual QWidget* toWidget() const;
        virtual IContext* context() const;

        virtual void zoomIn();
        virtual void zoomOut();
        virtual void zoomFitInBest();
        virtual void zoomOriginal();

        virtual IView* duplicate();

        virtual void updateSettingsChanges();
        // End of IView interface methods

    private Q_SLOTS:
        void onFocussed();

    private:
        WebPage *m_webPage;

        void setZoomLevel(qreal level);

        const qreal m_originalZoom;
        ZoomRange m_zoomRange;
        qreal m_currentZoom;
    };

} // namespace Caneda

#endif //WEBVIEW_H
