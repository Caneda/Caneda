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

#include "qucs-tools/global.h"
#include "componentssidebar.h"
#include "qucsmainwindow.h"
#include "schematicview.h"
#include "schematicscene.h"
#include "qucsview.h"
#include "item.h"
#include "library.h"
#include "xmlutilities/validators.h"
#include "dialogs/qucssettingsdialog.h"

#include <QtGui/QStatusBar>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QIcon>
#include <QtGui/QMenuBar>
#include <QtGui/QUndoGroup>
#include <QtGui/QUndoView>
#include <QtGui/QApplication>
#include <QtGui/QDockWidget>
#include <QtGui/QWhatsThis>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtCore/QDebug>
#include <QtGui/QFileDialog>
#include <QtGui/QGraphicsItem>
#include <QtCore/QTimer>

static QString qucsFilter;


QucsMainWindow::QucsMainWindow(QWidget *w) : MainWindowBase(w),
                                             titleText(QString("Qucs ") + (Qucs::version) + QString(" : %1[*]"))
{
   setObjectName("QucsMainWindow"); //for debugging purpose

   setDocumentTitle("Untitled");

   qucsFilter =
      tr("Schematic-xml")+" (*.xsch);;"+
      tr("Schematic")+" (*.sch);;"+
      tr("Data Display")+" (*.dpl);;"+
      tr("Qucs Documents")+" (*.sch *.dpl);;"+
      tr("VHDL Sources")+" (*.vhdl *.vhd);;"+
      tr("Any File")+" (*)";

   m_undoGroup = new QUndoGroup();

   statusBar()->show();
   initActions();
   initMenus();
   initToolBars();

   setupSidebar();


   connect(this, SIGNAL(currentWidgetChanged(QWidget*, QWidget*)), this,
           SLOT(slotCurrentChanged(QWidget*, QWidget*)));
   connect(this, SIGNAL(closedWidget(QWidget*)), this,
           SLOT(slotViewClosed(QWidget*)));

   SchematicView *view = new SchematicView(0, this);
   addSchematicView(view);
   m_undoGroup->setActiveStack(view->schematicScene()->undoStack());
   loadSettings();
}

void QucsMainWindow::setupSidebar()
{
   m_componentsSidebar = new ComponentsSidebar(this);
   connect(m_componentsSidebar, SIGNAL(itemClicked(const QString&, const QString&)), this,
           SLOT(slotSidebarItemClicked(const QString&, const QString&)));
   addAsDockWidget(m_componentsSidebar, m_componentsSidebar->windowTitle());

   QList<QPair<QString, QPixmap> > paintingItems;
   paintingItems << qMakePair(QObject::tr("Line"), Qucs::pixmapFor("line"));
   paintingItems << qMakePair(QObject::tr("Arrow"), Qucs::pixmapFor("arrow"));
   paintingItems << qMakePair(QObject::tr("Text"), Qucs::pixmapFor("text"));
   paintingItems << qMakePair(QObject::tr("Ellipse"), Qucs::pixmapFor("ellipse"));
   paintingItems << qMakePair(QObject::tr("Rectangle"), Qucs::pixmapFor("rectangle"));
   paintingItems << qMakePair(QObject::tr("Elliptic Arc"), Qucs::pixmapFor("ellipsearc"));

   m_componentsSidebar->plugItems(paintingItems, QObject::tr("paintings"));
}

