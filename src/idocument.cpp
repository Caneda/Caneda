/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "idocument.h"

#include "actionmanager.h"
#include "chartscene.h"
#include "chartview.h"
#include "documentviewmanager.h"
#include "fileformats.h"
#include "graphicsscene.h"
#include "icontext.h"
#include "iview.h"
#include "mainwindow.h"
#include "messagewidget.h"
#include "portsymbol.h"
#include "settings.h"
#include "statehandler.h"
#include "syntaxhighlighters.h"
#include "textedit.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPrinter>
#include <QProcess>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextStream>

namespace Caneda
{
    /*************************************************************************
     *                             IDocument                                 *
     *************************************************************************/
    /*!
     * \fn IView::context()
     *
     * \brief Returns the context that handles documents, views of specific type.
     * \note It is enough to create the context object only once per new type.
     * \sa IContext
     */

    /*!
     * \fn IDocument::isModified()
     *
     * \brief Returns modification status of this document.
     */

    /*!
     * \fn IDocument::canUndo()
     *
     * \brief Returns whether the undo action can be invoked or not.
     * This is used as clue to enable/disable the undo action.
     */

    /*!
     * \fn IDocument::canRedo()
     *
     * \brief Returns whether the redo action can be invoked or not.
     * This is used as clue to enable/disable the redo action.
     */

    /*!
     * \fn IDocument::undo()
     *
     * \brief Invokes undo action i.e undoes the last change.
     *
     */

    /*!
     * \fn IDocument::redo()
     *
     * \brief Invokes redo action i.e redoes the last undo change.
     */

    /*!
     * \fn IDocument::canCut()
     *
     * \brief Returns whether cut action can be invoked or not.
     * This is used as clue to enable/disable the cut action.
     */

    /*!
     * \fn IDocument::canCopy()
     *
     * \brief Returns whether copy action can be invoked or not.
     * This is used as clue to enable/disable the copy action.
     */

    /*!
     * \fn IDocument::canPaste()
     *
     * \brief Returns whether the paste action can be invoked or not.
     * This is used as clue to enable/disable the paste action.
     * Clipboard emptyness, relevance of clipboard content can be used to
     * return appropriate result.
     */

    /*!
     * \fn IDocument::cut()
     *
     * \brief Invokes the cut action.
     */

    /*!
     * \fn IDocument::copy()
     *
     * \brief Invokes the copy action.
     */

    /*!
     * \fn IDocument::paste()
     *
     * \brief Invokes the paste action.
     */

    /*!
     * \fn IDocument::selectAll()
     *
     * \brief Select all elements in the document.
     */

    /*!
     * \fn IDocument::enterHierarchy()
     *
     * \brief Open selected item for edition.
     *
     * This is used mainly in graphic elements, where each item can have a
     * whole new scene to describe it.
     *
     * In a schematic scene, for example, where each item is an electronic
     * element, one could open a selected item to edit its subcircuit. In the
     * case of a layout circuit using hierarchies serve its purpose to edit
     * certain circuit (for example a flip-flop or a not gate) and then use it
     * in read only mode in higher hierarchies. When one needs to edit that
     * subcircuit, the action enterHierarchy() comes into place.
     *
     * \sa exitHierarchy()
     */

    /*!
     * \fn IDocument::exitHierarchy()
     *
     * \brief Open parent item for edition.
     *
     * This is used mainly in graphic elements, where each item can be included
     * in a whole new scene to make more complex scenes.
     *
     * In a schematic scene, for example, each circuit described can be
     * included in a new circuit to make bigger or more complex circuits. In
     * the case of a layout, each circuit topology (for example a flip-flop or
     * a not gate) can be included in read-only mode in bigger circuits,
     * allowing for the creation of a bottom-up approach.
     *
     * \sa enterHierarchy()
     */

    /*!
     * \fn IDocument::alignTop()
     *
     * \brief Align elements in a row correponding to top most elements
     * coordinates.
     */

    /*!
     * \fn IDocument::alignBottom()
     *
     * \brief Align elements in a row correponding to bottom most elements
     * coordinates.
     */

    /*!
     * \fn IDocument::alignLeft()
     *
     * \brief Align elements in a column correponding to left most elements
     * coordinates.
     */

    /*!
     * \fn IDocument::alignRight()
     *
     * \brief Align elements in a column correponding to right most elements
     * coordinates.
     */

    /*!
     * \fn IDocument::distributeHorizontal()
     *
     * \brief Distribute elements in columns horizontally
     */

    /*!
     * \fn IDocument::distributeVertical()
     *
     * \brief Distribute elements in rows vertically
     */

    /*!
     * \fn IDocument::centerHorizontal()
     *
     * \brief Center elements horizontally
     */

    /*!
     * \fn IDocument::centerVertical()
     *
     * \brief Center elements vertically
     */

    /*!
     * \fn IDocument::simulate()
     *
     * \brief Simulate current document
     */

    /*!
     * \fn IDocument::exportImage()
     *
     * \brief Export current document to an image.
     *
     * This method is called directly by the ExportDialog class,
     * upon user input. Not all document types can be exported to
     * images.
     */

    /*!
     * \fn IDocument::documentSize()
     *
     * \brief Return current document size.
     *
     * This method is called from the ExportDialog class, to determine
     * the image default size to use in the export.
     *
     * \return Size of the document.
     *
     * \sa exportImage()
     */

