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

#include "layoutcontext.h"

#include "actionmanager.h"
#include "documentviewmanager.h"
#include "global.h"
#include "layoutdocument.h"
#include "mainwindow.h"
#include "singletonowner.h"
#include "statehandler.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>

namespace Caneda
{
    LayoutContext::LayoutContext(QObject *parent) : IContext(parent)
    {
    }

    LayoutContext* LayoutContext::instance()
    {
        static LayoutContext *context = 0;
        if (!context) {
            context = new LayoutContext(SingletonOwner::instance());
        }
        return context;
    }

    LayoutContext::~LayoutContext()
    {

    }

    void LayoutContext::init()
    {
    }

    bool LayoutContext::canOpen(const QFileInfo &info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "xsch";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList LayoutContext::fileNameFilters() const
    {
        return QStringList();
    }

    IDocument* LayoutContext::newDocument()
    {
        return new LayoutDocument;
    }

    IDocument* LayoutContext::open(const QString &fileName,
            QString *errorMessage)
    {
        LayoutDocument *document = new LayoutDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

    void LayoutContext::addNormalAction(Action *action)
    {
        m_normalActions << action;
    }

    void LayoutContext::addMouseAction(Action *action)
    {
        m_mouseActions << action;
    }

    /*!
     * \brief Opens the current schematics' symbol for editing
     */
    void LayoutContext::slotSymbolEdit()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();

        if (!document) {
            return;
        }

        if (document->fileName().isEmpty()) {
            QMessageBox::critical(0, tr("Critical"),
                    tr("Please, save schematic first!"));
            return;
        }

        QString filename = document->fileName();
        QFileInfo info(filename);
        filename = info.completeBaseName() + ".xsym";

        MainWindow::instance()->slotFileOpen(filename);
    }

    void LayoutContext::slotIntoHierarchy()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void LayoutContext::slotPopHierarchy()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void LayoutContext::slotSnapToGrid(bool snap)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        LayoutDocument *schDoc = qobject_cast<LayoutDocument*>(doc);

        if (schDoc) {
            schDoc->cGraphicsScene()->setSnapToGrid(snap);
        }
    }

    //! \brief Align elements in a row correponding to top most elements coords.
    void LayoutContext::slotAlignTop()
    {
        alignElements(Qt::AlignTop);
    }

    //! \brief Align elements in a row correponding to bottom most elements coords.
    void LayoutContext::slotAlignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    //! \brief Align elements in a column correponding to left most elements coords.
    void LayoutContext::slotAlignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    /*!
     * \brief Align elements in a column correponding to right most elements
     * coords.
     */
    void LayoutContext::slotAlignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void LayoutContext::slotDistributeHorizontal()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        LayoutDocument *schDoc = qobject_cast<LayoutDocument*>(doc);

        if (schDoc) {
            if (!schDoc->cGraphicsScene()->distributeElements(Qt::Horizontal)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void LayoutContext::slotDistributeVertical()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        LayoutDocument *schDoc = qobject_cast<LayoutDocument*>(doc);

        if (schDoc) {
            if (!schDoc->cGraphicsScene()->distributeElements(Qt::Vertical)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void LayoutContext::slotCenterHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void LayoutContext::slotCenterVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    void LayoutContext::slotInsertEntity()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void LayoutContext::slotToPage()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void LayoutContext::slotDCbias()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void LayoutContext::slotExportGraphAsCsv()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void LayoutContext::alignElements(Qt::Alignment alignment)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        LayoutDocument *schDoc = qobject_cast<LayoutDocument*>(doc);

        if (schDoc) {
            if (!schDoc->cGraphicsScene()->alignElements(alignment)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void LayoutContext::setNormalAction()
    {
        MainWindow::instance()->setNormalAction();
    }

} // namespace Caneda
