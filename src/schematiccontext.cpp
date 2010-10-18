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

#include "schematiccontext.h"

#include "actionmanager.h"
#include "documentviewmanager.h"
#include "global.h"
#include "mainwindow.h"
#include "schematicdocument.h"
#include "singletonowner.h"
#include "statehandler.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>

namespace Caneda
{
    SchematicContext::SchematicContext(QObject *parent) : IContext(parent)
    {
    }

    SchematicContext* SchematicContext::instance()
    {
        static SchematicContext *context = 0;
        if (!context) {
            context = new SchematicContext(SingletonOwner::instance());
        }
        return context;
    }

    SchematicContext::~SchematicContext()
    {

    }

    void SchematicContext::init()
    {
    }

    bool SchematicContext::canOpen(const QFileInfo &info) const
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

    QStringList SchematicContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Schematic-xml")+" (*.xsch);;";

        return nameFilters;
    }

    IDocument* SchematicContext::newDocument()
    {
        return new SchematicDocument;
    }

    IDocument* SchematicContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SchematicDocument *document = new SchematicDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

    void SchematicContext::addNormalAction(Action *action)
    {
        m_normalActions << action;
    }

    void SchematicContext::addMouseAction(Action *action)
    {
        m_mouseActions << action;
    }

    /*!
     * \brief Opens the current schematics' symbol for editing
     */
    void SchematicContext::slotSymbolEdit()
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

    void SchematicContext::slotIntoHierarchy()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void SchematicContext::slotPopHierarchy()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void SchematicContext::slotSnapToGrid(bool snap)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SchematicDocument *schDoc = qobject_cast<SchematicDocument*>(doc);

        if (schDoc) {
            schDoc->cGraphicsScene()->setSnapToGrid(snap);
        }
    }

    //! \brief Align elements in a row correponding to top most elements coords.
    void SchematicContext::slotAlignTop()
    {
        alignElements(Qt::AlignTop);
    }

    //! \brief Align elements in a row correponding to bottom most elements coords.
    void SchematicContext::slotAlignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    //! \brief Align elements in a column correponding to left most elements coords.
    void SchematicContext::slotAlignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    /*!
     * \brief Align elements in a column correponding to right most elements
     * coords.
     */
    void SchematicContext::slotAlignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void SchematicContext::slotDistributeHorizontal()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SchematicDocument *schDoc = qobject_cast<SchematicDocument*>(doc);

        if (schDoc) {
            if (!schDoc->cGraphicsScene()->distributeElements(Qt::Horizontal)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void SchematicContext::slotDistributeVertical()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SchematicDocument *schDoc = qobject_cast<SchematicDocument*>(doc);

        if (schDoc) {
            if (!schDoc->cGraphicsScene()->distributeElements(Qt::Vertical)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void SchematicContext::slotCenterHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void SchematicContext::slotCenterVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    void SchematicContext::slotInsertEntity()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void SchematicContext::slotToPage()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void SchematicContext::slotDCbias()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void SchematicContext::slotExportGraphAsCsv()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void SchematicContext::alignElements(Qt::Alignment alignment)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SchematicDocument *schDoc = qobject_cast<SchematicDocument*>(doc);

        if (schDoc) {
            if (!schDoc->cGraphicsScene()->alignElements(alignment)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void SchematicContext::setNormalAction()
    {
        MainWindow::instance()->setNormalAction();
    }

} // namespace Caneda
