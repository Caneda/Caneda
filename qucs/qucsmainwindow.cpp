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

#include "qucsmainwindow.h"

#include "actionmanager.h"
#include "componentssidebar.h"
#include "folderbrowser.h"
#include "item.h"
#include "library.h"
#include "qucsview.h"
#include "project.h"
#include "schematicscene.h"
#include "schematicstatehandler.h"
#include "schematicview.h"
#include "settings.h"
#include "xmlsymbolformat.h"

#include "dialogs/aboutqucs.h"
#include "dialogs/exportdialog.h"
#include "dialogs/printdialog.h"
#include "dialogs/savedocumentsdialog.h"
#include "dialogs/settingsdialog.h"

#include "qucs-qterm/qtermwidget.h"

#include "qucs-tools/global.h"

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

/*!
 * \brief Construct and setup the mainwindow for qucs.
 */
QucsMainWindow::QucsMainWindow(QWidget *w) : MainWindowBase(w)
{
    titleText = QString("Qucs ") + (Qucs::version) + QString(" : %1[*]");

    setObjectName("QucsMainWindow"); //for debugging purpose
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
QucsMainWindow::~QucsMainWindow()
{
}

QucsMainWindow* QucsMainWindow::instance()
{
    static QucsMainWindow* instance = 0;
    if (!instance) {
        instance = new QucsMainWindow();
    }
    return instance;
}

/*!
 * \brief Switches to \a fileName tab if it is opened else tries opening it
 * and then switches to that tab on success.
 */
bool QucsMainWindow::gotoPage(QString fileName, Qucs::Mode mode)
{
    fileName = QDir::toNativeSeparators(fileName);
    QFileInfo info(fileName);

    //If we are opening a symbol, we open the corresponding schematic in symbol mode instead
    if(info.suffix() == "xsym") {
        fileName.replace(".xsym", ".xsch");
        info = QFileInfo(fileName);
        mode = Qucs::SymbolMode;
    }

    QucsView *view = 0;
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
void QucsMainWindow::setupSidebar()
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
            QPixmap(Qucs::bitmapDirectory() + "arrow.svg"));
    paintingItems << qMakePair(QObject::tr("Ellipse"),
            QPixmap(Qucs::bitmapDirectory() + "ellipse.svg"));
    paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
            QPixmap(Qucs::bitmapDirectory() + "ellipsearc.svg"));
    paintingItems << qMakePair(QObject::tr("Line"),
            QPixmap(Qucs::bitmapDirectory() + "line.svg"));
    paintingItems << qMakePair(QObject::tr("Rectangle"),
            QPixmap(Qucs::bitmapDirectory() + "rectangle.svg"));
    paintingItems << qMakePair(QObject::tr("Text"),
            QPixmap(Qucs::bitmapDirectory() + "text.svg"));

    m_componentsSidebar->plugItem("Components", QPixmap(), "root");
    m_componentsSidebar->plugItems(paintingItems, QObject::tr("Paint Tools"));
}

/*!
 * \brief This initializes the projects sidebar.
 */
void QucsMainWindow::setupProjectsSidebar()
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

void QucsMainWindow::createUndoView()
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

void QucsMainWindow::createFolderView()
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

static QIcon icon(const QString& filename)
{
    static QString bitmapPath = Qucs::bitmapDirectory();
    return QIcon(bitmapPath + filename);
}

