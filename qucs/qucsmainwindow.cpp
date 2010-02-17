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

#include "componentssidebar.h"
#include "folderbrowser.h"
#include "item.h"
#include "library.h"
#include "qucsview.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "settings.h"
#include "xmlsymbolformat.h"

#include "dialogs/aboutqucs.h"
#include "dialogs/exportdialog.h"
#include "dialogs/printdialog.h"
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
#include <QProcess>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QUndoGroup>
#include <QUndoView>
#include <QVBoxLayout>
#include <QWhatsThis>

static QString qucsFilter;

/*!
 * \brief Construct and setup the mainwindow for qucs.
 */
QucsMainWindow::QucsMainWindow(QWidget *w) : MainWindowBase(w)
{
    titleText = QString("Qucs ") + (Qucs::version) + QString(" : %1[*]");

    setObjectName("QucsMainWindow"); //for debugging purpose
    setDocumentTitle("Untitled");

    qucsFilter =
        tr("Schematic-xml")+" (*.xsch);;"+
        tr("Symbol-xml")+" (*.xsym);;"+
        tr("Qucs Project")+" (*.xpro);;"+
        tr("Schematic")+" (*.sch);;"+
        tr("Data Display")+" (*.dpl);;"+
        tr("Qucs Documents")+" (*.sch *.dpl);;"+
        tr("VHDL Sources")+" (*.vhdl *.vhd);;"+
        tr("Any File")+" (*)";

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

    projectLibrary = 0;

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
    m_componentsSidebar = new ComponentsSidebar(tr("Schematic Items"), this);
    connect(m_componentsSidebar, SIGNAL(itemClicked(const QString&, const QString&)), this,
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
    m_projectsSidebar = new ComponentsSidebar(tr("Project View"), this);
    connect(m_projectsSidebar, SIGNAL(itemClicked(const QString&, const QString&)), this,
            SLOT(slotSidebarItemClicked(const QString&, const QString&)));

    sidebarDockWidget = new QDockWidget(m_projectsSidebar->windowTitle(),this);
    sidebarDockWidget->setWidget(m_projectsSidebar);
    sidebarDockWidget->setObjectName("projectsSidebar");
    sidebarDockWidget->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, sidebarDockWidget);
    viewMenu->addAction(sidebarDockWidget->toggleViewAction());

    m_projectsSidebar->addToolbarButton(action("projNew"));
    m_projectsSidebar->addToolbarButton(action("projOpen"));
    m_projectsSidebar->addToolbarButton(action("addToProj"));
    m_projectsSidebar->addToolbarButton(action("projDel"));
    m_projectsSidebar->addToolbarButton(action("projClose"));
}

void QucsMainWindow::createUndoView()
{
    undoView = new QUndoView(m_undoGroup);
    undoView->setWindowTitle(tr("Command List"));

    sidebarDockWidget = new QDockWidget(undoView->windowTitle(),this);
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

    sidebarDockWidget = new QDockWidget(m_folderBrowser->windowTitle(),this);
    sidebarDockWidget->setWidget(m_folderBrowser);
    sidebarDockWidget->setObjectName("folderBrowserSidebar");
    addDockWidget(Qt::LeftDockWidgetArea, sidebarDockWidget);
    viewMenu->addAction(sidebarDockWidget->toggleViewAction());
}

/*!
 * \brief Creates and intializes all the actions used.
 */
