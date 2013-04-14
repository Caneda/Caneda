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

#include "schematiccontext.h"

#include "library.h"
#include "schematicdocument.h"
#include "sidebarbrowser.h"
#include "singletonowner.h"
#include "statehandler.h"

#include <QDebug>
#include <QFileInfo>
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
            // Plug the components root
            m_sidebarBrowser->plugItem("Components", QPixmap(), "root");

            // Get the libraries list and sort them alphabetically
            QStringList libraries(libraryManager->librariesList());
            libraries.sort();

            // Plug each library into the sidebar browser
            foreach(const QString library, libraries) {
                m_sidebarBrowser->plugLibrary(library, "Components");
                qDebug() << "Loaded " + library + " library";
            }

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

} // namespace Caneda