    /*!
     * \fn IDocument::load(QString *errorMessage = 0)
     *
     * \brief Loads the document from file IDocument::fileName()
     * \return true on success, false on failure.
     * \param errorMessage If load operation fails and errorMessage points to valid
     *                     location, the appropriate message corresponding for failure
     *                     is set.
     *
     * \sa \ref DocumentFormats
     */

    /*!
     * \fn IDocument::save(QString *errorMessage = 0)
     *
     * \brief Saves the document into file IDocument::fileName()
     * \return true on success, false on failure.
     * \param errorMessage If save operation fails and errorMessage points to valid
     *                     location, the appropriate message corresponding for failure
     *                     is set.
     *
     * \sa \ref DocumentFormats
     */

    /*!
     * \fn IDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
     *
     * \brief Constructs and returns a context menu with the actions
     * corresponding to the selected object.
     */

    /*!
     * \fn IDocument::launchPropertiesDialog()
     *
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
     * be presented for the user.
     */

    /*!
     * \fn IDocument::documentChanged()
     *
     * \brief This signal is emitted when any of document state changes.
     * The following are the changes for which this signal is emitted.
     * fileName, isModified, canUndo, canRedo, canCut, canCopy and canPaste.
     */

    /*!
     * \fn IDocument::statusBarMessage(const QString& text)
     *
     * \brief This signal is emitted whenever the document desires to display any
     *        message in the status bar.
     */

    //! \brief Constructor.
    IDocument::IDocument(QObject *parent) : QObject(parent)
    {
    }

    //! \brief Returns the fileName represented by this document.
    QString IDocument::fileName() const
    {
        return m_fileName;
    }

    void IDocument::setFileName(const QString &fileName)
    {
        m_fileName = fileName;
        emit documentChanged(this);
    }

    /*!
     * \brief Returns a list of views viewing this document.
     */
    QList<IView*> IDocument::views() const
    {
        return DocumentViewManager::instance()->viewsForDocument(this);
    }

    void IDocument::emitDocumentChanged()
    {
        emit documentChanged(this);
    }