void QucsMainWindow::initActions()
{
    QAction *action = 0;
    using namespace Qt;

    QString bitmapPath = Qucs::bitmapDirectory();
    action = new QAction(QIcon(bitmapPath + "filenew.png"), tr("&New"), this);
    action->setShortcut(CTRL+Key_N);
    action->setStatusTip(tr("Creates a new document"));
    action->setWhatsThis(tr("New\n\nCreates a new schematic or data display document"));
    action->setObjectName("fileNew");
    connect(action, SIGNAL(triggered()), SLOT(slotFileNew()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "textnew.png"), tr("New &Text"), this);
    action->setShortcut(CTRL+SHIFT+Key_V);
    action->setStatusTip(tr("Creates a new text document"));
    action->setWhatsThis(tr("New Text\n\nCreates a new text document"));
    action->setObjectName("textNew");
    connect(action, SIGNAL(triggered()), SLOT(slotTextNew()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "fileopen.png"), tr("&Open..."), this);
    action->setShortcut(CTRL+Key_O);
    action->setStatusTip(tr("Opens an existing document"));
    action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
    action->setObjectName("fileOpen");
    connect(action, SIGNAL(triggered()), SLOT(slotFileOpen()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "filesave.png"), tr("&Save"), this);
    action->setShortcut(CTRL+Key_S);
    action->setStatusTip(tr("Saves the current document"));
    action->setWhatsThis(tr("Save File\n\nSaves the current document"));
    action->setObjectName("fileSave");
    connect(action, SIGNAL(triggered()), SLOT(slotFileSaveCurrent()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "filesaveas.png"), tr("Save as..."), this);
    action->setShortcut(CTRL+SHIFT+Key_S);
    action->setStatusTip(tr("Saves the current document under a new filename"));
    action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
    action->setObjectName("fileSaveAs");
    connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAsCurrent()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "filesaveall.png"), tr("Save &All"), this);
    action->setShortcut(CTRL+Key_Plus);
    action->setStatusTip(tr("Saves all open documents"));
    action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
    action->setObjectName("fileSaveAll");
    connect(action, SIGNAL(triggered()), SLOT(slotFileSaveAll()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "fileclose.png"), tr("&Close"), this);
    action->setShortcut(CTRL+Key_W);
    action->setStatusTip(tr("Closes the current document"));
    action->setWhatsThis(tr("Close File\n\nCloses the current document"));
    action->setObjectName("fileClose");
    connect(action, SIGNAL(triggered()), SLOT(slotFileCloseCurrent()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "fileprint.png"), tr("&Print..."), this);
    action->setShortcut(CTRL+Key_P);
    action->setStatusTip(tr("Prints the current document"));
    action->setWhatsThis(tr("Print File\n\nPrints the current document"));
    action->setObjectName("filePrint");
    connect(action, SIGNAL(triggered()), SLOT(slotFilePrint()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "export-image.png"), tr("&Export Image..."), this);
    action->setShortcut(CTRL+Key_E);
    action->setWhatsThis(tr("Export Image\n\n""Export current view to image file"));
    action->setObjectName("exportImage");
    connect(action, SIGNAL(triggered()), SLOT(slotExportImage()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "document-edit.png"), tr("&Document Settings..."), this);
    action->setShortcut(CTRL+Key_Period);
    action->setWhatsThis(tr("Settings\n\nSets properties of the file"));
    action->setObjectName("fileSettings");
    connect(action, SIGNAL(triggered()), SLOT(slotFileSettings()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "configure.png"), tr("Application Settings..."), this);
    action->setShortcut(CTRL+Key_Comma);
    action->setWhatsThis(tr("Qucs Settings\n\nSets properties of the application"));
    action->setObjectName("applSettings");
    connect(action, SIGNAL(triggered()), SLOT(slotApplSettings()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "application-exit.png"), tr("E&xit"), this);
    action->setShortcut(CTRL+Key_Q);
    action->setStatusTip(tr("Quits the application"));
    action->setWhatsThis(tr("Exit\n\nQuits the application"));
    action->setObjectName("fileQuit");
    connect(action, SIGNAL(triggered()), SLOT(close()));
    addActionToMap(action);

    action = m_undoGroup->createUndoAction(this);
    action->setIcon(QIcon(bitmapPath + "undo.png"));
    action->setShortcut(CTRL+Key_Z);
    action->setStatusTip(tr("Undoes the last command"));
    action->setWhatsThis(tr("Undo\n\nMakes the last action undone"));
    action->setObjectName("undo");
    addActionToMap(action);

    action = m_undoGroup->createRedoAction(this);
    action->setIcon(QIcon(bitmapPath + "redo.png"));
    action->setShortcut(CTRL+SHIFT+Key_Z);
    action->setStatusTip(tr("Redoes the last command"));
    action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
    action->setObjectName("redo");
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "editcut.png"), tr("Cu&t"), this);
    action->setShortcut(CTRL+Key_X);
    action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
    action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
    action->setObjectName("editCut");
    connect(action, SIGNAL(triggered()), SLOT(slotEditCut()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "editcopy.png"), tr("&Copy"), this);
    action->setShortcut(CTRL+Key_C);
    action->setStatusTip(tr("Copies the selection into the clipboard"));
    action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
    action->setObjectName("editCopy");
    connect(action, SIGNAL(triggered()), SLOT(slotEditCopy()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "editpaste.png"), tr("&Paste"), this);
    action->setShortcut(CTRL+Key_V);
    action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
    action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
    action->setObjectName("editPaste");
    connect(action, SIGNAL(triggered()), SLOT(slotEditPaste()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "editdelete.png"), tr("&Delete"), this);
    action->setShortcut(Key_Delete);
    action->setStatusTip(tr("Deletes the selected components"));
    action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
    action->setObjectName("editDelete");
    action->setData(QVariant(SchematicScene::Deleting));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotEditDelete(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "pointer.png"), tr("Select"), this);
    action->setShortcut(Key_Escape);
    action->setStatusTip(tr("Activate select mode"));
    action->setWhatsThis(tr("Select\n\nActivates select mode"));
    action->setObjectName("select");
    action->setData(QVariant(SchematicScene::Normal));
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotSelect(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "select-all.png"), tr("Select All"), this);
    action->setShortcut(CTRL+Key_A);
    action->setStatusTip(tr("Selects all elements"));
    action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
    action->setObjectName("selectAll");
    connect(action, SIGNAL(triggered()), SLOT(slotSelectAll()));
    addActionToMap(action);

    action = new QAction( tr("Select Markers"), this);
    action->setShortcut(CTRL+SHIFT+Key_M);
    action->setStatusTip(tr("Selects all markers"));
    action->setWhatsThis(tr("Select Markers\n\nSelects all diagram markers of the document"));
    action->setObjectName("selectMarker");
    connect(action, SIGNAL(triggered()), SLOT(slotSelectMarker()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "editfind.png"), tr("Find..."), this);
    action->setShortcut(CTRL+Key_F);
    action->setStatusTip(tr("Find a piece of text"));
    action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
    action->setObjectName("editFind");
    connect(action, SIGNAL(triggered()), SLOT(slotEditFind()));
    addActionToMap(action);

    action = new QAction( tr("Replace..."), this);
    action->setWhatsThis(tr("Replace\n\nChange component properties\nor\ntext in VHDL code"));
    action->setObjectName("changeProps");
    connect(action, SIGNAL(triggered()), SLOT(slotReplace()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "rotate_ccw.png"), tr("Rotate"), this);
    action->setShortcut(CTRL+Key_R);
    action->setStatusTip(tr("Rotates the selected component by 90°"));
    action->setWhatsThis(tr("Rotate\n\nRotates the selected component by 90° counter-clockwise"));
    action->setObjectName("editRotate");
    action->setData(QVariant(SchematicScene::Rotating));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotEditRotate(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "mirror.png"), tr("Mirror about X Axis"), this);
    action->setShortcut(Key_V);
    action->setWhatsThis(tr("Mirror about X Axis\n\nMirrors the selected item about X Axis"));
    action->setObjectName("editMirror");
    action->setData(QVariant(SchematicScene::MirroringX));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotEditMirrorX(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "mirrory.png"), tr("Mirror about Y Axis"), this);
    action->setShortcut(Key_H);
    action->setWhatsThis(tr("Mirror about Y Axis\n\nMirrors the selected item about Y Axis"));
    action->setObjectName("editMirrorY");
    action->setData(QVariant(SchematicScene::MirroringY));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotEditMirrorY(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "symbol-edit.png"), tr("&Edit Circuit Symbol/Schematic"), this);
    action->setShortcut(Key_F7);
    action->setStatusTip(tr("Switches between symbol and schematic edit"));
    action->setWhatsThis(tr("Edit Circuit Symbol/Schematic\n\nSwitches between symbol and schematic edit"));
    action->setObjectName("symEdit");
    connect(action, SIGNAL(triggered()), SLOT(slotSymbolEdit()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "bottom.png"), tr("Go into Subcircuit"), this);
    action->setShortcut(CTRL+Key_I);
    action->setWhatsThis(tr("Go into Subcircuit\n\nGoes inside the selected subcircuit"));
    action->setObjectName("intoH");
    connect(action, SIGNAL(triggered()), SLOT(slotIntoHierarchy()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "top.png"), tr("Pop out"), this);
    action->setShortcut(CTRL+SHIFT+Key_I);
    action->setStatusTip(tr("Pop outside subcircuit"));
    action->setWhatsThis(tr("Pop out\n\nGoes up one hierarchy level, i.e. leaves subcircuit"));
    action->setObjectName("popH");
    connect(action, SIGNAL(triggered()), SLOT(slotPopHierarchy()));
    addActionToMap(action);

    action = new QAction( tr("Set on Grid"), this);
    action->setShortcut(CTRL+Key_U);
    action->setWhatsThis(tr("Set on Grid\n\nSets selected elements on grid"));
    action->setObjectName("onGrid");
    action->setCheckable(true);
    action->setData(QVariant(SchematicScene::SettingOnGrid));
    connect(action, SIGNAL(toggled(bool)), SLOT(slotOnGrid(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "align-vertical-top.png"), tr("Align top"), this);
    action->setStatusTip(tr("Align top selected elements"));
    action->setWhatsThis(tr("Align top\n\nAlign selected elements to their upper edge"));
    action->setObjectName("alignTop");
    connect(action, SIGNAL(triggered()), SLOT(slotAlignTop()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "align-vertical-bottom.png"), tr("Align bottom"), this);
    action->setStatusTip(tr("Align bottom selected elements"));
    action->setWhatsThis(tr("Align bottom\n\nAlign selected elements to their lower edge"));
    action->setObjectName("alignBottom");
    connect(action, SIGNAL(triggered()), SLOT(slotAlignBottom()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "align-horizontal-left.png"), tr("Align left"), this);
    action->setStatusTip(tr("Align left selected elements"));
    action->setWhatsThis(tr("Align left\n\nAlign selected elements to their left edge"));
    action->setObjectName("alignLeft");
    connect(action, SIGNAL(triggered()), SLOT(slotAlignLeft()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "align-horizontal-right.png"), tr("Align right"), this);
    action->setStatusTip(tr("Align right selected elements"));
    action->setWhatsThis(tr("Align right\n\nAlign selected elements to their right edge"));
    action->setObjectName("alignRight");
    connect(action, SIGNAL(triggered()), SLOT(slotAlignRight()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "align-horizontal-center.png"), tr("Center horizontally"), this);
    action->setStatusTip(tr("Center horizontally selected elements"));
    action->setWhatsThis(tr("Center horizontally\n\nCenter horizontally selected elements"));
    action->setObjectName("centerHor");
    connect(action, SIGNAL(triggered()), SLOT(slotCenterHorizontal()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "align-vertical-center.png"), tr("Center vertically"), this);
    action->setStatusTip(tr("Center vertically selected elements"));
    action->setWhatsThis(tr("Center vertically\n\nCenter vertically selected elements"));
    action->setObjectName("centerVert");
    connect(action, SIGNAL(triggered()), SLOT(slotCenterVertical()));
    addActionToMap(action);

    action = new QAction( tr("Distribute horizontally"), this);
    action->setStatusTip(tr("Distribute equally horizontally"));
    action->setWhatsThis(tr("Distribute horizontally\n\n""Distribute horizontally selected elements"));
    action->setObjectName("distrHor");
    connect(action, SIGNAL(triggered()), SLOT(slotDistribHoriz()));
    addActionToMap(action);

    action = new QAction( tr("Distribute vertically"), this);
    action->setStatusTip(tr("Distribute equally vertically"));
    action->setWhatsThis(tr("Distribute vertically\n\n""Distribute vertically selected elements"));
    action->setObjectName("distrVert");
    connect(action, SIGNAL(triggered()), SLOT(slotDistribVert()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "project-new.png"), tr("&New Project..."), this);
    action->setShortcut(CTRL+SHIFT+Key_N);
    action->setStatusTip(tr("Creates a new project"));
    action->setWhatsThis(tr("New Project\n\nCreates a new project"));
    action->setObjectName("projNew");
    connect(action, SIGNAL(triggered()), SLOT(slotNewProject()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "fileopen.png"), tr("&Open Project..."), this);
    action->setShortcut(CTRL+SHIFT+Key_O);
    action->setStatusTip(tr("Opens an existing project"));
    action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
    action->setObjectName("projOpen");
    connect(action, SIGNAL(triggered()), SLOT(slotOpenProject()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "filenew.png"), tr("&Add File to Project..."), this);
    action->setShortcut(CTRL+SHIFT+Key_A);
    action->setStatusTip(tr("Adds a file to current project"));
    action->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));
    action->setObjectName("addToProj");
    connect(action, SIGNAL(triggered()), SLOT(slotAddToProject()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "fileclose.png"), tr("&Remove from Project"), this);
    action->setShortcut(CTRL+SHIFT+Key_R);
    action->setStatusTip(tr("Removes a file from current project"));
    action->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));
    action->setObjectName("projDel");
    connect(action, SIGNAL(triggered()), SLOT(slotRemoveFromProject()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "project-close.png"), tr("&Close Project"), this);
    action->setShortcut(CTRL+SHIFT+Key_W);
    action->setStatusTip(tr("Closes the current project"));
    action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
    action->setObjectName("projClose");
    connect(action, SIGNAL(triggered()), SLOT(slotCloseProject()));
    addActionToMap(action);

    action = new QAction( tr("Create &Library..."), this);
    action->setShortcut(CTRL+SHIFT+Key_L);
    action->setStatusTip(tr("Create Library from Subcircuits"));
    action->setWhatsThis(tr("Create Library\n\nCreate Library from Subcircuits"));
    action->setObjectName("createLib");
    connect(action, SIGNAL(triggered()), SLOT(slotCreateLib()));
    addActionToMap(action);

    action = new QAction( tr("Create &Package..."), this);
    action->setShortcut(CTRL+SHIFT+Key_K);
    action->setStatusTip(tr("Create compressed Package from Projects"));
    action->setWhatsThis(tr("Create Package\n\nCreate compressed Package from complete Projects"));
    action->setObjectName("createPkg");
    connect(action, SIGNAL(triggered()), SLOT(slotCreatePackage()));
    addActionToMap(action);

    action = new QAction( tr("E&xtract Package..."), this);
    action->setShortcut(CTRL+SHIFT+Key_X);
    action->setStatusTip(tr("Install Content of a Package"));
    action->setWhatsThis(tr("Extract Package\n\nInstall Content of a Package"));
    action->setObjectName("extractPkg");
    connect(action, SIGNAL(triggered()), SLOT(slotExtractPackage()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "wire.png"), tr("Wire"), this);
    action->setShortcut(Key_W);
    action->setWhatsThis(tr("Wire\n\nInserts a wire"));
    action->setObjectName("insWire");
    action->setData(QVariant(SchematicScene::Wiring));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotSetWire(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "nodename.png"), tr("Wire Label"), this);
    action->setShortcut(Key_L);
    action->setStatusTip(tr("Inserts a wire or pin label"));
    action->setWhatsThis(tr("Wire Label\n\nInserts a wire or pin label"));
    action->setObjectName("insLabel");
    action->setData(QVariant(SchematicScene::InsertingWireLabel));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotInsertLabel(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "equation.png"), tr("Insert Equation"), this);
    action->setShortcut(Key_E);
    action->setWhatsThis(tr("Insert Equation\n\nInserts a user defined equation"));
    action->setObjectName("insEquation");
    connect(action, SIGNAL(triggered()), SLOT(slotInsertEquation()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "ground.png"), tr("Insert Ground"), this);
    action->setShortcut(Key_G);
    action->setWhatsThis(tr("Insert Ground\n\nInserts a ground symbol"));
    action->setObjectName("insGround");
    connect(action, SIGNAL(triggered()), SLOT(slotInsertGround()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "port.png"), tr("Insert Port"), this);
    action->setShortcut(Key_P);
    action->setWhatsThis(tr("Insert Port\n\nInserts a port symbol"));
    action->setObjectName("insPort");
    connect(action, SIGNAL(triggered()), SLOT(slotInsertPort()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "vhdl-code.png"), tr("VHDL entity"), this);
    action->setShortcut(SHIFT+Key_V);
    action->setStatusTip(tr("Inserts skeleton of VHDL entity"));
    action->setWhatsThis(tr("VHDL entity\n\nInserts the skeleton of a VHDL entity"));
    action->setObjectName("insEntity");
    connect(action, SIGNAL(triggered()), SLOT(slotInsertEntity()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "deactiv.png"), tr("Deactivate/Activate"), this);
    action->setShortcut(Key_D);
    action->setStatusTip(tr("Deactivate/Activate selected components"));
    action->setWhatsThis(tr("Deactivate/Activate\n\nDeactivate/Activate the selected components"));
    action->setObjectName("editActivate");
    action->setData(QVariant(SchematicScene::ChangingActiveStatus));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotEditActivate(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "tools-wizard.png"), tr("Filter synthesis"), this);
    action->setShortcut(CTRL+Key_1);
    action->setStatusTip(tr("Starts QucsFilter"));
    action->setWhatsThis(tr("Filter synthesis\n\nStarts QucsFilter"));
    action->setObjectName("callFilter");
    connect(action, SIGNAL(triggered()), SLOT(slotCallFilter()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "tools-wizard.png"), tr("Line calculation"), this);
    action->setShortcut(CTRL+Key_2);
    action->setStatusTip(tr("Starts QucsTrans"));
    action->setWhatsThis(tr("Line calculation\n\nStarts transmission line calculator"));
    action->setObjectName("callLine");
    connect(action, SIGNAL(triggered()), SLOT(slotCallLine()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "tools-wizard.png"), tr("Matching Circuit"), this);
    action->setShortcut(CTRL+Key_3);
    action->setStatusTip(tr("Creates Matching Circuit"));
    action->setWhatsThis(tr("Matching Circuit\n\nDialog for Creating Matching Circuit"));
    action->setObjectName("callMatch");
    connect(action, SIGNAL(triggered()), SLOT(slotCallMatch()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "tools-wizard.png"), tr("Attenuator synthesis"), this);
    action->setShortcut(CTRL+Key_4);
    action->setStatusTip(tr("Starts QucsAttenuator"));
    action->setWhatsThis(tr("Attenuator synthesis\n\nStarts attenuator calculation program"));
    action->setObjectName("callAtt");
    connect(action, SIGNAL(triggered()), SLOT(slotCallAtt()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "library.png"), tr("Component Library"), this);
    action->setShortcut(CTRL+Key_5);
    action->setStatusTip(tr("Starts QucsLib"));
    action->setWhatsThis(tr("Component Library\n\nStarts component library program"));
    action->setObjectName("callLib");
    connect(action, SIGNAL(triggered()), SLOT(slotCallLibrary()));
    addActionToMap(action);

    action = new QAction( tr("&Import Data..."), this);
    action->setShortcut(CTRL+Key_6);
    action->setStatusTip(tr("Convert file to Qucs data file"));
    action->setWhatsThis(tr("Import Data\n\nConvert data file to Qucs data file"));
    action->setObjectName("importData");
    connect(action, SIGNAL(triggered()), SLOT(slotImportData()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "console.png"), tr("&Show Console..."), this);
    action->setShortcut(Key_F8);
    action->setStatusTip(tr("Show Console"));
    action->setWhatsThis(tr("Show Console\n\nOpen console terminal"));
    action->setObjectName("showConsole");
    connect(action, SIGNAL(triggered()), SLOT(slotShowConsole()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "start.png"), tr("Simulate"), this);
    action->setShortcut(Key_F5);
    action->setStatusTip(tr("Simulates the current schematic"));
    action->setWhatsThis(tr("Simulate\n\nSimulates the current schematic"));
    action->setObjectName("simulate");
    connect(action, SIGNAL(triggered()), SLOT(slotSimulate()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "switch-view.png"), tr("View Data Display/Schematic"), this);
    action->setShortcut(Key_F4);
    action->setStatusTip(tr("Changes to data display or schematic page"));
    action->setWhatsThis(tr("View Data Display/Schematic\n\n")+tr("Changes to data display or schematic page"));
    action->setObjectName("dpl_sch");
    connect(action, SIGNAL(triggered()), SLOT(slotToPage()));
    addActionToMap(action);

    action = new QAction( tr("Calculate DC bias"), this);
    action->setShortcut(Key_F3);
    action->setStatusTip(tr("Calculates DC bias and shows it"));
    action->setWhatsThis(tr("Calculate DC bias\n\nCalculates DC bias and shows it"));
    action->setObjectName("dcbias");
    connect(action, SIGNAL(triggered()), SLOT(slotDCbias()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "marker.png"), tr("Set Marker on Graph"), this);
    action->setShortcut(Key_F2);
    action->setStatusTip(tr("Sets a marker on a diagram's graph"));
    action->setWhatsThis(tr("Set Marker\n\nSets a marker on a diagram's graph"));
    action->setObjectName("setMarker");
    action->setData(QVariant(SchematicScene::Marking));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotSetMarker(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction( tr("Export to &CSV..."), this);
    action->setShortcut(Key_F6);
    action->setStatusTip(tr("Convert graph data to CSV file"));
    action->setWhatsThis(tr("Export to CSV\n\nConvert graph data to CSV file"));
    action->setObjectName("graph2csv");
    connect(action, SIGNAL(triggered()), SLOT(slotExportGraphAsCsv()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "document-preview.png"), tr("Show Last Messages"), this);
    action->setShortcut(Key_F9);
    action->setStatusTip(tr("Shows last simulation messages"));
    action->setWhatsThis(tr("Show Last Messages\n\nShows the messages of the last simulation"));
    action->setObjectName("showMsg");
    connect(action, SIGNAL(triggered()), SLOT(slotShowLastMsg()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "document-preview.png"), tr("Show Last Netlist"), this);
    action->setShortcut(Key_F10);
    action->setStatusTip(tr("Shows last simulation netlist"));
    action->setWhatsThis(tr("Show Last Netlist\n\nShows the netlist of the last simulation"));
    action->setObjectName("showNet");
    connect(action, SIGNAL(triggered()), SLOT(slotShowLastNetlist()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "viewmagfit.png"), tr("View All"), this);
    action->setShortcut(Key_0);
    action->setStatusTip(tr("Show the whole page"));
    action->setWhatsThis(tr("View All\n\nShows the whole page content"));
    action->setObjectName("magAll");
    connect(action, SIGNAL(triggered()), SLOT(slotShowAll()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "viewmag1.png"), tr("View 1:1"), this);
    action->setShortcut(Key_1);
    action->setStatusTip(tr("Views without magnification"));
    action->setWhatsThis(tr("View 1:1\n\nShows the page content without magnification"));
    action->setObjectName("magOne");
    connect(action, SIGNAL(triggered()), SLOT(slotShowOne()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "viewmag+.png"), tr("Zoom in"), this);
    action->setShortcut(Key_Plus);
    action->setStatusTip(tr("Zooms into the current view"));
    action->setWhatsThis(tr("Zoom in\n\nZooms the current view"));
    action->setObjectName("magPlus");
    action->setData(QVariant(SchematicScene::ZoomingAtPoint));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotZoomIn(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction(QIcon(bitmapPath + "viewmag-.png"), tr("Zoom out"), this);
    action->setShortcut(Key_Minus);
    action->setStatusTip(tr("Zooms out the current view"));
    action->setWhatsThis(tr("Zoom out\n\nZooms out the current view"));
    action->setObjectName("magMinus");
    action->setData(QVariant(SchematicScene::ZoomingOutAtPoint));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotZoomOut(bool)));
    addActionToMap(action);
    checkableActions << action;

    action = new QAction( tr("Tool&bar"), this);
    action->setStatusTip(tr("Enables/disables the toolbar"));
    action->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
    action->setObjectName("viewToolBar");
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotViewToolBar(bool)));
    addActionToMap(action);

    action = new QAction( tr("&Statusbar"), this);
    action->setStatusTip(tr("Enables/disables the statusbar"));
    action->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
    action->setObjectName("viewStatusBar");
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(slotViewStatusBar(bool)));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "help.png"), tr("Help Index..."), this);
    action->setShortcut(Key_F1);
    action->setStatusTip(tr("Index of Qucs Help"));
    action->setWhatsThis(tr("Help Index\n\nIndex of intern Qucs help"));
    action->setObjectName("helpIndex");
    connect(action, SIGNAL(triggered()), SLOT(slotHelpIndex()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "qucs.png"), tr("&About Qucs..."), this);
    action->setWhatsThis(tr("About\n\nAbout the application"));
    action->setObjectName("helpAboutApp");
    connect(action, SIGNAL(triggered()), SLOT(slotHelpAbout()));
    addActionToMap(action);

    action = new QAction(QIcon(bitmapPath + "qt.png"), tr("About Qt..."), this);
    action->setWhatsThis(tr("About Qt\n\nAbout Qt by Trolltech"));
    action->setObjectName("helpAboutQt");
    connect(action, SIGNAL(triggered()), SLOT(slotHelpAboutQt()));
    addActionToMap(action);

    action = QWhatsThis::createAction(this);
    action->setObjectName("whatsThis");
    addActionToMap(action);

    action = new QAction(tr("Insert item action"), this);
    action->setObjectName("insertItem");
    action->setData(QVariant(SchematicScene::InsertingItems));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(slotInsertItemAction(bool)));
    checkableActions << action;
    addActionToMap(action);

    action = new QAction(tr("Painting draw action"), this);
    action->setObjectName("paintingDraw");
    action->setData(QVariant(SchematicScene::PaintingDrawEvent));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(slotPaintingDrawAction(bool)));
    checkableActions << action;
    addActionToMap(action);
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
    editMenu->addAction(action("changeProps"));
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

    projMenu->addSeparator();

    projMenu->addAction(action("createLib"));
    projMenu->addAction(action("createPkg"));
    projMenu->addAction(action("extractPkg"));

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

    menuBar()->addSeparator();

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
    QStatusBar *statusBar = this->statusBar();
    // Initially its empty space.
    m_cursorLabel = new QLabel(QString("     "), statusBar);
    statusBar->addPermanentWidget(m_cursorLabel);
    statusBar->setVisible(action("viewStatusBar")->isChecked());
}

