/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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
#include "sidebarchartsbrowser.h"
#include "sidebaritemsbrowser.h"
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

    /*!
     * \fn IContext::canOpen()
     *
     * \brief Indicates if a particular file extension is managed by this
     * context.
     *
     * This method indicates if a particular file extension is managed by this
     * context. This allows to find what context is in charge of a particular
     * file type.
     */

    /*!
     * \fn IContext::fileNameFilters()
     *
     * \brief Returns the filename extensions or filters available for this
     * context.
     *
     * Filename filters are used in open/save dialogs to allow the user to
     * filter the files displayed and ease the selection of the wanted file. In
     * this way, for example, if opening a schematic document the dialog should
     * display only schematic files. The method fileNameFilters() is used to
     * know what extensions correspond to that particular type of context.
     */

    /*!
     * \fn IContext::defaultSuffix()
     *
     * \brief Returns the default suffix of the current content type.
     */

    /*!
     * \fn IContext::newDocument()
     *
     * \brief Create a new document of the current context type.
     */

    /*!
     * \fn IContext::open()
     *
     * \brief Open a document of the current context type.
     */

    /*!
     * \fn IContext::toolBar()
     *
     * \brief Returns the toolbar corresponding to this context.
     *
     * There are two type of toolbars:
     * \li Main toolbars, containing common actions as copy, cut, paste, undo,
     * etc.
     * \li Context sensitive toolbars, containing only those actions relative
     * to the current context as insert wire, rotate, etc.
     *
     * While the main toolbars are displayed in the main window for every
     * context, context sensitive toolbars are displayed inside each tab when
     * a specific type of context is opened. This method returns a pointer to
     * the current context toolbar.
     *
     * \todo Implement context sensitive toolbars and attach them inside each
     * tab.
     *
     * \sa sideBarWidget()
     */

    /*!
     * \fn IContext::sideBarWidget()
     *
     * \brief Returns the sideBarWidget corresponding to this context.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. In this sense, context sensitive sidebars are
     * changed every time a specific type of context is opened or changed to.
     * This method returns a pointer to the current context sideBarWidget.
     *
     * \sa toolBar(), updateSideBar()
     */

    /*!
     * \fn IContext::insertItems()
     *
     * \brief Opens an insert dialog for items available in this context.
     *
     * Insert items are context sensitive, containing only those items relative
     * to the current context as components, painting tools, code snippets,
     * etc. This method allows for an external event to request an insert items
     * dialog.
     *
     * \sa sideBarWidget(), filterSideBarItems()
     */

    /*!
     * \fn IContext::filterSideBarItems()
     *
     * \brief Filters available items in the sidebar.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. This method allows for an external event to request
     * the selection of the sidebar focus and filtering, for example when
     * inserting items.
     *
     * \sa sideBarWidget()
     */

    /*!
     * \fn IContext::updateSideBar()
     *
     * \brief Updates sidebar contents.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. Upon certain conditions, sidebars may need updating,
     * for example when inserting or removing libraries. This method allows for
     * an external event to request the update of the sidebar.
     *
     * \sa sideBarWidget()
     */

    /*************************************************************************
     *                          Layout Context                               *
     *************************************************************************/
    //! \brief Constructor.
    LayoutContext::LayoutContext(QObject *parent) : IContext(parent)
    {
        // We create the sidebar corresponding to this context
        StateHandler *handler = StateHandler::instance();
        m_sidebarItems = new SidebarItemsModel(this);
        m_sidebarBrowser = new SidebarItemsBrowser(m_sidebarItems);
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
                QPixmap(Caneda::imageDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::imageDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::imageDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::imageDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::imageDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::imageDirectory() + "text.svg"));

        m_sidebarItems->plugItems(layerItems, QObject::tr("Layout Tools"));
        m_sidebarItems->plugItems(paintingItems, QObject::tr("Paint Tools"));
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

    QWidget* LayoutContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void LayoutContext::filterSideBarItems()
    {
        m_sidebarBrowser->focusFilter();
    }

    /*************************************************************************
     *                         Schematic Context                             *
     *************************************************************************/
    //! \brief Constructor.
    SchematicContext::SchematicContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarItems = new SidebarItemsModel(this);
        m_sidebarBrowser = new SidebarItemsBrowser(m_sidebarItems);
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        // Load schematic libraries
        LibraryManager *libraryManager = LibraryManager::instance();
        if(libraryManager->loadLibraryTree()) {

            // Get the libraries list and sort them alphabetically
            QStringList libraries(libraryManager->librariesList());
            libraries.sort();

            // Plug each library into the sidebar browser
            foreach(const QString library, libraries) {
                m_sidebarItems->plugLibrary(library, "Components");
                qDebug() << "Loaded " + library + " library";
            }

            qDebug() << "Successfully loaded libraries!";
        }
        else {
            // Invalidate entry
            qDebug() << "Error loading component libraries";
            qDebug() << "Please set the appropriate libraries through Application settings and restart the application.";
        }

        QList<QPair<QString, QPixmap> > miscellaneousItems;
        miscellaneousItems << qMakePair(QObject::tr("Ground"),
                QPixmap(Caneda::imageDirectory() + "ground.svg"));
        miscellaneousItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::imageDirectory() + "portsymbol.svg"));

        QList<QPair<QString, QPixmap> > paintingItems;
        paintingItems << qMakePair(QObject::tr("Arrow"),
                QPixmap(Caneda::imageDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::imageDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::imageDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::imageDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::imageDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::imageDirectory() + "text.svg"));

        m_sidebarItems->plugItems(miscellaneousItems, QObject::tr("Miscellaneous"));
        m_sidebarItems->plugItems(paintingItems, QObject::tr("Paint Tools"));
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

    QWidget* SchematicContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void SchematicContext::filterSideBarItems()
    {
        m_sidebarBrowser->focusFilter();
    }

    /*************************************************************************
     *                        Simulation Context                             *
     *************************************************************************/
    //! \brief Constructor.
    SimulationContext::SimulationContext(QObject *parent) : IContext(parent)
    {
        m_sidebarBrowser = new SidebarChartsBrowser();
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

    QWidget *SimulationContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void SimulationContext::filterSideBarItems()
    {
        m_sidebarBrowser->filterItems();
    }

    void SimulationContext::updateSideBar()
    {
        if(m_sidebarBrowser) {
            m_sidebarBrowser->updateChartSeriesMap();
        }
    }

    /*************************************************************************
     *                          Symbol Context                               *
     *************************************************************************/
    //! \brief Constructor.
    SymbolContext::SymbolContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarItems = new SidebarItemsModel(this);
        m_sidebarBrowser = new SidebarItemsBrowser(m_sidebarItems);
        connect(m_sidebarBrowser, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        QList<QPair<QString, QPixmap> > miscellaneousItems;
        miscellaneousItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::imageDirectory() + "portsymbol.svg"));

        QList<QPair<QString, QPixmap> > paintingItems;
        paintingItems << qMakePair(QObject::tr("Arrow"),
                QPixmap(Caneda::imageDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::imageDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::imageDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::imageDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::imageDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::imageDirectory() + "text.svg"));

        m_sidebarItems->plugItems(miscellaneousItems, QObject::tr("Miscellaneous"));
        m_sidebarItems->plugItems(paintingItems, QObject::tr("Paint Tools"));
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

    QWidget* SymbolContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void SymbolContext::filterSideBarItems()
    {
        m_sidebarBrowser->focusFilter();
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

    QWidget* TextContext::sideBarWidget()
    {
        return m_sidebarTextBrowser;
    }

} // namespace Caneda
