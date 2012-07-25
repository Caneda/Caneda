/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2012 by Pablo Daniel Pareja Obregon                  *
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
#include "library.h"
#include "mainwindow.h"
#include "schematicdocument.h"
#include "settings.h"
#include "sidebarbrowser.h"
#include "singletonowner.h"
#include "statehandler.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStringList>

namespace Caneda
{
    SchematicContext::SchematicContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarBrowser = new SidebarBrowser();
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        // Load schematic libraries
        LibraryManager *libraryManager = LibraryManager::instance();
        if(libraryManager->loadLibraryTree()) {
            qDebug() << "Succesfully loaded libraries!";
        }
        else {
            // Invalidate entry
            qDebug() << "Error loading component libraries";
            qDebug() << "Please set the appropriate libraries through Application settings and restart the application.";
        }

        QList<QPair<QString, QPixmap> > paintingItems;
        paintingItems << qMakePair(QObject::tr("Arrow"),
                QPixmap(Caneda::bitmapDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::bitmapDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::bitmapDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::bitmapDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::bitmapDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::bitmapDirectory() + "text.svg"));

        m_sidebarBrowser->plugItem("Components", QPixmap(), "root");
        m_sidebarBrowser->plugLibrary("Passive", "Components");
        m_sidebarBrowser->plugLibrary("Active", "Components");
        m_sidebarBrowser->plugLibrary("Semiconductor", "Components");
        m_sidebarBrowser->plugItems(paintingItems, QObject::tr("Paint Tools"));
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

    QWidget* SchematicContext::sideBarWidget()
    {
        return m_sidebarBrowser;
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
        nameFilters << QObject::tr("Schematic-xml (*.xsch)")+" (*.xsch);;";

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

    void SchematicContext::slotSimulate()
    {
        setNormalAction();
        //TODO: implement this
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
