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

#include "icontext.h"

#include "idocument.h"
#include "library.h"
#include "settings.h"
#include "sidebarbrowser.h"
#include "sidebartextbrowser.h"
#include "statehandler.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringList>

namespace Caneda
{
    /*************************************************************************
     *                             IContext                                  *
     *************************************************************************/
    //! \brief Constructor.
    IContext::IContext(QObject *parent) : QObject(parent)
    {
    }


    /*************************************************************************
     *                          Layout Context                               *
     *************************************************************************/
    //! \brief Constructor.
    LayoutContext::LayoutContext(QObject *parent) : IContext(parent)
    {
        // We create the sidebar corresponding to this context
        StateHandler *handler = StateHandler::instance();
        m_sidebarBrowser = new SidebarBrowser();
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        Settings *settings = Settings::instance();

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

    //! \copydoc MainWindow::instance()
    LayoutContext* LayoutContext::instance()
    {
        static LayoutContext *context = 0;
        if (!context) {
            context = new LayoutContext();
        }
        return context;
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


    /*************************************************************************
     *                         Schematic Context                             *
     *************************************************************************/
    //! \brief Constructor.
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

        QList<QPair<QString, QPixmap> > miscellaneousItems;
        miscellaneousItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::bitmapDirectory() + "portsymbol.svg"));

        m_sidebarBrowser->plugItems(miscellaneousItems, QObject::tr("Miscellaneous"));

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

    //! \copydoc MainWindow::instance()
    SchematicContext* SchematicContext::instance()
    {
        static SchematicContext *context = 0;
        if (!context) {
            context = new SchematicContext();
        }
        return context;
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


    /*************************************************************************
     *                        Simulation Context                             *
     *************************************************************************/
    //! \brief Constructor.
    SimulationContext::SimulationContext(QObject *parent) : IContext(parent)
    {
    }

    //! \copydoc MainWindow::instance()
    SimulationContext* SimulationContext::instance()
    {
        static SimulationContext *context = 0;
        if (!context) {
            context = new SimulationContext();
        }
        return context;
    }

    bool SimulationContext::canOpen(const QFileInfo &info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "raw";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList SimulationContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Raw waveform data (*.raw)")+" (*.raw);;";

        return nameFilters;
    }

    IDocument* SimulationContext::newDocument()
    {
        return new SimulationDocument;
    }

    IDocument* SimulationContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SimulationDocument *document = new SimulationDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

    void SimulationContext::addNormalAction(Action *action)
    {
        m_normalActions << action;
    }

    void SimulationContext::addMouseAction(Action *action)
    {
        m_mouseActions << action;
    }

    void SimulationContext::exportCsv()
    {
        //! \todo Implement this
    }


    /*************************************************************************
     *                          Symbol Context                               *
     *************************************************************************/
    //! \brief Constructor.
    SymbolContext::SymbolContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarBrowser = new SidebarBrowser();
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        QList<QPair<QString, QPixmap> > miscellaneousItems;
        miscellaneousItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::bitmapDirectory() + "portsymbol.svg"));

        m_sidebarBrowser->plugItems(miscellaneousItems, QObject::tr("Miscellaneous"));

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

    //! \copydoc MainWindow::instance()
    SymbolContext* SymbolContext::instance()
    {
        static SymbolContext *context = 0;
        if (!context) {
            context = new SymbolContext();
        }
        return context;
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


    /*************************************************************************
     *                           Text Context                                *
     *************************************************************************/
    //! \brief Constructor.
    TextContext::TextContext(QObject *parent) : IContext(parent)
    {
        m_sidebarTextBrowser = new SidebarTextBrowser();
    }

    //! \copydoc MainWindow::instance()
    TextContext* TextContext::instance()
    {
        static TextContext *instance = 0;
        if (!instance) {
            instance = new TextContext();
        }
        return instance;
    }

    QWidget* TextContext::sideBarWidget()
    {
        return m_sidebarTextBrowser;
    }

    bool TextContext::canOpen(const QFileInfo& info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "";
        supportedSuffixes << "txt";
        supportedSuffixes << "log";
        supportedSuffixes << "net";
        supportedSuffixes << "cir";
        supportedSuffixes << "spc";
        supportedSuffixes << "sp";
        supportedSuffixes << "vhd";
        supportedSuffixes << "vhdl";
        supportedSuffixes << "v";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList TextContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Spice netlist (*.spc *.sp *.net *.cir)")+" (*.spc *.sp *.net *.cir);;";
        nameFilters << QObject::tr("HDL source (*.vhdl *.vhd *.v)")+" (*.vhdl *.vhd *.v);;";
        nameFilters << QObject::tr("Text file (*.txt)")+" (*.txt);;";

        return nameFilters;
    }

    IDocument* TextContext::newDocument()
    {
        return new TextDocument;
    }

    IDocument* TextContext::open(const QString& fileName, QString *errorMessage)
    {
        TextDocument *document = new TextDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }


    /*************************************************************************
     *                           Web Context                                 *
     *************************************************************************/
    //! \brief Constructor.
    WebContext::WebContext(QObject *parent) : IContext(parent)
    {
    }

    //! \copydoc MainWindow::instance()
    WebContext* WebContext::instance()
    {
        static WebContext *instance = 0;
        if (!instance) {
            instance = new WebContext();
        }
        return instance;
    }

    bool WebContext::canOpen(const QFileInfo& info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "htm";
        supportedSuffixes << "html";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList WebContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Web page (*.htm *.html)")+" (*.htm *.html);;";

        return nameFilters;
    }

    IDocument* WebContext::newDocument()
    {
        return new WebDocument;
    }

    IDocument* WebContext::open(const QString& fileName, QString *errorMessage)
    {
        WebDocument *document = new WebDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

} // namespace Caneda