/*!
 * \brief Toogles the action perfomed.
 *
 * This method toggles the action and calls the function pointed by
 * \a func if on is true. This method takes care to preserve the mutual
 * exclusiveness off the checkable actions.
 */
void QucsMainWindow::performToggleAction(const bool on, pActionFunc func, QAction *action)
{
    SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
    if(!view) {
        //nothing to do since it is not schematic view
        return;
    }

    SchematicScene *scene = view->schematicScene();
    SchematicScene::MouseAction ma = SchematicScene::MouseAction(action->data().toInt());
    QAction *norm = this->action("select");

    //toggling off any action switches normal select action "on"
    if(!on) {
        if(ma != SchematicScene::Normal) {
            foreach(QAction *act, checkableActions) {
                if(act != norm) {
                    act->blockSignals(true);
                    act->setChecked(false);
                    act->blockSignals(false);
                }
            }
        }
        norm->blockSignals(true);
        norm->setChecked(true);
        norm->blockSignals(false);
        scene->setCurrentMouseAction(SchematicScene::Normal);
        return;
    }

    //else part
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();

    do {
        if(!(selectedItems.isEmpty() || func == 0)) {
            QList<QucsItem*> funcable = filterItems<QucsItem>(selectedItems);

            if(funcable.isEmpty()) {
                break;
            }

            (scene->*func)(funcable, Qucs::PushUndoCmd);

            foreach(QAction *act, checkableActions) {
                if(act != norm) {
                    act->blockSignals(true);
                    act->setChecked(false);
                    act->blockSignals(false);
                }
            }
            norm->blockSignals(true);
            norm->setChecked(true);
            norm->blockSignals(false);
            //Safe to call repeatedly since this function performs change if
            //only the mouseAction is different from previous.
            scene->setCurrentMouseAction(SchematicScene::Normal);
            return;
        }
    } while(false); //For break

    foreach(QAction *act, checkableActions) {
        if(act != action) {
            act->blockSignals(true);
            act->setChecked(false);
            act->blockSignals(false);
        }
    }
    scene->setCurrentMouseAction(ma);
}

