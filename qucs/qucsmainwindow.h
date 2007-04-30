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

#ifndef __QUCSMAINWINDOW_H
#define __QUCSMAINWINDOW_H

#include "ideality/dtabbedmainwindow.h"
#include <QtCore/QMap>
#include <QtGui/QToolBar>
#include <QtGui/QMenu>

class ComponentsSidebar;
class QUndoGroup;
class SchematicView;

class QucsMainWindow : public DTabbedMainWindow
{
      Q_OBJECT;
   public:
      QucsMainWindow(QWidget *w=0);
      ~QucsMainWindow() {}

      void addView(SchematicView *view);
      void saveSettings();

   public slots:
      void slotFileNew();
      void slotTextNew();
      void slotFileOpen();
      void slotFileSave();
      void slotFileSaveAs();
      void slotFileSaveAll();
      void slotFileClose();
      void slotSymbolEdit();
      void slotFileSettings();
      void slotFilePrint();
      void slotFilePrintFit();
      void slotFileQuit();
      void slotApplSettings();
      void slotAlignTop();
      void slotAlignBottom();
      void slotAlignLeft();
      void slotAlignRight();
      void slotDistribHoriz();
      void slotDistribVert();
      void slotCenterHorizontal();
      void slotCenterVertical();
      void slotOnGrid(bool);
      void slotMoveText(bool);
      void slotChangeProps();
      void slotEditCut();
      void slotEditCopy();
      void slotEditPaste(bool);
      void slotEditDelete(bool);
      void slotEditFind();
      void slotEditFindAgain();
      void slotEditUndo();
      void slotEditRedo();
      void slotProjNewButt();
      void slotMenuOpenProject();
      void slotMenuDelProject();
      void slotMenuCloseProject();
      void slotAddToProject();
      void slotCreateLib();
      void slotCreatePackage();
      void slotExtractPackage();
      void slotImportData();
      void slotExportGraphAsCsv();
      void slotShowAll();
      void slotShowOne();
      void slotZoomIn(bool);
      void slotZoomOut();
      void slotSelect(bool);
      void slotSelectAll();
      void slotSelectMarker();
      void slotEditRotate(bool);
      void slotEditMirrorX(bool);
      void slotEditMirrorY(bool);
      void slotIntoHierarchy();
      void slotPopHierarchy();
      void slotEditActivate(bool);
      void slotInsertEquation(bool);
      void slotInsertGround(bool);
      void slotInsertPort(bool);
      void slotSetWire(bool);
      void slotInsertLabel(bool);
      void slotInsertEntity();
      void slotCallEditor();
      void slotCallFilter();
      void slotCallLine();
      void slotCallLibrary();
      void slotCallMatch();
      void slotCallAtt();
      void slotSimulate();
      void slotToPage();
      void slotDCbias();
      void slotSetMarker(bool);
      void slotShowLastMsg();
      void slotShowLastNetlist();
      void slotViewToolBar(bool);
      void slotViewStatusBar(bool);
      void slotViewBrowseDock(bool);
      void slotHelpIndex();
      void slotGettingStarted();
      void slotHelpAbout();
      void slotHelpAboutQt();

   protected:
         void closeEvent( QCloseEvent *closeEvent);

   private slots:
      void activateStackOf(QWidget *w);
      void newView();
      void loadSettings();
   private:
      void initActions();
      void initMenus();
      void initToolBars();

      // The following aim at reducing clutter by substituting
      // action pointers with a map container using object names
      // to identify them.
      inline void addActionToMap(QAction *act);
      inline QAction* action(const QString& name) const;
      QMap<QString ,QAction*> actionMap;

      // menus contain the items of their menubar
      QMenu *fileMenu, *editMenu, *insMenu, *projMenu, *simMenu, *viewMenu,
         *helpMenu, *alignMenu, *toolMenu;

      QToolBar *fileToolbar, *editToolbar, *viewToolbar, *workToolbar;
      QUndoGroup *m_undoGroup;
      ComponentsSidebar *m_componentsSidebar;

   public:
      int x, y, dx, dy;     // position and size
      float largeFontSize;
      unsigned int maxUndo; // size of undo stack
      QString savingFont;
      QString Editor;
      QString Language;
      QColor BGColor;
      QColor VHDL_Comment, VHDL_String, VHDL_Integer, VHDL_Real,
         VHDL_Character, VHDL_Types, VHDL_Attributes;
      // registered filename extensions with program to open the file
      QStringList FileTypes;
};

inline void QucsMainWindow::addActionToMap(QAction *action)
{
   actionMap[action->objectName()] = action;
}

inline QAction* QucsMainWindow::action(const QString& name) const
{
   if(actionMap.contains(name))
      return actionMap[name];
   qWarning("QucsMainWindow::action() - No action by name %s to return\nProbably bug",qPrintable(name));
   return new QAction(0); // To avoid crash
}

#endif //__QUCSMAINWINDOW_H
