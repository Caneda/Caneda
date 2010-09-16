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

#include "iview.h"

#include "documentviewmanager.h"
#include "idocument.h"
#include "global.h"

#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QToolBar>

namespace Caneda
{
    /*!
     * \class IView
     *
     * This class serves as an interface for any View visualization supported by
     * Caneda.
     */

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
     * \note It is enough to create the context object only once per new type.
     * \see IContext
     */

    /*!
     * \fn IView::setZoom(qreal percentage)
     *
     * \brief Zooms the view to the value pointed by percentage argument.
     */

    IView::IView(IDocument *document) : m_document(document)
    {
        Q_ASSERT(document != 0);
        m_toolBar = new QToolBar;

        DocumentViewManager *manager = DocumentViewManager::instance();
        connect(manager, SIGNAL(changed()), this, SLOT(onDocumentViewManagerChanged()));

        m_documentSelector = new QComboBox(m_toolBar);
        connect(m_documentSelector, SIGNAL(currentIndexChanged(int)), this,
                SLOT(onDocumentSelectorIndexChanged(int)));


        QToolButton *_splitHorizontalButton = new QToolButton(m_toolBar);
        _splitHorizontalButton->setIcon(Caneda::icon("view-split-left-right"));
        QToolButton *_splitVerticalButton = new QToolButton(m_toolBar);
        _splitVerticalButton->setIcon(Caneda::icon("view-split-top-bottom"));
        QToolButton *_closeViewButton = new QToolButton(m_toolBar);
        _closeViewButton->setIcon(Caneda::icon("view-left-close"));

        connect(_splitHorizontalButton, SIGNAL(clicked()), this, SLOT(slotSplitHorizontal()));
        connect(_splitVerticalButton, SIGNAL(clicked()), this, SLOT(slotSplitVertical()));
        connect(_closeViewButton, SIGNAL(clicked()), this, SLOT(slotCloseView()));

        m_toolBar->addWidget(m_documentSelector);
        m_toolBar->addSeparator();
        m_toolBar->addWidget(_splitHorizontalButton);
        m_toolBar->addWidget(_splitVerticalButton);
        m_toolBar->addWidget(_closeViewButton);

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

} // namespace Caneda
