/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "aboutdialog.h"
#include "actionmanager.h"
#include "documentviewmanager.h"
#include "exportdialog.h"
#include "filenewdialog.h"
#include "folderbrowser.h"
#include "global.h"
#include "icontext.h"
#include "idocument.h"
#include "iview.h"
#include "project.h"
#include "projectfileopendialog.h"
#include "printdialog.h"
#include "savedocumentsdialog.h"
#include "settings.h"
#include "settingsdialog.h"
#include "statehandler.h"
#include "tabs.h"

#include <QtWidgets>

namespace Caneda
{
    //! \brief Constructs and setups the MainWindow for the application.
    MainWindow::MainWindow(QWidget *w) : QMainWindow(w)
    {
        m_tabWidget = new TabWidget(this);
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        m_tabWidget->setTabsClosable(true);
        m_tabWidget->setMovable(true);
        connect(m_tabWidget, SIGNAL(statusBarMessage(QString)),
                this, SLOT(statusBarMessage(QString)));
        setCentralWidget(m_tabWidget);

        setObjectName("MainWindow"); // For debugging purposes

        // Be vary of the order as all the pointers are uninitialized at this moment.
        Settings *settings = Settings::instance();
        settings->load();

        initActions();
        initMenus();
        initToolBars();
        initStatusBar();

        setupSidebar();
        setupProjectsSidebar();
        setupFolderBrowserSidebar();

        loadSettings();  // Load window and docks geometry

        QTimer::singleShot(100, this, SLOT(initFile()));
    }

    /*!
     * \brief Returns the default instance of this class.
     *
     * This method is used to allow only one object instance of this class
     * thoughout all Caneda's process execution. This is specially useful for
     * classes that must be unique, to avoid, for example, modifying data at
     * the same time. Some examples of this are the Settings class, the
     * MainWindow class, or the document contexts (which must be unique).
     *
     * \return Default instance
     */
    MainWindow* MainWindow::instance()
    {
        static MainWindow* instance = 0;
        if (!instance) {
            instance = new MainWindow();
        }
        return instance;
    }

    //! \brief Returns a pointer to the window tabWidget.
    TabWidget* MainWindow::tabWidget() const
    {
        return m_tabWidget;
    }

    /*!
     * \brief Returns a pointer to the window sidebar.
     *
     * This method is called to be able to have context sensitive sidebars. In
     * this way, every time the context is changed the correspoding tools and
     * items from the IContext::sideBarWidget() method are used to populate the
     * MainWindow sidebar.
     */
    QDockWidget* MainWindow::sidebarDockWidget() const
    {
        return m_sidebarDockWidget;
    }

    //! \brief Initializes the Components sidebar.
    void MainWindow::setupSidebar()
    {
        m_sidebarDockWidget = new QDockWidget("Components Browser",this);
        m_sidebarDockWidget->setObjectName("componentsSidebar");
        addDockWidget(Qt::LeftDockWidgetArea, m_sidebarDockWidget);
        docksMenu->addAction(m_sidebarDockWidget->toggleViewAction());
    }

    //! \brief Initializes the Projects sidebar.
    void MainWindow::setupProjectsSidebar()
    {
        StateHandler *handler = StateHandler::instance();
        m_project = new Project(this);
        connect(m_project, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(slotSidebarItemClicked(const QString&, const QString&)));
        connect(m_project, SIGNAL(itemDoubleClicked(QString)), this,
                SLOT(open(QString)));

        m_projectDockWidget = new QDockWidget(m_project->windowTitle(), this);
        m_projectDockWidget->setWidget(m_project);
        m_projectDockWidget->setObjectName("projectsSidebar");
        m_projectDockWidget->setVisible(false);
        addDockWidget(Qt::LeftDockWidgetArea, m_projectDockWidget);
        //! \todo Uncomment once project is reimplemented.
        //        docksMenu->addAction(m_projectDockWidget->toggleViewAction());
    }

    //! \brief Initializes the FolderBrowser sidebar.
    void MainWindow::setupFolderBrowserSidebar()
    {
        FolderBrowser *m_folderBrowser = new FolderBrowser(this);

        connect(m_folderBrowser, SIGNAL(itemDoubleClicked(QString)), this,
                SLOT(open(QString)));

        m_browserDockWidget = new QDockWidget(m_folderBrowser->windowTitle(), this);
        m_browserDockWidget->setWidget(m_folderBrowser);
        m_browserDockWidget->setObjectName("folderBrowserSidebar");
        addDockWidget(Qt::LeftDockWidgetArea, m_browserDockWidget);
        tabifyDockWidget(m_browserDockWidget, m_projectDockWidget);
        docksMenu->addAction(m_browserDockWidget->toggleViewAction());
    }

