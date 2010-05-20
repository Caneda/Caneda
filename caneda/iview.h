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

#ifndef CANEDA_IVIEW_H
#define CANEDA_IVIEW_H

#include <QObject>

// Forward declaration
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class IContext;
    class IDocument;
    class DocumentViewManager;

    class IView : public QObject
    {
        Q_OBJECT
    public:
        IView(IDocument *document);
        virtual ~IView();

        IDocument* document() const;

        virtual QWidget* toWidget() const = 0;
        virtual IContext* context() const = 0;

        virtual void setZoom(qreal percentage) = 0;

        virtual IView* duplicate() const = 0;

    Q_SIGNALS:
        void focussedIn(IView *who);
        void closed(IView *who);

    protected:
        IDocument * const m_document;

        friend class DocumentViewManager;
    };
} // namespace Caneda

#endif //CANEDA_IVIEW_H
