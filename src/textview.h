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

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "global.h"
#include "iview.h"

namespace Caneda
{
    // Forward declrations
    class TextDocument;
    class TextEdit;

    class TextView : public IView
    {
        Q_OBJECT
    public:
        TextView(TextDocument *document);
        virtual ~TextView();

        TextEdit* textEdit() const;

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

        void setZoomLevel(qreal level);

    private Q_SLOTS:
        void onFocussed();

    private:
        TextEdit *m_textEdit;
        ZoomRange m_zoomRange;
        qreal m_currentZoom;
        const qreal m_originalZoom;
    };

} // namespace Caneda

#endif //TEXTVIEW_H
