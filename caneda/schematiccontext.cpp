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
#include "mainwindow.h"
#include "schematicdocument.h"
#include "schematicstatehandler.h"
#include "singletonowner.h"

#include "caneda-tools/global.h"

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
        supportedSuffixes << "xsym";
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
        return QStringList();
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
        //PORT:
#if 0
        CanedaView *currentView = viewFromWidget(tabWidget()->currentWidget());
        if(!currentView) {
            return;
        }

        if(!currentView->fileName().isEmpty()) {
            QString fileName = currentView->fileName();

            if(currentView->toSchematicWidget()->schematicScene()->currentMode() == Caneda::SchematicMode) {
                //First, we try to open the corresponding symbol file
                bool isLoaded = gotoPage(fileName, Caneda::SymbolMode);

                //If it's a new symbol, we create it
                if(!isLoaded){
                    addView(new SchematicWidget(0, this));

                    CanedaView *v = viewFromWidget(tabWidget()->currentWidget());
                    SchematicScene *sc = v->toSchematicWidget()->schematicScene();
                    sc->setMode(Caneda::SymbolMode);

                    v->setFileName(fileName);
                }
            }
            else if(currentView->toSchematicWidget()->schematicScene()->currentMode() == Caneda::SymbolMode) {
                gotoPage(fileName, Caneda::SchematicMode);
            }
        }
#endif

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
            schDoc->schematicScene()->setSnapToGrid(snap);
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
            if (!schDoc->schematicScene()->distributeElements(Qt::Horizontal)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected !"));
            }
        }
    }

    void SchematicContext::slotDistributeVertical()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SchematicDocument *schDoc = qobject_cast<SchematicDocument*>(doc);

        if (schDoc) {
            if (!schDoc->schematicScene()->distributeElements(Qt::Vertical)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected !"));
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

    void SchematicContext::slotSimulate()
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

    void SchematicContext::slotShowLastMsg()
    {
        setNormalAction();
        editFile(Caneda::pathForCanedaFile("log.txt"));
    }

    void SchematicContext::slotShowLastNetlist()
    {
        setNormalAction();
        editFile(Caneda::pathForCanedaFile("netlist.txt"));
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void SchematicContext::alignElements(Qt::Alignment alignment)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SchematicDocument *schDoc = qobject_cast<SchematicDocument*>(doc);

        if (schDoc) {
            if (!schDoc->schematicScene()->alignElements(alignment)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected !"));
            }
        }
    }

    void SchematicContext::editFile(const QString &file)
    {
        MainWindow::instance()->editFile(file);
    }

    void SchematicContext::setNormalAction()
    {
        MainWindow::instance()->setNormalAction();
    }

} // namespace Caneda