//! \brief Toggles the normal select action on.
void QucsMainWindow::setNormalAction()
{
    performToggleAction(true, 0, action("select"));
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
    SchematicView *prevView = qobject_cast<SchematicView*>(prev);
    if(prevView) {
        prevView->resetState();
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
    if(fileName == 0) {
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                "", qucsFilter);
    }

    if(!fileName.isEmpty()) {
        if(QFileInfo(fileName).suffix() == "xpro") {
                LibraryLoader *library = LibraryLoader::defaultInstance();

                if(!library->library(fileName)) {
                    if(library->load(fileName)) {
                        slotCloseProject();
                        projectLibrary = library->library(fileName);
                        qDebug() << "Succesfully loaded library!";
                        m_projectsSidebar->plugLibrary(fileName, "root");
                    }
                    else {
                        QMessageBox::critical(this, tr("Error"),
                                tr("Invalid project file!"));
                        return;
                    }
                }
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
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
            "", qucsFilter);
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
        if(view->isModified()) {
            QMessageBox::StandardButton res =
                QMessageBox::warning(0, tr("Closing qucs document"),
                        tr("The document contains unsaved changes!\n"
                            "Do you want to save the changes ?"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if(res == QMessageBox::Save) {
                slotFileSave(index);
            }
            else if(res == QMessageBox::Cancel) {
                return;
            }
        }
        closeTab(index);
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
    slotInsertItemAction(true);
    v->paste();
}

void QucsMainWindow::slotEditDelete(bool on)
{
    performToggleAction(on, &SchematicScene::deleteItems, action("editDelete"));
}

void QucsMainWindow::slotSelect(bool on)
{
    performToggleAction(on, 0, action("select"));
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

void QucsMainWindow::slotReplace()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotEditRotate(bool on)
{
    performToggleAction(on, &SchematicScene::rotateItems, action("editRotate"));
}

void QucsMainWindow::slotEditMirrorX(bool on)
{
    performToggleAction(on, &SchematicScene::mirrorXItems, action("editMirror"));
}

void QucsMainWindow::slotEditMirrorY(bool on)
{
    performToggleAction(on, &SchematicScene::mirrorYItems, action("editMirrorY"));
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

void QucsMainWindow::slotOnGrid(bool on)
{
    performToggleAction(on, &SchematicScene::setItemsOnGrid, action("onGrid"));
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
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
            "", tr("Qucs Projects (*.xpro)"));
    if(!fileName.isEmpty()) {
        if(QString(QFileInfo(fileName).suffix()).isEmpty()) {
            fileName = fileName + ".xpro";
        }

        LibraryLoader *library = LibraryLoader::defaultInstance();

        if(library->newLibrary(fileName)) {
            slotCloseProject();
            projectLibrary = library->library(fileName);
            projectLibrary->saveLibrary();
            qDebug() << "Succesfully created library!";
            m_projectsSidebar->plugLibrary(fileName, "root");
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                    tr("Invalid project file!"));
            return;
        }
    }
}

void QucsMainWindow::slotOpenProject()
{
    setNormalAction();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
            "", tr("Qucs Projects (*.xpro)"));
    if(!fileName.isEmpty()) {
        slotFileOpen(fileName);
    }
}

void QucsMainWindow::slotAddToProject()
{
    setNormalAction();
    if(projectLibrary) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Add File to Project"),
                "", tr("Component-xml (*.xsch *.xsym)"));
        if(!fileName.isEmpty()) {
            //If we selected a schematic, we must generate the corresponding symbol
            if(QString(QFileInfo(fileName).suffix()) == "xsch") {
                QucsView *view = new SchematicView(0, this);
                view->toSchematicView()->schematicScene()->setMode(Qucs::SymbolMode);

                if(!view->load(fileName)) {
                    QMessageBox::critical(this, tr("Error"),
                            tr("Could not open file!"));
                    delete view;
                    return;
                }

                fileName.replace(".xsch",".xsym");
                view->setFileName(fileName);
                XmlSymbolFormat *symbol = new XmlSymbolFormat(view->toSchematicView()->schematicScene());
                symbol->save();

                delete view;
            }
            projectLibrary->parseExternalComponent(fileName);
            projectLibrary->saveLibrary();
            m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
            m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
        }
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                tr("Invalid project!"));
        return;
    }
}

