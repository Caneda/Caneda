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
#include "tabs.h"

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

    //! \brief Constructor.
    DocumentViewManager::DocumentViewManager(QObject *parent) : QObject(parent)
    {
        setupContexts();
    }

    //! \brief Destructor.
    DocumentViewManager::~DocumentViewManager()
    {
        qDeleteAll(m_documentDataList);
        m_documentDataList.clear();
    }

    //! \copydoc MainWindow::instance()
    DocumentViewManager* DocumentViewManager::instance()
    {
        static DocumentViewManager* instance = 0;
        if (!instance) {
            instance = new DocumentViewManager();
        }
        return instance;
    }

    IView* DocumentViewManager::createView(IDocument *document)
    {
        IView *newView = document->createView();
        if (!newView) {
            qWarning() << Q_FUNC_INFO << "View creation failed";
            return 0;
        }

        DocumentData *data = documentDataForDocument(document);
        data->views.insert(0, newView);

        connect(newView, SIGNAL(focussedIn(IView*)), this,
                SLOT(onViewFocussedIn(IView*)));

        emit changed();

        return newView;
    }

    //! \brief Raises and makes focus on an opened view
    void DocumentViewManager::highlightView(IView *view)
    {
        tabWidget()->highlightView(view);
    }

    //! \brief Raises and makes focus on the view corresponding to a document
    void DocumentViewManager::highlightViewForDocument(IDocument *document)
    {
        DocumentData *data = documentDataForDocument(document);

        IView *view = 0;

        // If the document was just opened, a view must be created
        if (data->views.isEmpty()) {
            view = createView(document);
            tabWidget()->addTab(new Tab(view));
        }
        else {
            // Grab the first view of the document
            view = viewsForDocument(document).first();
        }

        highlightView(view);
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
        emit changed();

        highlightViewForDocument(data->document);
    }

    bool DocumentViewManager::openFile(const QString &fileName)
    {
        if (fileName.isEmpty()) {
            return false;
        }

        DocumentData *data = documentDataForFileName(fileName);

        // If the file is already opened, first close it to refresh the file
        // contents. This allows external programs to modify the data, and
        // refresh the data upon next opening. This is used, for example,
        // during simulations, when the simulation engine changes the waveform
        // file contents.
        if (data) {
            // Grab the first view of the document
            IView *view = 0;
            if (!data->views.isEmpty()) {
                view = viewsForDocument(data->document).first();
            }
            closeViewHelper(view, true, true);

            data = documentDataForFileName(fileName);
        }

        // Open the file which will create corresponding DocumentData
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
                    emit changed();

                    break;
                }
            }
        }

        // Once the document is loaded, go to the corresponding view
        if (data) {
            highlightViewForDocument(data->document);
        }

        return data != 0;
    }

    bool DocumentViewManager::saveDocuments(const QList<IDocument*> &documents)
    {
        // First collect only the modified documents and prompt for save.
        QList<IDocument*> modifiedDocuments;
        foreach (IDocument *document, documents) {
            if (document->isModified()) {
                modifiedDocuments << document;
            }
        }

        if (modifiedDocuments.isEmpty()) {
            return true;
        }

        QPointer<SaveDocumentsDialog> dialog(new SaveDocumentsDialog(modifiedDocuments));
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

        delete dialog.data();

        return true;
    }

    bool DocumentViewManager::closeDocuments(const QList<IDocument*> &documents,
            bool askForSave)
    {
        if (askForSave && !saveDocuments(documents)) {
            return false;
        }

        int closedDocumentsCount = 0;
        bool closedAllDocuments = true;

        foreach (IDocument *document, documents) {
            DocumentData *data = documentDataForDocument(document);

            Q_ASSERT(data != 0);

            bool closedAllViews = true;
            foreach (IView *view, data->views) {
                const bool askForSave = false;
                const bool closeDocIfLastView = false;
                if (!closeViewHelper(view, askForSave, closeDocIfLastView)) {
                    closedAllViews = false;
                    break;
                }
            }

            if (!closedAllViews) {
                closedAllDocuments = false;
                break;
            }

            m_documentDataList.removeAll(data);

            delete data->document;
            data->document = 0;

            delete data;
            ++closedDocumentsCount;
        }

        if (closedDocumentsCount > 0) {
            emit changed();
        }

        return closedAllDocuments;
    }

    QStringList DocumentViewManager::fileNameFilters() const
    {
        QStringList nameFilters;

        nameFilters << QObject::tr("Any File (*)")+" (*);;";
        nameFilters << QObject::tr("Caneda project (*.xpro)")+" (*.xpro);;";

        foreach (IContext *context, m_contexts) {
            nameFilters << context->fileNameFilters();
        }

        return nameFilters;
    }

    bool DocumentViewManager::splitView(IView *view, Qt::Orientation orientation)
    {
        IView *newView = createView(view->document());
        if (!newView) {
            return false;
        }

        Tab *tab = tabWidget()->tabForView(view);
        tab->splitView(view, newView, orientation);

        return true;
    }

    bool DocumentViewManager::closeView(IView *view, bool askForSave)
    {
        return closeViewHelper(view, askForSave, true);
    }

    bool DocumentViewManager::closeViewHelper(IView *view, bool askSaveIfModified,
            bool closeDocumentIfLastView)
    {
        DocumentData *data = documentDataForDocument(view->document());

        if (data->document->isModified()) {
            if (askSaveIfModified) {
                QList<IDocument*> list;
                list << data->document;
                if (saveDocuments(list) == false) {
                    return false;
                }
            }
        }

        tabWidget()->closeView(view);

        data->views.removeAll(view);
        delete view;

        emit changed();


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

    void DocumentViewManager::replaceView(IView *oldView, IDocument *document)
    {
        DocumentData *data = documentDataForDocument(document);

        IView *newView = createView(document);
        if (!newView) {
            return;
        }

        tabWidget()->replaceView(oldView, newView);

        DocumentData *oldData = documentDataForDocument(oldView->document());
        oldData->views.removeAll(oldView);

        // Delete later is used since this method is indirectly invoked through
        // oldView's slot and a QObject can't be deleted when its own slot call
        oldView->deleteLater();

        emit changed();
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
        m_contexts << SymbolContext::instance();
        m_contexts << LayoutContext::instance();
        m_contexts << SimulationContext::instance();
        m_contexts << TextContext::instance();
        m_contexts << WebContext::instance();
    }

    TabWidget* DocumentViewManager::tabWidget() const
    {
        return MainWindow::instance()->tabWidget();
    }

} // namespace Caneda
