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
    LayoutContext::LayoutContext(QObject *parent) : IContext(parent)
    {
        // We create the sidebar corresponding to this context
        StateHandler *handler = StateHandler::instance();
        m_sidebarBrowser = new SidebarBrowser();
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        QSettings qSettings;
        Settings *settings = Settings::instance();
        settings->load(qSettings);

        QPixmap layer(20,20);

        QList<QPair<QString, QPixmap> > layerItems;
        layer.fill(settings->currentValue("gui/layout/metal1").value<QColor>());
        layerItems << qMakePair(QObject::tr("Metal 1"), layer);
        layer.fill(settings->currentValue("gui/layout/metal2").value<QColor>());
        layerItems << qMakePair(QObject::tr("Metal 2"), layer);
        layer.fill(settings->currentValue("gui/layout/poly1").value<QColor>());
        layerItems << qMakePair(QObject::tr("Poly 1"), layer);
        layer.fill(settings->currentValue("gui/layout/poly2").value<QColor>());
        layerItems << qMakePair(QObject::tr("Poly 2"), layer);
        layer.fill(settings->currentValue("gui/layout/active").value<QColor>());
        layerItems << qMakePair(QObject::tr("Active"), layer);
        layer.fill(settings->currentValue("gui/layout/contact").value<QColor>());
        layerItems << qMakePair(QObject::tr("Contact"), layer);
        layer.fill(settings->currentValue("gui/layout/nwell").value<QColor>());
        layerItems << qMakePair(QObject::tr("N Well"), layer);
        layer.fill(settings->currentValue("gui/layout/pwell").value<QColor>());
        layerItems << qMakePair(QObject::tr("P Well"), layer);

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

        m_sidebarBrowser->plugItems(layerItems, QObject::tr("Layout Tools"));
        m_sidebarBrowser->plugItems(paintingItems, QObject::tr("Paint Tools"));
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

    QWidget* LayoutContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    bool LayoutContext::canOpen(const QFileInfo &info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "xlay";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList LayoutContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Layout-xml (*.xlay)")+" (*.xlay);;";

        return nameFilters;
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

    void LayoutContext::slotIntoHierarchy()
    {
        setNormalAction();
        //TODO: implement this
    }

    void LayoutContext::slotPopHierarchy()
    {
        setNormalAction();
        //TODO: implement this
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