void QucsMainWindow::slotRemoveFromProject()
{
    setNormalAction();
    if(projectLibrary) {
        if(!m_projectsSidebar->currentComponent().isEmpty()) {
            projectLibrary->removeComponent(m_projectsSidebar->currentComponent());
            projectLibrary->saveLibrary();
            m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
            m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
        }
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                tr("Invalid project!"));
        return;
    }
}

void QucsMainWindow::slotCloseProject()
{
    setNormalAction();
    if(projectLibrary) {
        m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
        LibraryLoader *library = LibraryLoader::defaultInstance();
        library->unload(projectLibrary->libraryFileName());
        projectLibrary = 0;
    }
}

void QucsMainWindow::slotCreateLib()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCreatePackage()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotExtractPackage()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotSetWire(bool on)
{
    performToggleAction(on, 0, action("insWire"));
}

void QucsMainWindow::slotInsertLabel(bool on)
{
    performToggleAction(on, 0, action("insLabel"));
}

void QucsMainWindow::slotInsertEquation()
{
}

void QucsMainWindow::slotInsertGround()
{
    slotSidebarItemClicked("Ground", "Passive");
}

void QucsMainWindow::slotInsertPort()
{
}

void QucsMainWindow::slotInsertEntity()
{
    setNormalAction();
}

