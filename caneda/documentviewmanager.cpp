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

#include "documentviewmanager.h"

#include "icontext.h"
#include "idocument.h"
#include "iview.h"
#include "mainwindow.h"
#include "schematiccontext.h"
#include "singletonowner.h"
#include "tabs.h"
#include "textcontext.h"

#include "dialogs/savedocumentsdialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>

namespace Caneda
{
    struct DocumentData
    {
        IDocument *document;
        QList<IView*> views;

        DocumentData() { document = 0; }
    };

    DocumentViewManager::DocumentViewManager(QObject *parent) : QObject(parent)
    {
        setupContexts();
    }

    DocumentViewManager::~DocumentViewManager()
    {
        qDeleteAll(m_documentDataList);
        m_documentDataList.clear();
    }

    DocumentViewManager* DocumentViewManager::instance()
    {
        static DocumentViewManager* instance = 0;
        if (!instance) {
            instance = new DocumentViewManager(SingletonOwner::instance());
        }
        return instance;
    }

    void DocumentViewManager::highlightView(IView *view)
    {
        tabWidget()->highlightView(view);
    }

    void DocumentViewManager::highlightViewForDocument(IDocument *document)
    {
        DocumentData *data = documentDataForDocument(document);

        if (data->views.isEmpty()) {
            IContext *context = document->context();
            IView *newView = document->createView();
            if (!newView) {
                qWarning() << Q_FUNC_INFO << "View creation failed";
                return;
            }
            data->views << newView;

            connect(newView, SIGNAL(focussedIn(IView*)), this,
                    SLOT(onViewFocussedIn(IView*)));

            tabWidget()->addTab(new Tab(newView));
        }

        highlightView(data->views.first());
    }

    void DocumentViewManager::newDocument(IContext *context)
    {
        IDocument *document = context->newDocument();
        if (!document) {
            return;
        }

        DocumentData *data = new DocumentData;
        data->document = document;
        m_documentDataList << data;

        highlightViewForDocument(data->document);
    }

    bool DocumentViewManager::openFile(const QString &fileName)
    {
        if (fileName.isEmpty()) return false;

        DocumentData *data = documentDataForFileName(fileName);

        if (!data) {
            QFileInfo fileInfo(fileName);
            foreach (IContext *context, m_contexts) {
                if (context->canOpen(fileInfo)) {
                    IDocument *document = context->open(fileName);
                    if (!document) {
                        continue;
                    }

                    data = new DocumentData;
                    data->document = document;
                    m_documentDataList << data;

                    break;
                }
            }
        }

        if (data) {
            highlightViewForDocument(data->document);
        }

        return data != 0;
    }

    bool DocumentViewManager::closeFile(const QString &fileName)
    {
        DocumentData *data = documentDataForFileName(fileName);

        if (data) {
            QList<IDocument*> documents;
            documents << data->document;
            return closeDocuments(documents);
        }

        return false;
    }

    bool DocumentViewManager::saveDocuments(const QList<IDocument*> &documents)
    {
        QList<IDocument*> modifiedDocuments;

        foreach (IDocument *document, documents) {
            if (document->isModified()) {
                modifiedDocuments << document;
            }
        }

        if (!modifiedDocuments.isEmpty()) {
            QPointer<SaveDocumentsDialog> dialog(new SaveDocumentsDialog(modifiedDocuments, 0));
            dialog->exec();

            int result = dialog->result();

            if (result == SaveDocumentsDialog::DoNotSave) {
                return true;
            }
            else if (result == SaveDocumentsDialog::Abort) {
                return false;
            }
            else {
                QList<QPair<IDocument*, QString> > newFilePaths = dialog->newFilePaths();
                QList<QPair<IDocument*, QString> >::iterator it;

                bool failedInBetween = false;
                for (it = newFilePaths.begin(); it != newFilePaths.end(); ++it) {
                    IDocument *document = it->first;
                    const QString newFileName = it->second;
                    QString oldFileName = document->fileName();

                    document->setFileName(newFileName);
                    if (!document->save()) {
                        failedInBetween = true;
                        document->setFileName(oldFileName);
                    }
                }

                if (failedInBetween) {
                    QMessageBox::critical(0, tr("File save error"),
                            tr("Could not save some files"));
                    return false;
                }
            }
        }

        return true;
    }

    bool DocumentViewManager::closeDocuments(const QList<IDocument*> &documents,
            bool askForSave)
    {
        if (askForSave && !saveDocuments(documents)) {
            return false;
        }

        foreach (IDocument *document, documents) {
            DocumentData *data = documentDataForDocument(document);

            Q_ASSERT(data != 0);

            foreach (IView *view, data->views) {
                const bool askForSave = false;
                const bool closeDocIfLastView = false;
                if (!closeViewHelper(view, askForSave, closeDocIfLastView)) {
                    return false;
                }
            }

            m_documentDataList.removeAll(data);

            delete data->document;
            data->document = 0;

            delete data;
        }

        return true;
    }

