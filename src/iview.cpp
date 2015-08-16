/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "iview.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "csimulationscene.h"
#include "csimulationview.h"
#include "documentviewmanager.h"
#include "icontext.h"
#include "idocument.h"
#include "global.h"
#include "statehandler.h"
#include "textedit.h"
#include "webpage.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFileInfo>
#include <QFontInfo>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

namespace Caneda
{
    /*************************************************************************
     *                               IView                                   *
     *************************************************************************/
    /*!
     * \fn IView::document()
     *
     * \brief Returns the document represented by this view.
     */

    /*!
     * \fn IView::toWidget()
     *
     * \brief Returns this view as a QWidget.
     * This has many advantages, like to add this view to tab widget,
     * connect signals/slots which expects a QObjet pointer etc.
     */

    /*!
     * \fn IView::context()
     *
     * \brief Returns the context that handles documents, views of specific type.
     *
     * Each new document type must create only one context object, shared by
     * all documents of the same type and all document views. This is
     * implemented by using the context classes as singleton classes, and their
     * only static instance (returned by instance()) must be used.
     *
     * \sa IContext
     */

    //! \brief Constructor.
    IView::IView(IDocument *document) : m_document(document)
    {
        Q_ASSERT(document != 0);
        m_toolBar = new QToolBar;

        DocumentViewManager *manager = DocumentViewManager::instance();
        connect(manager, SIGNAL(changed()), this, SLOT(onDocumentViewManagerChanged()));

        m_documentSelector = new QComboBox(m_toolBar);
        connect(m_documentSelector, SIGNAL(currentIndexChanged(int)), this,
                SLOT(onDocumentSelectorIndexChanged(int)));


        QToolButton *splitHorizontalButton = new QToolButton(m_toolBar);
        splitHorizontalButton->setIcon(Caneda::icon("view-split-left-right"));
        QToolButton *splitVerticalButton = new QToolButton(m_toolBar);
        splitVerticalButton->setIcon(Caneda::icon("view-split-top-bottom"));
        QToolButton *closeViewButton = new QToolButton(m_toolBar);
        closeViewButton->setIcon(Caneda::icon("view-left-close"));

        connect(splitHorizontalButton, SIGNAL(clicked()), this, SLOT(slotSplitHorizontal()));
        connect(splitVerticalButton, SIGNAL(clicked()), this, SLOT(slotSplitVertical()));
        connect(closeViewButton, SIGNAL(clicked()), this, SLOT(slotCloseView()));

        m_toolBar->addWidget(m_documentSelector);
        m_toolBar->addSeparator();
        m_toolBar->addWidget(splitHorizontalButton);
        m_toolBar->addWidget(splitVerticalButton);
        m_toolBar->addWidget(closeViewButton);

        onDocumentViewManagerChanged();
    }

    IDocument* IView::document() const
    {
        return m_document;
    }

    QToolBar* IView::toolBar() const
    {
        return m_toolBar;
    }

    void IView::onDocumentViewManagerChanged()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        QList<IDocument*> documents = manager->documents();

        int index = documents.indexOf(m_document);

        if (index < 0) {
            return;
        }

