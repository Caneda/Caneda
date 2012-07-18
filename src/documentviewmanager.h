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

#ifndef DOCUMENT_VIEW_MANAGER_H
#define DOCUMENT_VIEW_MANAGER_H

#include <QObject>

namespace Caneda
{
    class DocumentData;
    class IContext;
    class IDocument;
    class IView;
    class TabWidget;

    class DocumentViewManager : public QObject
    {
        Q_OBJECT

    public:
        static DocumentViewManager* instance();
        ~DocumentViewManager();

        IView* createView(IDocument *document);

        void highlightView(IView *view);
        void highlightViewForDocument(IDocument *document);

        void newDocument(IContext *context);
        bool openFile(const QString &fileName);
        bool saveDocuments(const QList<IDocument*> &documents);
        bool closeDocuments(const QList<IDocument*> &documents, bool askForSave = true);

        QStringList fileNameFilters() const;

        bool splitView(IView *view, Qt::Orientation orientation);
        bool closeView(IView *view, bool askForSave = true);
        void replaceView(IView *view, IDocument *withViewOf);

        IDocument* currentDocument() const;
        IView* currentView() const;

        IDocument* documentForFileName(const QString &fileName) const;
        QList<IView*> viewsForDocument(const IDocument *document) const;
        QList<IDocument*> documents() const;

        void updateSettingsChanges();

    Q_SIGNALS:
        void changed();

    private Q_SLOTS:
        void onViewFocussedIn(IView *who);
        void openLayout();
        void openSchematic();
        void openSymbol();

    private:
        DocumentViewManager(QObject *parent = 0);
        DocumentData* documentDataForFileName(const QString &fileName) const;
        DocumentData* documentDataForDocument(IDocument *document) const;
        bool closeViewHelper(IView *view, bool askForSave, bool closeDocumentIfLastView);

        void setupContexts();
        TabWidget* tabWidget() const;

        void openFileFormat(const QString &suffix);

        QList<DocumentData*> m_documentDataList;
        QList<IContext*> m_contexts;
    };

}

#endif //DOCUMENT_VIEW_MANAGER_H
