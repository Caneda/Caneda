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

#ifndef CANEDA_MAINWINDOW_H
#define CANEDA_MAINWINDOW_H

#include <QMainWindow>

// Forward declarations
class QLabel;

namespace Caneda
{
    // Forward declarations
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

        TabWidget* tabWidget() const;
        QDockWidget* sidebarDockWidget() const;

        void updateWindowTitle();

    private Q_SLOTS:
        void newFile();
        void open(QString fileName = QString());
        void openRecent();
        void openFileFormat(const QString &suffix);
        void save();
        void saveAs();
        bool saveAll();
        void closeFile();
        void print();
        void exportImage();
        void applicationSettings();

        void undo();
        void redo();
        void cut();
        void copy();
        void paste();
        void find();
        void selectAll();

        void openLayout();
        void openSchematic();
        void openSymbol();

        void intoHierarchy();
        void popHierarchy();

        void zoomIn();
        void zoomOut();
        void zoomBestFit();
        void zoomOriginal();
        void splitHorizontal();
        void splitVertical();
        void closeSplit();
        void viewToolBar(bool);
        void viewStatusBar(bool);

        void alignTop();
        void alignBottom();
        void alignLeft();
        void alignRight();
        void centerHorizontal();
        void centerVertical();
        void distributeHorizontal();
        void distributeVertical();

        void newProject();
        void openProject(QString fileName = QString());
        void addToProject();
        void removeFromProject();
        void closeProject();
        void backupAndHistory();

        void simulate();
        void openSimulation();
        void showLog();
        void showNetlist();

        void helpIndex();
        void helpExamples();
        void about();
        void aboutQt();

        void initFile();
        void launchPropertiesDialog();
        void statusBarMessage(const QString& newPos);

    protected:
        void closeEvent(QCloseEvent *closeEvent);

    private:
        MainWindow(QWidget *w=0);

        void initActions();
        void initMouseActions();
        void initMenus();
        void initToolBars();
        void initStatusBar();

        void setupSidebar();
        void setupProjectsSidebar();
        void setupFolderBrowserSidebar();

        void loadSettings();
        void saveSettings();

        void setNormalAction();

        TabWidget *m_tabWidget;
        Project *m_project;

        QMenu *docksMenu;
        QToolBar *fileToolbar, *editToolbar, *viewToolbar, *workToolbar;
        QDockWidget *m_sidebarDockWidget, *m_projectDockWidget,
                    *m_browserDockWidget;
        QLabel *m_statusLabel;
    };

} // namespace Caneda

#endif //CANEDA_MAINWINDOW_H