        m_documentSelector->blockSignals(true);
        m_documentSelector->clear();
        foreach (IDocument *document, documents) {
            QString text = document->fileName();
            if (text.isEmpty()) {
                text = tr("Untitled");
            } else {
                text = QFileInfo(text).fileName();
            }
            m_documentSelector->addItem(text);
        }
        m_documentSelector->setCurrentIndex(index);
        m_documentSelector->blockSignals(false);
    }

    void IView::onDocumentSelectorIndexChanged(int index)
    {
        if (index < 0) {
            return;
        }

        DocumentViewManager *manager = DocumentViewManager::instance();
        // This call will result in this view being destructed!
        manager->replaceView(this, manager->documents()[index]);
    }


    void IView::slotSplitHorizontal()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->splitView(this, Qt::Horizontal);
    }

    void IView::slotSplitVertical()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->splitView(this, Qt::Vertical);
    }

    void IView::slotCloseView()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->closeView(this);
    }


    /*************************************************************************
     *                             LayoutView                                *
     *************************************************************************/
    //! \brief Constructor.
    LayoutView::LayoutView(LayoutDocument *document) :
        IView(document)
    {
        m_cGraphicsView = new CGraphicsView(document->cGraphicsScene());
        StateHandler::instance()->registerWidget(m_cGraphicsView);
        connect(m_cGraphicsView, SIGNAL(focussedIn(CGraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_cGraphicsView, SIGNAL(focussedOut(CGraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_cGraphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    QWidget* LayoutView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* LayoutView::context() const
    {
        return LayoutContext::instance();
    }

    void LayoutView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void LayoutView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void LayoutView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void LayoutView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    IView* LayoutView::duplicate()
    {
        return document()->createView();
    }

    void LayoutView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
    }

    void LayoutView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void LayoutView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }


    /*************************************************************************
     *                           SchematicView                               *
     *************************************************************************/
    //! \brief Constructor.
    SchematicView::SchematicView(SchematicDocument *document) :
        IView(document)
    {
        m_cGraphicsView = new CGraphicsView(document->cGraphicsScene());
        StateHandler::instance()->registerWidget(m_cGraphicsView);
        connect(m_cGraphicsView, SIGNAL(focussedIn(CGraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_cGraphicsView, SIGNAL(focussedOut(CGraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_cGraphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    QWidget* SchematicView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* SchematicView::context() const
    {
        return SchematicContext::instance();
    }

    void SchematicView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void SchematicView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void SchematicView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void SchematicView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    IView* SchematicView::duplicate()
    {
        return document()->createView();
    }

    void SchematicView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
    }

    void SchematicView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void SchematicView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }


    /*************************************************************************
     *                           SimulationView                              *
     *************************************************************************/
    //! \brief Constructor.
    SimulationView::SimulationView(SimulationDocument *document) :
        IView(document)
    {
        m_simulationView = new CSimulationView(document->cSimulationScene(), 0);
        m_simulationView->populate();

        //! \todo Reimplement this
//        connect(m_simulationView, SIGNAL(focussedIn(CSimulationView*)), this,
//                SLOT(onWidgetFocussedIn()));
//        connect(m_simulationView, SIGNAL(focussedOut(CSimulationView*)), this,
//                SLOT(onWidgetFocussedOut()));
        connect(m_simulationView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    QWidget* SimulationView::toWidget() const
    {
        return m_simulationView;
    }

    IContext* SimulationView::context() const
    {
        return SimulationContext::instance();
    }

    void SimulationView::zoomIn()
    {
        m_simulationView->zoomIn();
    }

    void SimulationView::zoomOut()
    {
        m_simulationView->zoomOut();
    }

    void SimulationView::zoomFitInBest()
    {
        m_simulationView->zoomFitInBest();
    }

    void SimulationView::zoomOriginal()
    {
        m_simulationView->zoomOriginal();
    }

    IView* SimulationView::duplicate()
    {
        return document()->createView();
    }

    void SimulationView::updateSettingsChanges()
    {
        m_simulationView->loadUserSettings();
        m_simulationView->replot();
    }

    void SimulationView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void SimulationView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }


    /*************************************************************************
     *                             SymbolView                                *
     *************************************************************************/
    //! \brief Constructor.
    SymbolView::SymbolView(SymbolDocument *document) :
        IView(document)
    {
        m_cGraphicsView = new CGraphicsView(document->cGraphicsScene());
        StateHandler::instance()->registerWidget(m_cGraphicsView);
        connect(m_cGraphicsView, SIGNAL(focussedIn(CGraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_cGraphicsView, SIGNAL(focussedOut(CGraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_cGraphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    QWidget* SymbolView::toWidget() const
    {
        return m_cGraphicsView;
    }

    IContext* SymbolView::context() const
    {
        return SymbolContext::instance();
    }

    void SymbolView::zoomIn()
    {
        m_cGraphicsView->zoomIn();
    }

    void SymbolView::zoomOut()
    {
        m_cGraphicsView->zoomOut();
    }

    void SymbolView::zoomFitInBest()
    {
        m_cGraphicsView->zoomFitInBest();
    }

    void SymbolView::zoomOriginal()
    {
        m_cGraphicsView->zoomOriginal();
    }

    IView* SymbolView::duplicate()
    {
        return document()->createView();
    }

    void SymbolView::updateSettingsChanges()
    {
        m_cGraphicsView->invalidateScene();
        m_cGraphicsView->resetCachedContent();
    }

    void SymbolView::onWidgetFocussedIn()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void SymbolView::onWidgetFocussedOut()
    {
        emit focussedOut(static_cast<IView*>(this));
    }


    /*************************************************************************
     *                              TextView                                 *
     *************************************************************************/
    //! \brief Constructor.
    TextView::TextView(TextDocument *document) :
        IView(document),
        m_zoomRange(6.0, 30.0),
        m_originalZoom(QFontInfo(qApp->font()).pointSizeF())
    {
        m_currentZoom = m_originalZoom;
        m_textEdit = new TextEdit(document->textDocument());

        connect(m_textEdit, SIGNAL(focussed()), this,
                SLOT(onFocussed()));
        connect(m_textEdit, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    QWidget* TextView::toWidget() const
    {
        return m_textEdit;
    }

    IContext* TextView::context() const
    {
        return TextContext::instance();
    }

    void TextView::zoomIn()
    {
        setZoomLevel(m_currentZoom + 1);
    }

    void TextView::zoomOut()
    {
        setZoomLevel(m_currentZoom - 1);
    }

    void TextView::zoomFitInBest()
    {
        setZoomLevel(4);
    }

    void TextView::zoomOriginal()
    {
        setZoomLevel(m_originalZoom);
    }

    IView* TextView::duplicate()
    {
        return document()->createView();
    }

    void TextView::updateSettingsChanges()
    {
    }

    void TextView::onFocussed()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void TextView::setZoomLevel(qreal zoomLevel)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        if (qFuzzyCompare(zoomLevel, m_currentZoom)) {
            return;
        }

        m_currentZoom = zoomLevel;

        m_textEdit->setPointSize(m_currentZoom);
    }


    /*************************************************************************
     *                               WebView                                 *
     *************************************************************************/
    //! \brief Constructor.
    WebView::WebView(WebDocument *document) :
        IView(document),
        m_zoomRange(0.4, 10.0),
        m_originalZoom(QFontInfo(qApp->font()).pointSizeF()/10)
    {
        m_currentZoom = m_originalZoom;
        m_webPage = new WebPage(document->webUrl());

        connect(m_webPage, SIGNAL(focussed()), this, SLOT(onFocussed()));
        connect(m_webPage, SIGNAL(anchorClicked(QUrl)), this, SLOT(updateUrl(QUrl)));
    }

    QWidget* WebView::toWidget() const
    {
        return m_webPage;
    }

    IContext* WebView::context() const
    {
        return WebContext::instance();
    }

    void WebView::zoomIn()
    {
        setZoomLevel(m_currentZoom + 0.1);
    }

    void WebView::zoomOut()
    {
        setZoomLevel(m_currentZoom - 0.1);
    }

    void WebView::zoomFitInBest()
    {
        setZoomLevel(2);
    }

    void WebView::zoomOriginal()
    {
        setZoomLevel(m_originalZoom);
    }

    IView* WebView::duplicate()
    {
        return document()->createView();
    }

    void WebView::updateSettingsChanges()
    {
    }

    void WebView::onFocussed()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

    void WebView::updateUrl(const QUrl& link)
    {
        document()->setFileName(link.toString());

    }

    void WebView::setZoomLevel(qreal zoomLevel)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        if (qFuzzyCompare(zoomLevel, m_currentZoom)) {
            return;
        }

        m_currentZoom = zoomLevel;

        m_webPage->setPointSize(m_currentZoom);
    }

} // namespace Caneda
