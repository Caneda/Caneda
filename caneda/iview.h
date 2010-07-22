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

#include <QDebug>
#include <QObject>

// Forward declaration
class QComboBox;
class QToolBar;
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

        virtual void zoomIn() = 0;
        virtual void zoomOut() = 0;
        virtual void zoomFitInBest() = 0;
        virtual void zoomOriginal() = 0;

        virtual IView* duplicate() = 0;

        virtual void updateSettingsChanges() = 0;

        QToolBar* toolBar() const;

    Q_SIGNALS:
        void focussedIn(IView *who);
        void focussedOut(IView *who);
        void closed(IView *who);
        void statusBarMessage(const QString &text);

    private Q_SLOTS:
        void onDocumentViewManagerChanged();
        void onDocumentSelectorIndexChanged(int index);

    protected:
        IDocument * const m_document;
        QToolBar *m_toolBar;
        QComboBox *m_documentSelector;

        friend class DocumentViewManager;
    };

} // namespace Caneda

#endif //CANEDA_IVIEW_H
