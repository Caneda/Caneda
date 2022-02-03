/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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
#include "chartscene.h"
#include "chartview.h"
#include "documentviewmanager.h"
#include "graphicsview.h"
#include "icontext.h"
#include "idocument.h"
#include "global.h"
#include "statehandler.h"
#include "textedit.h"

#include <QAction>
#include <QComboBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>

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
    IView::IView(IDocument *document) :
        QObject(document),
        m_document(document)
    {
        Q_ASSERT(document != 0);
        m_toolBar = new QToolBar();

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

    IView::~IView()
    {
        delete m_toolBar;
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
    LayoutView::LayoutView(LayoutDocument *document) : IView(document)
    {
        m_graphicsView = new GraphicsView(document->graphicsScene());
        connect(m_graphicsView, SIGNAL(focussedIn(GraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_graphicsView, SIGNAL(focussedOut(GraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_graphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    //! \brief Destructor.
    LayoutView::~LayoutView()
    {
        delete m_graphicsView;
    }

    QWidget* LayoutView::toWidget() const
    {
        return m_graphicsView;
    }

    IContext* LayoutView::context() const
    {
        return LayoutContext::instance();
    }

    void LayoutView::zoomIn()
    {
        m_graphicsView->zoomIn();
    }

    void LayoutView::zoomOut()
    {
        m_graphicsView->zoomOut();
    }

    void LayoutView::zoomFitInBest()
    {
        m_graphicsView->zoomFitInBest();
    }

    void LayoutView::zoomOriginal()
    {
        m_graphicsView->zoomOriginal();
    }

    IView* LayoutView::duplicate()
    {
        return document()->createView();
    }

    void LayoutView::updateSettingsChanges()
    {
        m_graphicsView->invalidateScene();
        m_graphicsView->resetCachedContent();
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
    SchematicView::SchematicView(SchematicDocument *document) : IView(document)
    {
        m_graphicsView = new GraphicsView(document->graphicsScene());
        connect(m_graphicsView, SIGNAL(focussedIn(GraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_graphicsView, SIGNAL(focussedOut(GraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_graphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    //! \brief Destructor.
    SchematicView::~SchematicView()
    {
        delete m_graphicsView;
    }

    QWidget* SchematicView::toWidget() const
    {
        return m_graphicsView;
    }

    IContext* SchematicView::context() const
    {
        return SchematicContext::instance();
    }

    void SchematicView::zoomIn()
    {
        m_graphicsView->zoomIn();
    }

    void SchematicView::zoomOut()
    {
        m_graphicsView->zoomOut();
    }

    void SchematicView::zoomFitInBest()
    {
        m_graphicsView->zoomFitInBest();
    }

    void SchematicView::zoomOriginal()
    {
        m_graphicsView->zoomOriginal();
    }

    IView* SchematicView::duplicate()
    {
        return document()->createView();
    }

    void SchematicView::updateSettingsChanges()
    {
        m_graphicsView->invalidateScene();
        m_graphicsView->resetCachedContent();
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
        m_chartView = new ChartView(document->chartScene(), 0);
        m_chartView->populate();

        //! \todo Reimplement this
//        connect(m_chartView, SIGNAL(focussedIn(ChartView*)), this,
//                SLOT(onWidgetFocussedIn()));
//        connect(m_chartView, SIGNAL(focussedOut(ChartView*)), this,
//                SLOT(onWidgetFocussedOut()));
        connect(m_chartView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    //! \brief Destructor.
    SimulationView::~SimulationView()
    {
        //! \todo Quick fix for crash when one view is closed
        // delete m_chartView;
    }

    QWidget* SimulationView::toWidget() const
    {
        return m_chartView;
    }

    IContext* SimulationView::context() const
    {
        return SimulationContext::instance();
    }

    void SimulationView::zoomIn()
    {
        m_chartView->zoomIn();
    }

    void SimulationView::zoomOut()
    {
        m_chartView->zoomOut();
    }

    void SimulationView::zoomFitInBest()
    {
        m_chartView->zoomFitInBest();
    }

    void SimulationView::zoomOriginal()
    {
        m_chartView->zoomOriginal();
    }

    IView* SimulationView::duplicate()
    {
        return document()->createView();
    }

    void SimulationView::updateSettingsChanges()
    {
        m_chartView->loadUserSettings();
        m_chartView->replot();
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
    SymbolView::SymbolView(SymbolDocument *document) : IView(document)
    {
        m_graphicsView = new GraphicsView(document->graphicsScene());
        connect(m_graphicsView, SIGNAL(focussedIn(GraphicsView*)), this,
                SLOT(onWidgetFocussedIn()));
        connect(m_graphicsView, SIGNAL(focussedOut(GraphicsView*)), this,
                SLOT(onWidgetFocussedOut()));
        connect(m_graphicsView, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    //! \brief Destructor.
    SymbolView::~SymbolView()
    {
        delete m_graphicsView;
    }

    QWidget* SymbolView::toWidget() const
    {
        return m_graphicsView;
    }

    IContext* SymbolView::context() const
    {
        return SymbolContext::instance();
    }

    void SymbolView::zoomIn()
    {
        m_graphicsView->zoomIn();
    }

    void SymbolView::zoomOut()
    {
        m_graphicsView->zoomOut();
    }

    void SymbolView::zoomFitInBest()
    {
        m_graphicsView->zoomFitInBest();
    }

    void SymbolView::zoomOriginal()
    {
        m_graphicsView->zoomOriginal();
    }

    IView* SymbolView::duplicate()
    {
        return document()->createView();
    }

    void SymbolView::updateSettingsChanges()
    {
        m_graphicsView->invalidateScene();
        m_graphicsView->resetCachedContent();
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
    TextView::TextView(TextDocument *document) : IView(document)
    {
        m_textEdit = new TextEdit(document->textDocument());
        connect(m_textEdit, SIGNAL(focussed()), this,
                SLOT(onFocussed()));
        connect(m_textEdit, SIGNAL(cursorPositionChanged(const QString &)),
                this, SIGNAL(statusBarMessage(const QString &)));
    }

    //! \brief Destructor.
    TextView::~TextView()
    {
        delete m_textEdit;
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
        m_textEdit->zoomIn();
    }

    void TextView::zoomOut()
    {
        m_textEdit->zoomOut();
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

} // namespace Caneda
