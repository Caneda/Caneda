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
#include "folderbrowser.h"
#include "item.h"
#include "library.h"
#include "canedaview.h"
#include "project.h"
#include "schematicscene.h"
#include "schematicstatehandler.h"
#include "schematicview.h"
#include "settings.h"
#include "xmlsymbolformat.h"

#include "dialogs/aboutdialog.h"
#include "dialogs/exportdialog.h"
#include "dialogs/projectfiledialog.h"
#include "dialogs/printdialog.h"
#include "dialogs/savedocumentsdialog.h"
#include "dialogs/settingsdialog.h"

#include "caneda-qterm/qtermwidget.h"

#include "caneda-tools/global.h"

#include "xmlutilities/transformers.h"
#include "xmlutilities/validators.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QUndoGroup>
#include <QUndoView>
#include <QVBoxLayout>
#include <QWhatsThis>

namespace Caneda
{

    /*!
     * \brief Construct and setup the mainwindow for caneda.
     */
    MainWindow::MainWindow(QWidget *w) : MainWindowBase(w)
    {
        titleText = QString("Caneda ") + (Caneda::version) + QString(" : %1[*]");

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

        connect(tabWidget(), SIGNAL(tabCloseRequested(int)), this, SLOT(slotFileClose(int)));
        connect(this, SIGNAL(currentWidgetChanged(QWidget*, QWidget*)), this,
                SLOT(slotCurrentChanged(QWidget*, QWidget*)));
        connect(this, SIGNAL(closedWidget(QWidget*)), this,
                SLOT(slotViewClosed(QWidget*)));

        SchematicView *view = new SchematicView(0, this);
        addView(view);
        m_undoGroup->setActiveStack(view->schematicScene()->undoStack());
        loadSettings();
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

    /*!
     * \brief Switches to \a fileName tab if it is opened else tries opening it
     * and then switches to that tab on success.
     */
    bool MainWindow::gotoPage(QString fileName, Caneda::Mode mode)
    {
        fileName = QDir::toNativeSeparators(fileName);
        QFileInfo info(fileName);

        //If we are opening a symbol, we open the corresponding schematic in symbol mode instead
        if(info.suffix() == "xsym") {
            fileName.replace(".xsym", ".xsch");
            info = QFileInfo(fileName);
            mode = Caneda::SymbolMode;
        }

        CanedaView *view = 0;
        int i = 0;
        while(i < tabWidget()->count()) {
            view = viewFromWidget(tabWidget()->widget(i));
            if(QDir::toNativeSeparators(view->fileName()) == fileName &&
                    view->toSchematicView()->schematicScene()->currentMode() == mode) {
                break;
            }
            view = 0;
            ++i;
        }

        if(view) {
            tabWidget()->setCurrentIndex(i);
            return true;
        }

        if(info.suffix() == "xsch") {
            view = new SchematicView(0, this);
            view->toSchematicView()->schematicScene()->setMode(mode);
        }  //TODO: create other views (text, simulation) here
        else {
            //Unrecognized file type
            return false;
        }

        if(!view->load(fileName)) {
            delete view;
            return false;
        }

        addView(view);
        return true;
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
        viewMenu->addAction(sidebarDockWidget->toggleViewAction());

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

        m_componentsSidebar->plugItem("Components", QPixmap(), "root");
        m_componentsSidebar->plugItems(paintingItems, QObject::tr("Paint Tools"));
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
        viewMenu->addAction(projectDockWidget->toggleViewAction());
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
        viewMenu->addAction(sidebarDockWidget->toggleViewAction());
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
        viewMenu->addAction(sidebarDockWidget->toggleViewAction());
    }

    /*!
     * \brief Returns an icon from current theme or a fallback default.
     */
    static QIcon icon(const QString& iconName)
    {
        static QString bitmapPath = Caneda::bitmapDirectory();
        return QIcon::fromTheme(iconName, QIcon(bitmapPath + iconName + ".png"));
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

    /*!
     * \brief Creates and intializes all the actions used.
     */
    void MainWindow::initActions()
    {
        using namespace Qt;
        Action *action = 0;
        ActionManager *am = ActionManager::instance();
        SchematicStateHandler *handler = SchematicStateHandler::instance();

        action = am->createAction("fileNew", icon("document-new"), tr("&New"));
        action->setShortcut(CTRL+Key_N);
        action->setStatusTip(tr("Creates a new document"));
        action->setWhatsThis(tr("New\n\nCreates a new schematic or data display document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileNew()));

        action = am->createAction("textNew", icon("text-plain"), tr("New &Text"));
        action->setShortcut(CTRL+SHIFT+Key_V);
        action->setStatusTip(tr("Creates a new text document"));
        action->setWhatsThis(tr("New Text\n\nCreates a new text document"));
        connect(action, SIGNAL(triggered()), SLOT(slotTextNew()));

        action = am->createAction("fileOpen", icon("document-open"), tr("&Open..."));
        action->setShortcut(CTRL+Key_O);
        action->setStatusTip(tr("Opens an existing document"));
        action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileOpen()));

        action = am->createAction("fileSave", icon("document-save"), tr("&Save"));
        action->setShortcut(CTRL+Key_S);
        action->setStatusTip(tr("Saves the current document"));
        action->setWhatsThis(tr("Save File\n\nSaves the current document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSaveCurrent()));

        action = am->createAction("fileSaveAs", icon("document-save-as"), tr("Save as..."));
        action->setShortcut(CTRL+SHIFT+Key_S);
        action->setStatusTip(tr("Saves the current document under a new filename"));
        action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAsCurrent()));

        action = am->createAction("fileSaveAll", icon("document-save-all"), tr("Save &All"));
        action->setShortcut(CTRL+Key_Plus);
        action->setStatusTip(tr("Saves all open documents"));
        action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAll()));

        action = am->createAction("fileClose", icon("document-close"), tr("&Close"));
        action->setShortcut(CTRL+Key_W);
        action->setStatusTip(tr("Closes the current document"));
        action->setWhatsThis(tr("Close File\n\nCloses the current document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileCloseCurrent()));

        action = am->createAction("filePrint", icon("document-print"), tr("&Print..."));
        action->setShortcut(CTRL+Key_P);
        action->setStatusTip(tr("Prints the current document"));
        action->setWhatsThis(tr("Print File\n\nPrints the current document"));
        connect(action, SIGNAL(triggered()), SLOT(slotFilePrint()));

        action = am->createAction("exportImage", icon("image-x-generic"), tr("&Export Image..."));
        action->setShortcut(CTRL+Key_E);
        action->setWhatsThis(tr("Export Image\n\n""Export current view to image file"));
        connect(action, SIGNAL(triggered()), SLOT(slotExportImage()));

        action = am->createAction("fileSettings", icon("document-properties"), tr("&Document Settings..."));
        action->setShortcut(CTRL+Key_Period);
        action->setWhatsThis(tr("Settings\n\nSets properties of the file"));
        connect(action, SIGNAL(triggered()), SLOT(slotFileSettings()));

        action = am->createAction("applSettings", icon("preferences-other"), tr("Application Settings..."));
        action->setShortcut(CTRL+Key_Comma);
        action->setWhatsThis(tr("Caneda Settings\n\nSets properties of the application"));
        connect(action, SIGNAL(triggered()), SLOT(slotApplSettings()));

        action = am->createAction("fileQuit", icon("application-exit"), tr("E&xit"));
        action->setShortcut(CTRL+Key_Q);
        action->setStatusTip(tr("Quits the application"));
        action->setWhatsThis(tr("Exit\n\nQuits the application"));
        connect(action, SIGNAL(triggered()), SLOT(close()));

        action = am->createAction("undo", icon("edit-undo"), tr("&Undo"));
        action->setShortcut(CTRL+Key_Z);
        action->setStatusTip(tr("Undoes the last command"));
        action->setWhatsThis(tr("Undo\n\nMakes the last action undone"));
        connect(action, SIGNAL(triggered()), m_undoGroup, SLOT(undo()));

        action = am->createAction("redo", icon("edit-redo"), tr("&Redo"));
        action->setShortcut(CTRL+SHIFT+Key_Z);
        action->setStatusTip(tr("Redoes the last command"));
        action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
        connect(action, SIGNAL(triggered()), m_undoGroup, SLOT(redo()));

        action = am->createAction("editCut", icon("edit-cut"), tr("Cu&t"));
        action->setShortcut(CTRL+Key_X);
        action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
        action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditCut()));

        action = am->createAction("editCopy", icon("edit-copy"), tr("&Copy"));
        action->setShortcut(CTRL+Key_C);
        action->setStatusTip(tr("Copies the selection into the clipboard"));
        action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditCopy()));

        action = am->createAction("editPaste", icon("edit-paste"), tr("&Paste"));
        action->setShortcut(CTRL+Key_V);
        action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
        action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditPaste()));

        action = am->createAction("selectAll", icon("select-rectangular"), tr("Select All"));
        action->setShortcut(CTRL+Key_A);
        action->setStatusTip(tr("Selects all elements"));
        action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
        connect(action, SIGNAL(triggered()), SLOT(slotSelectAll()));

        action = am->createAction("editFind", icon("edit-find"), tr("Find..."));
        action->setShortcut(CTRL+Key_F);
        action->setStatusTip(tr("Find a piece of text"));
        action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
        connect(action, SIGNAL(triggered()), SLOT(slotEditFind()));

        action = am->createAction("symEdit", icon("draw-freehand"), tr("&Edit Circuit Symbol/Schematic"));
        action->setShortcut(Key_F7);
        action->setStatusTip(tr("Switches between symbol and schematic edit"));
        action->setWhatsThis(tr("Edit Circuit Symbol/Schematic\n\nSwitches between symbol and schematic edit"));
        connect(action, SIGNAL(triggered()), SLOT(slotSymbolEdit()));

        action = am->createAction("intoH", icon("go-bottom"), tr("Go into Subcircuit"));
        action->setShortcut(CTRL+Key_I);
        action->setWhatsThis(tr("Go into Subcircuit\n\nGoes inside the selected subcircuit"));
        connect(action, SIGNAL(triggered()), SLOT(slotIntoHierarchy()));

        action = am->createAction("popH", icon("go-top"), tr("Pop out"));
        action->setShortcut(CTRL+SHIFT+Key_I);
        action->setStatusTip(tr("Pop outside subcircuit"));
        action->setWhatsThis(tr("Pop out\n\nGoes up one hierarchy level, i.e. leaves subcircuit"));
        connect(action, SIGNAL(triggered()), SLOT(slotPopHierarchy()));

        action = am->createAction("snapToGrid", tr("Snap to Grid"));
        action->setShortcut(CTRL+Key_U);
        action->setStatusTip(tr("Set grid snap"));
        action->setWhatsThis(tr("Snap to Grid\n\nSets snap to grid"));
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), SLOT(slotSnapToGrid(bool)));

        action = am->createAction("alignTop", icon("align-vertical-top"), tr("Align top"));
        action->setStatusTip(tr("Align top selected elements"));
        action->setWhatsThis(tr("Align top\n\nAlign selected elements to their upper edge"));
        connect(action, SIGNAL(triggered()), SLOT(slotAlignTop()));

        action = am->createAction("alignBottom", icon("align-vertical-bottom"), tr("Align bottom"));
        action->setStatusTip(tr("Align bottom selected elements"));
        action->setWhatsThis(tr("Align bottom\n\nAlign selected elements to their lower edge"));
        connect(action, SIGNAL(triggered()), SLOT(slotAlignBottom()));

        action = am->createAction("alignLeft", icon("align-horizontal-left"), tr("Align left"));
        action->setStatusTip(tr("Align left selected elements"));
        action->setWhatsThis(tr("Align left\n\nAlign selected elements to their left edge"));
        connect(action, SIGNAL(triggered()), SLOT(slotAlignLeft()));

        action = am->createAction("alignRight", icon("align-horizontal-right"), tr("Align right"));
        action->setStatusTip(tr("Align right selected elements"));
        action->setWhatsThis(tr("Align right\n\nAlign selected elements to their right edge"));
        connect(action, SIGNAL(triggered()), SLOT(slotAlignRight()));

        action = am->createAction("centerHor", icon("align-horizontal-center"), tr("Center horizontally"));
        action->setStatusTip(tr("Center horizontally selected elements"));
        action->setWhatsThis(tr("Center horizontally\n\nCenter horizontally selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(slotCenterHorizontal()));

        action = am->createAction("centerVert", icon("align-vertical-center"), tr("Center vertically"));
        action->setStatusTip(tr("Center vertically selected elements"));
        action->setWhatsThis(tr("Center vertically\n\nCenter vertically selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(slotCenterVertical()));

        action = am->createAction("distrHor", icon("distribute-horizontal-center"), tr("Distribute horizontally"));
        action->setStatusTip(tr("Distribute equally horizontally"));
        action->setWhatsThis(tr("Distribute horizontally\n\n""Distribute horizontally selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(slotDistribHoriz()));

        action = am->createAction("distrVert", icon("distribute-vertical-center"), tr("Distribute vertically"));
        action->setStatusTip(tr("Distribute equally vertically"));
        action->setWhatsThis(tr("Distribute vertically\n\n""Distribute vertically selected elements"));
        connect(action, SIGNAL(triggered()), SLOT(slotDistribVert()));

        action = am->createAction("projNew", icon("project-new"), tr("&New Project..."));
        action->setShortcut(CTRL+SHIFT+Key_N);
        action->setStatusTip(tr("Creates a new project"));
        action->setWhatsThis(tr("New Project\n\nCreates a new project"));
        connect(action, SIGNAL(triggered()), SLOT(slotNewProject()));

        action = am->createAction("projOpen", icon("document-open"), tr("&Open Project..."));
        action->setShortcut(CTRL+SHIFT+Key_O);
        action->setStatusTip(tr("Opens an existing project"));
        action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
        connect(action, SIGNAL(triggered()), SLOT(slotOpenProject()));

        action = am->createAction("addToProj", icon("document-new"), tr("&Add File to Project..."));
        action->setShortcut(CTRL+SHIFT+Key_A);
        action->setStatusTip(tr("Adds a file to current project"));
        action->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));
        connect(action, SIGNAL(triggered()), SLOT(slotAddToProject()));

        action = am->createAction("projDel", icon("document-close"), tr("&Remove from Project"));
        action->setShortcut(CTRL+SHIFT+Key_R);
        action->setStatusTip(tr("Removes a file from current project"));
        action->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));
        connect(action, SIGNAL(triggered()), SLOT(slotRemoveFromProject()));

        action = am->createAction("projClose", icon("dialog-close"), tr("&Close Project"));
        action->setShortcut(CTRL+SHIFT+Key_W);
        action->setStatusTip(tr("Closes the current project"));
        action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
        connect(action, SIGNAL(triggered()), SLOT(slotCloseProject()));

        action = am->createAction("backupAndHistory", icon("chronometer"), tr("&Backup and History..."));
        action->setShortcut(CTRL+SHIFT+Key_B);
        action->setStatusTip(tr("Opens backup and history dialog"));
        action->setWhatsThis(tr("Backup and History\n\nOpens backup and history dialog"));
        connect(action, SIGNAL(triggered()), SLOT(slotBackupAndHistory()));

        action = am->createAction("insEquation", icon("formula"), tr("Insert Equation"));
        action->setCheckable(true);
        action->setShortcut(Key_E);
        action->setWhatsThis(tr("Insert Equation\n\nInserts a user defined equation"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotInsertToolbarComponent(const QString&, bool)));

        action = am->createAction("insGround", icon("ground"), tr("Insert Ground"));
        action->setCheckable(true);
        action->setShortcut(Key_G);
        action->setWhatsThis(tr("Insert Ground\n\nInserts a ground symbol"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotInsertToolbarComponent(const QString&, bool)));

        action = am->createAction("insPort", icon("port"), tr("Insert Port"));
        action->setCheckable(true);
        action->setShortcut(Key_P);
        action->setWhatsThis(tr("Insert Port\n\nInserts a port symbol"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotInsertToolbarComponent(const QString&, bool)));

        action = am->createAction("insEntity", icon("code-context"), tr("VHDL entity"));
        action->setShortcut(SHIFT+Key_V);
        action->setStatusTip(tr("Inserts skeleton of VHDL entity"));
        action->setWhatsThis(tr("VHDL entity\n\nInserts the skeleton of a VHDL entity"));
        connect(action, SIGNAL(triggered()), SLOT(slotInsertEntity()));

        action = am->createAction("callFilter", icon("tools-wizard"), tr("Filter synthesis"));
        action->setShortcut(CTRL+Key_1);
        action->setStatusTip(tr("Starts CanedaFilter"));
        action->setWhatsThis(tr("Filter synthesis\n\nStarts CanedaFilter"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallFilter()));

        action = am->createAction("callLine", icon("tools-wizard"), tr("Line calculation"));
        action->setShortcut(CTRL+Key_2);
        action->setStatusTip(tr("Starts CanedaTrans"));
        action->setWhatsThis(tr("Line calculation\n\nStarts transmission line calculator"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallLine()));

        action = am->createAction("callMatch", icon("tools-wizard"), tr("Matching Circuit"));
        action->setShortcut(CTRL+Key_3);
        action->setStatusTip(tr("Creates Matching Circuit"));
        action->setWhatsThis(tr("Matching Circuit\n\nDialog for Creating Matching Circuit"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallMatch()));

        action = am->createAction("callAtt", icon("tools-wizard"), tr("Attenuator synthesis"));
        action->setShortcut(CTRL+Key_4);
        action->setStatusTip(tr("Starts CanedaAttenuator"));
        action->setWhatsThis(tr("Attenuator synthesis\n\nStarts attenuator calculation program"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallAtt()));

        action = am->createAction("callLib", icon("library"), tr("Component Library"));
        action->setShortcut(CTRL+Key_5);
        action->setStatusTip(tr("Starts CanedaLib"));
        action->setWhatsThis(tr("Component Library\n\nStarts component library program"));
        connect(action, SIGNAL(triggered()), SLOT(slotCallLibrary()));

        action = am->createAction("importData",  tr("&Import Data..."));
        action->setShortcut(CTRL+Key_6);
        action->setStatusTip(tr("Convert file to Caneda data file"));
        action->setWhatsThis(tr("Import Data\n\nConvert data file to Caneda data file"));
        connect(action, SIGNAL(triggered()), SLOT(slotImportData()));

        action = am->createAction("showConsole", icon("terminal"), tr("&Show Console..."));
        action->setShortcut(Key_F8);
        action->setStatusTip(tr("Show Console"));
        action->setWhatsThis(tr("Show Console\n\nOpen console terminal"));
        connect(action, SIGNAL(triggered()), SLOT(slotShowConsole()));

        action = am->createAction("simulate", icon("media-playback-start"), tr("Simulate"));
        action->setShortcut(Key_F5);
        action->setStatusTip(tr("Simulates the current schematic"));
        action->setWhatsThis(tr("Simulate\n\nSimulates the current schematic"));
        connect(action, SIGNAL(triggered()), SLOT(slotSimulate()));

        action = am->createAction("dpl_sch", icon("system-switch-user"), tr("View Data Display/Schematic"));
        action->setShortcut(Key_F4);
        action->setStatusTip(tr("Changes to data display or schematic page"));
        action->setWhatsThis(tr("View Data Display/Schematic\n\n")+tr("Changes to data display or schematic page"));
        connect(action, SIGNAL(triggered()), SLOT(slotToPage()));

        action = am->createAction("dcbias",  tr("Calculate DC bias"));
        action->setShortcut(Key_F3);
        action->setStatusTip(tr("Calculates DC bias and shows it"));
        action->setWhatsThis(tr("Calculate DC bias\n\nCalculates DC bias and shows it"));
        connect(action, SIGNAL(triggered()), SLOT(slotDCbias()));

        action = am->createAction("graph2csv",  tr("Export to &CSV..."));
        action->setShortcut(Key_F6);
        action->setStatusTip(tr("Convert graph data to CSV file"));
        action->setWhatsThis(tr("Export to CSV\n\nConvert graph data to CSV file"));
        connect(action, SIGNAL(triggered()), SLOT(slotExportGraphAsCsv()));

        action = am->createAction("showMsg", icon("document-preview"), tr("Show Last Messages"));
        action->setShortcut(Key_F9);
        action->setStatusTip(tr("Shows last simulation messages"));
        action->setWhatsThis(tr("Show Last Messages\n\nShows the messages of the last simulation"));
        connect(action, SIGNAL(triggered()), SLOT(slotShowLastMsg()));

        action = am->createAction("showNet", icon("document-preview"), tr("Show Last Netlist"));
        action->setShortcut(Key_F10);
        action->setStatusTip(tr("Shows last simulation netlist"));
        action->setWhatsThis(tr("Show Last Netlist\n\nShows the netlist of the last simulation"));
        connect(action, SIGNAL(triggered()), SLOT(slotShowLastNetlist()));

        action = am->createAction("magAll", icon("zoom-fit-best"), tr("View All"));
        action->setShortcut(Key_0);
        action->setStatusTip(tr("Show the whole page"));
        action->setWhatsThis(tr("View All\n\nShows the whole page content"));
        connect(action, SIGNAL(triggered()), SLOT(slotShowAll()));

        action = am->createAction("magOne", icon("zoom-original"), tr("View 1:1"));
        action->setShortcut(Key_1);
        action->setStatusTip(tr("Views without magnification"));
        action->setWhatsThis(tr("View 1:1\n\nShows the page content without magnification"));
        connect(action, SIGNAL(triggered()), SLOT(slotShowOne()));

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

        action = am->createAction("helpIndex", icon("help-contents"), tr("Help Index..."));
        action->setShortcut(Key_F1);
        action->setStatusTip(tr("Index of Caneda Help"));
        action->setWhatsThis(tr("Help Index\n\nIndex of intern Caneda help"));
        connect(action, SIGNAL(triggered()), SLOT(slotHelpIndex()));

        action = am->createAction("helpAboutApp", icon("caneda"), tr("&About Caneda..."));
        action->setWhatsThis(tr("About\n\nAbout the application"));
        connect(action, SIGNAL(triggered()), SLOT(slotHelpAbout()));

        action = am->createAction("helpAboutQt", icon("qt"), tr("About Qt..."));
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

        ActionManager *am = ActionManager::instance();
        action = am->createMouseAction("editDelete", SchematicScene::Deleting,
                icon("edit-delete"), tr("&Delete"));
        action->setShortcut(Key_Delete);
        action->setStatusTip(tr("Deletes the selected components"));
        action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("select", SchematicScene::Normal,
                icon("edit-select"), tr("Select"));
        action->setShortcut(Key_Escape);
        action->setStatusTip(tr("Activate select mode"));
        action->setWhatsThis(tr("Select\n\nActivates select mode"));
        action->setChecked(true);
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("editRotate", SchematicScene::Rotating,
                icon("object-rotate-left"), tr("Rotate"));
        action->setShortcut(CTRL+Key_R);
        action->setStatusTip(tr("Rotates the selected component by 90°"));
        action->setWhatsThis(tr("Rotate\n\nRotates the selected component by 90° counter-clockwise"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("editMirror", SchematicScene::MirroringX,
                icon("object-flip-vertical"), tr("Mirror about X Axis"));
        action->setShortcut(Key_V);
        action->setWhatsThis(tr("Mirror about X Axis\n\nMirrors the selected item about X Axis"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("editMirrorY", SchematicScene::MirroringY,
                icon("object-flip-horizontal"), tr("Mirror about Y Axis"));
        action->setShortcut(Key_H);
        action->setWhatsThis(tr("Mirror about Y Axis\n\nMirrors the selected item about Y Axis"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("insWire", SchematicScene::Wiring,
                icon("wire"), tr("Wire"));
        action->setShortcut(Key_W);
        action->setWhatsThis(tr("Wire\n\nInserts a wire"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("insLabel", SchematicScene::InsertingWireLabel,
                icon("nodename"), tr("Wire Label"));
        action->setShortcut(Key_L);
        action->setStatusTip(tr("Inserts a wire or pin label"));
        action->setWhatsThis(tr("Wire Label\n\nInserts a wire or pin label"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("editActivate", SchematicScene::ChangingActiveStatus,
                icon("deactiv"), tr("Deactivate/Activate"));
        action->setShortcut(Key_D);
        action->setStatusTip(tr("Deactivate/Activate selected components"));
        action->setWhatsThis(tr("Deactivate/Activate\n\nDeactivate/Activate the selected components"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("setMarker", SchematicScene::Marking,
                icon("marker"), tr("Set Marker on Graph"));
        action->setShortcut(Key_F2);
        action->setStatusTip(tr("Sets a marker on a diagram's graph"));
        action->setWhatsThis(tr("Set Marker\n\nSets a marker on a diagram's graph"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("magPlus", SchematicScene::ZoomingAtPoint,
                icon("zoom-in"), tr("Zoom in"));
        action->setShortcut(Key_Plus);
        action->setStatusTip(tr("Zooms into the current view"));
        action->setWhatsThis(tr("Zoom in\n\nZooms the current view"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("magMinus", SchematicScene::ZoomingOutAtPoint,
                icon("zoom-out"), tr("Zoom out"));
        action->setShortcut(Key_Minus);
        action->setStatusTip(tr("Zooms out the current view"));
        action->setWhatsThis(tr("Zoom out\n\nZooms out the current view"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("insertItem", SchematicScene::InsertingItems,
                tr("Insert item action"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));

        action = am->createMouseAction("paintingDraw", SchematicScene::PaintingDrawEvent,
                tr("Painting draw action"));
        connect(action, SIGNAL(toggled(const QString&, bool)), handler,
                SLOT(slotPerformToggleAction(const QString&, bool)));
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
        fileMenu->addAction(action("exportImage"));

        fileMenu->addSeparator();

        fileMenu->addAction(action("fileSettings"));
        fileMenu->addAction(action("applSettings"));

        fileMenu->addSeparator();

        fileMenu->addAction(action("fileQuit"));

        editMenu = menuBar()->addMenu(tr("&Edit"));

        editMenu->addAction(action("undo"));
        editMenu->addAction(action("redo"));

        editMenu->addSeparator();

        editMenu->addAction(action("editCut"));
        editMenu->addAction(action("editCopy"));
        editMenu->addAction(action("editPaste"));
        editMenu->addAction(action("editDelete"));

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

        viewMenu = menuBar()->addMenu(tr("&View"));

        viewMenu->addAction(action("magAll"));
        viewMenu->addAction(action("magOne"));
        viewMenu->addAction(action("magPlus"));
        viewMenu->addAction(action("magMinus"));

        viewMenu->addSeparator();

        viewMenu->addAction(action("viewToolBar"));
        viewMenu->addAction(action("viewStatusBar"));

        viewMenu->addSeparator();

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
        editToolbar->addAction(action("editDelete"));
        editToolbar->addAction(action("undo"));
        editToolbar->addAction(action("redo"));

        viewToolbar  = addToolBar(tr("View"));
        viewToolbar->setObjectName("viewToolbar");

        viewToolbar->addAction(action("magAll"));
        viewToolbar->addAction(action("magOne"));
        viewToolbar->addAction(action("magPlus"));
        viewToolbar->addAction(action("magMinus"));

        workToolbar  = addToolBar(tr("Work"));
        workToolbar->setObjectName("workToolbar");

        workToolbar->addAction(action("select"));
        workToolbar->addAction(action("editActivate"));
        workToolbar->addAction(action("editMirror"));
        workToolbar->addAction(action("editMirrorY"));
        workToolbar->addAction(action("editRotate"));

        workToolbar->addSeparator();

        workToolbar->addAction(action("insWire"));
        workToolbar->addAction(action("insLabel"));
        workToolbar->addAction(action("insEquation"));
        workToolbar->addAction(action("insGround"));
        workToolbar->addAction(action("insPort"));
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
        m_cursorLabel = new QLabel(QString(""), statusBarWidget);
        statusBarWidget->addPermanentWidget(m_cursorLabel);
        statusBarWidget->setVisible(action("viewStatusBar")->isChecked());
    }

    //! \brief Toggles the normal select action on.
    void MainWindow::setNormalAction()
    {
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        handler->slotSetNormalAction();
    }

    /*!
     * \brief Adds the view to the tabwidget.
     *
     * Adds the view to the tabwidget and also adds its undostack if it is
     * scematicview..
     * Also the added view is set as current tab in tabwidget.
     */
    void MainWindow::addView(CanedaView *view)
    {
        if(view->isSchematicView()) {
            SchematicView *schematicView = view->toSchematicView();
            SchematicScene *schema = schematicView->schematicScene();
            m_undoGroup->addStack(schema->undoStack());
            // Register here and not in SchematicView constructor because here we
            // can assume that SchematicView and its scene are completely constructed.
            SchematicStateHandler *handler = SchematicStateHandler::instance();
            handler->registerView(schematicView);

            addChildWidget(view->toWidget());
            tabWidget()->setCurrentWidget(view->toWidget());

            ActionManager *am = ActionManager::instance();
            Action *action = am->actionForName("snapToGrid");
            action->setChecked(schema->gridSnap());
        }
        else {
            addChildWidget(view->toWidget());
            tabWidget()->setCurrentWidget(view->toWidget());
        }
    }

    /*!
     * \brief Updates the current undostack and the tabs/window title text.
     *
     * This slot updates the current undostack of undogroup and also updates
     * the tab's as well as window's title text. Also necessary connections between
     * view and this main window are made.
     */
    void MainWindow::slotCurrentChanged(QWidget *current, QWidget *prev)
    {
        if (prev) {
            prev->disconnect(this);
        }

        CanedaView *view = viewFromWidget(current);
        if (view) {
            connect(view->toWidget(), SIGNAL(titleToBeUpdated()), SLOT(updateTitleTabText()));
            updateTitleTabText();
            SchematicView *currView = view->toSchematicView();
            if (currView) {
                connect(currView, SIGNAL(cursorPositionChanged(const QString&)),
                        SLOT(slotUpdateCursorPositionStatus(const QString&)));
                SchematicScene *scene = currView->schematicScene();
                if (scene) {
                    m_undoGroup->setActiveStack(scene->undoStack());

                    ActionManager *am = ActionManager::instance();
                    Action *action = am->actionForName("snapToGrid");
                    action->setChecked(scene->gridSnap());
                }
            }
        }
    }

    /*!
     * \brief Remove the undostack of widget from undogroup on view close.
     */
    void MainWindow::slotViewClosed(QWidget *widget)
    {
        SchematicView *view = qobject_cast<SchematicView*>(widget);
        if(view) {
            m_undoGroup->removeStack(view->schematicScene()->undoStack());
        }
    }

    /*!
     * \brief Sync the settings to configuration file and close window.
     */
    void MainWindow::closeEvent(QCloseEvent *e)
    {
        if(slotFileSaveAll()) {
            e->accept();

            saveSettings();
            emit(signalKillWidgets());
            MainWindowBase::closeEvent(e);
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
        if(!m_project->isValid()) {
            addView(new SchematicView(0, this));
        }
        else {
            slotAddToProject();
        }
    }

    //! \brief Creates a new text view.
    void MainWindow::slotTextNew()
    {
        setNormalAction();
        editFile(QString(""));
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

        if(fileName == 0 && !m_project->isValid()) {
            fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                    Settings::instance()->currentValue("nosave/canedaFilter").toString());
        }
        else if(fileName == 0 && m_project->isValid()) {
            ProjectFileDialog *p = new ProjectFileDialog(m_project->libraryFileName(), this);
            if(p->result() == QDialog::Accepted) {
                fileName = p->fileName();
            }
        }

        if(!fileName.isEmpty()) {
            if(QFileInfo(fileName).suffix() == "xpro") {
                slotOpenProject(fileName);
            }
            else {
                bool isLoaded = gotoPage(fileName);
                if(!isLoaded) {
                    QMessageBox::critical(0, tr("File load error"),
                            tr("Cannot open file %1").arg(fileName));
                }
            }
        }
    }

    /*!
     * \brief Saves the file corresponding to index tab.
     */
    void MainWindow::slotFileSave(int index)
    {
        CanedaView* v = viewFromWidget(tabWidget()->widget(index));
        if(!v) {
            return;
        }
        if(v->fileName().isEmpty()) {
            slotFileSaveAs(index);
        }
        else {
            if(!v->save()) {
                QMessageBox::critical(this, tr("File save error"),
                        tr("Cannot save file %1").arg(v->fileName()));
            }
        }
    }

    /*!
     * \brief Saves the file corresponding to current tab.
     */
    void MainWindow::slotFileSaveCurrent()
    {
        slotFileSave(tabWidget()->currentIndex());
    }

    /*!
     * \brief Pops up dialog to select new filename and saves the file corresponding
     * to index tab.
     */
    void MainWindow::slotFileSaveAs(int index)
    {
        CanedaView* v = viewFromWidget(tabWidget()->widget(index));
        if(!v) {
            return;
        }
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                Settings::instance()->currentValue("nosave/canedaFilter").toString());
        if(fileName.isEmpty()) {
            return;
        }
        QString oldFileName = v->fileName();
        v->setFileName(fileName);
        if(!v->save()) {
            QMessageBox::critical(this, tr("File save error"),
                    tr("Cannot save file %1").arg(v->fileName()));
            v->setFileName(oldFileName);
            return;
        }

        v = 0;
        int i = 0;
        while(i < tabWidget()->count()) {
            v = viewFromWidget(tabWidget()->widget(i));
            if(QDir::toNativeSeparators(v->fileName()) == fileName && i != index) {
                slotFileClose(i);
            }
            v = 0;
            ++i;
        }
    }

    /*!
     * \brief Pops up dialog to select new filename and saves the file corresponding
     * to current tab.
     */
    void MainWindow::slotFileSaveAsCurrent()
    {
        slotFileSaveAs(tabWidget()->currentIndex());
    }

    /*!
     * \brief Opens a dialog giving the user options to save all modified files.
     *
     * @return True on success, false on cancel
     */
    bool MainWindow::slotFileSaveAll()
    {
        QSet<SchematicScene*> processedScenes;
        QSet<QPair<CanedaView*, int> > modifiedViews;

        for (int i = 0; i < tabWidget()->count(); ++i) {
            CanedaView *view = viewFromWidget(tabWidget()->widget(i));
            if (!view || view->isModified() == false) {
                continue;
            }

            SchematicScene *scene = view->isSchematicView() ?
                view->toSchematicView()->schematicScene() : 0;

            if (scene) {
                if (!processedScenes.contains(scene)) {
                    processedScenes << scene;
                    modifiedViews << qMakePair(view, i);
                }
            }
            else {
                modifiedViews << qMakePair(view, i);
            }
        }

        if (!modifiedViews.isEmpty()) {
            QPointer<SaveDocumentsDialog> dialog(new SaveDocumentsDialog(modifiedViews, this));
            dialog->exec();

            int result = dialog->result();

            if (result == SaveDocumentsDialog::DoNotSave) {
                return true;
            }
            else if (result == SaveDocumentsDialog::Abort) {
                return false;
            }
            else {
                QSet<QPair<CanedaView*, QString> > newFilePaths = dialog->newFilePaths();
                QSet<QPair<CanedaView*, QString> >::iterator it;

                bool failedInBetween = false;
                for (it = newFilePaths.begin(); it != newFilePaths.end(); ++it) {
                    CanedaView *view = it->first;
                    const QString newFileName = it->second;
                    QString oldFileName = view->fileName();

                    view->setFileName(newFileName);
                    if (!view->save()) {
                        failedInBetween = true;
                        view->setFileName(oldFileName);
                    }
                }

                if (failedInBetween) {
                    QMessageBox::critical(0, tr("File save error"),
                            tr("Could not save some files"));
                    return false;
                }
                else {
                    return true;
                }
            }
        }

        return true;
    }

    /*!
     * \brief Closes the selected tab.
     *
     * Before closing it prompts user whether to save or not if the document is
     * modified and takes necessary actions.
     */
    void MainWindow::slotFileClose(int index)
    {
        if(tabWidget()->count() > 0){
            CanedaView *view = viewFromWidget(tabWidget()->widget(index));
            bool saveAttempted = false;
            if(view->isModified()) {
                QMessageBox::StandardButton res =
                    QMessageBox::warning(0, tr("Closing caneda document"),
                            tr("The document contains unsaved changes!\n"
                                "Do you want to save the changes ?"),
                            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                if(res == QMessageBox::Save) {
                    saveAttempted = true;
                    slotFileSave(index);
                }
                else if(res == QMessageBox::Cancel) {
                    return;
                }
            }
            if (!saveAttempted || view->isModified() == false) {
                closeTab(index);
            }
        }
    }

    /*!
     * \brief Closes the current tab.
     */
    void MainWindow::slotFileCloseCurrent()
    {
        slotFileClose(tabWidget()->currentIndex());
    }

    /*!
     * \brief Opens the current schematics' symbol for editing
     */
    void MainWindow::slotSymbolEdit()
    {
        CanedaView *currentView = viewFromWidget(tabWidget()->currentWidget());
        if(!currentView) {
            return;
        }

        if(!currentView->fileName().isEmpty()) {
            QString fileName = currentView->fileName();

            if(currentView->toSchematicView()->schematicScene()->currentMode() == Caneda::SchematicMode) {
                //First, we try to open the corresponding symbol file
                bool isLoaded = gotoPage(fileName, Caneda::SymbolMode);

                //If it's a new symbol, we create it
                if(!isLoaded){
                    addView(new SchematicView(0, this));

                    CanedaView *v = viewFromWidget(tabWidget()->currentWidget());
                    SchematicScene *sc = v->toSchematicView()->schematicScene();
                    sc->setMode(Caneda::SymbolMode);

                    v->setFileName(fileName);
                }
            }
            else if(currentView->toSchematicView()->schematicScene()->currentMode() == Caneda::SymbolMode) {
                gotoPage(fileName, Caneda::SchematicMode);
            }
        }
    }

    void MainWindow::slotFilePrint()
    {
        setNormalAction();

        if(tabWidget()->count() > 0){
            CanedaView *view = viewFromWidget(tabWidget()->currentWidget());
            SchematicScene *scene = view->toSchematicView()->schematicScene();

            PrintDialog *p = new PrintDialog(scene, this);
        }
    }

    void MainWindow::slotExportImage()
    {
        setNormalAction();

        QList<SchematicScene *> schemasToExport;

        int i = 0;
        CanedaView *view = 0;
        while(i < tabWidget()->count()) {
            view = viewFromWidget(tabWidget()->widget(i));
            SchematicScene *scene = view->toSchematicView()->schematicScene();
            schemasToExport << scene;

            view = 0;
            ++i;
        }

        ExportDialog *expDial = new ExportDialog(schemasToExport, this);
        expDial->exec();
    }

    void MainWindow::slotApplSettings()
    {
        setNormalAction();

        QList<SettingsPage *> wantedPages;
        SettingsPage *page = new GeneralConfigurationPage(this);
        wantedPages << page;
        page = new VhdlConfigurationPage(this);
        wantedPages << page;
        page = new SimulationConfigurationPage(this);
        wantedPages << page;

        SettingsDialog *d = new SettingsDialog(wantedPages, "Configure Caneda", this);
        d->exec();
    }

    void MainWindow::slotFileSettings()
    {
        setNormalAction();

        if(tabWidget()->count() > 0){
            CanedaView *view = viewFromWidget(tabWidget()->currentWidget());
            SchematicScene *scene = view->toSchematicView()->schematicScene();

            QList<SettingsPage *> wantedPages;
            SettingsPage *page = new DocumentConfigurationPage(scene, this);
            wantedPages << page;
            page = new SimulationConfigurationPage(this);
            wantedPages << page;

            SettingsDialog *d = new SettingsDialog(wantedPages, "Configure Document", this);
            d->exec();
        }
    }

    void MainWindow::slotEditCut()
    {
        setNormalAction();
        CanedaView* v = viewFromWidget(tabWidget()->currentWidget());
        if(!v) {
            return;
        }
        v->cut();
    }

    void MainWindow::slotEditCopy()
    {
        setNormalAction();
        CanedaView* v = viewFromWidget(tabWidget()->currentWidget());
        if(!v) {
            return;
        }
        v->copy();
    }

    void MainWindow::slotEditPaste()
    {
        setNormalAction();
        CanedaView* v = viewFromWidget(tabWidget()->currentWidget());
        if(!v) {
            return;
        }
        v->paste();
    }

    void MainWindow::slotSelectAll()
    {
        setNormalAction();
        CanedaView* v = viewFromWidget(tabWidget()->currentWidget());
        foreach(QGraphicsItem* item, v->toSchematicView()->schematicScene()->items()) {
            item->setSelected(true);
        }
    }

    void MainWindow::slotEditFind()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotIntoHierarchy()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotPopHierarchy()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    //! \brief Align elements to grid
    void MainWindow::slotSnapToGrid(bool snap)
    {
        SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
        if(view) {
            view->schematicScene()->setSnapToGrid(snap);
        }
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void MainWindow::alignElements(Qt::Alignment alignment)
    {
        SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
        if(!view) {
            return;
        }

        if(!view->schematicScene()->alignElements(alignment)) {
            QMessageBox::information(this, tr("Info"),
                    tr("At least two elements must be selected !"));
        }
    }

    //! \brief Align elements in a row correponding to top most elements coords.
    void MainWindow::slotAlignTop()
    {
        alignElements(Qt::AlignTop);
    }

    //! \brief Align elements in a row correponding to bottom most elements coords.
    void MainWindow::slotAlignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    //! \brief Align elements in a column correponding to left most elements coords.
    void MainWindow::slotAlignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    /*!
     * \brief Align elements in a column correponding to right most elements
     * coords.
     */
    void MainWindow::slotAlignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void MainWindow::slotDistribHoriz()
    {
        SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
        if(!view) {
            return;
        }

        if(!view->schematicScene()->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(this, tr("Info"),
                    tr("At least two elements must be selected !"));
        }
    }

    void MainWindow::slotDistribVert()
    {
        SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
        if(!view) {
            return;
        }

        if(!view->schematicScene()->distributeElements(Qt::Vertical)) {
            QMessageBox::information(this, tr("Info"),
                    tr("At least two elements must be selected !"));
        }
    }

    void MainWindow::slotCenterHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void MainWindow::slotCenterVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    void MainWindow::slotNewProject()
    {
        setNormalAction();

        if(slotFileSaveAll()) {
            closeAllTabs();
            projectDockWidget->setVisible(true);
            projectDockWidget->raise();
            m_project->slotNewProject();
        }
    }

    void MainWindow::slotOpenProject(QString fileName)
    {
        setNormalAction();

        if(slotFileSaveAll()) {
            closeAllTabs();
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
            closeAllTabs();
        }
    }

    void MainWindow::slotBackupAndHistory()
    {
        setNormalAction();
        m_project->slotBackupAndHistory();
    }

    void MainWindow::slotInsertEntity()
    {
        setNormalAction();
    }

    void MainWindow::slotCallFilter()
    {
        setNormalAction();

        QProcess *CanedaFilter = new QProcess(this);
        CanedaFilter->start(QString(Caneda::binaryDir + "canedafilter"));

        connect(CanedaFilter, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

        // Kill before Caneda ends
        connect(this, SIGNAL(signalKillWidgets()), CanedaFilter, SLOT(kill()));
    }

    void MainWindow::slotCallLine()
    {
        setNormalAction();

        QProcess *CanedaLine = new QProcess(this);
        CanedaLine->start(QString(Caneda::binaryDir + "canedatrans"));

        connect(CanedaLine, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

        // Kill before Caneda ends
        connect(this, SIGNAL(signalKillWidgets()), CanedaLine, SLOT(kill()));
    }

    void MainWindow::slotCallMatch()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotCallAtt()
    {
        setNormalAction();

        QProcess *CanedaAtt = new QProcess(this);
        CanedaAtt->start(QString(Caneda::binaryDir + "canedaattenuator"));

        connect(CanedaAtt, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

        // Kill before Caneda ends
        connect(this, SIGNAL(signalKillWidgets()), CanedaAtt, SLOT(kill()));
    }

    void MainWindow::slotCallLibrary()
    {
        setNormalAction();

        QProcess *CanedaLib = new QProcess(this);
        CanedaLib->start(QString(Caneda::binaryDir + "canedalib"));

        connect(CanedaLib, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

        // Kill before Caneda ends
        connect(this, SIGNAL(signalKillWidgets()), CanedaLib, SLOT(kill()));
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

    void MainWindow::slotSimulate()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotToPage()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotDCbias()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotExportGraphAsCsv()
    {
        setNormalAction();
        //TODO: implement this or rather port directly
    }

    void MainWindow::slotShowLastMsg()
    {
        setNormalAction();
        editFile(Caneda::pathForCanedaFile("log.txt"));
    }

    void MainWindow::slotShowLastNetlist()
    {
        setNormalAction();
        editFile(Caneda::pathForCanedaFile("netlist.txt"));
    }

    void MainWindow::slotShowAll()
    {
        setNormalAction();
        CanedaView *view = viewFromWidget(tabWidget()->currentWidget());
        if(view) {
            view->showAll();
        }
    }

    void MainWindow::slotShowOne()
    {
        setNormalAction();
        CanedaView *view = viewFromWidget(tabWidget()->currentWidget());
        if(view) {
            view->showNoZoom();
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

    //! \brief Opens the editor given a filename
    void MainWindow::editFile(const QString& File)
    {
        QStringList arguments;
        if(!File.isEmpty()) {
            arguments << File;
        }
        QString textEditor = Settings::instance()->currentValue("gui/textEditor").toString();
        QProcess *CanedaEditor = new QProcess(this);
        CanedaEditor->start(textEditor, arguments);

        connect(CanedaEditor, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

        // Kill editor before Caneda ends
        connect(this, SIGNAL(signalKillWidgets()), CanedaEditor, SLOT(kill()));
    }

    //! \brief Opens the help browser
    void MainWindow::showHTML(const QString& Page)
    {
        QStringList arguments;
        if(!Page.isEmpty()) {
            arguments << Page;
        }
        QProcess *CanedaHelp = new QProcess(this);
        CanedaHelp->start(QString(Caneda::binaryDir + "canedahelp"), arguments);

        connect(CanedaHelp, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

        // Kill before Caneda ends
        connect(this, SIGNAL(signalKillWidgets()), CanedaHelp, SLOT(kill()));
    }

    void MainWindow::slotHelpIndex()
    {
        setNormalAction();
        showHTML("index.html");
    }

    void MainWindow::slotProccessError(QProcess::ProcessError error)
    {
        switch(error) {
            case QProcess::FailedToStart :
                {
                    QMessageBox::critical(0, tr("Process error"),
                            tr("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program."));
                    return;
                }
            case QProcess::Crashed :
                {
                    QMessageBox::critical(0, tr("Process error"),
                            tr("The process crashed while running."));
                    return;
                }
            case QProcess::UnknownError :
                {
                    QMessageBox::critical(0, tr("Process error"),
                            tr("An unknown error occurred."));
                    return;
                }
            default:
                {
                    QMessageBox::critical(0, tr("Process error"),
                            tr("An unknown error occurred."));
                    return;
                }
        }
    }

    void MainWindow::slotHelpAbout()
    {
        setNormalAction();
        AboutDialog *about = new AboutDialog();
        about->exec();
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

    void MainWindow::setTabTitle(const QString& title)
    {
        SchematicView *view = qobject_cast<SchematicView*>(sender());
        if(!view || title.isEmpty()) {
            return;
        }
        int index = tabWidget()->indexOf(view);
        if(index != -1) {
            tabWidget()->setTabText(index,title);
        }
    }

    CanedaView* MainWindow::viewFromWidget(QWidget *widget)
    {
        SchematicView *v = qobject_cast<SchematicView*>(widget);
        if(v) {
            return static_cast<CanedaView*>(v);
        }
        qDebug("MainWindow::viewFromWidget() : Couldn't identify view type.");
        return 0;
    }

    void MainWindow::setDocumentTitle(const QString& filename)
    {
        setWindowTitle(titleText.arg(filename));
    }

    void MainWindow::updateTitleTabText()
    {
        CanedaView *view = viewFromWidget(currentWidget());
        if(view) {
            int index = tabWidget()->indexOf(currentWidget());
            tabWidget()->setTabText(index, view->tabText());
            QIcon icon = view->isModified() ? view->modifiedTabIcon() :
                view->unmodifiedTabIcon();
            tabWidget()->setTabIcon(index, icon);

            setDocumentTitle(view->tabText());
            setWindowModified(view->isModified());
        }
    }

    void MainWindow::slotUpdateAllViews()
    {
        for (int i = 0; i < tabWidget()->count(); ++i) {
            SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->widget(i));
            if (view) {
                if (view->scene()) {
                    view->scene()->invalidate();
                }
                view->resetCachedContent();
            }
        }
    }

    void MainWindow::slotUpdateCursorPositionStatus(const QString& newPos)
    {
        m_cursorLabel->setText(newPos);
    }

} // namespace Caneda
