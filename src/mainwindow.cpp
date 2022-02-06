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
#include "quicklauncher.h"
#include "quickopen.h"
#include "savedocumentsdialog.h"
#include "settings.h"
#include "settingsdialog.h"
#include "shortcutsdialog.h"
#include "statehandler.h"
#include "tabs.h"

#include <QtWidgets>

namespace Caneda
{
    //! \brief Constructs and setups the MainWindow for the application.
    MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
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

    /*!
     * \brief Creates or opens a new file used for the program initial state.
     *
     * This method creates or opens a new file used for the program initial
     * state. If an argument with filenames is present, tries to open all files
     * in the argument, otherwise creates a new empty file.
     */
    void MainWindow::initFiles(QStringList files)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();

        if(!files.isEmpty()) {
            foreach(const QString &str, files) {
                open(str);
            }
        }
        else {
            manager->newDocument(SchematicContext::instance());
        }

        // Set the dialog to open in the current file folder
        QFileInfo info(manager->currentDocument()->fileName());
        QString path = info.path();
        m_folderBrowser->setCurrentFolder(path);
    }

    //! \brief Opens the file new dialog.
    void MainWindow::newFile()
    {
        if(m_project->isValid()) {
            addToProject();
        }
        else {
            QPointer<FileNewDialog> dialog = new FileNewDialog(this);
            dialog->exec();
            delete dialog;
        }
    }

    //! \brief Create a new schematic file.
    void MainWindow::newSchematic()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->newDocument(SchematicContext::instance());
    }

    //! \brief Create a new symbol file.
    void MainWindow::newSymbol()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->newDocument(SymbolContext::instance());
    }

    //! \brief Create a new layout file.
    void MainWindow::newLayout()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->newDocument(LayoutContext::instance());
    }

    //! \brief Create a new text file.
    void MainWindow::newText()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->newDocument(TextContext::instance());
    }

    /*!
     * \brief Opens the file open dialog.
     *
     * Opens the file open dialog. If the file is already opened, the
     * corresponding tab is set as the current one. Otherwise the file is
     * opened and its tab is set as current tab.
     *
     * \sa save(), saveAll(), saveAs()
     */
    void MainWindow::open(QString fileName)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();

        if(fileName.isEmpty()) {
            fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                    manager->fileNameFilters().join(QString(";;")));
        }

        if(!fileName.isEmpty()) {
            manager->openFile(fileName);
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

    //! \brief Clears the list of recently opened documents.
    void MainWindow::clearRecent()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->clearRecentFiles();
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

    /*!
     * \brief Opens a dialog to select a new filename and saves the current
     * file.
     *
     * Opens a dialog to select a new filename and saves the current file. A
     * custom QFileDialog is created (instead of using QFileDialog::getSaveFileName())
     * to be able to set the default suffix. By using a default suffix, it is
     * automatically appended to the end of the filename in the case the user
     * didn't add a custom suffix.
     *
     * \sa save(), saveAll(), open()
     */
    void MainWindow::saveAs()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if(!document) {
            return;
        }

        // Create custom dialog with default suffix
        QFileDialog dialog(this, tr("Save File"));
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setNameFilters(manager->currentDocument()->context()->fileNameFilters());
        dialog.setDefaultSuffix(manager->currentDocument()->context()->defaultSuffix());

        QString fileName;
        if (dialog.exec()) {
            fileName = dialog.selectedFiles().first();
        }

        if(fileName.isEmpty()) {
            return;
        }

        QString oldFileName = document->fileName();
        document->setFileName(fileName);

        QString errorMessage;
        if(!document->save(&errorMessage)) {
            QMessageBox::critical(this, tr("%1 : File save error").arg(document->fileName()), errorMessage);
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
     * \return True on success, false on cancel.
     *
     * \sa save(), saveAs()
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

        QPointer<PrintDialog> dialog = new PrintDialog(document, this);
        dialog->exec();
        delete dialog;
    }

    //! \brief Opens the export image dialog.
    void MainWindow::exportImage()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        QPointer<ExportDialog> dialog = new ExportDialog(document, this);
        dialog->exec();
        delete dialog;
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

        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        QFileInfo info(doc->fileName());
        QString filename = info.completeBaseName();
        QString path = info.path();
        filename = QDir::toNativeSeparators(path + "/" + filename + "." + sy->defaultSuffix());
        if (!QFileInfo::exists(filename)) {
            newSymbol();
            DocumentViewManager::instance()->currentDocument()->setFileName(filename);
            return;
        }

        openFileFormat(sy->defaultSuffix());
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
    void MainWindow::openLog()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        QFileInfo info(document->fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();

        manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".log"));
    }

    //! \brief Opens the netlist corresponding to the current file.
    void MainWindow::openNetlist()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (!document) {
            return;
        }

        QFileInfo info(document->fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();

        manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".net"));
    }

    //! \brief Opens the quick launcher dialog.
    void MainWindow::quickLauncher()
    {
        QuickLauncher *launcher = new QuickLauncher(this);
        launcher->exec(QCursor::pos());
        delete launcher;
    }

    //! \brief Opens the quick insert dialog.
    void MainWindow::quickInsert()
    {
        IView *view = DocumentViewManager::instance()->currentView();
        if (view) {
            view->context()->quickInsert();
        }
    }

    //! \brief Opens the quick open dialog.
    void MainWindow::quickOpen()
    {
        QuickOpen *quickBrowser = new QuickOpen(this);

        DocumentViewManager *manager = DocumentViewManager::instance();
        IDocument *document = manager->currentDocument();
        if (document) {
            // Set the dialog to open in the current file folder
            QFileInfo info(document->fileName());
            QString path = info.path();

            quickBrowser->setCurrentFolder(path);
        }

        connect(quickBrowser, SIGNAL(itemSelected(QString)), this, SLOT(open(QString)));

        quickBrowser->exec(QCursor::pos());
        delete quickBrowser;
    }

    //! \brief Opens the selected item file description for edition.
    void MainWindow::enterHierarchy()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->enterHierarchy();
        }
    }

    //! \brief Opens the parent document from where this item was opened.
    void MainWindow::exitHierarchy()
    {
        IDocument *document = DocumentViewManager::instance()->currentDocument();
        if (document) {
            document->exitHierarchy();
        }
    }

    //! \brief Opens the backup and history file dialog.
    void MainWindow::backupAndHistory()
    {
        m_project->slotBackupAndHistory();
    }

    //! \brief Updates the visibility of the different toolbars and widgets.
    void MainWindow::updateVisibility()
    {
        ActionManager* am = ActionManager::instance();

        menuBar()->setVisible(am->actionForName("showMenuBar")->isChecked());

        fileToolbar->setVisible(am->actionForName("showToolBar")->isChecked());
        editToolbar->setVisible(am->actionForName("showToolBar")->isChecked());
        viewToolbar->setVisible(am->actionForName("showToolBar")->isChecked());
        workToolbar->setVisible(am->actionForName("showToolBar")->isChecked());

        statusBar()->setVisible(am->actionForName("showStatusBar")->isChecked());

        m_sidebarDockWidget->setVisible(am->actionForName("showSideBarBrowser")->isChecked());
        m_browserDockWidget->setVisible(am->actionForName("showFolderBrowser")->isChecked());
    }

    //! \brief Toogles the visibility of all widgets at once.
    void MainWindow::showAll()
    {
        ActionManager* am = ActionManager::instance();

        bool show = am->actionForName("showAll")->isChecked();

        am->actionForName("showMenuBar")->setChecked(show);
        am->actionForName("showToolBar")->setChecked(show);
        am->actionForName("showStatusBar")->setChecked(show);
        am->actionForName("showSideBarBrowser")->setChecked(show);
        am->actionForName("showFolderBrowser")->setChecked(show);

        updateVisibility();
    }

    //! \brief Toogles the full screen mode.
    void MainWindow::showFullScreen()
    {
        ActionManager* am = ActionManager::instance();

        if(am->actionForName("showFullScreen")->isChecked()) {
            QMainWindow::showFullScreen();
        }
        else {
            showMaximized();
        }
    }

    //! \brief Opens the shortcuts editor dialog.
    void MainWindow::editShortcuts()
    {
        ShortcutsDialog *dialog = new ShortcutsDialog(this);
        dialog->exec();
        delete dialog;
    }

    //! \brief Opens the applications settings dialog.
    void MainWindow::applicationSettings()
    {
        SettingsDialog *dialog = new SettingsDialog(this);
        int result = dialog->exec();

        // Update all document views to reflect the current settings.
        if(result == QDialog::Accepted) {
            DocumentViewManager::instance()->updateSettingsChanges();
            repaint();
        }

        delete dialog;
    }

    //! \brief Opens the help index.
    void MainWindow::helpIndex()
    {
        QDesktopServices::openUrl(QUrl("http://docs.caneda.org"));
    }

    //! \brief Opens the examples repository in an external window.
    void MainWindow::helpExamples()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/Caneda/Examples"));
    }

    //! \brief Opens the about dialog.
    void MainWindow::about()
    {
        QPointer<AboutDialog> dialog = new AboutDialog(this);
        dialog->exec();
        delete dialog;
    }

    //! \brief Opens the about Qt dialog.
    void MainWindow::aboutQt()
    {
        QApplication::aboutQt();
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

     //! \brief Sets a new statusbar message.
     void MainWindow::statusBarMessage(const QString& newPos)
     {
         m_statusLabel->setText(newPos);
     }

    //! \brief Creates and initializes all the actions used.
    void MainWindow::initActions()
    {
        QAction *action = 0;
        ActionManager *am = ActionManager::instance();

        action = am->createAction("fileNew", Caneda::icon("document-new"), tr("&New..."));
        action->setStatusTip(tr("Creates a new file document"));
        action->setWhatsThis(tr("New file\n\nCreates a new file document"));
        connect(action, SIGNAL(triggered()), SLOT(newFile()));

        action = am->createAction("fileNewSchematic", Caneda::icon("application-x-caneda-schematic"), tr("&Schematic file"));
        action->setStatusTip(tr("Creates a new schematic file document"));
        action->setWhatsThis(tr("New schematic file\n\nCreates a new schematic file document"));
        connect(action, SIGNAL(triggered()), SLOT(newSchematic()));

        action = am->createAction("fileNewSymbol", Caneda::icon("application-x-caneda-symbol"), tr("S&ymbol file"));
        action->setStatusTip(tr("Creates a new symbol file document"));
        action->setWhatsThis(tr("New symbol file\n\nCreates a new symbol file document"));
        connect(action, SIGNAL(triggered()), SLOT(newSymbol()));

        action = am->createAction("fileNewLayout", Caneda::icon("application-x-caneda-layout"), tr("&Layout file"));
        action->setStatusTip(tr("Creates a new layout file document"));
        action->setWhatsThis(tr("New layout file\n\nCreates a new layout file document"));
        connect(action, SIGNAL(triggered()), SLOT(newLayout()));

        action = am->createAction("fileNewText", Caneda::icon("text-plain"), tr("&Text file"));
        action->setStatusTip(tr("Creates a new text file document"));
        action->setWhatsThis(tr("New text file\n\nCreates a new text file document"));
        connect(action, SIGNAL(triggered()), SLOT(newText()));

        action = am->createAction("fileOpen", Caneda::icon("document-open"), tr("&Open..."));
        action->setStatusTip(tr("Opens an existing document"));
        action->setWhatsThis(tr("Open File\n\nOpens an existing document"));
        connect(action, SIGNAL(triggered()), SLOT(open()));

        for(uint i=0; i<maxRecentFiles; i++) {
            action = am->createRecentFilesAction();
            connect(action, SIGNAL(triggered()), SLOT(openRecent()));
        }

        action = am->createAction("clearRecent", tr("&Clear list"));
        action->setStatusTip(tr("Clears the list of recently opened documents"));
        action->setWhatsThis(tr("Clear list\n\nClears the list of recently opened documents"));
        connect(action, SIGNAL(triggered()), SLOT(clearRecent()));

        action = am->createAction("fileSave", Caneda::icon("document-save"), tr("&Save"));
        action->setStatusTip(tr("Saves the current document"));
        action->setWhatsThis(tr("Save File\n\nSaves the current document"));
        connect(action, SIGNAL(triggered()), SLOT(save()));

        action = am->createAction("fileSaveAs", Caneda::icon("document-save-as"), tr("Save as..."));
        action->setStatusTip(tr("Saves the current document under a new filename"));
        action->setWhatsThis(tr("Save As\n\nSaves the current document under a new filename"));
        connect(action, SIGNAL(triggered()), SLOT(saveAs()));

        action = am->createAction("fileSaveAll", Caneda::icon("document-save-all"), tr("Save &all"));
        action->setStatusTip(tr("Saves all open documents"));
        action->setWhatsThis(tr("Save All Files\n\nSaves all open documents"));
        connect(action, SIGNAL(triggered()), SLOT(saveAll()));

        action = am->createAction("fileClose", Caneda::icon("document-close"), tr("&Close"));
        action->setStatusTip(tr("Closes the current document"));
        action->setWhatsThis(tr("Close File\n\nCloses the current document"));
        connect(action, SIGNAL(triggered()), SLOT(closeFile()));

        action = am->createAction("filePrint", Caneda::icon("document-print"), tr("&Print..."));
        action->setStatusTip(tr("Prints the current document"));
        action->setWhatsThis(tr("Print File\n\nPrints the current document"));
        connect(action, SIGNAL(triggered()), SLOT(print()));

        action = am->createAction("fileExportImage", Caneda::icon("image-x-generic"), tr("&Export image..."));
        action->setStatusTip(tr("Exports the current view to an image file"));
        action->setWhatsThis(tr("Export Image\n\n""Exports the current view to an image file"));
        connect(action, SIGNAL(triggered()), SLOT(exportImage()));

        action = am->createAction("fileQuit", Caneda::icon("application-exit"), tr("&Quit"));
        action->setStatusTip(tr("Quits the application"));
        action->setWhatsThis(tr("Quit\n\nQuits the application"));
        connect(action, SIGNAL(triggered()), SLOT(close()));

        action = am->createAction("editUndo", Caneda::icon("edit-undo"), tr("&Undo"));
        action->setStatusTip(tr("Undoes the last command"));
        action->setWhatsThis(tr("Undo\n\nMakes the last action undone"));
        connect(action, SIGNAL(triggered()), SLOT(undo()));

        action = am->createAction("editRedo", Caneda::icon("edit-redo"), tr("&Redo"));
        action->setStatusTip(tr("Redoes the last command"));
        action->setWhatsThis(tr("Redo\n\nRepeats the last action once more"));
        connect(action, SIGNAL(triggered()), SLOT(redo()));

        action = am->createAction("editCut", Caneda::icon("edit-cut"), tr("Cu&t"));
        action->setStatusTip(tr("Cuts out the selection and puts it into the clipboard"));
        action->setWhatsThis(tr("Cut\n\nCuts out the selection and puts it into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(cut()));

        action = am->createAction("editCopy", Caneda::icon("edit-copy"), tr("&Copy"));
        action->setStatusTip(tr("Copies the selection into the clipboard"));
        action->setWhatsThis(tr("Copy\n\nCopies the selection into the clipboard"));
        connect(action, SIGNAL(triggered()), SLOT(copy()));

        action = am->createAction("editPaste", Caneda::icon("edit-paste"), tr("&Paste"));
        action->setStatusTip(tr("Pastes the clipboard contents to the cursor position"));
        action->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to the cursor position"));
        connect(action, SIGNAL(triggered()), SLOT(paste()));

        action = am->createAction("selectAll", Caneda::icon("select-rectangular"), tr("Select all"));
        action->setStatusTip(tr("Selects all elements"));
        action->setWhatsThis(tr("Select All\n\nSelects all elements of the document"));
        connect(action, SIGNAL(triggered()), SLOT(selectAll()));

        action = am->createAction("editFind", Caneda::icon("edit-find"), tr("Find..."));
        action->setStatusTip(tr("Find a piece of text"));
        action->setWhatsThis(tr("Find\n\nSearches for a piece of text"));
        connect(action, SIGNAL(triggered()), SLOT(find()));

        action = am->createAction("zoomFitInBest", Caneda::icon("zoom-fit-best"), tr("View all"));
        action->setStatusTip(tr("Show the whole contents"));
        action->setWhatsThis(tr("View all\n\nShows the whole page content"));
        connect(action, SIGNAL(triggered()), SLOT(zoomBestFit()));

        action = am->createAction("zoomOriginal", Caneda::icon("zoom-original"), tr("View 1:1"));
        action->setStatusTip(tr("View without magnification"));
        action->setWhatsThis(tr("Zoom 1:1\n\nShows the page content without magnification"));
        connect(action, SIGNAL(triggered()), SLOT(zoomOriginal()));

        action = am->createAction("zoomIn", Caneda::icon("zoom-in"), tr("Zoom in"));
        action->setStatusTip(tr("Zooms in the content"));
        action->setWhatsThis(tr("Zoom In \n\nZooms in the content"));
        connect(action, SIGNAL(triggered()), SLOT(zoomIn()));

        action = am->createAction("zoomOut", Caneda::icon("zoom-out"), tr("Zoom out"));
        action->setStatusTip(tr("Zooms out the content"));
        action->setWhatsThis(tr("Zoom Out \n\nZooms out the content"));
        connect(action, SIGNAL(triggered()), SLOT(zoomOut()));

        action = am->createAction("splitHorizontal", Caneda::icon("view-split-left-right"), tr("Split &horizontal"));
        action->setStatusTip(tr("Splits the current view in horizontal orientation"));
        action->setWhatsThis(tr("Split Horizontal\n\nSplits the current view in horizontal orientation"));
        connect(action, SIGNAL(triggered()), SLOT(splitHorizontal()));

        action = am->createAction("splitVertical", Caneda::icon("view-split-top-bottom"), tr("Split &vertical"));
        action->setStatusTip(tr("Splits the current view in vertical orientation"));
        action->setWhatsThis(tr("Split Vertical\n\nSplits the current view in vertical orientation"));
        connect(action, SIGNAL(triggered()), SLOT(splitVertical()));

        action = am->createAction("splitClose", Caneda::icon("view-left-close"), tr("&Close split"));
        action->setStatusTip(tr("Closes the current split"));
        action->setWhatsThis(tr("Close Split\n\nCloses the current split"));
        connect(action, SIGNAL(triggered()), SLOT(closeSplit()));

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

        //! \todo Reenable these actions once project actions and tools are reimplemented.
        //        action = am->createAction("projNew", Caneda::icon("project-new"), tr("&New project..."));
        //        action->setStatusTip(tr("Creates a new project"));
        //        action->setWhatsThis(tr("New Project\n\nCreates a new project"));
        //        connect(action, SIGNAL(triggered()), SLOT(newProject()));

        //        action = am->createAction("projOpen", Caneda::icon("document-open"), tr("&Open project..."));
        //        action->setStatusTip(tr("Opens an existing project"));
        //        action->setWhatsThis(tr("Open Project\n\nOpens an existing project"));
        //        connect(action, SIGNAL(triggered()), SLOT(openProject()));

        //        action = am->createAction("addToProj", Caneda::icon("document-new"), tr("&Add file to project..."));
        //        action->setStatusTip(tr("Adds a file to current project"));
        //        action->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));
        //        connect(action, SIGNAL(triggered()), SLOT(addToProject()));

        //        action = am->createAction("projDel", Caneda::icon("document-close"), tr("&Remove from project"));
        //        action->setStatusTip(tr("Removes a file from current project"));
        //        action->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));
        //        connect(action, SIGNAL(triggered()), SLOT(removeFromProject()));

        //        action = am->createAction("projClose", Caneda::icon("dialog-close"), tr("&Close project"));
        //        action->setStatusTip(tr("Closes the current project"));
        //        action->setWhatsThis(tr("Close Project\n\nCloses the current project"));
        //        connect(action, SIGNAL(triggered()), SLOT(closeProject()));

        action = am->createAction("openSchematic", Caneda::icon("draw-freehand"), tr("&Edit circuit schematic"));
        action->setStatusTip(tr("Switches to schematic edit"));
        action->setWhatsThis(tr("Edit Circuit Schematic\n\nSwitches to schematic edit"));
        connect(action, SIGNAL(triggered()), SLOT(openSchematic()));

        action = am->createAction("openSymbol", Caneda::icon("draw-freehand"), tr("Edit circuit &symbol"));
        action->setStatusTip(tr("Switches to symbol edit"));
        action->setWhatsThis(tr("Edit Circuit Symbol\n\nSwitches to symbol edit"));
        connect(action, SIGNAL(triggered()), SLOT(openSymbol()));

        action = am->createAction("openLayout", Caneda::icon("draw-freehand"), tr("Edit circuit &layout"));
        action->setStatusTip(tr("Switches to layout edit"));
        action->setWhatsThis(tr("Edit Circuit Layout\n\nSwitches to layout edit"));
        connect(action, SIGNAL(triggered()), SLOT(openLayout()));

        action = am->createAction("simulate", Caneda::icon("media-playback-start"), tr("Simulate"));
        action->setStatusTip(tr("Simulates the current circuit"));
        action->setWhatsThis(tr("Simulate\n\nSimulates the current circuit"));
        connect(action, SIGNAL(triggered()), SLOT(simulate()));

        action = am->createAction("openSimulation", Caneda::icon("system-switch-user"), tr("View circuit simulation"));
        action->setStatusTip(tr("Changes to circuit simulation"));
        action->setWhatsThis(tr("View Circuit Simulation\n\n")+tr("Changes to circuit simulation"));
        connect(action, SIGNAL(triggered()), SLOT(openSimulation()));

        action = am->createAction("openLog", Caneda::icon("document-preview"), tr("Show simulation log"));
        action->setStatusTip(tr("Shows simulation log"));
        action->setWhatsThis(tr("Show Log\n\nShows the log of the current simulation"));
        connect(action, SIGNAL(triggered()), SLOT(openLog()));

        action = am->createAction("openNetlist", Caneda::icon("document-preview"), tr("Show circuit netlist"));
        action->setStatusTip(tr("Shows the circuit netlist"));
        action->setWhatsThis(tr("Show Netlist\n\nShows the netlist of the current circuit"));
        connect(action, SIGNAL(triggered()), SLOT(openNetlist()));

        action = am->createAction("quickLauncher", Caneda::icon("fork"), tr("&Quick launcher..."));
        action->setStatusTip(tr("Opens the quick launcher dialog"));
        action->setWhatsThis(tr("Quick launcher\n\nOpens the quick launcher dialog"));
        connect(action, SIGNAL(triggered()), SLOT(quickLauncher()));

        action = am->createAction("quickInsert", Caneda::icon("fork"), tr("&Quick insert..."));
        action->setStatusTip(tr("Opens the quick insert dialog"));
        action->setWhatsThis(tr("Quick insert\n\nOpens the quick insert dialog"));
        connect(action, SIGNAL(triggered()), SLOT(quickInsert()));

        action = am->createAction("quickOpen", Caneda::icon("fork"), tr("&Quick open..."));
        action->setStatusTip(tr("Opens the quick open dialog"));
        action->setWhatsThis(tr("Quick open\n\nOpens the quick open dialog"));
        connect(action, SIGNAL(triggered()), SLOT(quickOpen()));

        //! \todo Reenable these actions once reimplemented.
        //        action = am->createAction("enterHierarchy", Caneda::icon("go-bottom"), tr("Enter hierarchy"));
        //        action->setStatusTip(tr("Enters the selected subcircuit hierarchy"));
        //        action->setWhatsThis(tr("Enter hierarchy\n\nEnters the selected subcircuit hierarchy"));
        //        connect(action, SIGNAL(triggered()), SLOT(enterHierarchy()));

        //        action = am->createAction("exitHierarchy", Caneda::icon("go-top"), tr("Exit hierarchy"));
        //        action->setStatusTip(tr("Exits current subcircuit hierarchy"));
        //        action->setWhatsThis(tr("Exit hierarchy\n\nExits current subcircuit hierarchy"));
        //        connect(action, SIGNAL(triggered()), SLOT(exitHierarchy()));

        //        action = am->createAction("backupAndHistory", Caneda::icon("chronometer"), tr("&Backup and history..."));
        //        action->setStatusTip(tr("Opens backup and history dialog"));
        //        action->setWhatsThis(tr("Backup and History\n\nOpens backup and history dialog"));
        //        connect(action, SIGNAL(triggered()), SLOT(backupAndHistory()));

        action = am->createAction("showMenuBar", Caneda::icon("show-menu"), tr("Show &menubar"));
        action->setStatusTip(tr("Enables/disables the menubar"));
        action->setWhatsThis(tr("Show menubar\n\nEnables/disables the menubar"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(updateVisibility()));

        action = am->createAction("showToolBar", tr("Show &toolbar"));
        action->setStatusTip(tr("Enables/disables the toolbar"));
        action->setWhatsThis(tr("Show toolbar\n\nEnables/disables the toolbar"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(updateVisibility()));

        action = am->createAction("showStatusBar",  tr("Show &statusbar"));
        action->setStatusTip(tr("Enables/disables the statusbar"));
        action->setWhatsThis(tr("Show statusbar\n\nEnables/disables the statusbar"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(updateVisibility()));

        action = am->createAction("showSideBarBrowser", Caneda::icon("view-sidetree"), tr("Show sidebar browser"));
        action->setStatusTip(tr("Enables/disables the sidebar browser"));
        action->setWhatsThis(tr("Show sidebar browser\n\nEnables/disables the sidebar browser"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(updateVisibility()));

        action = am->createAction("showFolderBrowser", Caneda::icon("document-open"), tr("Show folder browser"));
        action->setStatusTip(tr("Enables/disables the folder browser"));
        action->setWhatsThis(tr("Show folder browser\n\nEnables/disables the folder browser"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(updateVisibility()));

        action = am->createAction("showAll", Caneda::icon("configure"), tr("Show all"));
        action->setStatusTip(tr("Show/hide all widgets"));
        action->setWhatsThis(tr("Show all\n\nShow/hide all widgets"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(showAll()));

        action = am->createAction("showFullScreen", Caneda::icon("view-fullscreen"), tr("&Full screen mode"));
        action->setStatusTip(tr("Enables/disables the full screen mode"));
        action->setWhatsThis(tr("Full screen mode\n\nEnables/disables the full screen mode"));
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(showFullScreen()));

        action = am->createAction("editShortcuts", Caneda::icon("configure-shortcuts"), tr("Configure s&hortcuts..."));
        action->setStatusTip(tr("Launches the configuration dialog for the application shortcuts"));
        action->setWhatsThis(tr("Configure shortcuts\n\nLaunches the configuration dialog for the application shortcuts"));
        connect(action, SIGNAL(triggered()), SLOT(editShortcuts()));

        action = am->createAction("settings", Caneda::icon("configure"), tr("&Configure Caneda..."));
        action->setStatusTip(tr("Launches the application settings dialog"));
        action->setWhatsThis(tr("Caneda Settings\n\nLaunches the application settings dialog"));
        connect(action, SIGNAL(triggered()), SLOT(applicationSettings()));

        action = am->createAction("propertiesDialog", Caneda::icon("document-properties"), tr("Edit parameters..."));
        action->setStatusTip(tr("Launches current selection properties dialog"));
        action->setWhatsThis(tr("Edit Parameters\n\nLaunches current selection properties dialog"));
        connect(action, SIGNAL(triggered()), SLOT(launchPropertiesDialog()));

        action = am->createAction("helpIndex", Caneda::icon("help-contents"), tr("&Help index..."));
        action->setStatusTip(tr("Index of Caneda Help"));
        action->setWhatsThis(tr("Help Index\n\nIndex of intern Caneda help"));
        connect(action, SIGNAL(triggered()), SLOT(helpIndex()));

        action = am->createAction("helpExamples", Caneda::icon("draw-freehand"), tr("&Example circuits..."));
        action->setStatusTip(tr("Open Caneda example circuits"));
        action->setWhatsThis(tr("Example circuits\n\nOpen Caneda example circuits"));
        connect(action, SIGNAL(triggered()), SLOT(helpExamples()));

        QAction *qAction = QWhatsThis::createAction(this);
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

        // Load action shortcuts and add the actions to the tabWidget to receive
        // shortcuts when the menu is hidden.
        Settings *settings = Settings::instance();

        const QList<QAction*> actions = am->actions();
        foreach(QAction *act, actions) {
            // Load this action shortcut
            QString name = "shortcuts/" + act->objectName();
            act->setShortcut(settings->currentValue(name).value<QKeySequence>());
            // Add the action to the tabWidget
            m_tabWidget->addAction(act);
        }

        // Initialize the mouse actions.
        initMouseActions();
    }

    //! \brief Creates and initializes the mouse actions.
    void MainWindow::initMouseActions()
    {
        QAction *action = 0;
        ActionManager *am = ActionManager::instance();

        StateHandler *handler = StateHandler::instance();

        action = am->createMouseAction("select", Caneda::Normal, Caneda::icon("edit-select"), tr("Select"));
        action->setStatusTip(tr("Activate select mode"));
        action->setWhatsThis(tr("Select\n\nActivates select mode"));
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("editDelete", Caneda::Deleting, Caneda::icon("edit-delete"), tr("&Delete"));
        action->setStatusTip(tr("Deletes the selected components"));
        action->setWhatsThis(tr("Delete\n\nDeletes the selected components"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("editRotate", Caneda::Rotating, Caneda::icon("object-rotate-right"), tr("Rotate"));
        action->setStatusTip(tr("Rotates the selected component"));
        action->setWhatsThis(tr("Rotate\n\nRotates the selected component counter-clockwise"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("editMirrorX", Caneda::MirroringX, Caneda::icon("object-flip-vertical"), tr("Mirror vertically"));
        action->setStatusTip(tr("Mirrors the selected components vertically"));
        action->setWhatsThis(tr("Mirror vertically Axis\n\nMirrors the selected components vertically"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("editMirrorY", Caneda::MirroringY, Caneda::icon("object-flip-horizontal"), tr("Mirror horizontally"));
        action->setStatusTip(tr("Mirrors the selected components horizontally"));
        action->setWhatsThis(tr("Mirror horizontally\n\nMirrors the selected components horizontally"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("insertWire", Caneda::Wiring, Caneda::icon("wire"), tr("Wire"));
        action->setStatusTip(tr("Inserts a wire"));
        action->setWhatsThis(tr("Wire\n\nInserts a wire"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("zoomArea", Caneda::ZoomingAreaEvent, Caneda::icon("transform-scale"), tr("Zoom area"));
        action->setStatusTip(tr("Zooms a selected area in the current view"));
        action->setWhatsThis(tr("Zooms a selected area in the current view"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("insertItem", Caneda::InsertingItems, tr("Insert item"));
        action->setStatusTip(tr("Inserts an item"));
        action->setWhatsThis(tr("Insert item\n\nInserts an item"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        action = am->createMouseAction("paintingDraw", Caneda::PaintingDrawEvent, tr("Painting draw operation"));
        action->setStatusTip(tr("Begins a painting draw operation"));
        action->setWhatsThis(tr("Painting draw operation\n\nBegins a painting draw operation"));
        connect(action, SIGNAL(toggled(bool)), handler, SLOT(performToggleAction(bool)));

        // Load action shortcuts and add the actions to the tabWidget to receive
        // shortcuts when the menu is hidden.
        Settings *settings = Settings::instance();

        const QList<QAction*> actions = am->mouseActions();
        foreach(QAction *act, actions) {
            // Load this action shortcut
            QString name = "shortcuts/" + act->objectName();
            act->setShortcut(settings->currentValue(name).value<QKeySequence>());
            // Add the action to the tabWidget
            m_tabWidget->addAction(act);
        }
    }

    //! \brief Creates and initializes the menus.
    void MainWindow::initMenus()
    {
        ActionManager* am = ActionManager::instance();
        QMenu *menu = 0;
        QMenu *subMenu = 0;

        // File menu
        menu = menuBar()->addMenu(tr("&File"));

        subMenu = menu->addMenu(Caneda::icon("document-new"), tr("&New"));
        subMenu->addAction(am->actionForName("fileNewSchematic"));
        subMenu->addAction(am->actionForName("fileNewSymbol"));
        subMenu->addAction(am->actionForName("fileNewLayout"));
        subMenu->addAction(am->actionForName("fileNewText"));

        menu->addAction(am->actionForName("fileOpen"));

        subMenu = menu->addMenu(Caneda::icon("document-open-recent"), tr("Open &Recent"));
        for(uint i=0; i<maxRecentFiles; i++) {
            subMenu->addAction(am->recentFilesActions().at(i));
        }
        DocumentViewManager::instance()->updateRecentFilesActionList();  // Update the list from the previosly saved configuration file
        subMenu->addSeparator();
        subMenu->addAction(am->actionForName("clearRecent"));

        menu->addAction(am->actionForName("fileClose"));

        menu->addSeparator();

        menu->addAction(am->actionForName("fileSave"));
        menu->addAction(am->actionForName("fileSaveAll"));
        menu->addAction(am->actionForName("fileSaveAs"));
        menu->addAction(am->actionForName("filePrint"));
        menu->addAction(am->actionForName("fileExportImage"));

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
        menu->addAction(am->actionForName("editMirrorX"));
        menu->addAction(am->actionForName("editMirrorY"));
        menu->addAction(am->actionForName("editRotate"));

        //! \todo Reenable this option once implemented
        //        menu->addSeparator();

        //        menu->addAction(am->actionForName("editFind"));

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

        // Align menu
        menu = menuBar()->addMenu(tr("&Placement"));

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

        // Tools menu
        menu = menuBar()->addMenu(tr("&Tools"));

        menu->addAction(am->actionForName("openSchematic"));
        menu->addAction(am->actionForName("openSymbol"));
        menu->addAction(am->actionForName("openLayout"));

        menu->addSeparator();

        menu->addAction(am->actionForName("simulate"));
        menu->addAction(am->actionForName("openSimulation"));

        menu->addSeparator();

        menu->addAction(am->actionForName("openLog"));
        menu->addAction(am->actionForName("openNetlist"));

        menu->addSeparator();
        menu->addAction(am->actionForName("quickLauncher"));
        menu->addAction(am->actionForName("quickInsert"));
        menu->addAction(am->actionForName("quickOpen"));

        //! \todo Reenable these options once implemented
        //        menu->addAction(am->actionForName("enterHierarchy"));
        //        menu->addAction(am->actionForName("exitHierarchy"));
        //        menu->addAction(am->actionForName("backupAndHistory"));

        // Settings menu
        menu = menuBar()->addMenu(tr("&Settings"));

        menu->addAction(am->actionForName("showMenuBar"));
        menu->addAction(am->actionForName("showToolBar"));
        menu->addAction(am->actionForName("showStatusBar"));

        subMenu = menu->addMenu(tr("&Docks and Toolbars"));
        subMenu->addAction(am->actionForName("showAll"));
        subMenu->addSeparator();
        subMenu->addAction(am->actionForName("showSideBarBrowser"));
        subMenu->addAction(am->actionForName("showFolderBrowser"));

        menu->addSeparator();

        menu->addAction(am->actionForName("showFullScreen"));

        menu->addSeparator();

        menu->addAction(am->actionForName("editShortcuts"));
        menu->addAction(am->actionForName("settings"));

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

        fileToolbar = addToolBar(tr("File"));
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

        workToolbar = addToolBar(tr("Work"));
        workToolbar->setObjectName("workToolbar");

        workToolbar->addAction(am->actionForName("select"));
        workToolbar->addAction(am->actionForName("editDelete"));
        workToolbar->addAction(am->actionForName("editMirrorX"));
        workToolbar->addAction(am->actionForName("editMirrorY"));
        workToolbar->addAction(am->actionForName("editRotate"));

        workToolbar->addSeparator();

        workToolbar->addAction(am->actionForName("insertWire"));
        //! \todo Reenable this option once implemented
        //        workToolbar->addAction(am->actionForName("enterHierarchy"));
        //        workToolbar->addAction(am->actionForName("exitHierarchy"));

        workToolbar->addSeparator();

        workToolbar->addAction(am->actionForName("simulate"));
        workToolbar->addAction(am->actionForName("openSimulation"));
    }

    //! \brief Creates and intializes the statusbar.
    void MainWindow::initStatusBar()
    {
        QStatusBar *statusBarWidget = statusBar();
        ActionManager* am = ActionManager::instance();

        // Initially the label is an empty space.
        m_statusLabel = new QLabel(QString(), statusBarWidget);

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
    }

    //! \brief Initializes the Components sidebar.
    void MainWindow::setupSidebar()
    {
        m_sidebarDockWidget = new QDockWidget(this);
        m_sidebarDockWidget->setObjectName("componentsSidebar");
        m_sidebarDockWidget->setMinimumWidth(260);
        addDockWidget(Qt::LeftDockWidgetArea, m_sidebarDockWidget);
    }

    //! \brief Initializes the Projects sidebar.
    void MainWindow::setupProjectsSidebar()
    {
        StateHandler *handler = StateHandler::instance();
        m_project = new Project(this);
        connect(m_project, SIGNAL(itemClicked(const QString&, const QString&)), handler,
                SLOT(insertItem(const QString&, const QString&)));
        connect(m_project, SIGNAL(itemDoubleClicked(QString)), this,
                SLOT(open(QString)));

        m_projectDockWidget = new QDockWidget(m_project->windowTitle(), this);
        m_projectDockWidget->setWidget(m_project);
        m_projectDockWidget->setObjectName("projectsSidebar");
        m_projectDockWidget->setVisible(false);
        addDockWidget(Qt::LeftDockWidgetArea, m_projectDockWidget);
    }

    //! \brief Initializes the FolderBrowser sidebar.
    void MainWindow::setupFolderBrowserSidebar()
    {
        m_folderBrowser = new FolderBrowser(this);

        connect(m_folderBrowser, SIGNAL(itemDoubleClicked(QString)), this,
                SLOT(open(QString)));

        m_browserDockWidget = new QDockWidget(m_folderBrowser->windowTitle(), this);
        m_browserDockWidget->setWidget(m_folderBrowser);
        m_browserDockWidget->setObjectName("folderBrowserSidebar");
        addDockWidget(Qt::LeftDockWidgetArea, m_browserDockWidget);
        tabifyDockWidget(m_browserDockWidget, m_projectDockWidget);
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

        // Load the actions checked state.
        ActionManager* am = ActionManager::instance();
        am->actionForName("showMenuBar")->setChecked(settings->currentValue("gui/showMenuBar").toBool());
        am->actionForName("showToolBar")->setChecked(settings->currentValue("gui/showToolBar").toBool());
        am->actionForName("showStatusBar")->setChecked(settings->currentValue("gui/showStatusBar").toBool());
        am->actionForName("showSideBarBrowser")->setChecked(settings->currentValue("gui/showSideBarBrowser").toBool());
        am->actionForName("showFolderBrowser")->setChecked(settings->currentValue("gui/showFolderBrowser").toBool());
        am->actionForName("showAll")->setChecked(settings->currentValue("gui/showAll").toBool());
        am->actionForName("showFullScreen")->setChecked(settings->currentValue("gui/showFullScreen").toBool());

        // Update the visibility status
        updateVisibility();
        showFullScreen();
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

        // Update current menu and statusbar visibility values before saving.
        ActionManager* am = ActionManager::instance();
        settings->setCurrentValue("gui/showMenuBar", am->actionForName("showMenuBar")->isChecked());
        settings->setCurrentValue("gui/showToolBar", am->actionForName("showToolBar")->isChecked());
        settings->setCurrentValue("gui/showStatusBar", am->actionForName("showStatusBar")->isChecked());
        settings->setCurrentValue("gui/showSideBarBrowser", am->actionForName("showSideBarBrowser")->isChecked());
        settings->setCurrentValue("gui/showFolderBrowser", am->actionForName("showFolderBrowser")->isChecked());
        settings->setCurrentValue("gui/showAll", am->actionForName("showAll")->isChecked());
        settings->setCurrentValue("gui/showFullScreen", am->actionForName("showFullScreen")->isChecked());

        settings->save();
    }

    //! \brief Launch the global context menu
    void MainWindow::contextMenuEvent(QContextMenuEvent *event)
    {
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu(this);

        _menu->addAction(am->actionForName("showAll"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("showMenuBar"));
        _menu->addAction(am->actionForName("showToolBar"));
        _menu->addAction(am->actionForName("showStatusBar"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("showSideBarBrowser"));
        _menu->addAction(am->actionForName("showFolderBrowser"));

        _menu->exec(event->globalPos());
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

} // namespace Caneda
