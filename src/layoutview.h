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

#ifndef LAYOUT_VIEW_H
#define LAYOUT_VIEW_H

#include "iview.h"

namespace Caneda
{
    // Forward declrations
    class CGraphicsView;
    class LayoutDocument;

    class LayoutView : public IView
    {
        Q_OBJECT
    public:
        LayoutView(LayoutDocument *document);
        virtual ~LayoutView();

        LayoutDocument* layoutDocument() const;

        // IView interface methods

        virtual QWidget* toWidget() const;
        virtual IContext* context() const;

        virtual void zoomIn();
        virtual void zoomOut();
        virtual void zoomFitInBest();
        virtual void zoomOriginal();
        virtual qreal currentZoom();

        virtual IView* duplicate();

        virtual void updateSettingsChanges();

        // End of IView interface methods

    private Q_SLOTS:
        void onWidgetFocussedIn();
        void onWidgetFocussedOut();

    private:
        CGraphicsView *m_cGraphicsView;
    };

} // namespace Caneda

#endif //LAYOUT_VIEW_H