    bool DocumentViewManager::splitView(IView *view, Qt::Orientation orientation)
    {
        IView *newView = view->duplicate();
        if (!newView) {
            return false;
        }

        DocumentData *data = documentDataForDocument(view->document());
        data->views.insert(0, newView);

        Tab *tab = tabWidget()->tabForView(view);
        if (!tab) {
            qDebug() << Q_FUNC_INFO << "View does not have Tab as its ancestor";
            delete newView;
            return false;
        }

        tab->splitView(view, newView, orientation);
        return true;
    }

    bool DocumentViewManager::saveView(IView *view)
    {
        QList<IDocument*> list;
        list << view->document();
        return saveDocuments(list);
    }

    bool DocumentViewManager::closeView(IView *view, bool askForSave)
    {
        return closeViewHelper(view, askForSave, true);
    }

    bool DocumentViewManager::closeViewHelper(IView *view, bool askSaveIfModified,
            bool closeDocumentIfLastView)
    {
        DocumentData *data = documentDataForDocument(view->document());

        Q_ASSERT(data != 0);
        if (data->document->isModified()) {
            if (askSaveIfModified) {
                QList<IDocument*> list;
                list << data->document;
                if (saveDocuments(list) == false) {
                    return false;
                }
            } else {
                if (false && data->views.size() == 1) {
                    // Don't close if modified, only one view exists and
                    // if asking for save isn't enabled.
                    return false;
                }
            }
        }

        // At this point its ensured that before closing, either there is
        // another view for same document or the document is saved.

        data->views.removeAll(view);

        tabWidget()->closeView(view);

        delete view;

        // Now if closeDocumentIfLastView is true and there are no more views
        // for the document, then just close the document too.
        if (closeDocumentIfLastView && data->views.isEmpty()) {
            QList<IDocument*> list;
            list << data->document;
            // Though recursing back to closeDocuments, the empty view list assists in
            // not having infinite recursion and also we don't need to ask user to save again.
            bool askForSaveAgain = false;
            closeDocuments(list, askForSaveAgain);
        }


        return true;
    }

    IDocument* DocumentViewManager::currentDocument() const
    {
        IView *view = currentView();
        if (view) {
            return view->document();
        }
        return 0;
    }

    IView* DocumentViewManager::currentView() const
    {
        Tab *tab = tabWidget()->currentTab();
        if (!tab) {
            return 0;
        }

        return tab->activeView();
    }

    IDocument* DocumentViewManager::documentForFileName(const QString &fileName) const
    {
        DocumentData *data = documentDataForFileName(fileName);
        if (data) {
            return data->document;
        }

        return 0;
    }

    QList<IView*> DocumentViewManager::viewsForDocument(const IDocument *document) const
    {
        QList<DocumentData*>::const_iterator it = m_documentDataList.constBegin(),
            end = m_documentDataList.constEnd();
        for (; it != end; ++it) {
            if ((*it)->document == document) {
                return (*it)->views;
            }
        }

        qDebug() << Q_FUNC_INFO << "Document " << (void*)document
            << " is not in manager's document data list";

        return QList<IView*>();
    }

    QList<IDocument*> DocumentViewManager::documents() const
    {
        QList<IDocument*> documents;
        QList<DocumentData*>::const_iterator it = m_documentDataList.constBegin(),
            end = m_documentDataList.constEnd();
        for (; it != end; ++it) {
            documents << (*it)->document;
        }

        return documents;
    }

    void DocumentViewManager::updateSettingsChanges()
    {
        QList<DocumentData*>::iterator it = m_documentDataList.begin(),
            end = m_documentDataList.end();
        for (; it != end; ++it) {
            DocumentData *data = *it;
            data->document->updateSettingsChanges();

            QList<IView*>::iterator jt = data->views.begin(),
                jend = data->views.end();
            for (; jt != jend; ++jt) {
                (*jt)->updateSettingsChanges();
            }
        }
    }

    void DocumentViewManager::onViewFocussedIn(IView *view)
    {
        DocumentData *data = documentDataForDocument(view->document());
        if (!data) {
            return;
        }

        // Apply LRU algo by pushing the least recently used
        // view to the front of the list.
        int i = data->views.indexOf(view);
        if (i >= 0 && i < data->views.size()) {
            data->views.takeAt(i);
            data->views.insert(0, view);
        }
    }

    DocumentData* DocumentViewManager::documentDataForFileName(const QString &fileName) const
    {
        if (fileName.isEmpty()) {
            return 0;
        }

        QList<DocumentData*>::const_iterator it = m_documentDataList.constBegin(),
            end = m_documentDataList.constEnd();

        for (; it != end; ++it) {
            if ((*it)->document->fileName() == fileName) {
                return *it;
            }
        }

        return 0;
    }

    DocumentData* DocumentViewManager::documentDataForDocument(IDocument *document) const
    {
        QList<DocumentData*>::const_iterator it = m_documentDataList.constBegin(),
            end = m_documentDataList.end();

        for (; it != end; ++it) {
            if ((*it)->document == document) {
                return *it;
            }
        }

        return 0;
    }

    void DocumentViewManager::setupContexts()
    {
        m_contexts << SchematicContext::instance();
        m_contexts << TextContext::instance();
    }

    TabWidget* DocumentViewManager::tabWidget() const
    {
        return MainWindow::instance()->tabWidget();
    }

}
