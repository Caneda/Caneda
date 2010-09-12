/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "mainwindow.h"

#include "actionmanager.h"
#include "componentssidebar.h"
#include "documentviewmanager.h"
#include "folderbrowser.h"
#include "global.h"
#include "idocument.h"
#include "iview.h"
#include "library.h"
#include "project.h"
#include "schematiccontext.h"
#include "schematicstatehandler.h"
#include "settings.h"
#include "tabs.h"
#include "textcontext.h"

#include "dialogs/aboutdialog.h"
#include "dialogs/exportdialog.h"
#include "dialogs/librarymanager.h"
#include "dialogs/projectfileopendialog.h"
#include "dialogs/printdialog.h"
#include "dialogs/savedocumentsdialog.h"
#include "dialogs/settingsdialog.h"

#include "tools/attenuator/attenuator.h"
#include "tools/filter/filterdialog.h"
#include "tools/qtermwidget/qtermwidget.h"
#include "tools/transmission/transmissiondialog.h"

#include "xmlutilities/transformers.h"
#include "xmlutilities/validators.h"

namespace Caneda
{
    /*!
     * \brief Construct and setup the mainwindow for caneda.
     */
    MainWindow::MainWindow(QWidget *w) : QMainWindow(w)
    {
        titleText = QString("Caneda ") + Caneda::version() + QString(" : %1[*]");

        m_tabWidget = new TabWidget(this);
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        m_tabWidget->setTabsClosable(true);
        m_tabWidget->setMovable(true);
        connect(m_tabWidget, SIGNAL(statusBarMessage(QString)),
                this, SLOT(slotStatusBarMessage(QString)));
        setCentralWidget(m_tabWidget);

        setObjectName("MainWindow"); //for debugging purpose
        setDocumentTitle("Untitled");

        console = 0;
        m_undoGroup = new QUndoGroup();

        // Be vary of the order as all the pointers are uninitialized at this moment.
        initActions();
        initMenus();
        initToolBars();
        initStatusBar();

        setupSidebar();
        setupProjectsSidebar();
        createFolderView();
        createUndoView();

        loadSettings();

        QTimer::singleShot(100, this, SLOT(slotFileNew()));
    }

    //! Destructor
    MainWindow::~MainWindow()
    {
    }

    MainWindow* MainWindow::instance()
    {
        static MainWindow* instance = 0;
        if (!instance) {
            instance = new MainWindow();
        }
        return instance;
    }

    TabWidget* MainWindow::tabWidget() const
    {
        return m_tabWidget;
    }

    /*!
     * \brief This initializes the components sidebar.
     *
     * \todo This method only fill the sidebar with painting items. The components
     * are loaded in loadSettings() as of now. This should be corrected.
     */
    void MainWindow::setupSidebar()
    {
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        m_componentsSidebar = new ComponentsSidebar(this);
        connect(m_componentsSidebar, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));

        sidebarDockWidget = new QDockWidget(m_componentsSidebar->windowTitle(),this);
        sidebarDockWidget->setWidget(m_componentsSidebar);
        sidebarDockWidget->setObjectName("componentsSidebar");
        addDockWidget(Qt::LeftDockWidgetArea, sidebarDockWidget);
        docksMenu->addAction(sidebarDockWidget->toggleViewAction());

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

