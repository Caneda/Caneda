/***************************************************************************
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

#include "symbolcontext.h"

#include "symboldocument.h"
#include "sidebarbrowser.h"
#include "statehandler.h"

#include <QFileInfo>
#include <QStringList>

namespace Caneda
{
    //! \brief Constructor.
    SymbolContext::SymbolContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarBrowser = new SidebarBrowser();
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        QList<QPair<QString, QPixmap> > paintingItems;
        paintingItems << qMakePair(QObject::tr("Arrow"),
                QPixmap(Caneda::bitmapDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::bitmapDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::bitmapDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::bitmapDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::bitmapDirectory() + "portsymbol.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::bitmapDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::bitmapDirectory() + "text.svg"));

        m_sidebarBrowser->plugItems(paintingItems, QObject::tr("Paint Tools"));
    }

    //! \copydoc MainWindow::instance()
    SymbolContext* SymbolContext::instance()
    {
        static SymbolContext *context = 0;
        if (!context) {
            context = new SymbolContext();
        }
        return context;
    }

    //! \brief Destructor.
    SymbolContext::~SymbolContext()
    {
    }

    QWidget* SymbolContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    bool SymbolContext::canOpen(const QFileInfo &info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "xsym";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList SymbolContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Symbol-xml (*.xsym)")+" (*.xsym);;";

        return nameFilters;
    }

    IDocument* SymbolContext::newDocument()
    {
        return new SymbolDocument;
    }

    IDocument* SymbolContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SymbolDocument *document = new SymbolDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

    void SymbolContext::addNormalAction(Action *action)
    {
        m_normalActions << action;
    }

    void SymbolContext::addMouseAction(Action *action)
    {
        m_mouseActions << action;
    }

} // namespace Caneda