void QucsMainWindow::test()
{
   SchematicView *v = qobject_cast<SchematicView*>(currentWidget());
   if(v)
      v->test();
}

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
   connect( action, SIGNAL(triggered()), SLOT(slotFileNew()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "textnew.png"), tr("New &Text"), this);
   action->setShortcut(CTRL+SHIFT+Key_V);
   action->setStatusTip(tr("Creates a new text document"));
   action->setWhatsThis(tr("New Text\n\nCreates a new text document"));
   action->setObjectName("textNew");
   connect( action, SIGNAL(triggered()), SLOT(slotTextNew()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "fileopen.png"), tr("&Open..."), this);
   action->setShortcut(CTRL+Key_O);
   action->setStatusTip(tr("Opens an existing document"));
   action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
   action->setObjectName("fileOpen");
   connect( action, SIGNAL(triggered()), SLOT(slotFileOpen()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "filesave.png"), tr("&Save"), this);
   action->setShortcut(CTRL+Key_S);
   action->setStatusTip(tr("Saves the current document"));
   action->setWhatsThis(tr("Save File\n\nSaves the current document"));
   action->setObjectName("fileSave");
   connect( action, SIGNAL(triggered()), SLOT(slotFileSave()));
   addActionToMap(action);

   action = new QAction( tr("Save as..."), this);
   action->setShortcut(CTRL+Key_Minus);
   action->setStatusTip(tr("Saves the current document under a new filename"));
   action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
   action->setObjectName("fileSaveAs");
   connect( action, SIGNAL(triggered()), SLOT(slotFileSaveAs()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "filesaveall.png"), tr("Save &All"), this);
   action->setShortcut(CTRL+Key_Plus);
   action->setStatusTip(tr("Saves all open documents"));
   action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
   action->setObjectName("fileSaveAll");
   connect( action, SIGNAL(triggered()), SLOT(slotFileSaveAll()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "fileclose.png"), tr("&Close"), this);
   action->setShortcut(CTRL+Key_W);
   action->setStatusTip(tr("Closes the current document"));
   action->setWhatsThis(tr("Close File\n\nCloses the current document"));
   action->setObjectName("fileClose");
   connect( action, SIGNAL(triggered()), SLOT(slotFileClose()));
   addActionToMap(action);

   action = new QAction( tr("&Edit Circuit Symbol"), this);
   action->setShortcut(Key_F9);
   action->setStatusTip(tr("Edits the symbol for this schematic"));
   action->setWhatsThis(tr("Edit Circuit Symbol\n\nEdits the symbol for this schematic"));
   action->setObjectName("symEdit");
   connect( action, SIGNAL(triggered()), SLOT(slotSymbolEdit()));
   addActionToMap(action);

   action = new QAction( tr("&Document Settings..."), this);
   action->setShortcut(CTRL+Key_Period);
   action->setWhatsThis(tr("Settings\n\nSets properties of the file"));
   action->setObjectName("fileSettings");
   connect( action, SIGNAL(triggered()), SLOT(slotFileSettings()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "fileprint.png"), tr("&Print..."), this);
   action->setShortcut(CTRL+Key_P);
   action->setStatusTip(tr("Prints the current document"));
   action->setWhatsThis(tr("Print File\n\nPrints the current document"));
   action->setObjectName("filePrint");
   connect( action, SIGNAL(triggered()), SLOT(slotFilePrint()));
   addActionToMap(action);

   action = new QAction( tr("Print Fit to Page..."), this);
   action->setShortcut(CTRL+SHIFT+Key_P);
   action->setWhatsThis(tr("Print Fit to Page\n\n""Print and fit content to the page size"));
   action->setObjectName("filePrintFit");
   connect( action, SIGNAL(triggered()), SLOT(slotFilePrintFit()));
   addActionToMap(action);

   action = new QAction( tr("E&xit"), this);
   action->setShortcut(CTRL+Key_Q);
   action->setStatusTip(tr("Quits the application"));
   action->setWhatsThis(tr("Exit\n\nQuits the application"));
   action->setObjectName("fileQuit");
   connect( action, SIGNAL(triggered()), SLOT(slotFileQuit()));
   addActionToMap(action);

   action = new QAction( tr("Application Settings..."), this);
   action->setShortcut(CTRL+Key_Comma);
   action->setWhatsThis(tr("Qucs Settings\n\nSets properties of the application"));
   action->setObjectName("applSettings");
   connect( action, SIGNAL(triggered()), SLOT(slotApplSettings()));
   addActionToMap(action);

   action = new QAction( tr("Align top"), this);
   action->setShortcut(CTRL+Key_T);
   action->setStatusTip(tr("Align top selected elements"));
   action->setWhatsThis(tr("Align top\n\nAlign selected elements to their upper edge"));
   action->setObjectName("alignTop");
   connect( action, SIGNAL(triggered()), SLOT(slotAlignTop()));
   addActionToMap(action);

   action = new QAction( tr("Align bottom"), this);
   action->setStatusTip(tr("Align bottom selected elements"));
   action->setWhatsThis(tr("Align bottom\n\nAlign selected elements to their lower edge"));
   action->setObjectName("alignBottom");
   connect( action, SIGNAL(triggered()), SLOT(slotAlignBottom()));
   addActionToMap(action);

   action = new QAction( tr("Align left"), this);
   action->setStatusTip(tr("Align left selected elements"));
   action->setWhatsThis(tr("Align left\n\nAlign selected elements to their left edge"));
   action->setObjectName("alignLeft");
   connect( action, SIGNAL(triggered()), SLOT(slotAlignLeft()));
   addActionToMap(action);

   action = new QAction( tr("Align right"), this);
   action->setStatusTip(tr("Align right selected elements"));
   action->setWhatsThis(tr("Align right\n\nAlign selected elements to their right edge"));
   action->setObjectName("alignRight");
   connect( action, SIGNAL(triggered()), SLOT(slotAlignRight()));
   addActionToMap(action);

   action = new QAction( tr("Distribute horizontally"), this);
   action->setStatusTip(tr("Distribute equally horizontally"));
   action->setWhatsThis(tr("Distribute horizontally\n\n""Distribute horizontally selected elements"));
   action->setObjectName("distrHor");
   connect( action, SIGNAL(triggered()), SLOT(slotDistribHoriz()));
   addActionToMap(action);

   action = new QAction( tr("Distribute vertically"), this);
   action->setStatusTip(tr("Distribute equally vertically"));
   action->setWhatsThis(tr("Distribute vertically\n\n""Distribute vertically selected elements"));
   action->setObjectName("distrVert");
   connect( action, SIGNAL(triggered()), SLOT(slotDistribVert()));
   addActionToMap(action);

   action = new QAction(tr("Center horizontally"), this);
   action->setStatusTip(tr("Center horizontally selected elements"));
   action->setWhatsThis(tr("Center horizontally\n\nCenter horizontally selected elements"));
   action->setObjectName("centerHor");
   connect(action, SIGNAL(triggered()), SLOT(slotCenterHorizontal()));
   addActionToMap(action);

   action = new QAction(tr("Center vertically"), this);
   action->setStatusTip(tr("Center vertically selected elements"));
   action->setWhatsThis(tr("Center vertically\n\nCenter vertically selected elements"));
   action->setObjectName("centerVert");
   connect(action, SIGNAL(triggered()), SLOT(slotCenterVertical()));
   addActionToMap(action);

   action = new QAction( tr("Set on Grid"), this);
   action->setShortcut(CTRL+Key_U);
   action->setWhatsThis(tr("Set on Grid\n\nSets selected elements on grid"));
   action->setObjectName("onGrid");
   action->setCheckable(true);
   action->setData(QVariant(SchematicScene::SettingOnGrid));
   connect( action, SIGNAL(toggled(bool)), SLOT(slotOnGrid(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction( tr("Replace..."), this);
   action->setShortcut(Key_F7);
   action->setWhatsThis(tr("Replace\n\nChange component properties\nor\ntext in VHDL code"));
   action->setObjectName("changeProps");
   connect( action, SIGNAL(triggered()), SLOT(slotChangeProps()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "editcut.png"), tr("Cu&t"), this);
   action->setShortcut(CTRL+Key_X);
   action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
   action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
   action->setObjectName("editCut");
   connect( action, SIGNAL(triggered()), SLOT(slotEditCut()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "editcopy.png"), tr("&Copy"), this);
   action->setShortcut(CTRL+Key_C);
   action->setStatusTip(tr("Copies the selection into the clipboard"));
   action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
   action->setObjectName("editCopy");
   connect( action, SIGNAL(triggered()), SLOT(slotEditCopy()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "editpaste.png"), tr("&Paste"), this);
   action->setShortcut(CTRL+Key_V);
   action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
   action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
   action->setObjectName("editPaste");
   connect( action, SIGNAL(triggered()), SLOT(slotEditPaste()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "editdelete.png"), tr("&Delete"), this);
   action->setShortcut(Key_Delete);
   action->setStatusTip(tr("Deletes the selected components"));
   action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
   action->setObjectName("editDelete");
   action->setData(QVariant(SchematicScene::Deleting));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotEditDelete(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction( tr("Find..."), this);
   action->setShortcut(CTRL+Key_F);
   action->setStatusTip(tr("Find a piece of text"));
   action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
   action->setObjectName("editFind");
   connect( action, SIGNAL(triggered()), SLOT(slotEditFind()));
   addActionToMap(action);

   action = new QAction( tr("Find Again"), this);
   action->setShortcut(Key_F3);
   action->setStatusTip(tr("Find same text again"));
   action->setWhatsThis(tr("Find\n\nSearches for the same piece of text again"));
   action->setObjectName("editFindAgain");
   connect( action, SIGNAL(triggered()), SLOT(slotEditFindAgain()));
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
   action->setShortcut(CTRL+Key_Y);
   action->setStatusTip(tr("Redoes the last command"));
   action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
   action->setObjectName("redo");
   addActionToMap(action);

   action = new QAction( tr("&New Project..."), this);
   action->setShortcut(CTRL+SHIFT+Key_N);
   action->setStatusTip(tr("Creates a new project"));
   action->setWhatsThis(tr("New Project\n\nCreates a new project"));
   action->setObjectName("projNew");
   connect( action, SIGNAL(triggered()), SLOT(slotProjNewButt()));
   addActionToMap(action);

   action = new QAction( tr("&Open Project..."), this);
   action->setShortcut(CTRL+SHIFT+Key_O);
   action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
   action->setObjectName("projOpen");
   connect( action, SIGNAL(triggered()), SLOT(slotMenuOpenProject()));
   addActionToMap(action);

   action = new QAction( tr("&Delete Project..."), this);
   action->setShortcut(CTRL+SHIFT+Key_D);
   action->setWhatsThis(tr("Delete Project\n\nDeletes an existing project"));
   action->setObjectName("projDel");
   connect( action, SIGNAL(triggered()), SLOT(slotMenuDelProject()));
   addActionToMap(action);

   action = new QAction( tr("&Close Project"), this);
   action->setShortcut(CTRL+SHIFT+Key_W);
   action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
   action->setObjectName("projClose");
   connect( action, SIGNAL(triggered()), SLOT(slotMenuCloseProject()));
   addActionToMap(action);

   action = new QAction( tr("&Add Files to Project..."), this);
   action->setShortcut(CTRL+SHIFT+Key_A);
   action->setStatusTip(tr("Copies files to project directory"));
   action->setWhatsThis(tr("Add Files to Project\n\nCopies files to project directory"));
   action->setObjectName("addToProj");
   connect( action, SIGNAL(triggered()), SLOT(slotAddToProject()));
   addActionToMap(action);

   action = new QAction( tr("Create &Library..."), this);
   action->setShortcut(CTRL+SHIFT+Key_L);
   action->setStatusTip(tr("Create Library from Subcircuits"));
   action->setWhatsThis(tr("Create Library\n\nCreate Library from Subcircuits"));
   action->setObjectName("createLib");
   connect( action, SIGNAL(triggered()), SLOT(slotCreateLib()));
   addActionToMap(action);

   action = new QAction( tr("Create &Package..."), this);
   action->setShortcut(CTRL+SHIFT+Key_Z);
   action->setStatusTip(tr("Create compressed Package from Projects"));
   action->setWhatsThis(tr("Create Package\n\nCreate compressed Package from complete Projects"));
   action->setObjectName("createPkg");
   connect( action, SIGNAL(triggered()), SLOT(slotCreatePackage()));
   addActionToMap(action);

   action = new QAction( tr("E&xtract Package..."), this);
   action->setShortcut(CTRL+SHIFT+Key_X);
   action->setStatusTip(tr("Install Content of a Package"));
   action->setWhatsThis(tr("Extract Package\n\nInstall Content of a Package"));
   action->setObjectName("extractPkg");
   connect( action, SIGNAL(triggered()), SLOT(slotExtractPackage()));
   addActionToMap(action);

   action = new QAction( tr("&Import Data..."), this);
   action->setShortcut(CTRL+SHIFT+Key_I);
   action->setStatusTip(tr("Convert file to Qucs data file"));
   action->setWhatsThis(tr("Import Data\n\nConvert data file to Qucs data file"));
   action->setObjectName("importData");
   connect( action, SIGNAL(triggered()), SLOT(slotImportData()));
   addActionToMap(action);

   action = new QAction( tr("Export to &CSV..."), this);
   action->setShortcut(CTRL+SHIFT+Key_C);
   action->setStatusTip(tr("Convert graph data to CSV file"));
   action->setWhatsThis(tr("Export to CSV\n\nConvert graph data to CSV file"));
   action->setObjectName("graph2csv");
   connect( action, SIGNAL(triggered()), SLOT(slotExportGraphAsCsv()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "viewmagfit.png"), tr("View All"), this);
   action->setShortcut(Key_0);
   action->setStatusTip(tr("Show the whole page"));
   action->setWhatsThis(tr("View All\n\nShows the whole page content"));
   action->setObjectName("magAll");
   connect( action, SIGNAL(triggered()), SLOT(slotShowAll()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "viewmag1.png"), tr("View 1:1"), this);
   action->setShortcut(Key_1);
   action->setStatusTip(tr("Views without magnification"));
   action->setWhatsThis(tr("View 1:1\n\nShows the page content without magnification"));
   action->setObjectName("magOne");
   connect( action, SIGNAL(triggered()), SLOT(slotShowOne()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "viewmag+.png"), tr("Zoom in"), this);
   action->setShortcut(Key_Plus);
   action->setStatusTip(tr("Zooms into the current view"));
   action->setWhatsThis(tr("Zoom in\n\nZooms the current view"));
   action->setObjectName("magPlus");
   action->setData(QVariant(SchematicScene::ZoomingAtPoint));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotZoomIn(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction(QIcon(bitmapPath + "viewmag-.png"), tr("Zoom out"), this);
   action->setShortcut(Key_Minus);
   action->setStatusTip(tr("Zooms out the current view"));
   action->setWhatsThis(tr("Zoom out\n\nZooms out the current view"));
   action->setObjectName("magMinus");
   connect( action, SIGNAL(triggered()), SLOT(slotZoomOut()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "pointer.png"), tr("Select"), this);
   action->setShortcut(Key_Escape);
   action->setStatusTip(tr("Activate select mode"));
   action->setWhatsThis(tr("Select\n\nActivates select mode"));
   action->setObjectName("select");
   action->setData(QVariant(SchematicScene::Normal));
   action->setCheckable(true);
   action->setChecked(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotSelect(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction( tr("Select All"), this);
   action->setShortcut(CTRL+Key_A);
   action->setStatusTip(tr("Selects all elements"));
   action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
   action->setObjectName("selectAll");
   connect( action, SIGNAL(triggered()), SLOT(slotSelectAll()));
   addActionToMap(action);

   action = new QAction( tr("Select Markers"), this);
   action->setShortcut(CTRL+SHIFT+Key_M);
   action->setStatusTip(tr("Selects all markers"));
   action->setWhatsThis(tr("Select Markers\n\nSelects all diagram markers of the document"));
   action->setObjectName("selectMarker");
   connect( action, SIGNAL(triggered()), SLOT(slotSelectMarker()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "rotate_ccw.png"), tr("Rotate"), this);
   action->setShortcut(CTRL+Key_R);
   action->setStatusTip(tr("Rotates the selected component by 90°"));
   action->setWhatsThis(tr("Rotate\n\nRotates the selected component by 90° counter-clockwise"));
   action->setObjectName("editRotate");
   action->setData(QVariant(SchematicScene::Rotating));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotEditRotate(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction(QIcon(bitmapPath + "mirror.png"), tr("Mirror about X Axis"), this);
   action->setShortcut(CTRL+Key_J);
   action->setWhatsThis(tr("Mirror about X Axis\n\nMirrors the selected item about X Axis"));
   action->setObjectName("editMirror");
   action->setData(QVariant(SchematicScene::MirroringX));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotEditMirrorX(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction(QIcon(bitmapPath + "mirrory.png"), tr("Mirror about Y Axis"), this);
   action->setShortcut(CTRL+Key_M);
   action->setWhatsThis(tr("Mirror about Y Axis\n\nMirrors the selected item about Y Axis"));
   action->setObjectName("editMirrorY");
   action->setData(QVariant(SchematicScene::MirroringY));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotEditMirrorY(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction(QIcon(bitmapPath + "bottom.png"), tr("Go into Subcircuit"), this);
   action->setShortcut(CTRL+Key_I);
   action->setWhatsThis(tr("Go into Subcircuit\n\nGoes inside the selected subcircuit"));
   action->setObjectName("intoH");
   connect( action, SIGNAL(triggered()), SLOT(slotIntoHierarchy()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "top.png"), tr("Pop out"), this);
   action->setShortcut(CTRL+Key_H);
   action->setStatusTip(tr("Pop outside subcircuit"));
   action->setWhatsThis(tr("Pop out\n\nGoes up one hierarchy level, i.e. leaves subcircuit"));
   action->setObjectName("popH");
   connect( action, SIGNAL(triggered()), SLOT(slotPopHierarchy()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "deactiv.png"), tr("Deactivate/Activate"), this);
   action->setShortcut(CTRL+Key_D);
   action->setStatusTip(tr("Deactivate/Activate selected components"));
   action->setWhatsThis(tr("Deactivate/Activate\n\nDeactivate/Activate the selected components"));
   action->setObjectName("editActivate");
   action->setData(QVariant(SchematicScene::ChangingActiveStatus));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotEditActivate(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction(QIcon(bitmapPath + "equation.png"), tr("Insert Equation"), this);
   action->setShortcut(CTRL+Key_Less);
   action->setWhatsThis(tr("Insert Equation\n\nInserts a user defined equation"));
   action->setObjectName("insEquation");
   connect(action, SIGNAL(triggered()), SLOT(slotInsertEquation()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "ground.png"), tr("Insert Ground"), this);
   action->setShortcut(CTRL+Key_G);
   action->setWhatsThis(tr("Insert Ground\n\nInserts a ground symbol"));
   action->setObjectName("insGround");
   connect(action, SIGNAL(triggered()), SLOT(slotInsertGround()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "port.png"), tr("Insert Port"), this);
   action->setWhatsThis(tr("Insert Port\n\nInserts a port symbol"));
   action->setObjectName("insPort");
   connect( action, SIGNAL(triggered()), SLOT(slotInsertPort()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "wire.png"), tr("Wire"), this);
   action->setShortcut(CTRL+Key_E);
   action->setWhatsThis(tr("Wire\n\nInserts a wire"));
   action->setObjectName("insWire");
   action->setData(QVariant(SchematicScene::Wiring));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotSetWire(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction(QIcon(bitmapPath + "nodename.png"), tr("Wire Label"), this);
   action->setShortcut(CTRL+Key_L);
   action->setStatusTip(tr("Inserts a wire or pin label"));
   action->setWhatsThis(tr("Wire Label\n\nInserts a wire or pin label"));
   action->setObjectName("insLabel");
   action->setData(QVariant(SchematicScene::InsertingWireLabel));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotInsertLabel(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction( tr("VHDL entity"), this);
   action->setShortcut(CTRL+Key_Space);
   action->setStatusTip(tr("Inserts skeleton of VHDL entity"));
   action->setWhatsThis(tr("VHDL entity\n\nInserts the skeleton of a VHDL entity"));
   action->setObjectName("insEntity");
   connect( action, SIGNAL(triggered()), SLOT(slotInsertEntity()));
   addActionToMap(action);

   action = new QAction( tr("Text Editor"), this);
   action->setShortcut(CTRL+Key_1);
   action->setStatusTip(tr("Starts the Qucs text editor"));
   action->setWhatsThis(tr("Text editor\n\nStarts the Qucs text editor"));
   action->setObjectName("callEditor");
   connect( action, SIGNAL(triggered()), SLOT(slotCallEditor()));
   addActionToMap(action);

   action = new QAction( tr("Filter synthesis"), this);
   action->setShortcut(CTRL+Key_2);
   action->setStatusTip(tr("Starts QucsFilter"));
   action->setWhatsThis(tr("Filter synthesis\n\nStarts QucsFilter"));
   action->setObjectName("callFilter");
   connect( action, SIGNAL(triggered()), SLOT(slotCallFilter()));
   addActionToMap(action);

   action = new QAction( tr("Line calculation"), this);
   action->setShortcut(CTRL+Key_3);
   action->setStatusTip(tr("Starts QucsTrans"));
   action->setWhatsThis(tr("Line calculation\n\nStarts transmission line calculator"));
   action->setObjectName("callLine");
   connect( action, SIGNAL(triggered()), SLOT(slotCallLine()));
   addActionToMap(action);

   action = new QAction( tr("Component Library"), this);
   action->setShortcut(CTRL+Key_4);
   action->setStatusTip(tr("Starts QucsLib"));
   action->setWhatsThis(tr("Component Library\n\nStarts component library program"));
   action->setObjectName("callLib");
   connect( action, SIGNAL(triggered()), SLOT(slotCallLibrary()));
   addActionToMap(action);

   action = new QAction( tr("Matching Circuit"), this);
   action->setShortcut(CTRL+Key_5);
   action->setStatusTip(tr("Creates Matching Circuit"));
   action->setWhatsThis(tr("Matching Circuit\n\nDialog for Creating Matching Circuit"));
   action->setObjectName("callMatch");
   connect( action, SIGNAL(triggered()), SLOT(slotCallMatch()));
   addActionToMap(action);

   action = new QAction( tr("Attenuator synthesis"), this);
   action->setShortcut(CTRL+Key_6);
   action->setStatusTip(tr("Starts QucsAttenuator"));
   action->setWhatsThis(tr("Attenuator synthesis\n\nStarts attenuator calculation program"));
   action->setObjectName("callAtt");
   connect( action, SIGNAL(triggered()), SLOT(slotCallAtt()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "gear.png"), tr("Simulate"), this);
   action->setShortcut(Key_F2);
   action->setStatusTip(tr("Simulates the current schematic"));
   action->setWhatsThis(tr("Simulate\n\nSimulates the current schematic"));
   action->setObjectName("simulate");
   connect( action, SIGNAL(triggered()), SLOT(slotSimulate()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "rebuild.png"), tr("View Data Display/Schematic"), this);
   action->setShortcut(Key_F4);
   action->setStatusTip(tr("Changes to data display or schematic page"));
   action->setWhatsThis(tr("View Data Display/Schematic\n\n")+tr("Changes to data display or schematic page"));
   action->setObjectName("dpl_sch");
   connect( action, SIGNAL(triggered()), SLOT(slotToPage()));
   addActionToMap(action);

   action = new QAction( tr("Calculate DC bias"), this);
   action->setShortcut(Key_F8);
   action->setStatusTip(tr("Calculates DC bias and shows it"));
   action->setWhatsThis(tr("Calculate DC bias\n\nCalculates DC bias and shows it"));
   action->setObjectName("dcbias");
   connect( action, SIGNAL(triggered()), SLOT(slotDCbias()));
   addActionToMap(action);

   action = new QAction(QIcon(bitmapPath + "marker.png"), tr("Set Marker on Graph"), this);
   action->setShortcut(CTRL+Key_B);
   action->setStatusTip(tr("Sets a marker on a diagram's graph"));
   action->setWhatsThis(tr("Set Marker\n\nSets a marker on a diagram's graph"));
   action->setObjectName("setMarker");
   action->setData(QVariant(SchematicScene::Marking));
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotSetMarker(bool)));
   addActionToMap(action);
   checkableActions << action;

   action = new QAction( tr("Show Last Messages"), this);
   action->setShortcut(Key_F5);
   action->setStatusTip(tr("Shows last simulation messages"));
   action->setWhatsThis(tr("Show Last Messages\n\nShows the messages of the last simulation"));
   action->setObjectName("showMsg");
   connect( action, SIGNAL(triggered()), SLOT(slotShowLastMsg()));
   addActionToMap(action);

   action = new QAction( tr("Show Last Netlist"), this);
   action->setShortcut(Key_F6);
   action->setStatusTip(tr("Shows last simulation netlist"));
   action->setWhatsThis(tr("Show Last Netlist\n\nShows the netlist of the last simulation"));
   action->setObjectName("showNet");
   connect( action, SIGNAL(triggered()), SLOT(slotShowLastNetlist()));
   addActionToMap(action);

   action = new QAction( tr("Tool&bar"), this);
   action->setStatusTip(tr("Enables/disables the toolbar"));
   action->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
   action->setObjectName("viewToolBar");
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotViewToolBar(bool)));
   addActionToMap(action);

   action = new QAction( tr("&Statusbar"), this);
   action->setStatusTip(tr("Enables/disables the statusbar"));
   action->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
   action->setObjectName("viewStatusBar");
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotViewStatusBar(bool)));
   addActionToMap(action);

   action = new QAction( tr("&Dock Window"), this);
   action->setStatusTip(tr("Enables/disables the browse dock window"));
   action->setWhatsThis(tr("Browse Window\n\nEnables/disables the browse dock window"));
   action->setObjectName("viewBrowseDock");
   action->setCheckable(true);
   connect( action, SIGNAL(toggled(bool)), SLOT(slotViewBrowseDock(bool)));
   addActionToMap(action);

   action = new QAction( tr("Help Index..."), this);
   action->setShortcut(Key_F1);
   action->setStatusTip(tr("Index of Qucs Help"));
   action->setWhatsThis(tr("Help Index\n\nIndex of intern Qucs help"));
   action->setObjectName("helpIndex");
   connect( action, SIGNAL(triggered()), SLOT(slotHelpIndex()));
   addActionToMap(action);

   action = new QAction( tr("Getting Started..."), this);
   action->setStatusTip(tr("Getting Started with Qucs"));
   action->setWhatsThis(tr("Getting Started\n\nShort introduction into Qucs"));
   action->setObjectName("helpGetStart");
   connect( action, SIGNAL(triggered()), SLOT(slotGettingStarted()));
   addActionToMap(action);

   action = new QAction( tr("&About Qucs..."), this);
   action->setWhatsThis(tr("About\n\nAbout the application"));
   action->setObjectName("helpAboutApp");
   connect( action, SIGNAL(triggered()), SLOT(slotHelpAbout()));
   addActionToMap(action);

   action = new QAction( tr("About Qt..."), this);
   action->setWhatsThis(tr("About Qt\n\nAbout Qt by Trolltech"));
   action->setObjectName("helpAboutQt");
   connect( action, SIGNAL(triggered()), SLOT(slotHelpAboutQt()));
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
   fileMenu->addAction(action("filePrintFit"));

   fileMenu->addSeparator();

   fileMenu->addAction(action("fileSettings"));
   fileMenu->addAction(action("symEdit"));

   fileMenu->addSeparator();

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
   editMenu->addAction(action("editFindAgain"));
   editMenu->addAction(action("changeProps"));
   editMenu->addAction(action("editRotate"));
   editMenu->addAction(action("editMirror"));
   editMenu->addAction(action("editMirrorY"));
   editMenu->addAction(action("editActivate"));

   editMenu->addSeparator();

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

   insMenu = menuBar()->addMenu(tr("&Insert"));

   insMenu->addAction(action("insWire"));
   insMenu->addAction(action("insLabel"));
   insMenu->addAction(action("insEquation"));
   insMenu->addAction(action("insGround"));
   insMenu->addAction(action("insPort"));
   insMenu->addAction(action("setMarker"));
   insMenu->addAction(action("insEntity"));

   projMenu = menuBar()->addMenu(tr("&Project"));

   projMenu->addAction(action("projNew"));
   projMenu->addAction(action("projOpen"));
   projMenu->addAction(action("addToProj"));
   projMenu->addAction(action("projClose"));
   projMenu->addAction(action("projDel"));

   projMenu->addSeparator();

   projMenu->addAction(action("createLib"));
   projMenu->addAction(action("createPkg"));
   projMenu->addAction(action("extractPkg"));

   projMenu->addSeparator();

   projMenu->addAction(action("importData"));
   projMenu->addAction(action("graph2csv"));

   toolMenu = menuBar()->addMenu(tr("&Tools"));

   toolMenu->addAction(action("callEditor"));
   toolMenu->addAction(action("callFilter"));
   toolMenu->addAction(action("callLine"));
   toolMenu->addAction(action("callLib"));
   toolMenu->addAction(action("callMatch"));
   toolMenu->addAction(action("callAtt"));

   simMenu = menuBar()->addMenu(tr("&Simulation"));

   simMenu->addAction(action("simulate"));
   simMenu->addAction(action("dpl_sch"));
   simMenu->addAction(action("dcbias"));
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
   viewMenu->addAction(action("viewBrowseDock"));

   menuBar()->addSeparator();

   helpMenu = menuBar()->addMenu(tr("&Help"));

   helpMenu->addAction(action("helpIndex"));
   helpMenu->addAction(action("helpGetStart"));
   helpMenu->addAction(action("whatsThis"));

   helpMenu->addSeparator();

   helpMenu->addAction(action("helpAboutApp"));
   helpMenu->addAction(action("helpAboutQt"));

}

void QucsMainWindow::initToolBars()
{
   fileToolbar  = addToolBar(tr("File"));

   fileToolbar->addAction(action("fileNew"));
   fileToolbar->addAction(action("textNew"));
   fileToolbar->addAction(action("fileOpen"));
   fileToolbar->addAction(action("fileSave"));
   fileToolbar->addAction(action("fileSaveAll"));
   fileToolbar->addAction(action("fileClose"));
   fileToolbar->addAction(action("filePrint"));

   editToolbar  = addToolBar(tr("Edit"));

   editToolbar->addAction(action("editCut"));
   editToolbar->addAction(action("editCopy"));
   editToolbar->addAction(action("editPaste"));
   editToolbar->addAction(action("editDelete"));
   editToolbar->addAction(action("undo"));
   editToolbar->addAction(action("redo"));

   viewToolbar  = addToolBar(tr("View"));

   viewToolbar->addAction(action("magAll"));
   viewToolbar->addAction(action("magOne"));
   viewToolbar->addAction(action("magPlus"));
   viewToolbar->addAction(action("magMinus"));

   workToolbar  = addToolBar(tr("Work"));

   workToolbar->addAction(action("select"));
   workToolbar->addAction(action("editActivate"));
   workToolbar->addAction(action("editMirror"));
   workToolbar->addAction(action("editMirrorY"));
   workToolbar->addAction(action("editRotate"));
   workToolbar->addAction(action("intoH"));
   workToolbar->addAction(action("popH"));
   workToolbar->addAction(action("insWire"));
   workToolbar->addAction(action("insLabel"));
   workToolbar->addAction(action("insEquation"));
   workToolbar->addAction(action("insGround"));
   workToolbar->addAction(action("insPort"));
   workToolbar->addAction(action("simulate"));
   workToolbar->addAction(action("dpl_sch"));
   workToolbar->addAction(action("setMarker"));

   workToolbar->addSeparator();

   workToolbar->addAction(action("whatsThis"));
}


void QucsMainWindow::performToggleAction(bool on, pActionFunc func, QAction *action)
{
   SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
   if(!view) {
      //nothing to do since it is not schematic view
      return;
   }

   SchematicScene *scene = view->schematicScene();
   SchematicScene::MouseAction ma = SchematicScene::MouseAction(action->data().toInt());
   QAction *norm = this->action("select");

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

   QList<QGraphicsItem*> selectedItems = scene->selectedItems();

   do {
      if(!(selectedItems.isEmpty() || func == 0)) {
         QList<QucsItem*> funcable = filterItems<QucsItem>(selectedItems);

         if(funcable.isEmpty())
            break;

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

void QucsMainWindow::setNormalAction()
{
   performToggleAction(true, 0, action("select"));
}

void QucsMainWindow::addSchematicView(SchematicView *view)
{
   m_undoGroup->addStack(view->schematicScene()->undoStack());
   addChildWidget(view);
   tabWidget()->setCurrentWidget(view);
}

void QucsMainWindow::slotCurrentChanged(QWidget *current, QWidget *prev)
{
   SchematicView *prevView = qobject_cast<SchematicView*>(prev);
   if(prevView) {
      prevView->disconnect(this);
      prevView->resetState();
   }

   SchematicView *currView = qobject_cast<SchematicView*>(current);
   if(currView) {
      connect(currView, SIGNAL(titleToBeUpdated()), this, SLOT(updateTitleTabText()));
      m_undoGroup->setActiveStack(currView->schematicScene()->undoStack());
      updateTitleTabText();
   }
}

void QucsMainWindow::slotViewClosed(QWidget *widget)
{
   SchematicView *view = qobject_cast<SchematicView*>(widget);
   if(view) {
      m_undoGroup->removeStack(view->schematicScene()->undoStack());
   }
}

void QucsMainWindow::closeEvent( QCloseEvent *e )
{
   saveSettings();
   MainWindowBase::closeEvent(e);
}

void QucsMainWindow::slotFileNew()
{
   setNormalAction();
   addSchematicView(new SchematicView(0, this));
}

void QucsMainWindow::slotTextNew()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotFileOpen()
{
   setNormalAction();
   QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "", qucsFilter);
   if(fileName.isEmpty()) return;

   QucsView *view = viewFromWidget(currentWidget());

   bool canReuseCurrentView = view && view->fileName().isEmpty() && !view->isModified();
   canReuseCurrentView = canReuseCurrentView && view->isSchematicView();

   //open new view if it cant be reused
   if(!canReuseCurrentView) {
      addSchematicView(new SchematicView(0, this));
      view = viewFromWidget(tabWidget()->currentWidget());
   }

   bool b = view->load(fileName);
   if(!b) {
      QMessageBox::critical(0,QObject::tr("Error"),
                            QObject::tr("Cannot load document!\nParse Error"));
      slotFileClose();
   }
}

void QucsMainWindow::slotFileSave()
{
   setNormalAction();
   QucsView* v = viewFromWidget(tabWidget()->currentWidget());
   if(!v) return;
   if(v->fileName().isEmpty())
      return slotFileSaveAs();
   v->save();
}

void QucsMainWindow::slotFileSaveAs()
{
   setNormalAction();
   QucsView* v = viewFromWidget(tabWidget()->currentWidget());
   if(!v) return;
   QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "", qucsFilter);
   if(fileName.isEmpty()) return;
   v->setFileName(fileName);
   slotFileSave();
}

void QucsMainWindow::slotFileSaveAll()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotFileClose()
{
   setNormalAction();
   //TODO: Verify if page document is modified
   closeCurrentTab();
}

void QucsMainWindow::slotSymbolEdit()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotFileSettings()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotFilePrint()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotFilePrintFit()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotFileQuit()
{
   setNormalAction();
   close();
}

void QucsMainWindow::slotApplSettings()
{
   setNormalAction();
   QucsSettingsDialog *d = new QucsSettingsDialog(this);
   d->exec();
}

void QucsMainWindow::slotAlignTop()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotAlignBottom()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotAlignLeft()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotAlignRight()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotDistribHoriz()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotDistribVert()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCenterHorizontal()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCenterVertical()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotOnGrid(bool on)
{
   performToggleAction(on, &SchematicScene::setItemsOnGrid, action("onGrid"));
}

void QucsMainWindow::slotChangeProps()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotEditCut()
{
   setNormalAction();
   QucsView* v = viewFromWidget(tabWidget()->currentWidget());
   if(!v) return;
   v->cut();
}

void QucsMainWindow::slotEditCopy()
{
   setNormalAction();
   QucsView* v = viewFromWidget(tabWidget()->currentWidget());
   if(!v) return;
   v->copy();
}

void QucsMainWindow::slotEditPaste()
{
   setNormalAction();
   QucsView* v = viewFromWidget(tabWidget()->currentWidget());
   if(!v) return;
   slotInsertItemAction(true);
   v->paste();
}

void QucsMainWindow::slotEditDelete(bool on)
{
   performToggleAction(on, &SchematicScene::deleteItems, action("editDelete"));
}

void QucsMainWindow::slotEditFind()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotEditFindAgain()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotEditUndo()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotEditRedo()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotProjNewButt()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotMenuOpenProject()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotMenuDelProject()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotMenuCloseProject()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotAddToProject()
{
   setNormalAction();
   //TODO: implement this or rather port directly
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

void QucsMainWindow::slotImportData()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotExportGraphAsCsv()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotShowAll()
{
   setNormalAction();
   QucsView *view = viewFromWidget(tabWidget()->currentWidget());
   if(view)
      view->showAll();
}

void QucsMainWindow::slotShowOne()
{
   setNormalAction();
   QucsView *view = viewFromWidget(tabWidget()->currentWidget());
   if(view)
      view->showNoZoom();
}

void QucsMainWindow::slotZoomIn(bool on)
{
   performToggleAction(on, 0, action("magPlus"));
}

void QucsMainWindow::slotZoomOut()
{
   setNormalAction();
   QucsView *view = viewFromWidget(tabWidget()->currentWidget());
   if(view)
      view->zoomOut();
}

void QucsMainWindow::slotSelect(bool on)
{
   performToggleAction(on, 0, action("select"));
}

void QucsMainWindow::slotSelectAll()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotSelectMarker()
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

void QucsMainWindow::slotEditActivate(bool on)
{
   performToggleAction(on, &SchematicScene::toggleActiveStatus, action("editActivate"));
}

void QucsMainWindow::slotInsertEquation()
{
}

void QucsMainWindow::slotInsertGround()
{
   slotSidebarItemClicked("Ground", "passive");
}

void QucsMainWindow::slotInsertPort()
{
}

void QucsMainWindow::slotSetWire(bool on)
{
   performToggleAction(on, 0, action("insWire"));
}

void QucsMainWindow::slotInsertLabel(bool on)
{
   performToggleAction(on, 0, action("insLabel"));
}

void QucsMainWindow::slotInsertEntity()
{
   setNormalAction();
}

void QucsMainWindow::slotCallEditor()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCallFilter()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCallLine()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCallLibrary()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCallMatch()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotCallAtt()
{
   setNormalAction();
   //TODO: implement this or rather port directly
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

void QucsMainWindow::slotShowLastMsg()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotShowLastNetlist()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotViewToolBar(bool)
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotViewStatusBar(bool)
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotViewBrowseDock(bool)
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotHelpIndex()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotGettingStarted()
{
   setNormalAction();
   //TODO: implement this or rather port directly
}

void QucsMainWindow::slotHelpAbout()
{
   setNormalAction();
   //TODO: implement this or rather port directly
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
   Qucs::Settings settings("qucs-qt4rc");

   settings.beginGroup("MainWindow");
   resize(settings.value("size", QSize(600, 400)).toSize());
   move(settings.value("pos", QPoint(0, 0)).toPoint());
   maxUndo = settings.value("undo", 20).toInt();
   largeFontSize = settings.value("largefontsize", 16.0).toDouble();
   Editor = settings.value("editor", Qucs::binaryDir + "qucsedit").toString();
   Language = settings.value("language", "").toString();
   savingFont = settings.value("font", "Helvetica,12").toString();
   settings.endGroup();

   settings.beginGroup("Colors");
   BGColor.setNamedColor(
     settings.value("bgcolor", QColor(255,250,225).name()).toString());
   settings.beginGroup("VHDL");
   VHDL_Comment.setNamedColor(
     settings.value("comment", QColor(Qt::gray).name()).toString());
   VHDL_String.setNamedColor(
     settings.value("string", QColor(Qt::red).name()).toString());
   VHDL_Integer.setNamedColor(
     settings.value("integer", QColor(Qt::blue).name()).toString());
   VHDL_Real.setNamedColor(
     settings.value("real", QColor(Qt::darkMagenta).name()).toString());
   VHDL_Character.setNamedColor(
     settings.value("character", QColor(Qt::magenta).name()).toString());
   VHDL_Types.setNamedColor(
     settings.value("types", QColor(Qt::darkRed).name()).toString());
   VHDL_Attributes.setNamedColor(
      settings.value("attributes", QColor(Qt::darkCyan).name()).toString());
   settings.endGroup();
   settings.endGroup();

   settings.beginGroup("FileTypes");
   FileTypes.clear();
   QStringList f = settings.childKeys();
   QStringList::Iterator it = f.begin();
   while(it != f.end()) {
     FileTypes.append((*it)+"/"+settings.value(*it, "").toString());
     it++;
   }
   settings.endGroup();

   /* Load library database settings */
   QString libpath = settings.value("SidebarLibrary", QString()).toString();
   if(libpath.isEmpty()) {
      libpath = QFileDialog::getExistingDirectory(0, tr("Component database tree"),
                                                 QDir::homePath(),
                                                 QFileDialog::ShowDirsOnly);
      if(libpath.isEmpty()) {
         QMessageBox::warning(0, "No sidebar library", "Sidebar won't have any library"
                              "as you haven't selected any!");
         return;
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

   LibraryLoader *library = LibraryLoader::defaultInstance();
   

   if(library->loadtree(libpath)) {
      settings.setValue("SidebarLibrary", libpath);
      qDebug() << "Succesfully loaded library!";
   }
   else {
      //invalidate entry.
      qWarning() << "QucsMainWindow::loadSettings() : Entry is invalid. Run once more to set"
                 << "the appropriate path.";
      settings.setValue("SidebarLibrary", "");
      return;
   }

   m_componentsSidebar->plugLibrary("passive");
   test();
}

void QucsMainWindow::saveSettings()
{
   Qucs::Settings settings("qucs-qt4rc");

   settings.beginGroup("MainWindow");
   settings.setValue("size", size());
   settings.setValue("pos", pos());
   settings.setValue("undo", maxUndo);
   settings.setValue("editor", Editor);
   settings.setValue("largefontsize", largeFontSize);
   settings.setValue("font", savingFont);
   if(Language.isEmpty())
     settings.remove("language");
   else
     settings.setValue("language", Language);
   settings.endGroup();

   settings.beginGroup("Colors");
   settings.setValue("bgcolor", BGColor.name());
   settings.beginGroup("VHDL");
   settings.setValue("comment", VHDL_Comment.name());
   settings.setValue("string", VHDL_String.name());
   settings.setValue("integer", VHDL_Integer.name());
   settings.setValue("real", VHDL_Real.name());
   settings.setValue("character", VHDL_Character.name());
   settings.setValue("types", VHDL_Types.name());
   settings.setValue("attributes", VHDL_Attributes.name());
   settings.endGroup();
   settings.endGroup();

   settings.beginGroup("FileTypes");
   QStringList::Iterator it = FileTypes.begin();
   while(it != FileTypes.end()) {
     settings.setValue((*it).section('/',0,0), (*it).section('/',1));
    it++;
   }
   settings.endGroup();
}

void QucsMainWindow::setTabTitle(const QString& title)
{
   SchematicView *view = qobject_cast<SchematicView*>(sender());
   if(!view || title.isEmpty())
      return;
   int index = tabWidget()->indexOf(view);
   if(index != -1)
      tabWidget()->setTabText(index,title);
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
   SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
   SchematicScene *scene = view->schematicScene();
   if(view && scene->sidebarItemClicked(item, category)) {
      if(scene->currentMouseAction() == SchematicScene::InsertingItems)
         slotInsertItemAction(true);
      else if(scene->currentMouseAction() == SchematicScene::PaintingDrawEvent)
         slotPaintingDrawAction(true);
   }
}

void QucsMainWindow::resetCurrentSceneState()
{
   SchematicView *view = qobject_cast<SchematicView*>(tabWidget()->currentWidget());
   if(view) {
      view->resetState();
   }
}
