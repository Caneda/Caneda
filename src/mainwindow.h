/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef CANEDA_MAINWINDOW_H
#define CANEDA_MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

// Forward declarations
class QLabel;
class QUndoGroup;
class QUndoView;

namespace Caneda
{
    // Forward declarations
    class Action;
    class SidebarBrowser;
    class FolderBrowser;
    class Project;
    class TabWidget;

    /*!
     * \brief The MainWindow class is one of Caneda's main classes (along with
     * the Document-View framework composed by IContext, IDocument and IView
     * classes). This class initializes the main window of the program and
     * manages all user interface interactions.
     *
     * Ideally, this class should contain as little application-related code as
     * possible if not at all. In this way, the gui should be able to be
     * replaced by any form factor or new user interface in the future. This is
     * accomplished by moving as much code as possible (not related to the gui)
     * into other classes.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa IContext, IDocument, IView, DocumentViewManager, Tab, TabWidget
     */
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        static MainWindow* instance();
        ~MainWindow();

        TabWidget* tabWidget() const;
        QDockWidget* sidebarDockWidget() const;

        void saveSettings();
        void setNormalAction();
        Action* action(const QString &name);
        QMenu* menubarMenu(const QString &name, bool createOnAbsence = true);
        QUndoGroup *m_undoGroup;

    public Q_SLOTS:
        void slotFileNew();
        void slotFileOpen(QString fileName = QString());
        void slotFileOpenFormat(const QString &suffix);
        void slotFileSave();
        void slotFileSaveAs();
        bool slotFileSaveAll();
        void slotFileClose();
        void slotFilePrint();
        void slotExportImage();
        void slotAppSettings();

        void slotEditUndo();
        void slotEditRedo();
        void slotEditCut();
        void slotEditCopy();
        void slotEditPaste();
        void slotEditFind();
        void slotSelectAll();

        void openLayout();
        void openSchematic();
        void openSymbol();

        void slotIntoHierarchy();
        void slotPopHierarchy();

        void slotZoomIn();
        void slotZoomOut();
        void slotZoomBestFit();
        void slotZoomOriginal();
        void slotSplitHorizontal();
        void slotSplitVertical();
        void slotCloseSplit();
        void slotViewToolBar(bool);
        void slotViewStatusBar(bool);

        void slotAlignTop();
        void slotAlignBottom();
        void slotAlignLeft();
        void slotAlignRight();
        void slotCenterHorizontal();
        void slotCenterVertical();
        void slotDistributeHorizontal();
        void slotDistributeVertical();

        void slotNewProject();
        void slotOpenProject(QString fileName = QString());
        void slotAddToProject();
        void slotRemoveFromProject();
        void slotCloseProject();
        void slotBackupAndHistory();

        void slotCallFilter();
        void slotCallLine();
        void slotCallMatch();
        void slotCallAtt();
        void slotImportData();

        void slotSimulate();
        void openSimulation();
        void slotShowLastMsg();
        void slotShowLastNetlist();

        void slotHelpIndex();
        void slotHelpAbout();
        void slotHelpAboutQt();

        void setDocumentTitle(const QString& title);
        void updateTitle();
        void slotLaunchPropertiesDialog();
        void slotUpdateSettingsChanges();
        void slotStatusBarMessage(const QString& newPos);

    protected:
        void closeEvent(QCloseEvent *closeEvent);

    private Q_SLOTS:
        void initFile();
        void loadSettings();

    private:
        MainWindow(QWidget *w=0);
        void addAsDockWidget(QWidget *w, const QString &title = QString(),
                Qt::DockWidgetArea area = Qt::LeftDockWidgetArea);

        void initActions();
        void initMouseActions();
        void initMenus();
        void initToolBars();
        void initStatusBar();

        void createUndoView();
        void createFolderView();
        void setupSidebar();
        void setupProjectsSidebar();

        // Menus contain the items of their menubar
        QMenu *fileMenu, *editMenu, *insMenu, *projMenu, *simMenu, *viewMenu,
              *docksMenu, *helpMenu, *alignMenu, *toolMenu;

        QLabel *m_statusLabel;
        QToolBar *fileToolbar, *editToolbar, *viewToolbar, *workToolbar;
        QDockWidget *m_sidebarDockWidget, *m_projectDockWidget,
                    *m_browserDockWidget,*m_undoDockWidget;
        Project *m_project;
        FolderBrowser *m_folderBrowser;
        QUndoView *undoView;
        TabWidget *m_tabWidget;
        QString titleText;
    };

} // namespace Caneda

#endif //CANEDA_MAINWINDOW_H