Action* QucsMainWindow::action(const QString& name)
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
void QucsMainWindow::initActions()
{
    using namespace Qt;
    Action *action = 0;
    ActionManager *am = ActionManager::instance();
    SchematicStateHandler *handler = SchematicStateHandler::instance();

    action = am->createAction("fileNew", icon("filenew.png"), tr("&New"));
    action->setShortcut(CTRL+Key_N);
    action->setStatusTip(tr("Creates a new document"));
    action->setWhatsThis(tr("New\n\nCreates a new schematic or data display document"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileNew()));

    action = am->createAction("textNew", icon("textnew.png"), tr("New &Text"));
    action->setShortcut(CTRL+SHIFT+Key_V);
    action->setStatusTip(tr("Creates a new text document"));
    action->setWhatsThis(tr("New Text\n\nCreates a new text document"));
    connect(action, SIGNAL(triggered()), SLOT(slotTextNew()));

    action = am->createAction("fileOpen", icon("fileopen.png"), tr("&Open..."));
    action->setShortcut(CTRL+Key_O);
    action->setStatusTip(tr("Opens an existing document"));
    action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileOpen()));

    action = am->createAction("fileSave", icon("filesave.png"), tr("&Save"));
    action->setShortcut(CTRL+Key_S);
    action->setStatusTip(tr("Saves the current document"));
    action->setWhatsThis(tr("Save File\n\nSaves the current document"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileSaveCurrent()));

    action = am->createAction("fileSaveAs", icon("filesaveas.png"), tr("Save as..."));
    action->setShortcut(CTRL+SHIFT+Key_S);
    action->setStatusTip(tr("Saves the current document under a new filename"));
    action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAsCurrent()));

    action = am->createAction("fileSaveAll", icon("filesaveall.png"), tr("Save &All"));
    action->setShortcut(CTRL+Key_Plus);
    action->setStatusTip(tr("Saves all open documents"));
    action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAll()));

    action = am->createAction("fileClose", icon("fileclose.png"), tr("&Close"));
    action->setShortcut(CTRL+Key_W);
    action->setStatusTip(tr("Closes the current document"));
    action->setWhatsThis(tr("Close File\n\nCloses the current document"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileCloseCurrent()));

    action = am->createAction("filePrint", icon("fileprint.png"), tr("&Print..."));
    action->setShortcut(CTRL+Key_P);
    action->setStatusTip(tr("Prints the current document"));
    action->setWhatsThis(tr("Print File\n\nPrints the current document"));
    connect(action, SIGNAL(triggered()), SLOT(slotFilePrint()));

    action = am->createAction("exportImage", icon("export-image.png"), tr("&Export Image..."));
    action->setShortcut(CTRL+Key_E);
    action->setWhatsThis(tr("Export Image\n\n""Export current view to image file"));
    connect(action, SIGNAL(triggered()), SLOT(slotExportImage()));

    action = am->createAction("fileSettings", icon("document-edit.png"), tr("&Document Settings..."));
    action->setShortcut(CTRL+Key_Period);
    action->setWhatsThis(tr("Settings\n\nSets properties of the file"));
    connect(action, SIGNAL(triggered()), SLOT(slotFileSettings()));

    action = am->createAction("applSettings", icon("configure.png"), tr("Application Settings..."));
    action->setShortcut(CTRL+Key_Comma);
    action->setWhatsThis(tr("Qucs Settings\n\nSets properties of the application"));
    connect(action, SIGNAL(triggered()), SLOT(slotApplSettings()));

    action = am->createAction("fileQuit", icon("application-exit.png"), tr("E&xit"));
    action->setShortcut(CTRL+Key_Q);
    action->setStatusTip(tr("Quits the application"));
    action->setWhatsThis(tr("Exit\n\nQuits the application"));
    connect(action, SIGNAL(triggered()), SLOT(close()));

    action = am->createAction("undo", icon("undo.png"), tr("&Undo"));
    action->setShortcut(CTRL+Key_Z);
    action->setStatusTip(tr("Undoes the last command"));
    action->setWhatsThis(tr("Undo\n\nMakes the last action undone"));
    connect(action, SIGNAL(triggered()), m_undoGroup, SLOT(undo()));

    action = am->createAction("redo", icon("redo.png"), tr("&Redo"));
    action->setShortcut(CTRL+SHIFT+Key_Z);
    action->setStatusTip(tr("Redoes the last command"));
    action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
    connect(action, SIGNAL(triggered()), m_undoGroup, SLOT(redo()));

    action = am->createAction("editCut", icon("editcut.png"), tr("Cu&t"));
    action->setShortcut(CTRL+Key_X);
    action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
    action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
    connect(action, SIGNAL(triggered()), SLOT(slotEditCut()));

    action = am->createAction("editCopy", icon("editcopy.png"), tr("&Copy"));
    action->setShortcut(CTRL+Key_C);
    action->setStatusTip(tr("Copies the selection into the clipboard"));
    action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
    connect(action, SIGNAL(triggered()), SLOT(slotEditCopy()));

    action = am->createAction("editPaste", icon("editpaste.png"), tr("&Paste"));
    action->setShortcut(CTRL+Key_V);
    action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
    action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
    connect(action, SIGNAL(triggered()), SLOT(slotEditPaste()));

    action = am->createAction("selectAll", icon("select-all.png"), tr("Select All"));
    action->setShortcut(CTRL+Key_A);
    action->setStatusTip(tr("Selects all elements"));
    action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
    connect(action, SIGNAL(triggered()), SLOT(slotSelectAll()));

    action = am->createAction("selectMarker",  tr("Select Markers"));
    action->setShortcut(CTRL+SHIFT+Key_M);
    action->setStatusTip(tr("Selects all markers"));
    action->setWhatsThis(tr("Select Markers\n\nSelects all diagram markers of the document"));
    connect(action, SIGNAL(triggered()), SLOT(slotSelectMarker()));

    action = am->createAction("editFind", icon("editfind.png"), tr("Find..."));
    action->setShortcut(CTRL+Key_F);
    action->setStatusTip(tr("Find a piece of text"));
    action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
    connect(action, SIGNAL(triggered()), SLOT(slotEditFind()));

    action = am->createAction("symEdit", icon("symbol-edit.png"), tr("&Edit Circuit Symbol/Schematic"));
    action->setShortcut(Key_F7);
    action->setStatusTip(tr("Switches between symbol and schematic edit"));
    action->setWhatsThis(tr("Edit Circuit Symbol/Schematic\n\nSwitches between symbol and schematic edit"));
    connect(action, SIGNAL(triggered()), SLOT(slotSymbolEdit()));

    action = am->createAction("intoH", icon("bottom.png"), tr("Go into Subcircuit"));
    action->setShortcut(CTRL+Key_I);
    action->setWhatsThis(tr("Go into Subcircuit\n\nGoes inside the selected subcircuit"));
    connect(action, SIGNAL(triggered()), SLOT(slotIntoHierarchy()));

    action = am->createAction("popH", icon("top.png"), tr("Pop out"));
    action->setShortcut(CTRL+SHIFT+Key_I);
    action->setStatusTip(tr("Pop outside subcircuit"));
    action->setWhatsThis(tr("Pop out\n\nGoes up one hierarchy level, i.e. leaves subcircuit"));
    connect(action, SIGNAL(triggered()), SLOT(slotPopHierarchy()));

    action = am->createAction("alignTop", icon("align-vertical-top.png"), tr("Align top"));
    action->setStatusTip(tr("Align top selected elements"));
    action->setWhatsThis(tr("Align top\n\nAlign selected elements to their upper edge"));
    connect(action, SIGNAL(triggered()), SLOT(slotAlignTop()));

    action = am->createAction("alignBottom", icon("align-vertical-bottom.png"), tr("Align bottom"));
    action->setStatusTip(tr("Align bottom selected elements"));
    action->setWhatsThis(tr("Align bottom\n\nAlign selected elements to their lower edge"));
    connect(action, SIGNAL(triggered()), SLOT(slotAlignBottom()));

    action = am->createAction("alignLeft", icon("align-horizontal-left.png"), tr("Align left"));
    action->setStatusTip(tr("Align left selected elements"));
    action->setWhatsThis(tr("Align left\n\nAlign selected elements to their left edge"));
    connect(action, SIGNAL(triggered()), SLOT(slotAlignLeft()));

    action = am->createAction("alignRight", icon("align-horizontal-right.png"), tr("Align right"));
    action->setStatusTip(tr("Align right selected elements"));
    action->setWhatsThis(tr("Align right\n\nAlign selected elements to their right edge"));
    connect(action, SIGNAL(triggered()), SLOT(slotAlignRight()));

    action = am->createAction("centerHor", icon("align-horizontal-center.png"), tr("Center horizontally"));
    action->setStatusTip(tr("Center horizontally selected elements"));
    action->setWhatsThis(tr("Center horizontally\n\nCenter horizontally selected elements"));
    connect(action, SIGNAL(triggered()), SLOT(slotCenterHorizontal()));

    action = am->createAction("centerVert", icon("align-vertical-center.png"), tr("Center vertically"));
    action->setStatusTip(tr("Center vertically selected elements"));
    action->setWhatsThis(tr("Center vertically\n\nCenter vertically selected elements"));
    connect(action, SIGNAL(triggered()), SLOT(slotCenterVertical()));

    action = am->createAction("distrHor",  tr("Distribute horizontally"));
    action->setStatusTip(tr("Distribute equally horizontally"));
    action->setWhatsThis(tr("Distribute horizontally\n\n""Distribute horizontally selected elements"));
    connect(action, SIGNAL(triggered()), SLOT(slotDistribHoriz()));

    action = am->createAction("distrVert",  tr("Distribute vertically"));
    action->setStatusTip(tr("Distribute equally vertically"));
    action->setWhatsThis(tr("Distribute vertically\n\n""Distribute vertically selected elements"));
    connect(action, SIGNAL(triggered()), SLOT(slotDistribVert()));

    action = am->createAction("projNew", icon("project-new.png"), tr("&New Project..."));
    action->setShortcut(CTRL+SHIFT+Key_N);
    action->setStatusTip(tr("Creates a new project"));
    action->setWhatsThis(tr("New Project\n\nCreates a new project"));
    connect(action, SIGNAL(triggered()), SLOT(slotNewProject()));

    action = am->createAction("projOpen", icon("fileopen.png"), tr("&Open Project..."));
    action->setShortcut(CTRL+SHIFT+Key_O);
    action->setStatusTip(tr("Opens an existing project"));
    action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
    connect(action, SIGNAL(triggered()), SLOT(slotOpenProject()));

    action = am->createAction("addToProj", icon("filenew.png"), tr("&Add File to Project..."));
    action->setShortcut(CTRL+SHIFT+Key_A);
    action->setStatusTip(tr("Adds a file to current project"));
    action->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));
    connect(action, SIGNAL(triggered()), SLOT(slotAddToProject()));

    action = am->createAction("projDel", icon("fileclose.png"), tr("&Remove from Project"));
    action->setShortcut(CTRL+SHIFT+Key_R);
    action->setStatusTip(tr("Removes a file from current project"));
    action->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));
    connect(action, SIGNAL(triggered()), SLOT(slotRemoveFromProject()));

    action = am->createAction("projClose", icon("project-close.png"), tr("&Close Project"));
    action->setShortcut(CTRL+SHIFT+Key_W);
    action->setStatusTip(tr("Closes the current project"));
    action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
    connect(action, SIGNAL(triggered()), SLOT(slotCloseProject()));

    action = am->createAction("insEquation", icon("equation.png"), tr("Insert Equation"));
    action->setCheckable(true);
    action->setShortcut(Key_E);
    action->setWhatsThis(tr("Insert Equation\n\nInserts a user defined equation"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotInsertToolbarComponent(const QString&, bool)));

    action = am->createAction("insGround", icon("ground.png"), tr("Insert Ground"));
    action->setCheckable(true);
    action->setShortcut(Key_G);
    action->setWhatsThis(tr("Insert Ground\n\nInserts a ground symbol"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotInsertToolbarComponent(const QString&, bool)));

    action = am->createAction("insPort", icon("port.png"), tr("Insert Port"));
    action->setCheckable(true);
    action->setShortcut(Key_P);
    action->setWhatsThis(tr("Insert Port\n\nInserts a port symbol"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotInsertToolbarComponent(const QString&, bool)));

    action = am->createAction("insEntity", icon("vhdl-code.png"), tr("VHDL entity"));
    action->setShortcut(SHIFT+Key_V);
    action->setStatusTip(tr("Inserts skeleton of VHDL entity"));
    action->setWhatsThis(tr("VHDL entity\n\nInserts the skeleton of a VHDL entity"));
    connect(action, SIGNAL(triggered()), SLOT(slotInsertEntity()));

    action = am->createAction("callFilter", icon("tools-wizard.png"), tr("Filter synthesis"));
    action->setShortcut(CTRL+Key_1);
    action->setStatusTip(tr("Starts QucsFilter"));
    action->setWhatsThis(tr("Filter synthesis\n\nStarts QucsFilter"));
    connect(action, SIGNAL(triggered()), SLOT(slotCallFilter()));

    action = am->createAction("callLine", icon("tools-wizard.png"), tr("Line calculation"));
    action->setShortcut(CTRL+Key_2);
    action->setStatusTip(tr("Starts QucsTrans"));
    action->setWhatsThis(tr("Line calculation\n\nStarts transmission line calculator"));
    connect(action, SIGNAL(triggered()), SLOT(slotCallLine()));

    action = am->createAction("callMatch", icon("tools-wizard.png"), tr("Matching Circuit"));
    action->setShortcut(CTRL+Key_3);
    action->setStatusTip(tr("Creates Matching Circuit"));
    action->setWhatsThis(tr("Matching Circuit\n\nDialog for Creating Matching Circuit"));
    connect(action, SIGNAL(triggered()), SLOT(slotCallMatch()));

    action = am->createAction("callAtt", icon("tools-wizard.png"), tr("Attenuator synthesis"));
    action->setShortcut(CTRL+Key_4);
    action->setStatusTip(tr("Starts QucsAttenuator"));
    action->setWhatsThis(tr("Attenuator synthesis\n\nStarts attenuator calculation program"));
    connect(action, SIGNAL(triggered()), SLOT(slotCallAtt()));

    action = am->createAction("callLib", icon("library.png"), tr("Component Library"));
    action->setShortcut(CTRL+Key_5);
    action->setStatusTip(tr("Starts QucsLib"));
    action->setWhatsThis(tr("Component Library\n\nStarts component library program"));
    connect(action, SIGNAL(triggered()), SLOT(slotCallLibrary()));

    action = am->createAction("importData",  tr("&Import Data..."));
    action->setShortcut(CTRL+Key_6);
    action->setStatusTip(tr("Convert file to Qucs data file"));
    action->setWhatsThis(tr("Import Data\n\nConvert data file to Qucs data file"));
    connect(action, SIGNAL(triggered()), SLOT(slotImportData()));

    action = am->createAction("showConsole", icon("console.png"), tr("&Show Console..."));
    action->setShortcut(Key_F8);
    action->setStatusTip(tr("Show Console"));
    action->setWhatsThis(tr("Show Console\n\nOpen console terminal"));
    connect(action, SIGNAL(triggered()), SLOT(slotShowConsole()));

    action = am->createAction("simulate", icon("start.png"), tr("Simulate"));
    action->setShortcut(Key_F5);
    action->setStatusTip(tr("Simulates the current schematic"));
    action->setWhatsThis(tr("Simulate\n\nSimulates the current schematic"));
    connect(action, SIGNAL(triggered()), SLOT(slotSimulate()));

    action = am->createAction("dpl_sch", icon("switch-view.png"), tr("View Data Display/Schematic"));
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

    action = am->createAction("showMsg", icon("document-preview.png"), tr("Show Last Messages"));
    action->setShortcut(Key_F9);
    action->setStatusTip(tr("Shows last simulation messages"));
    action->setWhatsThis(tr("Show Last Messages\n\nShows the messages of the last simulation"));
    connect(action, SIGNAL(triggered()), SLOT(slotShowLastMsg()));

    action = am->createAction("showNet", icon("document-preview.png"), tr("Show Last Netlist"));
    action->setShortcut(Key_F10);
    action->setStatusTip(tr("Shows last simulation netlist"));
    action->setWhatsThis(tr("Show Last Netlist\n\nShows the netlist of the last simulation"));
    connect(action, SIGNAL(triggered()), SLOT(slotShowLastNetlist()));

    action = am->createAction("magAll", icon("viewmagfit.png"), tr("View All"));
    action->setShortcut(Key_0);
    action->setStatusTip(tr("Show the whole page"));
    action->setWhatsThis(tr("View All\n\nShows the whole page content"));
    connect(action, SIGNAL(triggered()), SLOT(slotShowAll()));

    action = am->createAction("magOne", icon("viewmag1.png"), tr("View 1:1"));
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

    action = am->createAction("helpIndex", icon("help.png"), tr("Help Index..."));
    action->setShortcut(Key_F1);
    action->setStatusTip(tr("Index of Qucs Help"));
    action->setWhatsThis(tr("Help Index\n\nIndex of intern Qucs help"));
    connect(action, SIGNAL(triggered()), SLOT(slotHelpIndex()));

    action = am->createAction("helpAboutApp", icon("qucs.png"), tr("&About Qucs..."));
    action->setWhatsThis(tr("About\n\nAbout the application"));
    connect(action, SIGNAL(triggered()), SLOT(slotHelpAbout()));

    action = am->createAction("helpAboutQt", icon("qt.png"), tr("About Qt..."));
    action->setWhatsThis(tr("About Qt\n\nAbout Qt by Trolltech"));
    connect(action, SIGNAL(triggered()), SLOT(slotHelpAboutQt()));

    QAction *qAction = QWhatsThis::createAction(this);
    action = am->createAction("whatsThis", qAction->icon(), qAction->text());
    connect(action, SIGNAL(triggered()), qAction, SLOT(trigger()));
    connect(action, SIGNAL(hovered()), qAction, SLOT(hover()));

    initMouseActions();
}