    //! \brief Creates and intializes all the actions used.
    void MainWindow::initActions()
    {
        QAction *action = 0;
        ActionManager *am = ActionManager::instance();

        action = am->createAction("fileNew", Caneda::icon("document-new"), tr("&New..."));
        action->setShortcut(QKeySequence(QKeySequence::New));
        action->setStatusTip(tr("Creates a new file document"));
        action->setWhatsThis(tr("New file\n\nCreates a new file document"));
        connect(action, SIGNAL(triggered()), SLOT(newFile()));

        action = am->createAction("fileOpen", Caneda::icon("document-open"), tr("&Open..."));
        action->setShortcut(QKeySequence(QKeySequence::Open));
        action->setStatusTip(tr("Opens an existing document"));
        action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
        connect(action, SIGNAL(triggered()), SLOT(open()));

        for(int i=0; i<maxRecentFiles; i++) {
            action = am->createRecentFilesAction();
            connect(action, SIGNAL(triggered()), SLOT(openRecent()));
        }

        action = am->createAction("fileSave", Caneda::icon("document-save"), tr("&Save"));
        action->setShortcut(QKeySequence(QKeySequence::Save));
        action->setStatusTip(tr("Saves the current document"));
        action->setWhatsThis(tr("Save File\n\nSaves the current document"));
        connect(action, SIGNAL(triggered()), SLOT(save()));

        action = am->createAction("fileSaveAs", Caneda::icon("document-save-as"), tr("Save as..."));
        action->setShortcut(QKeySequence(QKeySequence::SaveAs));
        action->setStatusTip(tr("Saves the current document under a new filename"));
        action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
        connect(action, SIGNAL(triggered()), SLOT(saveAs()));

        action = am->createAction("fileSaveAll", Caneda::icon("document-save-all"), tr("Save &all"));
        action->setStatusTip(tr("Saves all open documents"));
        action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
        connect(action, SIGNAL(triggered()), SLOT(saveAll()));

        action = am->createAction("fileClose", Caneda::icon("document-close"), tr("&Close"));
        action->setShortcut(QKeySequence(QKeySequence::Close));
        action->setStatusTip(tr("Closes the current document"));
        action->setWhatsThis(tr("Close File\n\nCloses the current document"));
        connect(action, SIGNAL(triggered()), SLOT(closeFile()));

        action = am->createAction("filePrint", Caneda::icon("document-print"), tr("&Print..."));
        action->setShortcut(QKeySequence(QKeySequence::Print));
        action->setStatusTip(tr("Prints the current document"));
        action->setWhatsThis(tr("Print File\n\nPrints the current document"));
        connect(action, SIGNAL(triggered()), SLOT(print()));

        action = am->createAction("fileExportImage", Caneda::icon("image-x-generic"), tr("&Export image..."));
        action->setShortcut(QKeySequence(tr("Ctrl+E")));
        action->setStatusTip(tr("Exports the current view to an image file"));
        action->setWhatsThis(tr("Export Image\n\n""Exports the current view to an image file"));
        connect(action, SIGNAL(triggered()), SLOT(exportImage()));

        action = am->createAction("appSettings", Caneda::icon("preferences-other"), tr("Application settings..."));
        action->setShortcut(QKeySequence(QKeySequence::Preferences));
        action->setStatusTip(tr("Sets the properties of the application"));
        action->setWhatsThis(tr("Caneda Settings\n\nSets the properties of the application"));
        connect(action, SIGNAL(triggered()), SLOT(applicationSettings()));

        action = am->createAction("propertiesDialog", Caneda::icon("document-properties"), tr("Edit parameters..."));
        action->setStatusTip(tr("Launches current selection properties dialog"));
        action->setWhatsThis(tr("Edit Parameters\n\nLaunches current selection properties dialog"));
        connect(action, SIGNAL(triggered()), SLOT(launchPropertiesDialog()));

        action = am->createAction("fileQuit", Caneda::icon("application-exit"), tr("E&xit"));
        action->setShortcut(QKeySequence(QKeySequence::Quit));
        action->setStatusTip(tr("Quits the application"));
        action->setWhatsThis(tr("Exit\n\nQuits the application"));
        connect(action, SIGNAL(triggered()), SLOT(close()));

        action = am->createAction("editUndo", Caneda::icon("edit-undo"), tr("&Undo"));
        action->setShortcut(QKeySequence(QKeySequence::Undo));
        action->setStatusTip(tr("Undoes the last command"));
        action->setWhatsThis(tr("Undo\n\nMakes the last action undone"));
        connect(action, SIGNAL(triggered()), SLOT(undo()));

        action = am->createAction("editRedo", Caneda::icon("edit-redo"), tr("&Redo"));
        action->setShortcut(QKeySequence(QKeySequence::Redo));
        action->setStatusTip(tr("Redoes the last command"));
        action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
        connect(action, SIGNAL(triggered()), SLOT(redo()));

        action = am->createAction("editCut", Caneda::icon("edit-cut"), tr("Cu&t"));
        action->setShortcut(QKeySequence(QKeySequence::Cut));
        action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
        action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(cut()));