void QucsMainWindow::slotEditActivate(bool on)
{
    performToggleAction(on, &SchematicScene::toggleActiveStatus, action("editActivate"));
}

void QucsMainWindow::slotCallFilter()
{
    setNormalAction();

    QProcess *QucsFilter = new QProcess(this);
    QucsFilter->start(QString(Qucs::binaryDir + "qucsfilter"));

    //TODO Emit error in case there are problems
    // Kill editor before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsFilter, SLOT(kill()));
}

void QucsMainWindow::slotCallLine()
{
    setNormalAction();

    QProcess *QucsLine = new QProcess(this);
    QucsLine->start(QString(Qucs::binaryDir + "qucstrans"));

    //TODO Emit error in case there are problems
    // Kill editor before qucs ends
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

    //TODO Emit error in case there are problems
    // Kill editor before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsAtt, SLOT(kill()));
}

void QucsMainWindow::slotCallLibrary()
{
    setNormalAction();

    QProcess *QucsLib = new QProcess(this);
    QucsLib->start(QString(Qucs::binaryDir + "qucslib"));

    //TODO Emit error in case there are problems
    // Kill editor before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsLib, SLOT(kill()));
}

void QucsMainWindow::slotImportData()
{
    setNormalAction();
    //TODO: implement this or rather port directly
}