void QucsMainWindow::initMouseActions()
{
    using namespace Qt;
    Action *action = 0;
    SchematicStateHandler *handler = SchematicStateHandler::instance();

    ActionManager *am = ActionManager::instance();
    action = am->createMouseAction("editDelete", SchematicScene::Deleting,
            icon("editdelete.png"), tr("&Delete"));
    action->setShortcut(Key_Delete);
    action->setStatusTip(tr("Deletes the selected components"));
    action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("select", SchematicScene::Normal,
            icon("pointer.png"), tr("Select"));
    action->setShortcut(Key_Escape);
    action->setStatusTip(tr("Activate select mode"));
    action->setWhatsThis(tr("Select\n\nActivates select mode"));
    action->setChecked(true);
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("editRotate", SchematicScene::Rotating,
            icon("rotate_ccw.png"), tr("Rotate"));
    action->setShortcut(CTRL+Key_R);
    action->setStatusTip(tr("Rotates the selected component by 90°"));
    action->setWhatsThis(tr("Rotate\n\nRotates the selected component by 90° counter-clockwise"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("editMirror", SchematicScene::MirroringX,
            icon("mirror.png"), tr("Mirror about X Axis"));
    action->setShortcut(Key_V);
    action->setWhatsThis(tr("Mirror about X Axis\n\nMirrors the selected item about X Axis"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("editMirrorY", SchematicScene::MirroringY,
            icon("mirrory.png"), tr("Mirror about Y Axis"));
    action->setShortcut(Key_H);
    action->setWhatsThis(tr("Mirror about Y Axis\n\nMirrors the selected item about Y Axis"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("onGrid", SchematicScene::SettingOnGrid,
            tr("Set on Grid"));
    action->setShortcut(CTRL+Key_U);
    action->setWhatsThis(tr("Set on Grid\n\nSets selected elements on grid"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("insWire", SchematicScene::Wiring,
            icon("wire.png"), tr("Wire"));
    action->setShortcut(Key_W);
    action->setWhatsThis(tr("Wire\n\nInserts a wire"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("insLabel", SchematicScene::InsertingWireLabel,
            icon("nodename.png"), tr("Wire Label"));
    action->setShortcut(Key_L);
    action->setStatusTip(tr("Inserts a wire or pin label"));
    action->setWhatsThis(tr("Wire Label\n\nInserts a wire or pin label"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("editActivate", SchematicScene::ChangingActiveStatus,
            icon("deactiv.png"), tr("Deactivate/Activate"));
    action->setShortcut(Key_D);
    action->setStatusTip(tr("Deactivate/Activate selected components"));
    action->setWhatsThis(tr("Deactivate/Activate\n\nDeactivate/Activate the selected components"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("setMarker", SchematicScene::Marking,
            icon("marker.png"), tr("Set Marker on Graph"));
    action->setShortcut(Key_F2);
    action->setStatusTip(tr("Sets a marker on a diagram's graph"));
    action->setWhatsThis(tr("Set Marker\n\nSets a marker on a diagram's graph"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("magPlus", SchematicScene::ZoomingAtPoint,
            icon("viewmag+.png"), tr("Zoom in"));
    action->setShortcut(Key_Plus);
    action->setStatusTip(tr("Zooms into the current view"));
    action->setWhatsThis(tr("Zoom in\n\nZooms the current view"));
    connect(action, SIGNAL(toggled(const QString&, bool)), handler,
            SLOT(slotPerformToggleAction(const QString&, bool)));

    action = am->createMouseAction("magMinus", SchematicScene::ZoomingOutAtPoint,
            icon("viewmag-.png"), tr("Zoom out"));
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
void QucsMainWindow::initMenus()
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
    editMenu->addAction(action("selectMarker"));
    editMenu->addAction(action("editFind"));
    editMenu->addAction(action("editRotate"));
    editMenu->addAction(action("editMirror"));
    editMenu->addAction(action("editMirrorY"));

    editMenu->addSeparator();

    editMenu->addAction(action("symEdit"));
    editMenu->addAction(action("intoH"));
    editMenu->addAction(action("popH"));

    alignMenu = menuBar()->addMenu(tr("P&ositioning"));

    alignMenu->addAction(action("onGrid"));

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
void QucsMainWindow::initToolBars()
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

void QucsMainWindow::initStatusBar()
{
    QStatusBar *statusBarWidget = statusBar();
    // Initially its empty space.
    m_cursorLabel = new QLabel(QString(""), statusBarWidget);
    statusBarWidget->addPermanentWidget(m_cursorLabel);
    statusBarWidget->setVisible(action("viewStatusBar")->isChecked());
}

//! \brief Toggles the normal select action on.
void QucsMainWindow::setNormalAction()
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
void QucsMainWindow::addView(QucsView *view)
{
    if(view->isSchematicView()) {
        SchematicView *schematicView = view->toSchematicView();
        SchematicScene *schema = schematicView->schematicScene();
        m_undoGroup->addStack(schema->undoStack());
        // Register here and not in SchematicView constructor because here we
        // can assume that SchematicView and its scene are completely constructed.
        SchematicStateHandler *handler = SchematicStateHandler::instance();
        handler->registerView(schematicView);
    }
    addChildWidget(view->toWidget());
    tabWidget()->setCurrentWidget(view->toWidget());
}

/*!
 * \brief Updates the current undostack and the tabs/window title text.
 *
 * This slot updates the current undostack of undogroup and also updates
 * the tab's as well as window's title text. Also necessary connections between
 * view and this main window are made.
 */
void QucsMainWindow::slotCurrentChanged(QWidget *current, QWidget *prev)
{
    if (prev) {
        prev->disconnect(this);
    }

    QucsView *view = viewFromWidget(current);
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
            }
        }
    }
}

/*!
 * \brief Remove the undostack of widget from undogroup on view close.
 */
void QucsMainWindow::slotViewClosed(QWidget *widget)
{
    SchematicView *view = qobject_cast<SchematicView*>(widget);
    if(view) {
        m_undoGroup->removeStack(view->schematicScene()->undoStack());
    }
}

/*!
 * \brief Sync the settings to configuration file and close window.
 */
void QucsMainWindow::closeEvent( QCloseEvent *e )
{
    QSet<SchematicScene*> processedScenes;
    QSet<QPair<QucsView*, int> > modifiedViews;

    for (int i = 0; i < tabWidget()->count(); ++i) {
        QucsView *view = viewFromWidget(tabWidget()->widget(i));
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
        } else {
            modifiedViews << qMakePair(view, i);
        }
    }

    if (!modifiedViews.isEmpty()) {
        QPointer<SaveDocumentsDialog> dialog(new SaveDocumentsDialog(modifiedViews, this));
        dialog->exec();

        int result = dialog->result();

        if (result == SaveDocumentsDialog::DoNotSave) {
            e->accept();
        } else if (result == SaveDocumentsDialog::AbortClosing) {
            e->ignore();
            return;
        } else {
            QSet<QPair<QucsView*, QString> > newFilePaths = dialog->newFilePaths();
            QSet<QPair<QucsView*, QString> >::iterator it;

            bool failedInBetween = false;
            for (it = newFilePaths.begin(); it != newFilePaths.end(); ++it) {
                QucsView *view = it->first;
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
                e->ignore();
                return;
            } else {
                e->accept();
            }
        }
    }

    saveSettings();
    emit(signalKillWidgets());
    MainWindowBase::closeEvent(e);
}

/*!
 * \brief Creates a new schematic view and adds it the tabwidget.
 */
void QucsMainWindow::slotFileNew()
{
    addView(new SchematicView(0, this));
}

//! \brief Creates a new text(vhdl-verilog-simple) view.
void QucsMainWindow::slotTextNew()
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
void QucsMainWindow::slotFileOpen(QString fileName)
{
    setNormalAction();

    if(fileName == 0) {
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                Settings::instance()->currentValue("nosave/qucsFilter").toString());
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
void QucsMainWindow::slotFileSave(int index)
{
    QucsView* v = viewFromWidget(tabWidget()->widget(index));
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
void QucsMainWindow::slotFileSaveCurrent()
{
    slotFileSave(tabWidget()->currentIndex());
}

/*!
 * \brief Pops up dialog to select new filename and saves the file corresponding
 * to index tab.
 */
void QucsMainWindow::slotFileSaveAs(int index)
{
    QucsView* v = viewFromWidget(tabWidget()->widget(index));
    if(!v) {
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",
            Settings::instance()->currentValue("nosave/qucsFilter").toString());
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
void QucsMainWindow::slotFileSaveAsCurrent()
{
    slotFileSaveAs(tabWidget()->currentIndex());
}

/*!
 * \brief Switches to each opened tab and issues save to that.
 */
void QucsMainWindow::slotFileSaveAll()
{
    for(int i=0; i < tabWidget()->count(); ++i) {
        slotFileSave(i);
    }
}

/*!
 * \brief Closes the selected tab.
 *
 * Before closing it prompts user whether to save or not if the document is
 * modified and takes necessary actions.
 */
void QucsMainWindow::slotFileClose(int index)
{
    if(tabWidget()->count() > 0){
        QucsView *view = viewFromWidget(tabWidget()->widget(index));
        bool saveAttempted = false;
        if(view->isModified()) {
            QMessageBox::StandardButton res =
                QMessageBox::warning(0, tr("Closing qucs document"),
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
void QucsMainWindow::slotFileCloseCurrent()
{
    slotFileClose(tabWidget()->currentIndex());
}

/*!
 * \brief Opens the current schematics' symbol for editing
 */
void QucsMainWindow::slotSymbolEdit()
{
    QucsView *currentView = viewFromWidget(tabWidget()->currentWidget());
    if(!currentView) {
        return;
    }

    if(!currentView->fileName().isEmpty()) {
        QString fileName = currentView->fileName();

        if(currentView->toSchematicView()->schematicScene()->currentMode() == Qucs::SchematicMode) {
            //First, we try to open the corresponding symbol file
            bool isLoaded = gotoPage(fileName, Qucs::SymbolMode);

            //If it's a new symbol, we create it
            if(!isLoaded){
                addView(new SchematicView(0, this));

                QucsView *v = viewFromWidget(tabWidget()->currentWidget());
                SchematicScene *sc = v->toSchematicView()->schematicScene();
                sc->setMode(Qucs::SymbolMode);

                v->setFileName(fileName);
            }
        }
        else if(currentView->toSchematicView()->schematicScene()->currentMode() == Qucs::SymbolMode) {
            gotoPage(fileName, Qucs::SchematicMode);
        }
    }
}

void QucsMainWindow::slotFilePrint()
{
    setNormalAction();

    if(tabWidget()->count() > 0){
        QucsView *view = viewFromWidget(tabWidget()->currentWidget());
        SchematicScene *scene = view->toSchematicView()->schematicScene();

        PrintDialog *p = new PrintDialog(scene, this);
    }
}

void QucsMainWindow::slotExportImage()
{
    setNormalAction();

    QList<SchematicScene *> schemasToExport;

    int i = 0;
    QucsView *view = 0;
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

void QucsMainWindow::slotApplSettings()
{
    setNormalAction();

    QList<SettingsPage *> wantedPages;
    SettingsPage *page = new GeneralConfigurationPage(this);
    wantedPages << page;
    page = new VhdlConfigurationPage(this);
    wantedPages << page;
    page = new SimulationConfigurationPage(this);
    wantedPages << page;

    SettingsDialog *d = new SettingsDialog(wantedPages, "Configure Qucs", this);
    d->exec();
}

void QucsMainWindow::slotFileSettings()
{
    setNormalAction();

    if(tabWidget()->count() > 0){
        QucsView *view = viewFromWidget(tabWidget()->currentWidget());
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

void QucsMainWindow::slotEditCut()
{
    setNormalAction();
    QucsView* v = viewFromWidget(tabWidget()->currentWidget());
    if(!v) {
        return;
    }
    v->cut();
}

void QucsMainWindow::slotEditCopy()
{
    setNormalAction();
    QucsView* v = viewFromWidget(tabWidget()->currentWidget());
    if(!v) {
        return;
    }
    v->copy();
}

void QucsMainWindow::slotEditPaste()
{
    setNormalAction();
    QucsView* v = viewFromWidget(tabWidget()->currentWidget());
    if(!v) {
        return;
    }
    v->paste();
}

void QucsMainWindow::slotSelectAll()
{
    setNormalAction();
    QucsView* v = viewFromWidget(tabWidget()->currentWidget());
    foreach(QGraphicsItem* item, v->toSchematicView()->schematicScene()->items()) {
        item->setSelected(true);
    }
}

void QucsMainWindow::slotSelectMarker()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotEditFind()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotIntoHierarchy()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotPopHierarchy()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

//! \brief Align selected elements appropriately based on \a alignment
void QucsMainWindow::alignElements(Qt::Alignment alignment)
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
void QucsMainWindow::slotAlignTop()
{
    alignElements(Qt::AlignTop);
}

//! \brief Align elements in a row correponding to bottom most elements coords.
void QucsMainWindow::slotAlignBottom()
{
    alignElements(Qt::AlignBottom);
}

//! \brief Align elements in a column correponding to left most elements coords.
void QucsMainWindow::slotAlignLeft()
{
    alignElements(Qt::AlignLeft);
}

/*!
 * \brief Align elements in a column correponding to right most elements
 * coords.
 */
void QucsMainWindow::slotAlignRight()
{
    alignElements(Qt::AlignRight);
}

void QucsMainWindow::slotDistribHoriz()
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

void QucsMainWindow::slotDistribVert()
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

void QucsMainWindow::slotCenterHorizontal()
{
    alignElements(Qt::AlignHCenter);
}

void QucsMainWindow::slotCenterVertical()
{
    alignElements(Qt::AlignVCenter);
}

void QucsMainWindow::slotNewProject()
{
    setNormalAction();
    projectDockWidget->setVisible(true);
    projectDockWidget->raise();
    m_project->slotNewProject();
}

void QucsMainWindow::slotOpenProject(QString fileName)
{
    setNormalAction();
    projectDockWidget->setVisible(true);
    projectDockWidget->raise();
    m_project->slotOpenProject(fileName);
}

void QucsMainWindow::slotAddToProject()
{
    setNormalAction();
    projectDockWidget->setVisible(true);
    projectDockWidget->raise();
    m_project->slotAddToProject();
}

void QucsMainWindow::slotRemoveFromProject()
{
    setNormalAction();
    projectDockWidget->setVisible(true);
    projectDockWidget->raise();
    m_project->slotRemoveFromProject();
}

void QucsMainWindow::slotCloseProject()
{
    setNormalAction();
    m_project->slotCloseProject();
}

void QucsMainWindow::slotInsertEntity()
{
    setNormalAction();
}

void QucsMainWindow::slotCallFilter()
{
    setNormalAction();

    QProcess *QucsFilter = new QProcess(this);
    QucsFilter->start(QString(Qucs::binaryDir + "qucsfilter"));

    connect(QucsFilter, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

    // Kill before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsFilter, SLOT(kill()));
}

void QucsMainWindow::slotCallLine()
{
    setNormalAction();

    QProcess *QucsLine = new QProcess(this);
    QucsLine->start(QString(Qucs::binaryDir + "qucstrans"));

    connect(QucsLine, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

    // Kill before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsLine, SLOT(kill()));
}

void QucsMainWindow::slotCallMatch()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCallAtt()
{
    setNormalAction();

    QProcess *QucsAtt = new QProcess(this);
    QucsAtt->start(QString(Qucs::binaryDir + "qucsattenuator"));

    connect(QucsAtt, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

    // Kill before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsAtt, SLOT(kill()));
}

void QucsMainWindow::slotCallLibrary()
{
    setNormalAction();

    QProcess *QucsLib = new QProcess(this);
    QucsLib->start(QString(Qucs::binaryDir + "qucslib"));

    connect(QucsLib, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

    // Kill before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsLib, SLOT(kill()));
}

void QucsMainWindow::slotImportData()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotShowConsole()
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

void QucsMainWindow::slotSimulate()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotToPage()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotDCbias()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotExportGraphAsCsv()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotShowLastMsg()
{
    setNormalAction();
    editFile(Qucs::pathForQucsFile("log.txt"));
}

void QucsMainWindow::slotShowLastNetlist()
{
    setNormalAction();
    editFile(Qucs::pathForQucsFile("netlist.txt"));
}

void QucsMainWindow::slotShowAll()
{
    setNormalAction();
    QucsView *view = viewFromWidget(tabWidget()->currentWidget());
    if(view) {
        view->showAll();
    }
}

void QucsMainWindow::slotShowOne()
{
    setNormalAction();
    QucsView *view = viewFromWidget(tabWidget()->currentWidget());
    if(view) {
        view->showNoZoom();
    }
}

void QucsMainWindow::slotViewToolBar(bool toogle)
{
    setNormalAction();
    fileToolbar->setVisible(toogle);
    editToolbar->setVisible(toogle);
    viewToolbar->setVisible(toogle);
    workToolbar->setVisible(toogle);
}

void QucsMainWindow::slotViewStatusBar(bool toogle)
{
    setNormalAction();
    statusBar()->setVisible(toogle);
}

//! \brief Opens the editor given a filename
void QucsMainWindow::editFile(const QString& File)
{
    QStringList arguments;
    if(!File.isEmpty()) {
        arguments << File;
    }
    QString textEditor = Settings::instance()->currentValue("gui/textEditor").toString();
    QProcess *QucsEditor = new QProcess(this);
    QucsEditor->start(textEditor, arguments);

    connect(QucsEditor, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

    // Kill editor before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsEditor, SLOT(kill()));
}

//! \brief Opens the help browser
void QucsMainWindow::showHTML(const QString& Page)
{
    QStringList arguments;
    if(!Page.isEmpty()) {
        arguments << Page;
    }
    QProcess *QucsHelp = new QProcess(this);
    QucsHelp->start(QString(Qucs::binaryDir + "qucshelp"), arguments);

    connect(QucsHelp, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProccessError(QProcess::ProcessError)));

    // Kill before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsHelp, SLOT(kill()));
}

void QucsMainWindow::slotHelpIndex()
{
    setNormalAction();
    showHTML("index.html");
}

void QucsMainWindow::slotProccessError(QProcess::ProcessError error)
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

void QucsMainWindow::slotHelpAbout()
{
    setNormalAction();
    AboutQUCS *about = new AboutQUCS();
    about->exec();
}

void QucsMainWindow::slotHelpAboutQt()
{
    setNormalAction();
    QApplication::aboutQt();
}

void QucsMainWindow::loadSettings()
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
    Qucs::validators * validator = Qucs::validators::defaultInstance();
    if(validator->load(libpath)) {
        qDebug() << "Succesfully loaded validators!";
    }
    else {
        //invalidate entry.
        qWarning() << "QucsMainWindow::loadSettings() : Could not load validators. "
                   << "Expect crashing in case of incorrect xml file";
    }

    /* Load transformers */
    Qucs::transformers * transformer = Qucs::transformers::defaultInstance();
    if(transformer->load(libpath)) {
        qDebug() << "Succesfully loaded transformers!";
    }
    else {
        //invalidate entry.
        qWarning() << "QucsMainWindow::loadSettings() : Could not load XSLT transformers. "
                   << "Expect strange schematic symbols";
    }

    LibraryLoader *library = LibraryLoader::instance();

    if(library->loadtree(libpath)) {
        qDebug() << "Succesfully loaded library!";
    }
    else {
        //invalidate entry.
        qWarning() << "QucsMainWindow::loadSettings() : Entry is invalid. Run once more to set"
                   << "the appropriate path.";
        settings->setCurrentValue("sidebarLibrary",
                settings->defaultValue("sidebarLibrary").toString());
        return;
    }

    m_componentsSidebar->plugLibrary("Passive", "Components");
}

void QucsMainWindow::saveSettings()
{
    QSettings qSettings;

    Settings *settings = Settings::instance();

    // Update current geometry and dockPosition values before saving.
    settings->setCurrentValue("gui/geometry", saveGeometry());
    settings->setCurrentValue("gui/dockPositions", saveState());

    settings->save(qSettings);
}

void QucsMainWindow::setTabTitle(const QString& title)
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

QucsView* QucsMainWindow::viewFromWidget(QWidget *widget)
{
    SchematicView *v = qobject_cast<SchematicView*>(widget);
    if(v) {
        return static_cast<QucsView*>(v);
    }
    qDebug("QucsMainWindow::viewFromWidget() : Couldn't identify view type.");
    return 0;
}

void QucsMainWindow::setDocumentTitle(const QString& filename)
{
    setWindowTitle(titleText.arg(filename));
}

void QucsMainWindow::updateTitleTabText()
{
    QucsView *view = viewFromWidget(currentWidget());
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

void QucsMainWindow::slotUpdateAllViews()
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

void QucsMainWindow::slotUpdateCursorPositionStatus(const QString& newPos)
{
    m_cursorLabel->setText(newPos);
}