    /*************************************************************************
     *                           LayoutDocument                              *
     *************************************************************************/
    //! \brief Constructor.
    LayoutDocument::LayoutDocument(QObject *parent) : IDocument(parent)
    {
        m_graphicsScene = new GraphicsScene(this);
        connect(m_graphicsScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_graphicsScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_graphicsScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_graphicsScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    //! \brief Destructor.
    LayoutDocument::~LayoutDocument()
    {
        delete m_graphicsScene;
    }

    IContext* LayoutDocument::context()
    {
        return LayoutContext::instance();
    }

    bool LayoutDocument::isModified() const
    {
        return !m_graphicsScene->undoStack()->isClean();
    }

    bool LayoutDocument::canUndo() const
    {
        return m_graphicsScene->undoStack()->canUndo();
    }

    bool LayoutDocument::canRedo() const
    {
        return m_graphicsScene->undoStack()->canRedo();
    }

    void LayoutDocument::undo()
    {
        m_graphicsScene->undoStack()->undo();
    }

    void LayoutDocument::redo()
    {
        m_graphicsScene->undoStack()->redo();
    }

    bool LayoutDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        return schItems.isEmpty() == false;
    }

    bool LayoutDocument::canCopy() const
    {
        return LayoutDocument::canCut();
    }

    void LayoutDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_graphicsScene->cutItems(schItems);
        }
    }

    void LayoutDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_graphicsScene->copyItems(schItems);
        }
    }

    void LayoutDocument::paste()
    {
        StateHandler::instance()->paste();
    }

    void LayoutDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_graphicsScene->sceneRect());
        m_graphicsScene->setSelectionArea(path);
    }

    void LayoutDocument::enterHierarchy()
    {
        //! \todo Implement this
    }

    void LayoutDocument::exitHierarchy()
    {
        //! \todo Implement this
    }

    void LayoutDocument::alignTop()
    {
        alignElements(Qt::AlignTop);
    }

    void LayoutDocument::alignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    void LayoutDocument::alignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    void LayoutDocument::alignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void LayoutDocument::distributeHorizontal()
    {
        if (!m_graphicsScene->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void LayoutDocument::distributeVertical()
    {
        if (!m_graphicsScene->distributeElements(Qt::Vertical)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void LayoutDocument::centerHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void LayoutDocument::centerVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    void LayoutDocument::simulate()
    {
        //! \todo Implement this
    }

    void LayoutDocument::print(QPrinter *printer, bool fitInView)
    {
        m_graphicsScene->print(printer, fitInView);
    }

    void LayoutDocument::exportImage(QPaintDevice &device)
    {
        m_graphicsScene->exportImage(device);
    }

    QSizeF LayoutDocument::documentSize()
    {
        return m_graphicsScene->itemsBoundingRect().size();
    }

    bool LayoutDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "xlay") {
            FormatXmlLayout *format = new FormatXmlLayout(this);
            return format->load();
        }

        if (errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    bool LayoutDocument::save(QString *errorMessage)
    {
        if(fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty file name");
            }
            return false;
        }

        QFileInfo info(fileName());

        if(info.suffix() == "xlay") {
            FormatXmlLayout *format = new FormatXmlLayout(this);
            if(!format->save()) {
                return false;
            }

            m_graphicsScene->undoStack()->clear();
            return true;
        }

        if(errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    IView* LayoutDocument::createView()
    {
        return new LayoutView(this);
    }

    void LayoutDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        // Launch the context of the current document
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();
        _menu->addAction(am->actionForName("select"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editPaste"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("openSchematic"));
        _menu->addAction(am->actionForName("openSymbol"));

        _menu->exec(event->screenPos());
    }

    void LayoutDocument::launchPropertiesDialog()
    {
        // Get a list of selected items
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        // If there is any selection, launch corresponding properties dialog.
        if(!schItems.isEmpty()) {
            foreach(GraphicsItem *item, schItems) {
                item->launchPropertiesDialog();
            }
        }
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void LayoutDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_graphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }


    /*************************************************************************
     *                          SchematicDocument                            *
     *************************************************************************/
    //! \brief Constructor.
    SchematicDocument::SchematicDocument(QObject *parent) : IDocument(parent)
    {
        m_graphicsScene = new GraphicsScene(this);
        connect(m_graphicsScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_graphicsScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_graphicsScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_graphicsScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    //! \brief Destructor.
    SchematicDocument::~SchematicDocument()
    {
        delete m_graphicsScene;
    }

    IContext* SchematicDocument::context()
    {
        return SchematicContext::instance();
    }

    bool SchematicDocument::isModified() const
    {
        return !m_graphicsScene->undoStack()->isClean();
    }

    bool SchematicDocument::canUndo() const
    {
        return m_graphicsScene->undoStack()->canUndo();
    }

    bool SchematicDocument::canRedo() const
    {
        return m_graphicsScene->undoStack()->canRedo();
    }

    void SchematicDocument::undo()
    {
        m_graphicsScene->undoStack()->undo();
    }

    void SchematicDocument::redo()
    {
        m_graphicsScene->undoStack()->redo();
    }

    bool SchematicDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        return schItems.isEmpty() == false;
    }

    bool SchematicDocument::canCopy() const
    {
        return SchematicDocument::canCut();
    }

    void SchematicDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_graphicsScene->cutItems(schItems);
        }
    }

    void SchematicDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_graphicsScene->copyItems(schItems);
        }
    }

    void SchematicDocument::paste()
    {
        StateHandler::instance()->paste();
    }

    void SchematicDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_graphicsScene->sceneRect());
        m_graphicsScene->setSelectionArea(path);
    }

    void SchematicDocument::enterHierarchy()
    {
        //! \todo Implement this
    }

    void SchematicDocument::exitHierarchy()
    {
        //! \todo Implement this
    }

    void SchematicDocument::alignTop()
    {
        alignElements(Qt::AlignTop);
    }

    void SchematicDocument::alignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    void SchematicDocument::alignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    void SchematicDocument::alignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void SchematicDocument::distributeHorizontal()
    {
        if (!m_graphicsScene->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void SchematicDocument::distributeVertical()
    {
        if (!m_graphicsScene->distributeElements(Qt::Vertical)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void SchematicDocument::centerHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void SchematicDocument::centerVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    /*!
     * \brief Start a simulation
     *
     * Start a simulation, first generating the schematic netlist and then
     * opening the waveform viewer (could be internal or external acording to
     * the user settings).
     *
     * \sa simulationReady(), simulationError(), performBasicChecks()
     */
    void SchematicDocument::simulate()
    {
        /*! \todo In the future (after Qt5.6 is released), instead of this
         * first check, simply connect the simulation process with the slot
         * simulationError, in a way similar to the following:
         * connect(simulationProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(simulationError(QProcess::ProcessError)));
         * This will check against any simulator, instead of the current
         * check that only verifies the ngspice installation.
         * \sa simulationError()
         */
        if(!simulationError()) {
            return;
        }

        if(!performBasicChecks()) {
            return;
        }

        QFileInfo info(fileName());
        QString baseName = info.completeBaseName();
        QString path = info.path();

        // First export the schematic to a spice netlist
        if(info.suffix() == "xsch") {
            FormatSpice *format = new FormatSpice(this);
            format->save();
        }

        // Invoke a spice simulator in batch mode
        Settings *settings = Settings::instance();
        QString simulationCommand = settings->currentValue("sim/simulationCommand").toString();
        simulationCommand.replace("%filename", baseName);  // Replace all ocurrencies of %filename by the actual filename

        QProcess *simulationProcess = new QProcess(this);
        simulationProcess->setWorkingDirectory(path);
        simulationProcess->setProcessChannelMode(QProcess::MergedChannels);  // Output std:error and std:output together into the same file
        simulationProcess->setStandardOutputFile(path + "/" + baseName + ".log", QIODevice::WriteOnly);  // Create a log file

        // Set the environment variable to get a binary or an ascii raw file.
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        if(settings->currentValue("sim/outputFormat").toString() == "binary") {
            env.insert("SPICE_ASCIIRAWFILE", "0"); // Add an environment variable
        }
        else if(settings->currentValue("sim/outputFormat").toString() == "ascii") {
            env.insert("SPICE_ASCIIRAWFILE", "1"); // Add an environment variable
        }
        simulationProcess->setProcessEnvironment(env);

        // Start the simulation
        simulationProcess->start(simulationCommand);

        // The simulation results are opened in the simulationReady slot, to avoid blocking the interface while simulating
        connect(simulationProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(simulationReady(int)));
    }

    void SchematicDocument::print(QPrinter *printer, bool fitInView)
    {
        m_graphicsScene->print(printer, fitInView);
    }

    void SchematicDocument::exportImage(QPaintDevice &device)
    {
        m_graphicsScene->exportImage(device);
    }

    QSizeF SchematicDocument::documentSize()
    {
        return m_graphicsScene->itemsBoundingRect().size();
    }

    bool SchematicDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "xsch") {
            FormatXmlSchematic *format = new FormatXmlSchematic(this);
            return format->load();
        }

        if (errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    bool SchematicDocument::save(QString *errorMessage)
    {
        if(fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty file name");
            }
            return false;
        }

        QFileInfo info(fileName());

        if(info.suffix() == "xsch") {
            FormatXmlSchematic *format = new FormatXmlSchematic(this);
            if(!format->save()) {
                return false;
            }

            m_graphicsScene->undoStack()->clear();
            return true;
        }

        if(errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    IView* SchematicDocument::createView()
    {
        return new SchematicView(this);
    }

    void SchematicDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        // Launch the context of the current document
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();
        _menu->addAction(am->actionForName("select"));
        _menu->addAction(am->actionForName("insertWire"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editPaste"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("openSymbol"));
        _menu->addAction(am->actionForName("openLayout"));
        _menu->addAction(am->actionForName("openSimulation"));

        //! \todo Reenable these options once implemented
        //                _menu->addAction(am->actionForName("enterHierarchy"));
        //                _menu->addAction(am->actionForName("exitHierarchy"));

        _menu->exec(event->screenPos());
    }

    void SchematicDocument::launchPropertiesDialog()
    {
        // Get a list of selected items
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        // If there is any selection, launch corresponding properties dialog.
        if(!schItems.isEmpty()) {
            foreach(GraphicsItem *item, schItems) {
                item->launchPropertiesDialog();
            }
        }
    }

    /*!
     * \brief Open simulation results.
     *
     * Once a simulation is started, the simulation process is connected to
     * this slot, to achive non-modal simulations (ie, the gui is responsive
     * during simulations).
     *
     * Once the simulation has finished, this slot is invoked and if no error
     * ocurred, simulation results are shown to the user. The waveform viewer
     * can be internal or external acording to the user settings.
     *
     * \sa simulate()
     */
    void SchematicDocument::simulationReady(int error)
    {
        // Test for errors, and open log file (in case something went wrong).
        // If there was an error, do not display the waveforms
        if(error) {

            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *view = manager->currentView();

            MessageWidget *dialog = new MessageWidget("There was an error during the simulation...", view->toWidget());
            dialog->setMessageType(MessageWidget::Error);
            dialog->setIcon(Caneda::icon("dialog-error"));

            ActionManager* am = ActionManager::instance();
            QAction* act = am->actionForName("openLog");
            dialog->addAction(act);

            act = am->actionForName("openNetlist");
            dialog->addAction(act);

            dialog->show();

            return;
        }

        // Open the resulting waveforms
        DocumentViewManager *manager = DocumentViewManager::instance();

        QFileInfo info(fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();

        manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".raw"));
    }

    /*!
     * \brief Check what error has occured and show a message to the user.
     *
     * This method checks the kind of error present on the process and shows a
     * message to the user. Basically, this method is called when the process
     * could not start due to a missing or incorrect installation of the
     * simulation backend. Other checks are performed in the performBasicChecks()
     * method.
     *
     * \todo In the future, when Qt5.6 is out, use this method to connect to
     * the QProcess::errorOccurred(QProcess::ProcessError) signal in the
     * simulate method. That would allow to check against a generic simulator,
     * without needing to use a predefined simulation command. As in Qt
     * previous versions to 5.6 that signal was not present, this method was
     * used with a return value and called before calling the simulator. As
     * beforehand we do not know what simulator is to be used and what commands
     * are available we only check for the default simulator, ie, ngspice. The
     * ngspice simulator has the -v parameter which is used to check if
     * it is installed without needing to call the entire simulation just to
     * check if the binary is present. When the signal is available (after
     * Qt5.6 is released and available in the most common distributions) we
     * can call this method in the form of a slot connected to the previous
     * signal. In that way, we do not need to know the simulation command and
     * instead this method is called if there's any problem.
     * In that time, this method should be declared as:
     * void SchematicDocument::simulationError(QProcess::ProcessError).
     * In the implementation, instead of using all the presently involved checks,
     * simply we should check the passed argument QProcess::ProcessError and
     * checked against QProcess::FailedToStart, triggering a message similar to
     * the currently present.
     *
     * \sa simulate(), performBasicChecks()
     */
    bool SchematicDocument::simulationError()
    {
        // Get the current spice command
        Settings *settings = Settings::instance();
        QString simulationCommand = settings->currentValue("sim/simulationCommand").toString();

        // If using ngspice (the default spice backend) check if its installed.
        if(simulationCommand.startsWith("ngspice")) {

            simulationCommand = QString("ngspice -v");
            QProcess *simulationProcess = new QProcess(this);
            simulationProcess->start(simulationCommand);
            simulationProcess->waitForFinished();

            if(simulationProcess->error() == QProcess::FailedToStart) {
                // If FailedToStart either ngspice is missing, the user has
                // insufficient permissions to invoke the program.
                DocumentViewManager *manager = DocumentViewManager::instance();
                IView *view = manager->currentView();

                MessageWidget *dialog = new MessageWidget(tr("Missing simulator backend..."), view->toWidget());
                dialog->setMessageType(MessageWidget::Error);
                dialog->setIcon(Caneda::icon("dialog-error"));

                QAction *action = new QAction(Caneda::icon("help-contents"), tr("More info..."), this);
                connect(action, SIGNAL(triggered()), SLOT(showSimulationHelp()));

                dialog->addAction(action);
                dialog->show();

                return false;
            }
        }

        return true;
    }

    //! \brief Opens the simulation help.
    void SchematicDocument::showSimulationHelp()
    {
        QDesktopServices::openUrl(QUrl("http://docs.caneda.org/en/latest/simulationerrors.html"));
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void SchematicDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_graphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    /*!
     * \brief Perform basic checks to determine if we are ready to perform a
     * simulation.
     *
     * This method performs the basic checks and throws an error if we are
     * not ready to simulate the schematic. Some of the checks performed are:
     * \li The presence of a filename to simulate.
     * \li The presence of a ground node in the schematic.
     * \li The presence of a simulation profile in the schematic.
     *
     * \return True if all checks are ok, false if there are errors.
     *
     * \sa simulate(), simulationError()
     */
    bool SchematicDocument::performBasicChecks()
    {
        //***************************************
        // Check if there is a filename
        //***************************************
        // Although there's no need to save the file before simulating,
        // we need at least a filename to perform the simulation.
        if(fileName().isEmpty()) {
            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *view = manager->currentView();

            MessageWidget *dialog = new MessageWidget(tr("Missing filename. Please save at least once before performing a simulation..."),
                                                      view->toWidget());
            dialog->setMessageType(MessageWidget::Error);
            dialog->setIcon(Caneda::icon("dialog-error"));
            dialog->show();

            return false;
        }

        //***************************************
        // Check for the presence of a ground net
        //***************************************
        // Get a list of all the items in the scene
        QList<QGraphicsItem*> items = m_graphicsScene->items();

        bool foundGroundNet = false;

        // Iterate over all PortSymbols
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);

        foreach(PortSymbol *p, portSymbols) {
            if(p->label().toLower() == "ground" || p->label().toLower() == "gnd") {
                foundGroundNet = true;
            }
        }

        // If didn't find a ground net display an error
        if(!foundGroundNet) {
            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *view = manager->currentView();

            MessageWidget *dialog = new MessageWidget(tr("Missing ground net..."), view->toWidget());
            dialog->setMessageType(MessageWidget::Error);
            dialog->setIcon(Caneda::icon("dialog-error"));

            QAction *action = new QAction(Caneda::icon("help-contents"), tr("More info..."), this);
            connect(action, SIGNAL(triggered()), SLOT(showSimulationHelp()));

            dialog->addAction(action);
            dialog->show();

            return false;
        }

        //***********************************************
        // Check for the presence of a simulation profile
        //***********************************************
        bool foundSimulationProfile = false;

        // Iterate over all components
        QList<Component*> components = filterItems<Component>(items);

        // Check for a component that starts with "Sim" as keyword. Although
        // theoretically any component could be named like this, this is the
        // best check we can do.
        foreach(Component *c, components) {
            if(c->label().startsWith("Sim")) {
                foundSimulationProfile = true;
            }
        }

        // If didn't find a simulation profile display an error
        if(!foundSimulationProfile) {
            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *view = manager->currentView();

            MessageWidget *dialog = new MessageWidget(tr("Missing simulation profile..."), view->toWidget());
            dialog->setMessageType(MessageWidget::Error);
            dialog->setIcon(Caneda::icon("dialog-error"));

            QAction *action = new QAction(Caneda::icon("help-contents"), tr("More info..."), this);
            connect(action, SIGNAL(triggered()), SLOT(showSimulationHelp()));

            dialog->addAction(action);
            dialog->show();

            return false;
        }

        // Default return value
        return true;
    }


    /*************************************************************************
     *                         SimulationDocument                            *
     *************************************************************************/
    //! \brief Constructor.
    SimulationDocument::SimulationDocument(QObject *parent) : IDocument(parent)
    {
        m_chartScene = new ChartScene;
    }

    //! \brief Destructor.
    SimulationDocument::~SimulationDocument()
    {
        delete m_chartScene;
    }

    IContext* SimulationDocument::context()
    {
        return SimulationContext::instance();
    }

    void SimulationDocument::distributeHorizontal()
    {
        /*!
         * \todo Implement this. This method should distribute the available
         * waveforms into several graphs, distributed horizontally.
         */
    }

    void SimulationDocument::distributeVertical()
    {
        /*!
         * \todo Implement this. This method should distribute the available
         * waveforms into several graphs, distributed vertically.
         */
    }

    void SimulationDocument::centerHorizontal()
    {
        //! \todo Implement this. Merge horizontally distributed waveforms.
    }

    void SimulationDocument::centerVertical()
    {
        //! \todo Implement this. Merge vertically distributed waveforms.
    }

    void SimulationDocument::print(QPrinter *printer, bool fitInView)
    {
        /*!
         * Get current view, and print it. This method differs from
         * the other idocument implementations, as the scene has no
         * way to know the actual curves being displayed on the current
         * view.
         */
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView *v = manager->currentView();
        ChartView *cv = qobject_cast<ChartView*>(v->toWidget());

        cv->print(printer, fitInView);
    }

    void SimulationDocument::exportImage(QPaintDevice &device)
    {
        /*!
         * Get current view, and print it. This method differs from
         * the other idocument implementations, as the scene has no
         * way to know the actual curves being displayed on the current
         * view.
         */
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView *v = manager->currentView();
        ChartView *cv = qobject_cast<ChartView*>(v->toWidget());

        cv->exportImage(device);
    }

    QSizeF SimulationDocument::documentSize()
    {
        //! \todo Using fixed size for document export. Should we make this configurable?
        QSizeF size(297, 210);
        return size;
    }

    bool SimulationDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "raw") {
            FormatRawSimulation *format = new FormatRawSimulation(this);
            return format->load();
        }

        if (errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    IView* SimulationDocument::createView()
    {
        return new SimulationView(this);
    }

    void SimulationDocument::launchPropertiesDialog()
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        IView *v = manager->currentView();
        ChartView *cv = qobject_cast<ChartView*>(v->toWidget());

        if(cv) {
            cv->launchPropertiesDialog();
        }
    }


    /*************************************************************************
     *                           SymbolDocument                              *
     *************************************************************************/
    //! \brief Constructor.
    SymbolDocument::SymbolDocument(QObject *parent) : IDocument(parent)
    {
        m_graphicsScene = new GraphicsScene(this);
        connect(m_graphicsScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_graphicsScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_graphicsScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_graphicsScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    //! \brief Destructor.
    SymbolDocument::~SymbolDocument()
    {
        delete m_graphicsScene;
    }

    IContext* SymbolDocument::context()
    {
        return SymbolContext::instance();
    }

    bool SymbolDocument::isModified() const
    {
        return !m_graphicsScene->undoStack()->isClean();
    }

    bool SymbolDocument::canUndo() const
    {
        return m_graphicsScene->undoStack()->canUndo();
    }

    bool SymbolDocument::canRedo() const
    {
        return m_graphicsScene->undoStack()->canRedo();
    }

    void SymbolDocument::undo()
    {
        m_graphicsScene->undoStack()->undo();
    }

    void SymbolDocument::redo()
    {
        m_graphicsScene->undoStack()->redo();
    }

    bool SymbolDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> symItems = filterItems<GraphicsItem>(qItems);

        return symItems.isEmpty() == false;
    }

    bool SymbolDocument::canCopy() const
    {
        return SymbolDocument::canCut();
    }

    void SymbolDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> symItems = filterItems<GraphicsItem>(qItems);

        if(!symItems.isEmpty()) {
            m_graphicsScene->cutItems(symItems);
        }
    }

    void SymbolDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> symItems = filterItems<GraphicsItem>(qItems);

        if(!symItems.isEmpty()) {
            m_graphicsScene->copyItems(symItems);
        }
    }

    void SymbolDocument::paste()
    {
        StateHandler::instance()->paste();
    }

    void SymbolDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_graphicsScene->sceneRect());
        m_graphicsScene->setSelectionArea(path);
    }

    void SymbolDocument::enterHierarchy()
    {
        //! \todo Implement this. This should return to the schematic document.
    }

    void SymbolDocument::exitHierarchy()
    {
        //! \todo Implement this. This should return to the schematic document.
    }

    void SymbolDocument::alignTop()
    {
        alignElements(Qt::AlignTop);
    }

    void SymbolDocument::alignBottom()
    {
        alignElements(Qt::AlignBottom);
    }

    void SymbolDocument::alignLeft()
    {
        alignElements(Qt::AlignLeft);
    }

    void SymbolDocument::alignRight()
    {
        alignElements(Qt::AlignRight);
    }

    void SymbolDocument::distributeHorizontal()
    {
        if (!m_graphicsScene->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void SymbolDocument::distributeVertical()
    {
        if (!m_graphicsScene->distributeElements(Qt::Vertical)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void SymbolDocument::centerHorizontal()
    {
        alignElements(Qt::AlignHCenter);
    }

    void SymbolDocument::centerVertical()
    {
        alignElements(Qt::AlignVCenter);
    }

    void SymbolDocument::print(QPrinter *printer, bool fitInView)
    {
        m_graphicsScene->print(printer, fitInView);
    }

    void SymbolDocument::exportImage(QPaintDevice &device)
    {
        m_graphicsScene->exportImage(device);
    }

    QSizeF SymbolDocument::documentSize()
    {
        return m_graphicsScene->itemsBoundingRect().size();
    }

    bool SymbolDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "xsym") {
            FormatXmlSymbol *format = new FormatXmlSymbol(this);
            return format->load();
        }

        if (errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    bool SymbolDocument::save(QString *errorMessage)
    {
        if(fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty file name");
            }
            return false;
        }

        QFileInfo info(fileName());

        if(info.suffix() == "xsym") {
            FormatXmlSymbol *format = new FormatXmlSymbol(this);
            if(!format->save()) {
                return false;
            }

            m_graphicsScene->undoStack()->clear();
            return true;
        }

        if(errorMessage) {
            *errorMessage = tr("Unknown file format!");
        }

        return false;
    }

    IView* SymbolDocument::createView()
    {
        return new SymbolView(this);
    }

    void SymbolDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        // Launch the context of the current document
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();
        _menu->addAction(am->actionForName("select"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editPaste"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("openSchematic"));
        _menu->addAction(am->actionForName("openLayout"));
        _menu->addAction(am->actionForName("propertiesDialog"));

        _menu->exec(event->screenPos());
    }

    void SymbolDocument::launchPropertiesDialog()
    {
        // Get a list of selected items
        QList<QGraphicsItem*> qItems = m_graphicsScene->selectedItems();
        QList<GraphicsItem*> schItems = filterItems<GraphicsItem>(qItems);

        // If there is any selection, launch corresponding properties dialog,
        // else launch the properties dialog corresponding to the current scene
        if(!schItems.isEmpty()) {
            foreach(GraphicsItem *item, schItems) {
                item->launchPropertiesDialog();
            }
        }
        else {
            m_graphicsScene->properties()->launchPropertiesDialog();
        }
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void SymbolDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_graphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }


    /*************************************************************************
     *                            TextDocument                               *
     *************************************************************************/
    //! \brief Constructor.
    TextDocument::TextDocument(QObject *parent) : IDocument(parent)
    {
        m_textDocument = new QTextDocument;
        m_textDocument->setModified(false);

        connect(m_textDocument, SIGNAL(undoAvailable(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_textDocument, SIGNAL(redoAvailable(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_textDocument, SIGNAL(modificationChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_textDocument, SIGNAL(contentsChanged()),
                this, SLOT(onContentsChanged()));
    }

    //! \brief Destructor.
    TextDocument::~TextDocument()
    {
        delete m_textDocument;
    }

    IContext* TextDocument::context()
    {
        return TextContext::instance();
    }

    bool TextDocument::isModified() const
    {
        return m_textDocument->isModified();
    }

    bool TextDocument::canUndo() const
    {
        return m_textDocument->isUndoAvailable();
    }

    bool TextDocument::canRedo() const
    {
        return m_textDocument->isRedoAvailable();
    }

    void TextDocument::undo()
    {
        m_textDocument->undo();
    }

    void TextDocument::redo()
    {
        m_textDocument->redo();
    }

    void TextDocument::cut()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->cut();
    }

    void TextDocument::copy()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->copy();
    }

    void TextDocument::paste()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->paste();
    }

    void TextDocument::selectAll()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->selectAll();
    }

    void TextDocument::enterHierarchy()
    {
        //! \todo Implement this. This should open currently selected file.
    }

    void TextDocument::exitHierarchy()
    {
        //! \todo Implement this. This should return to previously opened file.
    }

    /*!
     * \brief Start a simulation
     *
     * Start a simulation, invoking the correct simulator depending on the
     * file extension, and then open the waveform viewer (could be internal
     * or external acording to the user settings).
     *
     * \sa simulationReady()
     */
    void TextDocument::simulate()
    {
        simulationErrorStatus = false;

        QFileInfo info(fileName());
        QString baseName = info.completeBaseName();
        QString suffix = info.suffix();
        QString path = info.path();

        QProcess *simulationProcess = new QProcess(this);
        simulationProcess->setWorkingDirectory(path);
        simulationProcess->setProcessChannelMode(QProcess::MergedChannels);  // Output std:error and std:output together into the same file
        simulationProcess->setStandardOutputFile(path + "/" + baseName + ".log", QIODevice::WriteOnly);  // Create a log file

        connect(simulationProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(simulationLog(int)));


        if (suffix == "net" || suffix == "cir" || suffix == "spc" || suffix == "sp") {
            // It is a netlist file, we should invoke a spice simulator in batch mode
            Settings *settings = Settings::instance();
            QString simulationCommand = settings->currentValue("sim/simulationCommand").toString();
            simulationCommand.replace("%filename", baseName);  // Replace all ocurrencies of %filename by the actual filename

            // Set the environment variable to get a binary or an ascii raw file.
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            if(settings->currentValue("sim/outputFormat").toString() == "binary") {
                env.insert("SPICE_ASCIIRAWFILE", "0"); // Add an environment variable
            }
            else if(settings->currentValue("sim/outputFormat").toString() == "ascii") {
                env.insert("SPICE_ASCIIRAWFILE", "1"); // Add an environment variable
            }
            simulationProcess->setProcessEnvironment(env);

            // Start the simulation
            simulationProcess->start(simulationCommand);
        }
        else if (suffix == "vhd" || suffix == "vhdl") {
            // It is a vhdl file, we should invoke ghdl simulator
            /*! \todo Here we should analize (ghdl -a) all included files of the vhdl project.
             *  This could be done in a recursive way (although that is not optimal). That is,
             *  try to compile all files in the directory. At least of the files will compile,
             *  as it does not depend on the others. Then add that file to a list, and compile
             *  the rest. Again add the files that compile and go on until no file is left to
             *  compile.
             */

            simulationProcess->start(QString("ghdl -a ") + fileName());  // Analize the files
            simulationProcess->waitForFinished();
            simulationProcess->start(QString("ghdl -e ") + baseName);  // Create the simulation
            simulationProcess->waitForFinished();
            simulationProcess->start(QString("./") + baseName + " --wave=waveforms.ghw");  // Run the simulation
        }
        else if (suffix == "v") {
            // Is is a verilog file, we should invoke iverilog
            simulationProcess->start(QString("iverilog ") + fileName());  // Analize the files
            simulationProcess->waitForFinished();
            simulationProcess->start(QString("./a.out"));  // Run the simulation
        }

        // The simulation results are opened in the simulationReady slot, to achieve non-modal simulations
        connect(simulationProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(simulationReady(int)));
    }

    void TextDocument::print(QPrinter *printer, bool fitInView)
    {
        Q_UNUSED(fitInView);

        m_textDocument->print(printer);
    }

    QSizeF TextDocument::documentSize()
    {
        return m_textDocument->size();
    }

    bool TextDocument::load(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty filename");
            }
            return false;
        }

        QFile file(fileName());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage) {
                *errorMessage = tr("Could not open file for reading");
            }
            return false;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        QString content = stream.readAll();
        m_textDocument->setPlainText(content);

        file.close();

        QFileInfo fileInfo(fileName());
        if ( fileInfo.suffix() == "vhdl" || fileInfo.suffix() == "vhd" ) {
            (void) new VhdlHighlighter(m_textDocument);
        }
        else if ( fileInfo.suffix() == "v" ) {
            (void) new VerilogHighlighter(m_textDocument);
        }
        else if ( fileInfo.suffix() == "net" ||
                  fileInfo.suffix() == "cir" ||
                  fileInfo.suffix() == "spc" ) {
            (void) new SpiceHighlighter(m_textDocument);
        }

        m_textDocument->setModified(false);
        return true;
    }

    bool TextDocument::save(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty filename");
            }
            return false;
        }

        QFile file(fileName());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            if (errorMessage) {
                *errorMessage = tr("Could not open file for writing");
            }
            return false;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        stream << m_textDocument->toPlainText();

        file.close();

        m_textDocument->setModified(false);
        return true;
    }

    IView* TextDocument::createView()
    {
        return new TextView(this);
    }

    void TextDocument::pasteTemplate(const QString& text)
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->insertPlainText(text);
    }

    TextEdit* TextDocument::activeTextEdit()
    {
        IView *view = DocumentViewManager::instance()->currentView();
        TextView *tv = qobject_cast<TextView*>(view);
        TextEdit *te = qobject_cast<TextEdit*>(tv->toWidget());

        if (te) {
            return te;
        }

        return 0;
    }

    void TextDocument::onContentsChanged()
    {
        if (!m_textDocument->isModified()) {
            m_textDocument->setModified(true);
        }
    }

    /*!
     * \brief Open simulation results.
     *
     * Once a simulation is started, the simulation process is connected to
     * this slot, to achive non-modal simulations (ie, the gui is responsive
     * during simulations).
     *
     * Once the simulation has finished, this slot is invoked and if no error
     * ocurred, simulation results are shown to the user. The waveform viewer
     * can be internal or external acording to the user settings.
     *
     * \sa simulate()
     */
    void TextDocument::simulationReady(int error)
    {
        // If there was any error during the process, do not display the waveforms
        if(error || simulationErrorStatus) {
            return;
        }

        QFileInfo info(fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();
        QString suffix = info.suffix();

        QProcess *simulationProcess = new QProcess(this);
        simulationProcess->setWorkingDirectory(path);
        simulationProcess->setProcessChannelMode(QProcess::MergedChannels);  // Output std:error and std:output together into the same file
        simulationProcess->setStandardOutputFile(path + "/" + baseName + ".log", QIODevice::Append);  // Create a log file
        connect(simulationProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(simulationLog(int)));

        // Open the resulting waveforms
        if (suffix == "net" || suffix == "cir" || suffix == "spc" || suffix == "sp") {
            DocumentViewManager *manager = DocumentViewManager::instance();
            manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".raw"));
        }
        else if (suffix == "vhd" || suffix == "vhdl") {
            simulationProcess->start(QString("gtkwave waveforms.ghw"));  // Open the waveforms
        }
        else if (suffix == "v") {
            simulationProcess->start(QString("gtkwave ") + baseName + ".vcd");  // Open the waveforms
        }
    }

    /*!
     * \brief Test for errors, and open log file (in case something went
     * wrong).
     *
     * This method is called whenever simulationProcess emits the finished
     * signal, to keep track of the different logs available.
     *
     * \sa simulate(), simulationReady()
     */
    void TextDocument::simulationLog(int error)
    {
        QFileInfo info(fileName());
        QString path = info.path();
        QString baseName = info.completeBaseName();

        // If there was any error during the process, open the log
        if(error) {
            simulationErrorStatus = true;

            DocumentViewManager *manager = DocumentViewManager::instance();
            manager->openFile(QDir::toNativeSeparators(path + "/" + baseName + ".log"));
        }

    }

} // namespace Caneda
