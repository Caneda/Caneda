/***************************************************************************
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#include "actionmanager.h"
#include "documentviewmanager.h"
#include "global.h"
#include "mainwindow.h"
#include "symboldocument.h"
#include "sidebarbrowser.h"
#include "singletonowner.h"
#include "statehandler.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>

namespace Caneda
{
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

    SymbolContext* SymbolContext::instance()
    {
        static SymbolContext *context = 0;
        if (!context) {
            context = new SymbolContext(SingletonOwner::instance());
        }
        return context;
    }

    SymbolContext::~SymbolContext()
    {
    }

    void SymbolContext::init()
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

    /*!
     * \brief Opens the current symbols' schematic for editing
     */
    void SymbolContext::slotSymbolEdit()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();

        if (!document) {
            return;
        }

        if (document->fileName().isEmpty()) {
            QMessageBox::critical(0, tr("Critical"),
                    tr("Please, save symbol first!"));
            return;
        }

        QString filename = document->fileName();
        QFileInfo info(filename);
        filename = info.completeBaseName() + ".xsch";

        MainWindow::instance()->slotFileOpen(filename);
    }

    //! \brief Align elements in a row correponding to top most elements coords.
    void SymbolContext::slotAlignTop()
    {
        alignElements(Qt::AlignTop);
    }

    //! \brief Align elements in a row correponding to bottom most elements coords.
    void SymbolContext::slotAlignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    //! \brief Align elements in a column correponding to left most elements coords.
    void SymbolContext::slotAlignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    /*!
     * \brief Align elements in a column correponding to right most elements
     * coords.
     */
    void SymbolContext::slotAlignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void SymbolContext::slotDistributeHorizontal()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SymbolDocument *symDoc = qobject_cast<SymbolDocument*>(doc);

        if (symDoc) {
            if (!symDoc->cGraphicsScene()->distributeElements(Qt::Horizontal)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void SymbolContext::slotDistributeVertical()
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SymbolDocument *symDoc = qobject_cast<SymbolDocument*>(doc);

        if (symDoc) {
            if (!symDoc->cGraphicsScene()->distributeElements(Qt::Vertical)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void SymbolContext::slotCenterHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void SymbolContext::slotCenterVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void SymbolContext::alignElements(Qt::Alignment alignment)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        SymbolDocument *symDoc = qobject_cast<SymbolDocument*>(doc);

        if (symDoc) {
            if (!symDoc->cGraphicsScene()->alignElements(alignment)) {
                QMessageBox::information(0, tr("Info"),
                        tr("At least two elements must be selected!"));
            }
        }
    }

    void SymbolContext::setNormalAction()
    {
        MainWindow::instance()->setNormalAction();
    }

} // namespace Caneda