        action = am->createAction("editCopy", Caneda::icon("edit-copy"), tr("&Copy"));
        action->setShortcut(QKeySequence(QKeySequence::Copy));
        action->setStatusTip(tr("Copies the selection into the clipboard"));
        action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(copy()));

        action = am->createAction("editPaste", Caneda::icon("edit-paste"), tr("&Paste"));
        action->setShortcut(QKeySequence(QKeySequence::Paste));
        action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
        action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
        connect(action, SIGNAL(triggered()), SLOT(paste()));

        action = am->createAction("selectAll", Caneda::icon("select-rectangular"), tr("Select all"));
        action->setShortcut(QKeySequence(QKeySequence::SelectAll));
        action->setStatusTip(tr("Selects all elements"));
        action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
        connect(action, SIGNAL(triggered()), SLOT(selectAll()));

        action = am->createAction("editFind", Caneda::icon("edit-find"), tr("Find..."));
        action->setShortcut(QKeySequence(QKeySequence::Find));
        action->setStatusTip(tr("Find a piece of text"));
        action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
        connect(action, SIGNAL(triggered()), SLOT(find()));

        action = am->createAction("schEdit", Caneda::icon("draw-freehand"), tr("&Edit circuit schematic"));
        action->setShortcut(QKeySequence(tr("F2")));
        action->setStatusTip(tr("Switches to schematic edit"));
        action->setWhatsThis(tr("Edit Circuit Schematic\n\nSwitches to schematic edit"));
        connect(action, SIGNAL(triggered()), SLOT(openSchematic()));

        action = am->createAction("symEdit", Caneda::icon("draw-freehand"), tr("Edit circuit &symbol"));
        action->setShortcut(QKeySequence(tr("F3")));
        action->setStatusTip(tr("Switches to symbol edit"));
        action->setWhatsThis(tr("Edit Circuit Symbol\n\nSwitches to symbol edit"));
        connect(action, SIGNAL(triggered()), SLOT(openSymbol()));

        action = am->createAction("layEdit", Caneda::icon("draw-freehand"), tr("Edit circuit &layout"));
        action->setShortcut(QKeySequence(tr("F4")));
        action->setStatusTip(tr("Switches to layout edit"));
        action->setWhatsThis(tr("Edit Circuit Layout\n\nSwitches to layout edit"));
        connect(action, SIGNAL(triggered()), SLOT(openLayout()));

        action = am->createAction("intoH", Caneda::icon("go-bottom"), tr("Go into subcircuit"));
        action->setShortcut(QKeySequence(tr("Ctrl+I")));
        action->setToolTip(tr("Go into Subcircuit") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Goes inside the selected subcircuit"));
        action->setWhatsThis(tr("Go into Subcircuit\n\nGoes inside the selected subcircuit"));
        connect(action, SIGNAL(triggered()), SLOT(intoHierarchy()));

        action = am->createAction("popH", Caneda::icon("go-top"), tr("Pop out"));
        action->setShortcut(QKeySequence(tr("Ctrl+Shift+I")));
        action->setToolTip(tr("Pop Out") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Goes outside the current subcircuit"));
        action->setWhatsThis(tr("Pop Out\n\nGoes up one hierarchy level (leaves the current subcircuit)"));
        connect(action, SIGNAL(triggered()), SLOT(popHierarchy()));

        action = am->createAction("zoomFitInBest", Caneda::icon("zoom-fit-best"), tr("View all"));
        action->setShortcut(QKeySequence(tr("0")));
        action->setStatusTip(tr("Show the whole contents"));
        action->setWhatsThis(tr("View all\n\nShows the whole page content"));
        connect(action, SIGNAL(triggered()), SLOT(zoomBestFit()));

        action = am->createAction("zoomOriginal", Caneda::icon("zoom-original"), tr("View 1:1"));
        action->setShortcut(QKeySequence(tr("1")));
        action->setStatusTip(tr("View without magnification"));
        action->setWhatsThis(tr("Zoom 1:1\n\nShows the page content without magnification"));
        connect(action, SIGNAL(triggered()), SLOT(zoomOriginal()));

        action = am->createAction("zoomIn", Caneda::icon("zoom-in"), tr("Zoom in"));
        action->setShortcut(QKeySequence(tr("+")));
        action->setStatusTip(tr("Zooms in the content"));
        action->setWhatsThis(tr("Zoom In \n\nZooms in the content"));
        connect(action, SIGNAL(triggered()), SLOT(zoomIn()));

        action = am->createAction("zoomOut", Caneda::icon("zoom-out"), tr("Zoom out"));
        action->setShortcut(QKeySequence(tr("-")));
        action->setStatusTip(tr("Zooms out the content"));
        action->setWhatsThis(tr("Zoom Out \n\nZooms out the content"));
        connect(action, SIGNAL(triggered()), SLOT(zoomOut()));

        action = am->createAction("splitHorizontal", Caneda::icon("view-split-left-right"), tr("Split &horizontal"));
        action->setShortcut(QKeySequence(tr("Ctrl+Shift+L")));
        action->setStatusTip(tr("Splits the current view in horizontal orientation"));
        action->setWhatsThis(tr("Split Horizontal\n\nSplits the current view in horizontal orientation"));
        connect(action, SIGNAL(triggered()), SLOT(splitHorizontal()));

        action = am->createAction("splitVertical", Caneda::icon("view-split-top-bottom"), tr("Split &vertical"));
        action->setShortcut(QKeySequence(tr("Ctrl+Shift+T")));
        action->setStatusTip(tr("Splits the current view in vertical orientation"));
        action->setWhatsThis(tr("Split Vertical\n\nSplits the current view in vertical orientation"));
        connect(action, SIGNAL(triggered()), SLOT(splitVertical()));

        action = am->createAction("splitClose", Caneda::icon("view-left-close"), tr("&Close split"));
        action->setShortcut(QKeySequence(tr("Ctrl+Shift+R")));
        action->setStatusTip(tr("Closes the current split"));
        action->setWhatsThis(tr("Close Split\n\nCloses the current split"));
        connect(action, SIGNAL(triggered()), SLOT(closeSplit()));

        action = am->createAction("viewToolBar",  tr("Tool&bar"));
        action->setStatusTip(tr("Enables/disables the toolbar"));
        action->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), SLOT(viewToolBar(bool)));

        action = am->createAction("viewStatusBar",  tr("&Statusbar"));
        action->setStatusTip(tr("Enables/disables the statusbar"));
        action->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), SLOT(viewStatusBar(bool)));

        action = am->createAction("alignTop", Caneda::icon("align-vertical-top"), tr("Align top"));
        action->setStatusTip(tr("Align top selected elements"));
        action->setWhatsThis(tr("Align top\n\nAlign selected elements to their upper edge"));
        connect(action, SIGNAL(triggered()), SLOT(alignTop()));

        action = am->createAction("alignBottom", Caneda::icon("align-vertical-bottom"), tr("Align bottom"));
        action->setStatusTip(tr("Align bottom selected elements"));
        action->setWhatsThis(tr("Align bottom\n\nAlign selected elements to their lower edge"));
        connect(action, SIGNAL(triggered()), SLOT(alignBottom()));

        action = am->createAction("alignLeft", Caneda::icon("align-horizontal-left"), tr("Align left"));
        action->setStatusTip(tr("Align left selected elements"));
        action->setWhatsThis(tr("Align left\n\nAlign selected elements to their left edge"));
        connect(action, SIGNAL(triggered()), SLOT(alignLeft()));

        action = am->createAction("alignRight", Caneda::icon("align-horizontal-right"), tr("Align right"));
        action->setStatusTip(tr("Align right selected elements"));
        action->setWhatsThis(tr("Align right\n\nAlign selected elements to their right edge"));
        connect(action, SIGNAL(triggered()), SLOT(alignRight()));

        action = am->createAction("centerHor", Caneda::icon("align-horizontal-center"), tr("Center horizontally"));
        action->setStatusTip(tr("Center horizontally selected elements"));
        action->setWhatsThis(tr("Center horizontally\n\nCenter horizontally selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(centerHorizontal()));

        action = am->createAction("centerVert", Caneda::icon("align-vertical-center"), tr("Center vertically"));
        action->setStatusTip(tr("Center vertically selected elements"));
        action->setWhatsThis(tr("Center vertically\n\nCenter vertically selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(centerVertical()));

        action = am->createAction("distrHor", Caneda::icon("distribute-horizontal-center"), tr("Distribute horizontally"));
        action->setStatusTip(tr("Distribute equally horizontally"));
        action->setWhatsThis(tr("Distribute horizontally\n\n""Distribute horizontally selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(distributeHorizontal()));

        action = am->createAction("distrVert", Caneda::icon("distribute-vertical-center"), tr("Distribute vertically"));
        action->setStatusTip(tr("Distribute equally vertically"));
        action->setWhatsThis(tr("Distribute vertically\n\n""Distribute vertically selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(distributeVertical()));

        action = am->createAction("projNew", Caneda::icon("project-new"), tr("&New project..."));
        action->setStatusTip(tr("Creates a new project"));
        action->setWhatsThis(tr("New Project\n\nCreates a new project"));
        connect(action, SIGNAL(triggered()), SLOT(newProject()));

        action = am->createAction("projOpen", Caneda::icon("document-open"), tr("&Open project..."));
        action->setStatusTip(tr("Opens an existing project"));
        action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
        connect(action, SIGNAL(triggered()), SLOT(openProject()));

        action = am->createAction("addToProj", Caneda::icon("document-new"), tr("&Add file to project..."));
        action->setStatusTip(tr("Adds a file to current project"));
        action->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));
        connect(action, SIGNAL(triggered()), SLOT(addToProject()));

        action = am->createAction("projDel", Caneda::icon("document-close"), tr("&Remove from project"));
        action->setStatusTip(tr("Removes a file from current project"));
        action->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));
        connect(action, SIGNAL(triggered()), SLOT(removeFromProject()));

        action = am->createAction("projClose", Caneda::icon("dialog-close"), tr("&Close project"));
        action->setStatusTip(tr("Closes the current project"));
        action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
        connect(action, SIGNAL(triggered()), SLOT(closeProject()));

        action = am->createAction("backupAndHistory", Caneda::icon("chronometer"), tr("&Backup and history..."));
        action->setStatusTip(tr("Opens backup and history dialog"));
        action->setWhatsThis(tr("Backup and History\n\nOpens backup and history dialog"));
        connect(action, SIGNAL(triggered()), SLOT(backupAndHistory()));

        action = am->createAction("simulate", Caneda::icon("media-playback-start"), tr("Simulate"));
        action->setShortcut(QKeySequence(QKeySequence::Refresh));
        action->setToolTip(tr("Simulate") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Simulates the current circuit"));
        action->setWhatsThis(tr("Simulate\n\nSimulates the current circuit"));
        connect(action, SIGNAL(triggered()), SLOT(simulate()));

        action = am->createAction("openSym", Caneda::icon("system-switch-user"), tr("View circuit simulation"));
        action->setShortcut(tr("F6"));
        action->setToolTip(tr("View Circuit Simulation") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Changes to circuit simulation"));
        action->setWhatsThis(tr("View Circuit Simulation\n\n")+tr("Changes to circuit simulation"));
        connect(action, SIGNAL(triggered()), SLOT(openSimulation()));

        action = am->createAction("showLog", Caneda::icon("document-preview"), tr("Show simulation log"));
        action->setShortcut(tr("F7"));
        action->setStatusTip(tr("Shows simulation log"));
        action->setWhatsThis(tr("Show Log\n\nShows the log of the current simulation"));
        connect(action, SIGNAL(triggered()), SLOT(showLog()));

        action = am->createAction("showNetlist", Caneda::icon("document-preview"), tr("Show circuit netlist"));
        action->setShortcut(tr("F8"));
        action->setStatusTip(tr("Shows the circuit netlist"));
        action->setWhatsThis(tr("Show Netlist\n\nShows the netlist of the current circuit"));
        connect(action, SIGNAL(triggered()), SLOT(showNetlist()));

        action = am->createAction("helpIndex", Caneda::icon("help-contents"), tr("&Help index..."));
        action->setShortcut(QKeySequence(QKeySequence::HelpContents));
        action->setStatusTip(tr("Index of Caneda Help"));
        action->setWhatsThis(tr("Help Index\n\nIndex of intern Caneda help"));
        connect(action, SIGNAL(triggered()), SLOT(helpIndex()));

        action = am->createAction("helpExamples", Caneda::icon("draw-freehand"), tr("&Example circuits..."));
        action->setStatusTip(tr("Open Caneda example circuits"));
        action->setWhatsThis(tr("Example circuits\n\nOpen Caneda example circuits"));
        connect(action, SIGNAL(triggered()), SLOT(helpExamples()));

        QAction *qAction = QWhatsThis::createAction(this);
        action->setShortcut(QKeySequence(QKeySequence::WhatsThis));
        action = am->createAction("whatsThis", qAction->icon(), qAction->text());
        connect(action, SIGNAL(triggered()), qAction, SLOT(trigger()));
        connect(action, SIGNAL(hovered()), qAction, SLOT(hover()));

        action = am->createAction("helpAboutApp", Caneda::icon("caneda"), tr("&About Caneda..."));
        action->setStatusTip(tr("About the application"));
        action->setWhatsThis(tr("About\n\nAbout the application"));
        connect(action, SIGNAL(triggered()), SLOT(about()));

        action = am->createAction("helpAboutQt", Caneda::icon("qt"), tr("About &Qt..."));
        action->setStatusTip(tr("About Qt by Nokia"));
        action->setWhatsThis(tr("About Qt\n\nAbout Qt by Nokia"));
        connect(action, SIGNAL(triggered()), SLOT(aboutQt()));

        initMouseActions();
    }

    //! \brief Creates and intializes the mouse actions.
    void MainWindow::initMouseActions()
    {
        QAction *action = 0;
        ActionManager *am = ActionManager::instance();

        StateHandler *handler = StateHandler::instance();

        action = am->createMouseAction("select", Caneda::Normal,
                Caneda::icon("edit-select"), tr("Select"));
        action->setShortcut(QKeySequence(tr("Esc")));
        action->setToolTip(tr("Select") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Activate select mode"));
        action->setWhatsThis(tr("Select\n\nActivates select mode"));
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("editDelete", Caneda::Deleting,
                Caneda::icon("edit-delete"), tr("&Delete"));
        action->setShortcut(QKeySequence(QKeySequence::Delete));
        action->setToolTip(tr("Delete") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Deletes the selected components"));
        action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("editRotate", Caneda::Rotating,
                Caneda::icon("object-rotate-right"), tr("Rotate"));
        action->setShortcut(QKeySequence(tr("R")));
        action->setToolTip(tr("Rotate") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Rotates the selected component"));
        action->setWhatsThis(tr("Rotate\n\nRotates the selected component counter-clockwise"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("editMirror", Caneda::MirroringX,
                Caneda::icon("object-flip-vertical"), tr("Mirror vertically"));
        action->setShortcut(QKeySequence(tr("V")));
        action->setToolTip(tr("Mirror vertically") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Mirrors the selected components vertically"));
        action->setWhatsThis(tr("Mirror vertically Axis\n\nMirrors the selected components vertically"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("editMirrorY", Caneda::MirroringY,
                Caneda::icon("object-flip-horizontal"), tr("Mirror horizontally"));
        action->setShortcut(QKeySequence(tr("H")));
        action->setToolTip(tr("Mirror horizontally") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Mirrors the selected components horizontally"));
        action->setWhatsThis(tr("Mirror horizontally\n\nMirrors the selected components horizontally"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("insWire", Caneda::Wiring,
                Caneda::icon("wire"), tr("Wire"));
        action->setShortcut(QKeySequence(tr("W")));
        action->setToolTip(tr("Wire") + " (" + action->shortcut().toString() + ")");
        action->setStatusTip(tr("Inserts a wire"));
        action->setWhatsThis(tr("Wire\n\nInserts a wire"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("zoomArea", Caneda::ZoomingAreaEvent,
                Caneda::icon("transform-scale"), tr("Zoom area"));
        action->setStatusTip(tr("Zooms a selected area in the current view"));
        action->setWhatsThis(tr("Zooms a selected area in the current view"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("insertItem", Caneda::InsertingItems,
                tr("Insert item action"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));

        action = am->createMouseAction("paintingDraw", Caneda::PaintingDrawEvent,
                tr("Painting draw action"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(slotPerformToggleAction(bool)));
    }

    //! \brief Creates and initializes the menus.
    void MainWindow::initMenus()
    {
        ActionManager* am = ActionManager::instance();
        QMenu *menu = 0;
        QMenu *recentFilesMenu = 0;

        // File menu
        menu = menuBar()->addMenu(tr("&File"));

        menu->addAction(am->actionForName("fileNew"));
        menu->addAction(am->actionForName("fileOpen"));

        recentFilesMenu = menu->addMenu(Caneda::icon("document-open-recent"), tr("Open &Recent"));
        for(int i=0; i<maxRecentFiles; i++) {
            recentFilesMenu->addAction(am->recentFilesActions().at(i));
        }
        DocumentViewManager::instance()->updateRecentFilesActionList();  // Update the list from the previosly saved configuration file

        menu->addAction(am->actionForName("fileClose"));

        menu->addSeparator();

        menu->addAction(am->actionForName("fileSave"));
        menu->addAction(am->actionForName("fileSaveAll"));
        menu->addAction(am->actionForName("fileSaveAs"));
        menu->addAction(am->actionForName("filePrint"));
        menu->addAction(am->actionForName("fileExportImage"));

        menu->addSeparator();

        menu->addAction(am->actionForName("appSettings"));

        menu->addSeparator();

        menu->addAction(am->actionForName("fileQuit"));

        // Edit menu
        menu = menuBar()->addMenu(tr("&Edit"));

        menu->addAction(am->actionForName("editUndo"));
        menu->addAction(am->actionForName("editRedo"));

        menu->addSeparator();

        menu->addAction(am->actionForName("editCut"));
        menu->addAction(am->actionForName("editCopy"));
        menu->addAction(am->actionForName("editPaste"));

        menu->addSeparator();

        menu->addAction(am->actionForName("select"));
        menu->addAction(am->actionForName("selectAll"));
        menu->addAction(am->actionForName("editDelete"));
        menu->addAction(am->actionForName("editMirror"));
        menu->addAction(am->actionForName("editMirrorY"));
        menu->addAction(am->actionForName("editRotate"));

        //! \todo Reenable this option once implemented
        //        menu->addAction(am->actionForName("editFind"));

        menu->addSeparator();

        menu->addAction(am->actionForName("schEdit"));
        menu->addAction(am->actionForName("symEdit"));
        menu->addAction(am->actionForName("layEdit"));

        //! \todo Reenable these options once implemented
        //        menu->addSeparator();

        //        menu->addAction(am->actionForName("intoH"));
        //        menu->addAction(am->actionForName("popH"));

        // View menu
        menu = menuBar()->addMenu(tr("&View"));

        menu->addAction(am->actionForName("zoomFitInBest"));
        menu->addAction(am->actionForName("zoomOriginal"));
        menu->addAction(am->actionForName("zoomIn"));
        menu->addAction(am->actionForName("zoomOut"));
        menu->addAction(am->actionForName("zoomArea"));

        menu->addSeparator();

        menu->addAction(am->actionForName("splitHorizontal"));
        menu->addAction(am->actionForName("splitVertical"));
        menu->addAction(am->actionForName("splitClose"));

        menu->addSeparator();

        docksMenu = menu->addMenu(tr("&Docks and Toolbars"));

        docksMenu->addAction(am->actionForName("viewToolBar"));
        docksMenu->addAction(am->actionForName("viewStatusBar"));

        docksMenu->addSeparator();

        // Align menu
        menu = menuBar()->addMenu(tr("P&ositioning"));

        menu->addAction(am->actionForName("centerHor"));
        menu->addAction(am->actionForName("centerVert"));

        menu->addSeparator();

        menu->addAction(am->actionForName("alignTop"));
        menu->addAction(am->actionForName("alignBottom"));
        menu->addAction(am->actionForName("alignLeft"));
        menu->addAction(am->actionForName("alignRight"));

        menu->addSeparator();

        menu->addAction(am->actionForName("distrHor"));
        menu->addAction(am->actionForName("distrVert"));

        // Project menu
        //! \todo Reenable these menus once project and tools reimplemented.
        //        menu = menuBar()->addMenu(tr("&Project"));

        //        menu->addAction(am->actionForName("projNew"));
        //        menu->addAction(am->actionForName("projOpen"));
        //        menu->addAction(am->actionForName("addToProj"));
        //        menu->addAction(am->actionForName("projDel"));
        //        menu->addAction(am->actionForName("projClose"));

        //        menu->addSeparator();

        //        menu->addAction(am->actionForName("backupAndHistory"));

        // Tools menu
        //! \todo Implement tools menu with a plugins' infrastructure.
        //        menu = menuBar()->addMenu(tr("&Tools"));

        // Simulation menu
        menu = menuBar()->addMenu(tr("&Simulation"));

        menu->addAction(am->actionForName("simulate"));
        menu->addAction(am->actionForName("openSym"));

        menu->addSeparator();

        menu->addAction(am->actionForName("showLog"));
        menu->addAction(am->actionForName("showNetlist"));

        // Help menu
        menu = menuBar()->addMenu(tr("&Help"));

        menu->addAction(am->actionForName("helpIndex"));
        menu->addAction(am->actionForName("helpExamples"));
        menu->addAction(am->actionForName("whatsThis"));

        menu->addSeparator();

        menu->addAction(am->actionForName("helpAboutApp"));
        menu->addAction(am->actionForName("helpAboutQt"));
    }

    //! \brief Creates and intializes the toolbars.
    void MainWindow::initToolBars()
    {
        ActionManager* am = ActionManager::instance();

        fileToolbar  = addToolBar(tr("File"));
        fileToolbar->setObjectName("fileToolBar");

        fileToolbar->addAction(am->actionForName("fileNew"));
        fileToolbar->addAction(am->actionForName("fileOpen"));
        fileToolbar->addAction(am->actionForName("fileSave"));
        fileToolbar->addAction(am->actionForName("fileSaveAs"));

        editToolbar  = addToolBar(tr("Edit"));
        editToolbar->setObjectName("editToolbar");

        editToolbar->addAction(am->actionForName("editCut"));
        editToolbar->addAction(am->actionForName("editCopy"));
        editToolbar->addAction(am->actionForName("editPaste"));
        editToolbar->addAction(am->actionForName("editUndo"));
        editToolbar->addAction(am->actionForName("editRedo"));

        workToolbar  = addToolBar(tr("Work"));
        workToolbar->setObjectName("workToolbar");

        workToolbar->addAction(am->actionForName("select"));
        workToolbar->addAction(am->actionForName("editDelete"));
        workToolbar->addAction(am->actionForName("editMirror"));
        workToolbar->addAction(am->actionForName("editMirrorY"));
        workToolbar->addAction(am->actionForName("editRotate"));

        workToolbar->addSeparator();

        workToolbar->addAction(am->actionForName("insWire"));
        //! \todo Reenable this option once implemented
        //        workToolbar->addAction(am->actionForName("intoH"));
        //        workToolbar->addAction(am->actionForName("popH"));

        workToolbar->addSeparator();

        workToolbar->addAction(am->actionForName("simulate"));
        workToolbar->addAction(am->actionForName("openSym"));
    }

    //! \brief Creates and intializes the statusbar.
    void MainWindow::initStatusBar()
    {
        QStatusBar *statusBarWidget = statusBar();
        ActionManager* am = ActionManager::instance();

        // Initially the label is an empty space.
        m_statusLabel = new QLabel(QString(""), statusBarWidget);

        // Configure viewToolbar
        viewToolbar  = addToolBar(tr("View"));
        viewToolbar->setObjectName("viewToolbar");

        viewToolbar->addSeparator();
        viewToolbar->addAction(am->actionForName("zoomFitInBest"));
        viewToolbar->addAction(am->actionForName("zoomOriginal"));
        viewToolbar->addAction(am->actionForName("zoomIn"));
        viewToolbar->addAction(am->actionForName("zoomOut"));
        viewToolbar->addAction(am->actionForName("zoomArea"));

        viewToolbar->setIconSize(QSize(10, 10));

        // Add the widgets to the toolbar
        statusBarWidget->addPermanentWidget(m_statusLabel);
        statusBarWidget->addPermanentWidget(viewToolbar);

        statusBarWidget->setVisible(am->actionForName("viewStatusBar")->isChecked());
    }

    //! \brief Syncs the settings to the configuration file and closes the window.
    void MainWindow::closeEvent(QCloseEvent *e)
    {
        if(saveAll()) {
            saveSettings();
            e->accept();
        }
        else {
            e->ignore();
        }
    }

    //! \brief Opens the file new dialog.
    void MainWindow::newFile()
    {
        if(m_project->isValid()) {
            addToProject();
        }
        else {
            QPointer<FileNewDialog> p = new FileNewDialog(this);
            p->exec();
            delete p;
        }
    }

    /*!
     * \brief Opens the file open dialog.
     *
     * Opens the file open dialog. If the file is already opened, the
     * corresponding tab is set as the current one. Otherwise the file is
     * opened and its tab is set as current tab.
     */
    void MainWindow::open(QString fileName)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();

        if(fileName.isEmpty()) {
            if(!m_project->isValid()) {
                fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                        manager->fileNameFilters().join(""));
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
                openProject(fileName);
            }
            else {
                manager->openFile(fileName);
            }
        }
    }

    /*!
     * \brief Open recently opened file.
     *
     * First identify the Action that called the slot and then load the
     * corresponding file. The entire file path is stored in action->data()
     * and the name of the file without the path is stored in action->text().
     *
     * \sa open(), DocumentViewManager::updateRecentFilesActionList()
     */
    void MainWindow::openRecent()
    {
        QAction *action = qobject_cast<QAction *>(sender());
        if(action) {
            open(action->data().toString());
        }
    }

    //! \brief Opens the selected file format given an opened file.
    void MainWindow::openFileFormat(const QString &suffix)
    {
        IDocument *doc = DocumentViewManager::instance()->currentDocument();

        if (!doc) return;

        if (doc->fileName().isEmpty()) {
            QMessageBox::critical(0, tr("Critical"),
                                  tr("Please, save current file first!"));
            return;
        }

        QFileInfo info(doc->fileName());
        QString filename = info.completeBaseName();
        QString path = info.path();
        filename = QDir::toNativeSeparators(path + "/" + filename + "." + suffix);

        open(filename);
    }

    //! \brief Saves the current active document.
    void MainWindow::save()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        if (document->fileName().isEmpty()) {
            saveAs();
            return;
        }

        QString errorMessage;
        if (!document->save(&errorMessage)) {
            QMessageBox::critical(this,
                    tr("%1 : File save error").arg(document->fileName()), errorMessage);
        }
    }

    //! \brief Opens a dialog to select a new filename and saves the current file.
    void MainWindow::saveAs()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if(!document) {
            return;
        }

        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                manager->currentDocument()->context()->fileNameFilters().join(""));
        if(fileName.isEmpty()) {
            return;
        }

        QString oldFileName = document->fileName();
        document->setFileName(fileName);

        QString errorMessage;
        if(!document->save(&errorMessage)) {
            QMessageBox::critical(this,
                    tr("%1 : File save error").arg(document->fileName()), errorMessage);
            document->setFileName(oldFileName);
        }
        else {
            open(fileName); // The document was saved ok, now reopen the document to load text highlighting
        }

        //! \todo Update/close other open document having same name as the above saved one.
    }

    /*!
     * \brief Opens a dialog giving the user options to save all modified files.
     *
     * \return True on success, false on cancel
     */
    bool MainWindow::saveAll()
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
    void MainWindow::closeFile()
    {
        Tab *current = tabWidget()->currentTab();
        if (current) {
            current->close();
        }
    }

    //! \brief Opens the print dialog.
    void MainWindow::print()
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

    //! \brief Opens the export image dialog.
    void MainWindow::exportImage()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        QPointer<ExportDialog> p = new ExportDialog(document, this);
        p->exec();
        delete p;
    }

    //! \brief Opens the applications settings dialog.
    void MainWindow::applicationSettings()
    {
        QList<SettingsPage *> wantedPages;
        SettingsPage *page = new GeneralConfigurationPage(this);
        wantedPages << page;
        page = new HdlConfigurationPage(this);
        wantedPages << page;
        page = new LibrariesConfigurationPage(this);
        wantedPages << page;
        page = new LayoutConfigurationPage(this);
        wantedPages << page;
        page = new SimulationConfigurationPage(this);
        wantedPages << page;

        SettingsDialog *d = new SettingsDialog(wantedPages, "Configure Caneda", this);
        int result = d->exec();

        // Update all document views to reflect the current settings.
        if(result == QDialog::Accepted) {
            DocumentViewManager::instance()->updateSettingsChanges();
            repaint();
        }
    }

    //! \brief Calls the current document undo action.
    void MainWindow::undo()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->undo();
        }

    }

    //! \brief Calls the current document redo action.
    void MainWindow::redo()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->redo();
        }
    }

    //! \brief Calls the current document cut action.
    void MainWindow::cut()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->cut();
        }
    }

    //! \brief Calls the current document copy action.
    void MainWindow::copy()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->copy();
        }
    }

    //! \brief Calls the current document paste action.
    void MainWindow::paste()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->paste();
        }
    }

    //! \brief Calls the current document find action.
    void MainWindow::find()
    {
        //! \todo Implement this or rather port directly
    }

    //! \brief Calls the current document select all action.
    void MainWindow::selectAll()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->selectAll();
        }
    }

    //! \brief Opens the layout document corresponding to the current file.
    void MainWindow::openLayout()
    {
        LayoutContext *ly = LayoutContext::instance();
        openFileFormat(ly->defaultSuffix());
    }

    //! \brief Opens the schematic document corresponding to the current file.
    void MainWindow::openSchematic()
    {
        SchematicContext *sc = SchematicContext::instance();
        openFileFormat(sc->defaultSuffix());
    }

    //! \brief Opens the symbol document corresponding to the current file.
    void MainWindow::openSymbol()
    {
        SymbolContext *sy = SymbolContext::instance();
        openFileFormat(sy->defaultSuffix());
    }

    //! \brief Opens the selected item file description for edition.
    void MainWindow::intoHierarchy()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->intoHierarchy();
        }
    }

    //! \brief Opens the parent document from where this item was opened.
    void MainWindow::popHierarchy()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->popHierarchy();
        }
    }

    //! \brief Calls the current view zoom in action.
    void MainWindow::zoomIn()
    {
        IView* view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->zoomIn();
        }
    }

    //! \brief Calls the current view zoom out action.
    void MainWindow::zoomOut()
    {
        IView* view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->zoomOut();
        }
    }

    //! \brief Calls the current view zoom best fit action.
    void MainWindow::zoomBestFit()
    {
        IView* view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->zoomFitInBest();
        }
    }

    //! \brief Calls the current view zoom original action.
    void MainWindow::zoomOriginal()
    {
        IView* view = DocumentViewManager::instance()->currentView();
        if(view) {
            view->zoomOriginal();
        }
    }

    //! \brief Calls the current view split horizontal action.
    void MainWindow::splitHorizontal()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView* view = manager->currentView();
        if(view) {
            manager->splitView(view, Qt::Horizontal);
        }
    }

    //! \brief Calls the current view split vertical action.
    void MainWindow::splitVertical()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView* view = manager->currentView();
        if(view) {
            manager->splitView(view, Qt::Vertical);
        }
    }

    //! \brief Calls the current view close split action.
    void MainWindow::closeSplit()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView* view = manager->currentView();
        if(view) {
            manager->closeView(view);
        }
    }

    //! \brief Toogles the visibility of the toolbars.
    void MainWindow::viewToolBar(bool toogle)
    {
        fileToolbar->setVisible(toogle);
        editToolbar->setVisible(toogle);
        viewToolbar->setVisible(toogle);
        workToolbar->setVisible(toogle);
    }

    //! \brief Toogles the visibility of the statusbar.
    void MainWindow::viewStatusBar(bool toogle)
    {
        statusBar()->setVisible(toogle);
    }

    //! \brief Aligns the selected elements to the top
    void MainWindow::alignTop()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->alignTop();
        }
    }

    //! \brief Aligns the selected elements to the bottom
    void MainWindow::alignBottom()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->alignBottom();
        }
    }

    //! \brief Aligns the selected elements to the left
    void MainWindow::alignLeft()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->alignLeft();
        }
    }

    //! \brief Aligns the selected elements to the right
    void MainWindow::alignRight()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->alignRight();
        }
    }

    //! \brief Centers the selected elements horizontally.
    void MainWindow::centerHorizontal()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->centerHorizontal();
        }
    }

    //! \brief Centers the selected elements vertically.
    void MainWindow::centerVertical()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->centerVertical();
        }
    }

    //! \brief Distributes the selected elements horizontally.
    void MainWindow::distributeHorizontal()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->distributeHorizontal();
        }
    }

    //! \brief Distributes the selected elements vertically.
    void MainWindow::distributeVertical()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->distributeVertical();
        }
    }

    //! \brief Prepares the window and created a new project.
    void MainWindow::newProject()
    {
        if(saveAll()) {
            m_tabWidget->closeAllTabs();
            m_projectDockWidget->setVisible(true);
            m_projectDockWidget->raise();
            m_project->slotNewProject();
        }
    }

    //! \brief Prepares the window and opens a new project.
    void MainWindow::openProject(QString fileName)
    {
        if(saveAll()) {
            m_tabWidget->closeAllTabs();
            m_projectDockWidget->setVisible(true);
            m_projectDockWidget->raise();
            m_project->slotOpenProject(fileName);
        }
    }

    //! \brief Adds a file to the current project.
    void MainWindow::addToProject()
    {
        m_projectDockWidget->setVisible(true);
        m_projectDockWidget->raise();
        m_project->slotAddToProject();
    }

    //! \brief Removes a file from the current project.
    void MainWindow::removeFromProject()
    {
        m_projectDockWidget->setVisible(true);
        m_projectDockWidget->raise();
        m_project->slotRemoveFromProject();
    }

    //! \brief Closes the current project.
    void MainWindow::closeProject()
    {
        if(saveAll()) {
            m_project->slotCloseProject();
            m_tabWidget->closeAllTabs();
        }
    }

    //! \brief Opens the backup and history file dialog.
    void MainWindow::backupAndHistory()
    {
        m_project->slotBackupAndHistory();
    }

    //! \brief Simulates the current document.
    void MainWindow::simulate()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->simulate();
        }
    }

    //! \brief Opens the simulation corresponding to the current file.
    void MainWindow::openSimulation()
    {
        SimulationContext *si = SimulationContext::instance();
        openFileFormat(si->defaultSuffix());
    }

    //! \brief Opens the log corresponding to the current file.
    void MainWindow::showLog()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();

        QFileInfo info(manager->currentDocument()->fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();

        manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".log"));
    }

    //! \brief Opens the netlist corresponding to the current file.
    void MainWindow::showNetlist()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();

        QFileInfo info(manager->currentDocument()->fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();

        manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".net"));
    }

    //! \brief Opens the help index.
    void MainWindow::helpIndex()
    {
        open(QString("index.html"));
    }

    //! \brief Opens the examples repository in an external window.
    void MainWindow::helpExamples()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/Caneda/Examples"));
    }

    //! \brief Opens the about dialog.
    void MainWindow::about()
    {
        QPointer<AboutDialog> p = new AboutDialog(this);
        p->exec();
        delete p;
    }

    //! \brief Opens the about Qt dialog.
    void MainWindow::aboutQt()
    {
        QApplication::aboutQt();
    }

    //! \brief Creates a new file used for the program initial state.
    void MainWindow::initFile()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->newDocument(SchematicContext::instance());
    }

    /*!
     * \brief Loads window and docks geometry.
     *
     * Load window and docks geometry, saved in a previous program execution.
     * This method should be called after the instance Settings::load() method
     * has been called, thus effectively loading correct settings.
     *
     * \sa saveSettings(), Settings::load()
     */
    void MainWindow::loadSettings()
    {
        Settings *settings = Settings::instance();

        // Load geometry and docks positions
        const QByteArray geometryData = settings->currentValue("gui/geometry").toByteArray();
        if (geometryData.isEmpty() == false) {
            restoreGeometry(geometryData);
        }

        const QByteArray dockData = settings->currentValue("gui/dockPositions").toByteArray();
        if (dockData.isEmpty() == false) {
            restoreState(dockData);
        }
    }

    /*!
     * \brief Saves window and docks geometry.
     *
     * Save window and docks geometry, to be loaded in the next program
     * execution. This method should be called just before closing the program,
     * saving the last used position and geometry of the window.
     *
     * \sa loadSettings(), Settings::save()
     */
    void MainWindow::saveSettings()
    {
        Settings *settings = Settings::instance();

        // Update current geometry and dockPosition values before saving.
        settings->setCurrentValue("gui/geometry", saveGeometry());
        settings->setCurrentValue("gui/dockPositions", saveState());

        settings->save();
    }

   /*!
    * \brief Launches the properties dialog corresponding to current document.
    *
    * The properties dialog should be some kind of settings dialog, but specific
    * to the current document and context.
    *
    * This method should be implemented according to the calling context. For
    * example, the properties dialog for a schematic scene should be a simple
    * dialog to add or remove properties. In that case, if a component is selected
    * the properties dialog should contain the component properties. On the
    * other hand, when called from a simulation context, simulation options may
    * be presented for the user. In order to do so, and be context aware, the
    * IDocument::launchPropertiesDialog() of the current document is invoked.
    *
    * \sa IDocument::launchPropertiesDialog()
    */
    void MainWindow::launchPropertiesDialog()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->launchPropertiesDialog();
        }
    }

    /*!
     * \brief Updates the window and tab title.
     *
     *  Update the window and tab title. The [*] in the title indicates to the
     *  application where to insert the file modified character * when
     *  setWindowModified is invoked.
     */
    void MainWindow::updateWindowTitle()
    {
        Tab *tab = tabWidget()->currentTab();
        if(!tab) {
            setWindowTitle(QString("Caneda"));
            return;
        }

        IView *view = tab->activeView();
        if (!view) {
            return;
        }

        setWindowTitle(tab->tabText() + QString("[*] - Caneda"));
        setWindowModified(view->document()->isModified());
    }

    //! \brief Sets a new statusbar message.
    void MainWindow::statusBarMessage(const QString& newPos)
    {
        m_statusLabel->setText(newPos);
    }

} // namespace Caneda