void QucsMainWindow::slotShowConsole()
{
    QTermWidget *console = new QTermWidget();
    console->setScrollBarPosition(QTermWidget::ScrollBarRight);

    sidebarDockWidget = new QDockWidget("Console",this);
    sidebarDockWidget->setWidget(console);
    addDockWidget(Qt::BottomDockWidgetArea, sidebarDockWidget);

    connect(console, SIGNAL(finished()), sidebarDockWidget, SLOT(close()));
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

void QucsMainWindow::slotSetMarker(bool on)
{
    performToggleAction(on, 0, action("selectMarker"));
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

void QucsMainWindow::slotZoomIn(bool on)
{
    performToggleAction(on, 0, action("magPlus"));
}

void QucsMainWindow::slotZoomOut(bool on)
{
    performToggleAction(on, 0, action("magMinus"));
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
    QString textEditor = Settings::instance()->currentValue("textEditor").toString();
    QProcess *QucsEditor = new QProcess(this);
    QucsEditor->start(textEditor, arguments);

    //TODO Emit error in case there are problems
    // Kill editor before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsEditor, SLOT(kill()));
}

void QucsMainWindow::showHTML(const QString& Page)
{
    QStringList arguments;
    if(!Page.isEmpty()) {
        arguments << Page;
    }
    QProcess *QucsHelp = new QProcess(this);
    QucsHelp->start(QString(Qucs::binaryDir + "qucshelp"),arguments);

    //TODO Emit error in case there are problems
    // Kill editor before qucs ends
    connect(this, SIGNAL(signalKillWidgets()), QucsHelp, SLOT(kill()));
}

void QucsMainWindow::slotHelpIndex()
{
    setNormalAction();
    showHTML("index.html");
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

void QucsMainWindow::slotInsertItemAction(bool on)
{
    performToggleAction(on, 0, action("insertItem"));
}

void QucsMainWindow::slotPaintingDrawAction(bool on)
{
    performToggleAction(on, 0, action("paintingDraw"));
}

void QucsMainWindow::loadSettings()
{
    QSettings qSettings;

    Settings *settings = Settings::instance();
    settings->load(qSettings);
    const QSize iconSize = settings->currentValue("gui/iconSize").toSize();
    setIconSize(iconSize);

    const QRect geometry = settings->currentValue("gui/geometry").toRect();
    move(geometry.topLeft());
    resize(geometry.size());

    /* Load library database settings */
    QString libpath = qSettings.value("SidebarLibrary", QString()).toString();
    if(libpath.isEmpty()) {
        libpath = QFileDialog::getExistingDirectory(0, tr("Component database tree"),
                QDir::homePath(),
                QFileDialog::ShowDirsOnly);
        if(libpath.isEmpty()) {
            QMessageBox::warning(0, "No sidebar library", "Sidebar won't have any library"
                    "as you haven't selected any!");
            return;
        }

        // Ensure libpath always ends with separator as other subpaths are
        // built by appending to libpath.
        libpath = QDir::toNativeSeparators(libpath);
        if(!libpath.endsWith(QDir::separator())) {
            libpath.append(QDir::separator());
        }
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

    LibraryLoader *library = LibraryLoader::defaultInstance();


    if(library->loadtree(libpath)) {
        qSettings.setValue("SidebarLibrary", libpath);
        qDebug() << "Succesfully loaded library!";
    }
    else {
        //invalidate entry.
        qWarning() << "QucsMainWindow::loadSettings() : Entry is invalid. Run once more to set"
                   << "the appropriate path.";
        qSettings.setValue("SidebarLibrary", "");
        return;
    }

    m_componentsSidebar->plugLibrary(libpath + "/components/basic/passive.xpro", "Components");

    //Next we restore qucsmainwindow docks positions
    this->restoreState(qSettings.value("DocksPositions").toByteArray());
}

void QucsMainWindow::saveSettings()
{
    QSettings qSettings;

    Settings *settings = Settings::instance();
    settings->save(qSettings);

    //Now we save qucsmainwindow docks positions
    qSettings.setValue("DocksPositions", this->saveState());
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

void QucsMainWindow::slotSidebarItemClicked(const QString& item, const QString& category)
{
    if(tabWidget()->count() > 0){
        SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
        SchematicScene *scene = view->schematicScene();
        if(view && scene->sidebarItemClicked(item, category)) {
            if(scene->currentMouseAction() == SchematicScene::InsertingItems) {
                slotInsertItemAction(true);
            }
            else if(scene->currentMouseAction() == SchematicScene::PaintingDrawEvent) {
                slotPaintingDrawAction(true);
            }
        }
    }
}

void QucsMainWindow::slotUpdateAllViews()
{
    for (int i = 0; i < tabWidget()->count(); ++i) {
        SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->widget(i));
        if (view) {
            if (view->scene()) {
                view->scene()->update();
            }
            view->update();
        }
    }
}

void QucsMainWindow::slotUpdateCursorPositionStatus(const QString& newPos)
{
    m_cursorLabel->setText(newPos);
}

void QucsMainWindow::resetCurrentSceneState()
{
    SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
    if(view) {
        view->resetState();
    }
}
