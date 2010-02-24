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

#ifndef QUCSMAINWINDOW_H
#define QUCSMAINWINDOW_H

#include "mainwindowbase.h"
#include "schematicscene.h"
#include "undocommands.h"

#include <QMap>
#include <QMenu>
#include <QToolBar>
#include <QUndoView>

class ComponentsSidebar;
class FolderBrowser;
class Library;
class QucsItem;
class QucsView;
class QLabel;
class QUndoGroup;
class SchematicScene;
class SchematicView;

typedef void (SchematicScene::*pActionFunc) (QList<QucsItem*>&, const Qucs::UndoOption);

class QucsMainWindow : public MainWindowBase
{
    Q_OBJECT;
public:
    ~QucsMainWindow();

    static QucsMainWindow* instance();

    bool gotoPage(QString fileName, Qucs::Mode mode=Qucs::SchematicMode);
    void addView(QucsView *view);
    void saveSettings();

public Q_SLOTS:
    void slotFileNew();
    void slotTextNew();
    void slotFileOpen(QString fileName = 0);
    void slotFileSave(int index);
    void slotFileSaveCurrent();
    void slotFileSaveAs(int index);
    void slotFileSaveAsCurrent();
    void slotFileSaveAll();
    void slotFileClose(int index);
    void slotFileCloseCurrent();
    void slotFilePrint();
    void slotExportImage();
    void slotFileSettings();
    void slotApplSettings();

    void slotEditCut();
    void slotEditCopy();
    void slotEditPaste();
    void slotEditDelete(bool);
    void slotEditFind();
    void slotSelect(bool);
    void slotSelectAll();
    void slotSelectMarker();
    void slotEditRotate(bool);
    void slotEditMirrorX(bool);
    void slotEditMirrorY(bool);
    void slotSymbolEdit();
    void slotIntoHierarchy();
    void slotPopHierarchy();

    void slotOnGrid(bool);
    void slotAlignTop();
    void slotAlignBottom();
    void slotAlignLeft();
    void slotAlignRight();
    void slotDistribHoriz();
    void slotDistribVert();
    void slotCenterHorizontal();
    void slotCenterVertical();

    void slotNewProject();
    void slotOpenProject();
    void slotAddToProject();
    void slotRemoveFromProject();
    void slotCloseProject();

    void slotSetWire(bool);
    void slotInsertLabel(bool);
    void slotInsertEquation();
    void slotInsertGround();
    void slotInsertPort();
    void slotInsertEntity();
    void slotEditActivate(bool);
    void slotCallFilter();
    void slotCallLine();
    void slotCallMatch();
    void slotCallAtt();
    void slotCallLibrary();
    void slotImportData();
    void slotShowConsole();

    void slotSimulate();
    void slotToPage();
    void slotDCbias();
    void slotSetMarker(bool);
    void slotExportGraphAsCsv();
    void slotShowLastMsg();
    void slotShowLastNetlist();

    void slotShowAll();
    void slotShowOne();
    void slotZoomIn(bool);
    void slotZoomOut(bool);
    void slotViewToolBar(bool);
    void slotViewStatusBar(bool);

    void slotHelpIndex();
    void slotHelpAbout();
    void slotHelpAboutQt();

    void slotInsertItemAction(bool state);
    void slotPaintingDrawAction(bool state);
    void setDocumentTitle(const QString& title);
    void updateTitleTabText();
    void slotSidebarItemClicked(const QString& item, const QString& category);
    void slotUpdateAllViews();
    void slotUpdateCursorPositionStatus(const QString& newPos);

signals:
    void signalKillWidgets();


protected:
    void closeEvent( QCloseEvent *closeEvent);

private Q_SLOTS:
    void loadSettings();
    void setTabTitle(const QString& str);

    void slotCurrentChanged(QWidget *current, QWidget *prev);
    void slotViewClosed(QWidget *widget);

private:
    QucsMainWindow(QWidget *w=0);
    void initActions();
    void initMenus();
    void initToolBars();
    void initStatusBar();

    void performToggleAction(const bool on, pActionFunc func, QAction *action);
    void setNormalAction();
    void alignElements(Qt::Alignment alignment);
    void editFile(const QString& File);
    void showHTML(const QString& Page);

    void createUndoView();
    void createFolderView();
    void setupSidebar();
    void setupProjectsSidebar();

    QucsView* viewFromWidget(QWidget *widget);

    void resetCurrentSceneState();
    // The following aim at reducing clutter by substituting
    // action pointers with a map container using object names
    // to identify them.
    inline void addActionToMap(QAction *act);
    inline QAction* action(const QString& name) const;
    QMap<QString ,QAction*> actionMap;
    QList<QAction*> checkableActions;

    // menus contain the items of their menubar
    QMenu *fileMenu, *editMenu, *insMenu, *projMenu, *simMenu, *viewMenu,
          *helpMenu, *alignMenu, *toolMenu;

    QLabel *m_cursorLabel;
    QToolBar *fileToolbar, *editToolbar, *viewToolbar, *workToolbar;
    QDockWidget *sidebarDockWidget;
    QUndoGroup *m_undoGroup;
    QUndoView *undoView;
    ComponentsSidebar *m_componentsSidebar;
    ComponentsSidebar *m_projectsSidebar;
    Library *projectLibrary;
    FolderBrowser *m_folderBrowser;
    QString titleText;
};

inline void QucsMainWindow::addActionToMap(QAction *action)
{
    actionMap[action->objectName()] = action;
}

inline QAction* QucsMainWindow::action(const QString& name) const
{
    if(actionMap.contains(name)) {
        return actionMap[name];
    }
    qWarning("QucsMainWindow::action() - No action by name %s to return\nProbably bug",
            qPrintable(name));
    return new QAction(0); // To avoid crash
}

#endif //QUCSMAINWINDOW_H