        m_componentsSidebar->plugItem("Components", QPixmap(), "root");
        m_componentsSidebar->plugItems(paintingItems, QObject::tr("Paint Tools"));
        m_componentsSidebar->plugItems(layerItems, QObject::tr("Layout Tools"));
    }

    /*!
     * \brief This initializes the projects sidebar.
     */
    void MainWindow::setupProjectsSidebar()
    {
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        m_project = new Project(this);
        connect(m_project, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));
        connect(m_project, SIGNAL(itemDoubleClicked(QString)), this,
                SLOT(slotFileOpen(QString)));

        projectDockWidget = new QDockWidget(m_project->windowTitle(), this);
        projectDockWidget->setWidget(m_project);
        projectDockWidget->setObjectName("projectsSidebar");
        projectDockWidget->setVisible(false);
        addDockWidget(Qt::LeftDockWidgetArea, projectDockWidget);
        docksMenu->addAction(projectDockWidget->toggleViewAction());
    }

    void MainWindow::createUndoView()
    {
        undoView = new QUndoView(m_undoGroup);
        undoView->setWindowTitle(tr("Command List"));

        sidebarDockWidget = new QDockWidget(undoView->windowTitle(), this);
        sidebarDockWidget->setWidget(undoView);
        sidebarDockWidget->setObjectName("undoSidebar");
        sidebarDockWidget->setVisible(false);
        addDockWidget(Qt::RightDockWidgetArea, sidebarDockWidget);
        docksMenu->addAction(sidebarDockWidget->toggleViewAction());
    }

    void MainWindow::createFolderView()
    {
        m_folderBrowser = new FolderBrowser(this);

        connect(m_folderBrowser, SIGNAL(itemDoubleClicked(QString)), this,
                SLOT(slotFileOpen(QString)));

        sidebarDockWidget = new QDockWidget(m_folderBrowser->windowTitle(), this);
        sidebarDockWidget->setWidget(m_folderBrowser);
        sidebarDockWidget->setObjectName("folderBrowserSidebar");
        addDockWidget(Qt::LeftDockWidgetArea, sidebarDockWidget);
        tabifyDockWidget(sidebarDockWidget, projectDockWidget);
        docksMenu->addAction(sidebarDockWidget->toggleViewAction());
    }

    Action* MainWindow::action(const QString& name)
    {
        ActionManager* am = ActionManager::instance();
        Action* act = am->actionForName(name);
        if (!act) {
            qWarning() << Q_FUNC_INFO << "Encountered null action for name = " << name;
        }
        return act;
    }

    void MainWindow::addAsDockWidget(QWidget *widget, const QString &title,
            Qt::DockWidgetArea area)
    {
        QDockWidget *dw = new QDockWidget(title);
        dw->setWidget(widget);
        addDockWidget(area, dw);
    }

    /*!
     * \brief Creates and intializes all the actions used.
     */
    void MainWindow::initActions()
    {
        using namespace Qt;
        Action *action = 0;
        ActionManager *am = ActionManager::instance();
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        SchematicContext *sc = SchematicContext::instance();

        action = am->createAction("fileNew", Caneda::icon("document-new"), tr("&New"));
        action->setShortcut(CTRL+Key_N);
        action->setStatusTip(tr("Creates a new document"));
        action->setWhatsThis(tr("New\n\nCreates a new schematic or data display document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileNew()));

        action = am->createAction("textNew", Caneda::icon("text-plain"), tr("New &Text"));
        action->setShortcut(CTRL+SHIFT+Key_V);
        action->setStatusTip(tr("Creates a new text document"));
        action->setWhatsThis(tr("New Text\n\nCreates a new text document"));
        connect(action, SIGNAL(triggered()), SLOT(slotTextNew()));

        action = am->createAction("fileOpen", Caneda::icon("document-open"), tr("&Open..."));
        action->setShortcut(CTRL+Key_O);
        action->setStatusTip(tr("Opens an existing document"));
        action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileOpen()));

        action = am->createAction("fileSave", Caneda::icon("document-save"), tr("&Save"));
        action->setShortcut(CTRL+Key_S);
        action->setStatusTip(tr("Saves the current document"));
        action->setWhatsThis(tr("Save File\n\nSaves the current document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSave()));

        action = am->createAction("fileSaveAs", Caneda::icon("document-save-as"), tr("Save as..."));
        action->setShortcut(CTRL+SHIFT+Key_S);
        action->setStatusTip(tr("Saves the current document under a new filename"));
        action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAs()));

        action = am->createAction("fileSaveAll", Caneda::icon("document-save-all"), tr("Save &All"));
        action->setShortcut(CTRL+Key_Plus);
        action->setStatusTip(tr("Saves all open documents"));
        action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAll()));

        action = am->createAction("fileClose", Caneda::icon("document-close"), tr("&Close"));
        action->setShortcut(CTRL+Key_W);
        action->setStatusTip(tr("Closes the current document"));
        action->setWhatsThis(tr("Close File\n\nCloses the current document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileClose()));

        action = am->createAction("filePrint", Caneda::icon("document-print"), tr("&Print..."));
        action->setShortcut(CTRL+Key_P);
        action->setStatusTip(tr("Prints the current document"));
        action->setWhatsThis(tr("Print File\n\nPrints the current document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFilePrint()));

        action = am->createAction("fileExportImage", Caneda::icon("image-x-generic"), tr("&Export Image..."));
        action->setShortcut(CTRL+Key_E);
        action->setWhatsThis(tr("Export Image\n\n""Export current view to image file"));
        connect(action, SIGNAL(triggered()), SLOT(slotExportImage()));

        action = am->createAction("fileSettings", Caneda::icon("document-properties"), tr("&Document Settings..."));
        action->setShortcut(CTRL+Key_Period);
        action->setWhatsThis(tr("Settings\n\nSets properties of the file"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSettings()));

        action = am->createAction("applSettings", Caneda::icon("preferences-other"), tr("Application Settings..."));
        action->setShortcut(CTRL+Key_Comma);
        action->setWhatsThis(tr("Caneda Settings\n\nSets properties of the application"));
        connect(action, SIGNAL(triggered()), SLOT(slotApplSettings()));

        action = am->createAction("fileQuit", Caneda::icon("application-exit"), tr("E&xit"));
        action->setShortcut(CTRL+Key_Q);
        action->setStatusTip(tr("Quits the application"));
        action->setWhatsThis(tr("Exit\n\nQuits the application"));
        connect(action, SIGNAL(triggered()), SLOT(close()));

        action = am->createAction("editUndo", Caneda::icon("edit-undo"), tr("&Undo"));
        action->setShortcut(CTRL+Key_Z);
        action->setStatusTip(tr("Undoes the last command"));
        action->setWhatsThis(tr("Undo\n\nMakes the last action undone"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditUndo()));

        action = am->createAction("editRedo", Caneda::icon("edit-redo"), tr("&Redo"));
        action->setShortcut(CTRL+SHIFT+Key_Z);
        action->setStatusTip(tr("Redoes the last command"));
        action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditRedo()));

        action = am->createAction("editCut", Caneda::icon("edit-cut"), tr("Cu&t"));
        action->setShortcut(CTRL+Key_X);
        action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
        action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditCut()));

        action = am->createAction("editCopy", Caneda::icon("edit-copy"), tr("&Copy"));
        action->setShortcut(CTRL+Key_C);
        action->setStatusTip(tr("Copies the selection into the clipboard"));
        action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditCopy()));

        action = am->createAction("editPaste", Caneda::icon("edit-paste"), tr("&Paste"));
        action->setShortcut(CTRL+Key_V);
        action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
        action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditPaste()));

        action = am->createAction("selectAll", Caneda::icon("select-rectangular"), tr("Select All"));
        action->setShortcut(CTRL+Key_A);
        action->setStatusTip(tr("Selects all elements"));
        action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
        connect(action, SIGNAL(triggered()), SLOT(slotSelectAll()));

        action = am->createAction("editFind", Caneda::icon("edit-find"), tr("Find..."));
        action->setShortcut(CTRL+Key_F);
        action->setStatusTip(tr("Find a piece of text"));
        action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditFind()));

        action = am->createAction("symEdit", Caneda::icon("draw-freehand"), tr("&Edit Circuit Symbol/Schematic"));
        action->setShortcut(Key_F7);
        action->setStatusTip(tr("Switches between symbol and schematic edit"));
        action->setWhatsThis(tr("Edit Circuit Symbol/Schematic\n\nSwitches between symbol and schematic edit"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotSymbolEdit()));
        sc->addNormalAction(action);

        action = am->createAction("intoH", Caneda::icon("go-bottom"), tr("Go into Subcircuit"));
        action->setShortcut(CTRL+Key_I);
        action->setWhatsThis(tr("Go into Subcircuit\n\nGoes inside the selected subcircuit"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotIntoHierarchy()));
        sc->addNormalAction(action);

        action = am->createAction("popH", Caneda::icon("go-top"), tr("Pop out"));
        action->setShortcut(CTRL+SHIFT+Key_I);
        action->setStatusTip(tr("Pop outside subcircuit"));
        action->setWhatsThis(tr("Pop out\n\nGoes up one hierarchy level, i.e. leaves subcircuit"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotPopHierarchy()));
        sc->addNormalAction(action);

        action = am->createAction("zoomFitInBest", Caneda::icon("zoom-fit-best"), tr("View all"));
        action->setShortcut(Key_0);
        action->setStatusTip(tr("Show the whole contents"));
        action->setWhatsThis(tr("View all\n\nShows the whole page content"));
        connect(action, SIGNAL(triggered()), SLOT(slotZoomBestFit()));

        action = am->createAction("zoomOriginal", Caneda::icon("zoom-original"), tr("View 1:1"));
        action->setShortcut(Key_1);
        action->setStatusTip(tr("View without magnification"));
        action->setWhatsThis(tr("Zoom 1:1\n\nShows the page content without magnification"));
        connect(action, SIGNAL(triggered()), SLOT(slotZoomOriginal()));

        action = am->createAction("zoomIn", Caneda::icon("zoom-in"), tr("Zoom in"));
        action->setShortcut(Key_Plus);
        action->setStatusTip(tr("Zooms in the content"));
        action->setWhatsThis(tr("Zoom In \n\nZooms in the content"));
        connect(action, SIGNAL(triggered()), SLOT(slotZoomIn()));

        action = am->createAction("zoomOut", Caneda::icon("zoom-out"), tr("Zoom out"));
        action->setShortcut(Key_Minus);
        action->setStatusTip(tr("Zooms out the content"));
        action->setWhatsThis(tr("Zoom Out \n\nZooms out the content"));
        connect(action, SIGNAL(triggered()), SLOT(slotZoomOut()));

        action = am->createAction("splitHorizontal", Caneda::icon("view-split-left-right"), tr("Split &Horizontal"));
        action->setShortcut(ALT+Key_1);
        action->setStatusTip(tr("Splits the current view in horizontal orientation"));
        action->setWhatsThis(tr("Split Horizontal\n\nSplits the current view in horizontal orientation"));
        connect(action, SIGNAL(triggered()), SLOT(slotSplitHorizontal()));

        action = am->createAction("splitVertical", Caneda::icon("view-split-top-bottom"), tr("Split &Vertical"));
        action->setShortcut(ALT+Key_2);
        action->setStatusTip(tr("Splits the current view in vertical orientation"));
        action->setWhatsThis(tr("Split Vertical\n\nSplits the current view in vertical orientation"));
        connect(action, SIGNAL(triggered()), SLOT(slotSplitVertical()));

        action = am->createAction("splitClose", Caneda::icon("view-left-close"), tr("&Close Split"));
        action->setShortcut(ALT+Key_3);
        action->setStatusTip(tr("Closes the current split"));
        action->setWhatsThis(tr("Close Split\n\nCloses the current split"));
        connect(action, SIGNAL(triggered()), SLOT(slotCloseSplit()));

        action = am->createAction("viewToolBar",  tr("Tool&bar"));
        action->setStatusTip(tr("Enables/disables the toolbar"));
        action->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), SLOT(slotViewToolBar(bool)));

        action = am->createAction("viewStatusBar",  tr("&Statusbar"));
        action->setStatusTip(tr("Enables/disables the statusbar"));
        action->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), SLOT(slotViewStatusBar(bool)));

        action = am->createAction("snapToGrid", Caneda::icon("view-grid"), tr("Snap to Grid"));
        action->setShortcut(CTRL+Key_U);
        action->setStatusTip(tr("Set grid snap"));
        action->setWhatsThis(tr("Snap to Grid\n\nSets snap to grid"));
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), sc, SLOT(slotSnapToGrid(bool)));
        sc->addNormalAction(action);

        action = am->createAction("alignTop", Caneda::icon("align-vertical-top"), tr("Align top"));
        action->setStatusTip(tr("Align top selected elements"));
        action->setWhatsThis(tr("Align top\n\nAlign selected elements to their upper edge"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotAlignTop()));
        sc->addNormalAction(action);

        action = am->createAction("alignBottom", Caneda::icon("align-vertical-bottom"), tr("Align bottom"));
        action->setStatusTip(tr("Align bottom selected elements"));
        action->setWhatsThis(tr("Align bottom\n\nAlign selected elements to their lower edge"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotAlignBottom()));
        sc->addNormalAction(action);

        action = am->createAction("alignLeft", Caneda::icon("align-horizontal-left"), tr("Align left"));
        action->setStatusTip(tr("Align left selected elements"));
        action->setWhatsThis(tr("Align left\n\nAlign selected elements to their left edge"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotAlignLeft()));
        sc->addNormalAction(action);

        action = am->createAction("alignRight", Caneda::icon("align-horizontal-right"), tr("Align right"));
        action->setStatusTip(tr("Align right selected elements"));
        action->setWhatsThis(tr("Align right\n\nAlign selected elements to their right edge"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotAlignRight()));
        sc->addNormalAction(action);

        action = am->createAction("centerHor", Caneda::icon("align-horizontal-center"), tr("Center horizontally"));
        action->setStatusTip(tr("Center horizontally selected elements"));
        action->setWhatsThis(tr("Center horizontally\n\nCenter horizontally selected elements"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotCenterHorizontal()));
        sc->addNormalAction(action);

        action = am->createAction("centerVert", Caneda::icon("align-vertical-center"), tr("Center vertically"));
        action->setStatusTip(tr("Center vertically selected elements"));
        action->setWhatsThis(tr("Center vertically\n\nCenter vertically selected elements"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotCenterVertical()));
        sc->addNormalAction(action);

        action = am->createAction("distrHor", Caneda::icon("distribute-horizontal-center"), tr("Distribute horizontally"));
        action->setStatusTip(tr("Distribute equally horizontally"));
        action->setWhatsThis(tr("Distribute horizontally\n\n""Distribute horizontally selected elements"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotDistributeHorizontal()));
        sc->addNormalAction(action);

        action = am->createAction("distrVert", Caneda::icon("distribute-vertical-center"), tr("Distribute vertically"));
        action->setStatusTip(tr("Distribute equally vertically"));
        action->setWhatsThis(tr("Distribute vertically\n\n""Distribute vertically selected elements"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotDistributeVertical()));
        sc->addNormalAction(action);

        action = am->createAction("projNew", Caneda::icon("project-new"), tr("&New Project..."));
        action->setShortcut(CTRL+SHIFT+Key_N);
        action->setStatusTip(tr("Creates a new project"));
        action->setWhatsThis(tr("New Project\n\nCreates a new project"));
        connect(action, SIGNAL(triggered()), SLOT(slotNewProject()));

        action = am->createAction("projOpen", Caneda::icon("document-open"), tr("&Open Project..."));
        action->setShortcut(CTRL+SHIFT+Key_O);
        action->setStatusTip(tr("Opens an existing project"));
        action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
        connect(action, SIGNAL(triggered()), SLOT(slotOpenProject()));

        action = am->createAction("addToProj", Caneda::icon("document-new"), tr("&Add File to Project..."));
        action->setShortcut(CTRL+SHIFT+Key_A);
        action->setStatusTip(tr("Adds a file to current project"));
        action->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));
        connect(action, SIGNAL(triggered()), SLOT(slotAddToProject()));

        action = am->createAction("projDel", Caneda::icon("document-close"), tr("&Remove from Project"));
        action->setShortcut(CTRL+SHIFT+Key_R);
        action->setStatusTip(tr("Removes a file from current project"));
        action->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));
        connect(action, SIGNAL(triggered()), SLOT(slotRemoveFromProject()));

        action = am->createAction("projClose", Caneda::icon("dialog-close"), tr("&Close Project"));
        action->setShortcut(CTRL+SHIFT+Key_W);
        action->setStatusTip(tr("Closes the current project"));
        action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
        connect(action, SIGNAL(triggered()), SLOT(slotCloseProject()));

        action = am->createAction("backupAndHistory", Caneda::icon("chronometer"), tr("&Backup and History..."));
        action->setShortcut(CTRL+SHIFT+Key_B);
        action->setStatusTip(tr("Opens backup and history dialog"));
        action->setWhatsThis(tr("Backup and History\n\nOpens backup and history dialog"));
        connect(action, SIGNAL(triggered()), SLOT(slotBackupAndHistory()));

        action = am->createAction("insEquation", Caneda::icon("formula"), tr("Insert Equation"));
        action->setCheckable(true);
        action->setShortcut(Key_E);
        action->setWhatsThis(tr("Insert Equation\n\nInserts a user defined equation"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotInsertToolbarComponent(const QString&, bool)));

        action = am->createAction("insGround", Caneda::icon("ground"), tr("Insert Ground"));
        action->setCheckable(true);
        action->setShortcut(Key_G);
        action->setWhatsThis(tr("Insert Ground\n\nInserts a ground symbol"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotInsertToolbarComponent(const QString&, bool)));

        action = am->createAction("insPort", Caneda::icon("port"), tr("Insert Port"));
        action->setCheckable(true);
        action->setShortcut(Key_P);
        action->setWhatsThis(tr("Insert Port\n\nInserts a port symbol"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotInsertToolbarComponent(const QString&, bool)));

        action = am->createAction("insEntity", Caneda::icon("code-context"), tr("VHDL entity"));
        action->setShortcut(SHIFT+Key_V);
        action->setStatusTip(tr("Inserts skeleton of VHDL entity"));
        action->setWhatsThis(tr("VHDL entity\n\nInserts the skeleton of a VHDL entity"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotInsertEntity()));
        sc->addNormalAction(action);

        action = am->createAction("callFilter", Caneda::icon("tools-wizard"), tr("Filter synthesis"));
        action->setShortcut(CTRL+Key_1);
        action->setStatusTip(tr("Starts CanedaFilter"));
        action->setWhatsThis(tr("Filter synthesis\n\nStarts CanedaFilter"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallFilter()));

        action = am->createAction("callLine", Caneda::icon("tools-wizard"), tr("Transmission line"));
        action->setShortcut(CTRL+Key_2);
        action->setStatusTip(tr("Starts transmission line calculator"));
        action->setWhatsThis(tr("Transmission line\n\nStarts transmission line calculator"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallLine()));

        action = am->createAction("callMatch", Caneda::icon("tools-wizard"), tr("Matching circuit"));
        action->setShortcut(CTRL+Key_3);
        action->setStatusTip(tr("Creates Matching Circuit"));
        action->setWhatsThis(tr("Matching Circuit\n\nDialog for Creating Matching Circuit"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallMatch()));

        action = am->createAction("callAtt", Caneda::icon("tools-wizard"), tr("Attenuator synthesis"));
        action->setShortcut(CTRL+Key_4);
        action->setStatusTip(tr("Starts CanedaAttenuator"));
        action->setWhatsThis(tr("Attenuator synthesis\n\nStarts attenuator calculation program"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallAtt()));

        action = am->createAction("callLib", Caneda::icon("library"), tr("Library manager"));
        action->setShortcut(CTRL+Key_5);
        action->setStatusTip(tr("Opens library manager"));
        action->setWhatsThis(tr("Library Manager\n\nOpens library manager dialog"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallLibrary()));

        action = am->createAction("importData",  tr("&Import Data..."));
        action->setShortcut(CTRL+Key_6);
        action->setStatusTip(tr("Convert file to Caneda data file"));
        action->setWhatsThis(tr("Import Data\n\nConvert data file to Caneda data file"));
        connect(action, SIGNAL(triggered()), SLOT(slotImportData()));

        action = am->createAction("showConsole", Caneda::icon("terminal"), tr("&Show Console..."));
        action->setShortcut(Key_F8);
        action->setStatusTip(tr("Show Console"));
        action->setWhatsThis(tr("Show Console\n\nOpen console terminal"));
        connect(action, SIGNAL(triggered()), SLOT(slotShowConsole()));

        action = am->createAction("simulate", Caneda::icon("media-playback-start"), tr("Simulate"));
        action->setShortcut(Key_F5);
        action->setStatusTip(tr("Simulates the current schematic"));
        action->setWhatsThis(tr("Simulate\n\nSimulates the current schematic"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotSimulate()));
        sc->addNormalAction(action);

        action = am->createAction("dpl_sch", Caneda::icon("system-switch-user"), tr("View Data Display/Schematic"));
        action->setShortcut(Key_F4);
        action->setStatusTip(tr("Changes to data display or schematic page"));
        action->setWhatsThis(tr("View Data Display/Schematic\n\n")+tr("Changes to data display or schematic page"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotToPage()));
        sc->addNormalAction(action);

        action = am->createAction("dcbias",  tr("Calculate DC bias"));
        action->setShortcut(Key_F3);
        action->setStatusTip(tr("Calculates DC bias and shows it"));
        action->setWhatsThis(tr("Calculate DC bias\n\nCalculates DC bias and shows it"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotDCbias()));
        sc->addNormalAction(action);

        action = am->createAction("graph2csv",  tr("Export to &CSV..."));
        action->setShortcut(Key_F6);
        action->setStatusTip(tr("Convert graph data to CSV file"));
        action->setWhatsThis(tr("Export to CSV\n\nConvert graph data to CSV file"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotExportGraphAsCsv()));
        sc->addNormalAction(action);

        action = am->createAction("showMsg", Caneda::icon("document-preview"), tr("Show Last Messages"));
        action->setShortcut(Key_F9);
        action->setStatusTip(tr("Shows last simulation messages"));
        action->setWhatsThis(tr("Show Last Messages\n\nShows the messages of the last simulation"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotShowLastMsg()));
        sc->addNormalAction(action);

        action = am->createAction("showNet", Caneda::icon("document-preview"), tr("Show Last Netlist"));
        action->setShortcut(Key_F10);
        action->setStatusTip(tr("Shows last simulation netlist"));
        action->setWhatsThis(tr("Show Last Netlist\n\nShows the netlist of the last simulation"));
        connect(action, SIGNAL(triggered()), sc, SLOT(slotShowLastNetlist()));
        sc->addNormalAction(action);

        action = am->createAction("helpIndex", Caneda::icon("help-contents"), tr("Help Index..."));
        action->setShortcut(Key_F1);
        action->setStatusTip(tr("Index of Caneda Help"));
        action->setWhatsThis(tr("Help Index\n\nIndex of intern Caneda help"));
        connect(action, SIGNAL(triggered()), SLOT(slotHelpIndex()));

        action = am->createAction("helpAboutApp", Caneda::icon("caneda"), tr("&About Caneda..."));
        action->setWhatsThis(tr("About\n\nAbout the application"));
        connect(action, SIGNAL(triggered()), SLOT(slotHelpAbout()));

        action = am->createAction("helpAboutQt", Caneda::icon("qt"), tr("About Qt..."));
        action->setWhatsThis(tr("About Qt\n\nAbout Qt by Trolltech"));
        connect(action, SIGNAL(triggered()), SLOT(slotHelpAboutQt()));

        QAction *qAction = QWhatsThis::createAction(this);
        action = am->createAction("whatsThis", qAction->icon(), qAction->text());
        connect(action, SIGNAL(triggered()), qAction, SLOT(trigger()));
        connect(action, SIGNAL(hovered()), qAction, SLOT(hover()));

        initMouseActions();
    }

    void MainWindow::initMouseActions()
    {
        using namespace Qt;
        Action *action = 0;
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        SchematicContext *sc = SchematicContext::instance();

        ActionManager *am = ActionManager::instance();
        action = am->createMouseAction("editDelete", SchematicScene::Deleting,
                Caneda::icon("edit-delete"), tr("&Delete"));
        action->setShortcut(Key_Delete);
        action->setStatusTip(tr("Deletes the selected components"));
        action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addMouseAction(action);

        action = am->createMouseAction("select", SchematicScene::Normal,
                Caneda::icon("edit-select"), tr("Select"));
        action->setShortcut(Key_Escape);
        action->setStatusTip(tr("Activate select mode"));
        action->setWhatsThis(tr("Select\n\nActivates select mode"));
        action->setChecked(true);
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("editRotate", SchematicScene::Rotating,
                Caneda::icon("object-rotate-left"), tr("Rotate"));
        action->setShortcut(CTRL+Key_R);
        action->setStatusTip(tr("Rotates the selected component by 90°"));
        action->setWhatsThis(tr("Rotate\n\nRotates the selected component by 90° counter-clockwise"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("editMirror", SchematicScene::MirroringX,
                Caneda::icon("object-flip-vertical"), tr("Mirror about X Axis"));
        action->setShortcut(Key_V);
        action->setWhatsThis(tr("Mirror about X Axis\n\nMirrors the selected item about X Axis"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("editMirrorY", SchematicScene::MirroringY,
                Caneda::icon("object-flip-horizontal"), tr("Mirror about Y Axis"));
        action->setShortcut(Key_H);
        action->setWhatsThis(tr("Mirror about Y Axis\n\nMirrors the selected item about Y Axis"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("insWire", SchematicScene::Wiring,
                Caneda::icon("wire"), tr("Wire"));
        action->setShortcut(Key_W);
        action->setWhatsThis(tr("Wire\n\nInserts a wire"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("insLabel", SchematicScene::InsertingWireLabel,
                Caneda::icon("nodename"), tr("Wire Label"));
        action->setShortcut(Key_L);
        action->setStatusTip(tr("Inserts a wire or pin label"));
        action->setWhatsThis(tr("Wire Label\n\nInserts a wire or pin label"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("editActivate", SchematicScene::ChangingActiveStatus,
                Caneda::icon("deactiv"), tr("Deactivate/Activate"));
        action->setShortcut(Key_D);
        action->setStatusTip(tr("Deactivate/Activate selected components"));
        action->setWhatsThis(tr("Deactivate/Activate\n\nDeactivate/Activate the selected components"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("setMarker", SchematicScene::Marking,
                Caneda::icon("marker"), tr("Set Marker on Graph"));
        action->setShortcut(Key_F2);
        action->setStatusTip(tr("Sets a marker on a diagram's graph"));
        action->setWhatsThis(tr("Set Marker\n\nSets a marker on a diagram's graph"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("zoomArea", SchematicScene::ZoomingAreaEvent,
                Caneda::icon("transform-scale"), tr("Zoom area"));
        action->setStatusTip(tr("Zooms a selected are in the current view"));
        action->setWhatsThis(tr("Zooms a selected are in the current view"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("insertItem", SchematicScene::InsertingItems,
                tr("Insert item action"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);

        action = am->createMouseAction("paintingDraw", SchematicScene::PaintingDrawEvent,
                tr("Painting draw action"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
        sc->addNormalAction(action);
    }

    //! \brief Create and initialize menus.
    void MainWindow::initMenus()
    {
        fileMenu = menuBar()->addMenu(tr("&File"));

        fileMenu->addAction(action("fileNew"));
        fileMenu->addAction(action("textNew"));
        fileMenu->addAction(action("fileOpen"));
        fileMenu->addAction(action("fileClose"));

        fileMenu->addSeparator();

        fileMenu->addAction(action("fileSave"));
        fileMenu->addAction(action("fileSaveAll"));
        fileMenu->addAction(action("fileSaveAs"));
        fileMenu->addAction(action("filePrint"));
        fileMenu->addAction(action("fileExportImage"));

        fileMenu->addSeparator();

        fileMenu->addAction(action("fileSettings"));
        fileMenu->addAction(action("applSettings"));

        fileMenu->addSeparator();

        fileMenu->addAction(action("fileQuit"));

        editMenu = menuBar()->addMenu(tr("&Edit"));

        editMenu->addAction(action("editUndo"));
        editMenu->addAction(action("editRedo"));

        editMenu->addSeparator();

        editMenu->addAction(action("editCut"));
        editMenu->addAction(action("editCopy"));
        editMenu->addAction(action("editPaste"));

        editMenu->addSeparator();

        editMenu->addAction(action("select"));
        editMenu->addAction(action("selectAll"));
        editMenu->addAction(action("editFind"));
        editMenu->addAction(action("editRotate"));
        editMenu->addAction(action("editMirror"));
        editMenu->addAction(action("editMirrorY"));

        editMenu->addSeparator();

        editMenu->addAction(action("symEdit"));
        editMenu->addAction(action("intoH"));
        editMenu->addAction(action("popH"));

        viewMenu = menuBar()->addMenu(tr("&View"));

        viewMenu->addAction(action("zoomFitInBest"));
        viewMenu->addAction(action("zoomOriginal"));
        viewMenu->addAction(action("zoomIn"));
        viewMenu->addAction(action("zoomOut"));
        viewMenu->addAction(action("zoomArea"));

        viewMenu->addSeparator();

        viewMenu->addAction(action("splitHorizontal"));
        viewMenu->addAction(action("splitVertical"));
        viewMenu->addAction(action("splitClose"));

        viewMenu->addSeparator();

        docksMenu = viewMenu->addMenu(tr("&Docks and Toolbars"));

        docksMenu->addAction(action("viewToolBar"));
        docksMenu->addAction(action("viewStatusBar"));

        docksMenu->addSeparator();

        viewMenu->addSeparator();

        alignMenu = menuBar()->addMenu(tr("P&ositioning"));

        alignMenu->addAction(action("snapToGrid"));

        alignMenu->addSeparator();

        alignMenu->addAction(action("centerHor"));
        alignMenu->addAction(action("centerVert"));

        alignMenu->addSeparator();

        alignMenu->addAction(action("alignTop"));
        alignMenu->addAction(action("alignBottom"));
        alignMenu->addAction(action("alignLeft"));
        alignMenu->addAction(action("alignRight"));

        alignMenu->addSeparator();

        alignMenu->addAction(action("distrHor"));
        alignMenu->addAction(action("distrVert"));

        projMenu = menuBar()->addMenu(tr("&Project"));

        projMenu->addAction(action("projNew"));
        projMenu->addAction(action("projOpen"));
        projMenu->addAction(action("addToProj"));
        projMenu->addAction(action("projDel"));
        projMenu->addAction(action("projClose"));

        projMenu->addSeparator();

        projMenu->addAction(action("backupAndHistory"));

        toolMenu = menuBar()->addMenu(tr("&Tools"));

        toolMenu->addAction(action("insWire"));
        toolMenu->addAction(action("insLabel"));
        toolMenu->addAction(action("insEquation"));
        toolMenu->addAction(action("insGround"));
        toolMenu->addAction(action("insPort"));
        toolMenu->addAction(action("insEntity"));
        toolMenu->addAction(action("editActivate"));
        toolMenu->addAction(action("editDelete"));

        toolMenu->addSeparator();

        toolMenu->addAction(action("callFilter"));
        toolMenu->addAction(action("callLine"));
        toolMenu->addAction(action("callMatch"));
        toolMenu->addAction(action("callAtt"));

        toolMenu->addSeparator();

        toolMenu->addAction(action("callLib"));
        toolMenu->addAction(action("importData"));

        toolMenu->addSeparator();

        toolMenu->addAction(action("showConsole"));

        simMenu = menuBar()->addMenu(tr("&Simulation"));

        simMenu->addAction(action("simulate"));
        simMenu->addAction(action("dpl_sch"));

        simMenu->addSeparator();

        simMenu->addAction(action("dcbias"));
        simMenu->addAction(action("setMarker"));
        simMenu->addAction(action("graph2csv"));

        simMenu->addSeparator();

        simMenu->addAction(action("showMsg"));
        simMenu->addAction(action("showNet"));

        helpMenu = menuBar()->addMenu(tr("&Help"));

        helpMenu->addAction(action("helpIndex"));
        helpMenu->addAction(action("whatsThis"));

        helpMenu->addSeparator();

        helpMenu->addAction(action("helpAboutApp"));
        helpMenu->addAction(action("helpAboutQt"));

    }

    //! \brief Create and intialize the toolbars
    void MainWindow::initToolBars()
    {
        fileToolbar  = addToolBar(tr("File"));
        fileToolbar->setObjectName("fileToolBar");

        fileToolbar->addAction(action("fileNew"));
        fileToolbar->addAction(action("textNew"));
        fileToolbar->addAction(action("fileOpen"));
        fileToolbar->addAction(action("fileSave"));
        fileToolbar->addAction(action("fileSaveAll"));
        fileToolbar->addAction(action("filePrint"));

        editToolbar  = addToolBar(tr("Edit"));
        editToolbar->setObjectName("editToolbar");

        editToolbar->addAction(action("editCut"));
        editToolbar->addAction(action("editCopy"));
        editToolbar->addAction(action("editPaste"));
        editToolbar->addAction(action("editUndo"));
        editToolbar->addAction(action("editRedo"));

        viewToolbar  = addToolBar(tr("View"));
        viewToolbar->setObjectName("viewToolbar");

        viewToolbar->addAction(action("zoomFitInBest"));
        viewToolbar->addAction(action("zoomOriginal"));
        viewToolbar->addAction(action("zoomIn"));
        viewToolbar->addAction(action("zoomOut"));
        viewToolbar->addAction(action("zoomArea"));

        workToolbar  = addToolBar(tr("Work"));
        workToolbar->setObjectName("workToolbar");

        workToolbar->addAction(action("select"));
        workToolbar->addAction(action("editDelete"));
        workToolbar->addAction(action("editMirror"));
        workToolbar->addAction(action("editMirrorY"));
        workToolbar->addAction(action("editRotate"));

        workToolbar->addSeparator();

        workToolbar->addAction(action("insWire"));
        workToolbar->addAction(action("insLabel"));
        workToolbar->addAction(action("insEquation"));
        workToolbar->addAction(action("insGround"));
        workToolbar->addAction(action("insPort"));
        workToolbar->addAction(action("editActivate"));
        workToolbar->addAction(action("intoH"));
        workToolbar->addAction(action("popH"));

        workToolbar->addSeparator();

        workToolbar->addAction(action("simulate"));
        workToolbar->addAction(action("dpl_sch"));
        workToolbar->addAction(action("setMarker"));

        workToolbar->addSeparator();

        workToolbar->addAction(action("whatsThis"));
    }

    void MainWindow::initStatusBar()
    {
        QStatusBar *statusBarWidget = statusBar();
        // Initially its empty space.
        m_statusLabel = new QLabel(QString(""), statusBarWidget);
        statusBarWidget->addPermanentWidget(m_statusLabel);
        statusBarWidget->setVisible(action("viewStatusBar")->isChecked());
    }

    //! \brief Toggles the normal select action on.
    void MainWindow::setNormalAction()
    {
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        handler->slotSetNormalAction();
    }

    /*!
     * \brief Sync the settings to configuration file and close window.
     */
    void MainWindow::closeEvent(QCloseEvent *e)
    {
        if(slotFileSaveAll()) {
            e->accept();
            saveSettings();
        }
        else {
            e->ignore();
            return;
        }
    }

    /*!
     * \brief Creates a new schematic view and adds it the tabwidget.
     */
    void MainWindow::slotFileNew()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        if(m_project->isValid()) {
            slotAddToProject();
        }
        else {
            manager->newDocument(SchematicContext::instance());
        }
    }

    //! \brief Creates a new text view.
    void MainWindow::slotTextNew()
    {
        setNormalAction();
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->newDocument(TextContext::instance());
    }

    /*!
     * \brief Tries to open a file by prompting the user for fileName.
     *
     * If the file is already opened, that tab is set as current. Otherwise the file
     * opened is set as current tab.
     */
    void MainWindow::slotFileOpen(QString fileName)
    {
        setNormalAction();
        DocumentViewManager *manager = DocumentViewManager::instance();

        if (fileName.isEmpty()) {
            if(!m_project->isValid()) {
                fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                        Settings::instance()->currentValue("nosave/canedaFilter").toString());
            }
            else {
                ProjectFileOpenDialog *p =
                        new ProjectFileOpenDialog(m_project->libraryFileName(), this);
                int status = p->exec();

                if(status == QDialog::Accepted) {
                    fileName = p->fileName();
                }

                delete p;
            }
        }

        if(!fileName.isEmpty()) {
            if(QFileInfo(fileName).suffix() == "xpro") {
                slotOpenProject(fileName);
            }
            else {
                bool isLoaded = manager->openFile(fileName);
                if(!isLoaded) {
                    QMessageBox::critical(0, tr("File load error"),
                            tr("Cannot open file %1").arg(fileName));
                }
            }
        }
    }

    /*!
     * \brief Saves the current active document.
     */
    void MainWindow::slotFileSave()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        if (document->fileName().isEmpty()) {
            slotFileSaveAs();
            return;
        }

        QString errorMessage;
        if (!document->save(&errorMessage)) {
            QMessageBox::critical(this,
                    tr("%1 : File save error").arg(document->fileName()), errorMessage);
        }
    }

    /*!
     * \brief Pops up dialog to select new filename and saves the file corresponding
     * to index tab.
     */
    void MainWindow::slotFileSaveAs()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                Settings::instance()->currentValue("nosave/canedaFilter").toString());
        if(fileName.isEmpty()) {
            return;
        }

        QString oldFileName = document->fileName();
        document->setFileName(fileName);

        QString errorMessage;
        if (!document->save(&errorMessage)) {
            QMessageBox::critical(this,
                    tr("%1 : File save error").arg(document->fileName()), errorMessage);
            document->setFileName(oldFileName);
        }

        //FIXME: Probably update/close other open document having same name as the above saved one.
    }

    /*!
     * \brief Opens a dialog giving the user options to save all modified files.
     *
     * @return True on success, false on cancel
     */
    bool MainWindow::slotFileSaveAll()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        QList<IDocument*> openDocuments = manager->documents();

        return manager->saveDocuments(openDocuments);
    }

    /*!
     * \brief Closes the selected tab.
     *
     * Before closing it prompts user whether to save or not if the document is
     * modified and takes necessary actions.
     */
    void MainWindow::slotFileClose()
    {
        Tab *current = tabWidget()->currentTab();
        if (current) {
            current->close();
        }
    }

    void MainWindow::slotFilePrint()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        QPointer<PrintDialog> p = new PrintDialog(document, this);
        p->exec();
        delete p;
    }

    void MainWindow::slotExportImage()
    {
        //PORT:
#if 0
        QList<SchematicScene *> schemasToExport;

        int i = 0;
        CanedaView *view = 0;
        while(i < tabWidget()->count()) {
            view = viewFromWidget(tabWidget()->widget(i));
            SchematicScene *scene = view->toSchematicWidget()->schematicScene();
            schemasToExport << scene;

            view = 0;
            ++i;
        }

        ExportDialog *expDial = new ExportDialog(schemasToExport, this);
        expDial->exec();
#endif
    }

    void MainWindow::slotApplSettings()
    {
        QList<SettingsPage *> wantedPages;
        SettingsPage *page = new GeneralConfigurationPage(this);
        wantedPages << page;
        page = new HdlConfigurationPage(this);
        wantedPages << page;
        page = new LayoutConfigurationPage(this);
        wantedPages << page;
        page = new SimulationConfigurationPage(this);
        wantedPages << page;

        SettingsDialog *d = new SettingsDialog(wantedPages, "Configure Caneda", this);
        d->exec();
    }

    void MainWindow::slotFileSettings()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        document->documentSettings();
    }

    void MainWindow::slotEditUndo()
    {
        setNormalAction();
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->undo();
        }

    }

    void MainWindow::slotEditRedo()
    {
        setNormalAction();
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->redo();
        }
    }

    void MainWindow::slotEditCut()
    {
        setNormalAction();
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->cut();
        }
    }

    void MainWindow::slotEditCopy()
    {
        setNormalAction();
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->copy();
        }
    }

    void MainWindow::slotEditPaste()
    {
        setNormalAction();
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->paste();
        }
    }

    void MainWindow::slotSelectAll()
    {
        setNormalAction();
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->selectAll();
        }
    }

    void MainWindow::slotEditFind()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotZoomBestFit()
    {
        setNormalAction();
        IView* view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->zoomFitInBest();
        }
    }

    void MainWindow::slotZoomOriginal()
    {
        setNormalAction();
        IView* view = DocumentViewManager::instance()->currentView();
        if(view) {
            view->zoomOriginal();
        }
    }

    void MainWindow::slotZoomIn()
    {
        setNormalAction();
        IView* view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->zoomIn();
        }
    }

    void MainWindow::slotZoomOut()
    {
        setNormalAction();
        IView* view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->zoomOut();
        }
    }

    void MainWindow::slotSplitHorizontal()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView* view = manager->currentView();
        if(view) {
            manager->splitView(view, Qt::Horizontal);
        }
    }

    void MainWindow::slotSplitVertical()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView* view = manager->currentView();
        if(view) {
            manager->splitView(view, Qt::Vertical);
        }
    }

    void MainWindow::slotCloseSplit()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView* view = manager->currentView();
        if(view) {
            manager->closeView(view);
        }
    }

    void MainWindow::slotViewToolBar(bool toogle)
    {
        setNormalAction();
        fileToolbar->setVisible(toogle);
        editToolbar->setVisible(toogle);
        viewToolbar->setVisible(toogle);
        workToolbar->setVisible(toogle);
    }

    void MainWindow::slotViewStatusBar(bool toogle)
    {
        setNormalAction();
        statusBar()->setVisible(toogle);
    }

    void MainWindow::slotNewProject()
    {
        setNormalAction();

        if(slotFileSaveAll()) {
            m_tabWidget->closeAllTabs();
            projectDockWidget->setVisible(true);
            projectDockWidget->raise();
            m_project->slotNewProject();
        }
    }

    void MainWindow::slotOpenProject(QString fileName)
    {
        setNormalAction();

        if(slotFileSaveAll()) {
            m_tabWidget->closeAllTabs();
            projectDockWidget->setVisible(true);
            projectDockWidget->raise();
            m_project->slotOpenProject(fileName);
        }
    }

    void MainWindow::slotAddToProject()
    {
        setNormalAction();
        projectDockWidget->setVisible(true);
        projectDockWidget->raise();
        m_project->slotAddToProject();
    }

    void MainWindow::slotRemoveFromProject()
    {
        setNormalAction();
        projectDockWidget->setVisible(true);
        projectDockWidget->raise();
        m_project->slotRemoveFromProject();
    }

    void MainWindow::slotCloseProject()
    {
        setNormalAction();

        if(slotFileSaveAll()) {
            m_project->slotCloseProject();
            m_tabWidget->closeAllTabs();
        }
    }

    void MainWindow::slotBackupAndHistory()
    {
        setNormalAction();
        m_project->slotBackupAndHistory();
    }

    void MainWindow::slotCallFilter()
    {
        setNormalAction();

        QPointer<FilterDialog> p = new FilterDialog(this);
        p->exec();
        delete p;
    }

    void MainWindow::slotCallLine()
    {
        setNormalAction();

        QPointer<TransmissionDialog> p = new TransmissionDialog(this);
        p->exec();
        delete p;
    }

    void MainWindow::slotCallMatch()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotCallAtt()
    {
        setNormalAction();

        QPointer<Attenuator> p = new Attenuator(this);
        p->exec();
        delete p;
    }

    void MainWindow::slotCallLibrary()
    {
        setNormalAction();

        QPointer<LibraryManager> p = new LibraryManager(this);
        p->exec();
        delete p;
    }

    void MainWindow::slotImportData()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotShowConsole()
    {
        if(!console) {
            console = new QTermWidget();
            console->setScrollBarPosition(QTermWidget::ScrollBarRight);
            console->setMinimumHeight(40);

            consoleDockWidget = new QDockWidget("Console",this);
            consoleDockWidget->setWidget(console);
            consoleDockWidget->setObjectName("consoleWidget");
            addDockWidget(Qt::BottomDockWidgetArea, consoleDockWidget);

            connect(console, SIGNAL(finished()), consoleDockWidget, SLOT(close()));
        }
        else if(consoleDockWidget->isHidden()) {
            consoleDockWidget->setVisible(true);
        }
        else if(consoleDockWidget->isVisible()) {
            consoleDockWidget->setVisible(false);
        }
    }

    void MainWindow::slotHelpIndex()
    {
        setNormalAction();
        slotFileOpen(QString(docDirectory() + "/en/index.html"));
    }

    void MainWindow::slotHelpAbout()
    {
        setNormalAction();

        QPointer<AboutDialog> p = new AboutDialog(this);
        p->exec();
        delete p;
    }

    void MainWindow::slotHelpAboutQt()
    {
        setNormalAction();
        QApplication::aboutQt();
    }

    void MainWindow::loadSettings()
    {
        QSettings qSettings;

        Settings *settings = Settings::instance();
        settings->load(qSettings);

        // First load geometry and dock pos as later setting values might
        // depend on these values
        const QByteArray geometryData = settings->currentValue("gui/geometry").toByteArray();
        if (geometryData.isEmpty() == false) {
            restoreGeometry(geometryData);
        }

        const QByteArray dockData = settings->currentValue("gui/dockPositions").toByteArray();
        if (dockData.isEmpty() == false) {
            restoreState(dockData);
        }

        const QSize iconSize = settings->currentValue("gui/iconSize").toSize();
        setIconSize(iconSize);

        /* Load library database settings */
        QString libpath = settings->currentValue("sidebarLibrary").toString();
        if(QFileInfo(libpath).exists() == false) {
            QMessageBox::warning(0, tr("Cannot load Components library in the sidebar"),
                    tr("Please set the appropriate path to components library through Application settings and restart the application to load components in the sidebar"));
            return;
        }

        /* Load validators */
        Caneda::validators * validator = Caneda::validators::defaultInstance();
        if(validator->load(libpath)) {
            qDebug() << "Succesfully loaded validators!";
        }
        else {
            //invalidate entry.
            qWarning() << "MainWindow::loadSettings() : Could not load validators. "
                                                        << "Expect crashing in case of incorrect xml file";
        }

        /* Load transformers */
        Caneda::transformers * transformer = Caneda::transformers::defaultInstance();
        if(transformer->load(libpath)) {
            qDebug() << "Succesfully loaded transformers!";
        }
        else {
            //invalidate entry.
            qWarning() << "MainWindow::loadSettings() : Could not load XSLT transformers. "
                                                        << "Expect strange schematic symbols";
        }

        LibraryLoader *library = LibraryLoader::instance();

        if(library->loadtree(libpath)) {
            qDebug() << "Succesfully loaded library!";
        }
        else {
            //invalidate entry.
            qWarning() << "MainWindow::loadSettings() : Entry is invalid. Run once more to set"
                                                        << "the appropriate path.";
            settings->setCurrentValue("sidebarLibrary",
                    settings->defaultValue("sidebarLibrary").toString());
            return;
        }

        m_componentsSidebar->plugLibrary("Passive", "Components");
        m_componentsSidebar->plugLibrary("Active", "Components");
        m_componentsSidebar->plugLibrary("Semiconductor", "Components");
    }

    void MainWindow::saveSettings()
    {
        QSettings qSettings;

        Settings *settings = Settings::instance();

        // Update current geometry and dockPosition values before saving.
        settings->setCurrentValue("gui/geometry", saveGeometry());
        settings->setCurrentValue("gui/dockPositions", saveState());

        settings->save(qSettings);
    }

    void MainWindow::setDocumentTitle(const QString& filename)
    {
        setWindowTitle(titleText.arg(filename));
    }

    void MainWindow::updateTitle()
    {
        Tab *tab = tabWidget()->currentTab();
        if (!tab) {
            return;
        }
        setDocumentTitle(tab->tabText());


        IView *view = tab->activeView();
        if (!view) {
            return;
        }

        setWindowModified(view->document()->isModified());
    }

    void MainWindow::slotUpdateSettingsChanges()
    {
        DocumentViewManager::instance()->updateSettingsChanges();
    }

    void MainWindow::slotStatusBarMessage(const QString& newPos)
    {
        m_statusLabel->setText(newPos);
    }

} // namespace Caneda
