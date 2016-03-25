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
#include "cgraphicsscene.h"
#include "csimulationscene.h"
#include "csimulationview.h"
#include "documentviewmanager.h"
#include "fileformats.h"
#include "fileexport.h"
#include "fileimport.h"
#include "icontext.h"
#include "iview.h"
#include "mainwindow.h"
#include "messagewidget.h"
#include "settings.h"
#include "statehandler.h"
#include "syntaxhighlighters.h"
#include "textedit.h"
#include "webpage.h"

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
#include <QUndoStack>
#include <QUrl>

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
     * \fn IDocument::intoHierarchy()
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
     * subcircuit, the action intoHierarchy() comes into place.
     *
     * \sa popHierarchy()
     */

    /*!
     * \fn IDocument::popHierarchy()
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
     * \sa intoHierarchy()
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
    IDocument::IDocument()
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
    LayoutDocument::LayoutDocument()
    {
        m_cGraphicsScene = new CGraphicsScene(this);
        connect(m_cGraphicsScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    IContext* LayoutDocument::context()
    {
        return LayoutContext::instance();
    }

    bool LayoutDocument::isModified() const
    {
        return m_cGraphicsScene->isModified();
    }

    bool LayoutDocument::canUndo() const
    {
        return m_cGraphicsScene->undoStack()->canUndo();
    }

    bool LayoutDocument::canRedo() const
    {
        return m_cGraphicsScene->undoStack()->canRedo();
    }

    void LayoutDocument::undo()
    {
        m_cGraphicsScene->undoStack()->undo();
    }

    void LayoutDocument::redo()
    {
        m_cGraphicsScene->undoStack()->redo();
    }

    QUndoStack* LayoutDocument::undoStack()
    {
        return m_cGraphicsScene->undoStack();
    }

    bool LayoutDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        return schItems.isEmpty() == false;
    }

    bool LayoutDocument::canCopy() const
    {
        return LayoutDocument::canCut();
    }

    void LayoutDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_cGraphicsScene->cutItems(schItems);
        }
    }

    void LayoutDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_cGraphicsScene->copyItems(schItems);
        }
    }

    void LayoutDocument::paste()
    {
        StateHandler::instance()->slotHandlePaste();
    }

    void LayoutDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_cGraphicsScene->sceneRect());
        m_cGraphicsScene->setSelectionArea(path);
    }

    void LayoutDocument::intoHierarchy()
    {
        //! \todo Implement this
    }

    void LayoutDocument::popHierarchy()
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
        if (!m_cGraphicsScene->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void LayoutDocument::distributeVertical()
    {
        if (!m_cGraphicsScene->distributeElements(Qt::Vertical)) {
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
        m_cGraphicsScene->print(printer, fitInView);
    }

    void LayoutDocument::exportImage(QPaintDevice &device)
    {
        m_cGraphicsScene->exportImage(device);
    }

    QSizeF LayoutDocument::documentSize()
    {
        return m_cGraphicsScene->itemsBoundingRect().size();
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

            m_cGraphicsScene->undoStack()->clear();
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

    void LayoutDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
    {
        // Launch the context of the current document
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();
        _menu->addAction(am->actionForName("select"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editPaste"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("schEdit"));
        _menu->addAction(am->actionForName("symEdit"));

        _menu->exec(e->screenPos());
    }

    void LayoutDocument::launchPropertiesDialog()
    {
        // Get a list of selected items
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        // If there is any selection, launch corresponding properties dialog.
        if(!schItems.isEmpty()) {
            foreach(CGraphicsItem *item, schItems) {
                item->launchPropertyDialog(Caneda::PushUndoCmd);
            }
        }
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void LayoutDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_cGraphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }


    /*************************************************************************
     *                          SchematicDocument                            *
     *************************************************************************/
    //! \brief Constructor.
    SchematicDocument::SchematicDocument()
    {
        m_cGraphicsScene = new CGraphicsScene(this);
        connect(m_cGraphicsScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    IContext* SchematicDocument::context()
    {
        return SchematicContext::instance();
    }

    bool SchematicDocument::isModified() const
    {
        return m_cGraphicsScene->isModified();
    }

    bool SchematicDocument::canUndo() const
    {
        return m_cGraphicsScene->undoStack()->canUndo();
    }

    bool SchematicDocument::canRedo() const
    {
        return m_cGraphicsScene->undoStack()->canRedo();
    }

    void SchematicDocument::undo()
    {
        m_cGraphicsScene->undoStack()->undo();
    }

    void SchematicDocument::redo()
    {
        m_cGraphicsScene->undoStack()->redo();
    }

    QUndoStack* SchematicDocument::undoStack()
    {
        return m_cGraphicsScene->undoStack();
    }

    bool SchematicDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        return schItems.isEmpty() == false;
    }

    bool SchematicDocument::canCopy() const
    {
        return SchematicDocument::canCut();
    }

    void SchematicDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_cGraphicsScene->cutItems(schItems);
        }
    }

    void SchematicDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        if(!schItems.isEmpty()) {
            m_cGraphicsScene->copyItems(schItems);
        }
    }

    void SchematicDocument::paste()
    {
        StateHandler::instance()->slotHandlePaste();
    }

    void SchematicDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_cGraphicsScene->sceneRect());
        m_cGraphicsScene->setSelectionArea(path);
    }

    void SchematicDocument::intoHierarchy()
    {
        //! \todo Implement this
    }

    void SchematicDocument::popHierarchy()
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
        if (!m_cGraphicsScene->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void SchematicDocument::distributeVertical()
    {
        if (!m_cGraphicsScene->distributeElements(Qt::Vertical)) {
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
     * \sa simulationReady(), simulationLog()
     */
    void SchematicDocument::simulate()
    {
        if(fileName().isEmpty()) {
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
        simulationProcess->setStandardOutputFile(path + "/" + baseName + ".log", QIODevice::Append);  // Create a log file

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
        m_cGraphicsScene->print(printer, fitInView);
    }

    void SchematicDocument::exportImage(QPaintDevice &device)
    {
        m_cGraphicsScene->exportImage(device);
    }

    QSizeF SchematicDocument::documentSize()
    {
        return m_cGraphicsScene->itemsBoundingRect().size();
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

            m_cGraphicsScene->undoStack()->clear();
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

    void SchematicDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
    {
        // Launch the context of the current document
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();
        _menu->addAction(am->actionForName("select"));
        _menu->addAction(am->actionForName("insWire"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editPaste"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("symEdit"));
        _menu->addAction(am->actionForName("layEdit"));
        _menu->addAction(am->actionForName("openSym"));

        //! \todo Reenable these options once implemented
        //                _menu->addAction(am->actionForName("intoH"));
        //                _menu->addAction(am->actionForName("popH"));

        _menu->exec(e->screenPos());
    }

    void SchematicDocument::launchPropertiesDialog()
    {
        // Get a list of selected items
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        // If there is any selection, launch corresponding properties dialog.
        if(!schItems.isEmpty()) {
            foreach(CGraphicsItem *item, schItems) {
                item->launchPropertyDialog(Caneda::PushUndoCmd);
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
     * \sa simulate(), simulationLog()
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
            Action* act = am->actionForName("showLog");
            dialog->addAction(act);

            act = am->actionForName("showNetlist");
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

    //! \brief Align selected elements appropriately based on \a alignment
    void SchematicDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_cGraphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }


    /*************************************************************************
     *                         SimulationDocument                            *
     *************************************************************************/
    //! \brief Constructor.
    SimulationDocument::SimulationDocument()
    {
        m_cSimulationScene = new CSimulationScene();
    }

    IContext* SimulationDocument::context()
    {
        return SimulationContext::instance();
    }

    QUndoStack* SimulationDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
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
        CSimulationView *sv = qobject_cast<CSimulationView*>(v->toWidget());

        sv->print(printer, fitInView);
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
        CSimulationView *sv = qobject_cast<CSimulationView*>(v->toWidget());

        sv->exportImage(device);
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
        CSimulationView *sv = qobject_cast<CSimulationView*>(v->toWidget());

        if(sv) {
            sv->launchPropertyDialog();
        }
    }


    /*************************************************************************
     *                           SymbolDocument                              *
     *************************************************************************/
    //! \brief Constructor.
    SymbolDocument::SymbolDocument()
    {
        m_cGraphicsScene = new CGraphicsScene(this);
        connect(m_cGraphicsScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_cGraphicsScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    IContext* SymbolDocument::context()
    {
        return SymbolContext::instance();
    }

    bool SymbolDocument::isModified() const
    {
        return m_cGraphicsScene->isModified();
    }

    bool SymbolDocument::canUndo() const
    {
        return m_cGraphicsScene->undoStack()->canUndo();
    }

    bool SymbolDocument::canRedo() const
    {
        return m_cGraphicsScene->undoStack()->canRedo();
    }

    void SymbolDocument::undo()
    {
        m_cGraphicsScene->undoStack()->undo();
    }

    void SymbolDocument::redo()
    {
        m_cGraphicsScene->undoStack()->redo();
    }

    QUndoStack* SymbolDocument::undoStack()
    {
        return m_cGraphicsScene->undoStack();
    }

    bool SymbolDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> symItems = filterItems<CGraphicsItem>(qItems);

        return symItems.isEmpty() == false;
    }

    bool SymbolDocument::canCopy() const
    {
        return SymbolDocument::canCut();
    }

    void SymbolDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> symItems = filterItems<CGraphicsItem>(qItems);

        if(!symItems.isEmpty()) {
            m_cGraphicsScene->cutItems(symItems);
        }
    }

    void SymbolDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> symItems = filterItems<CGraphicsItem>(qItems);

        if(!symItems.isEmpty()) {
            m_cGraphicsScene->copyItems(symItems);
        }
    }

    void SymbolDocument::paste()
    {
        StateHandler::instance()->slotHandlePaste();
    }

    void SymbolDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_cGraphicsScene->sceneRect());
        m_cGraphicsScene->setSelectionArea(path);
    }

    void SymbolDocument::intoHierarchy()
    {
        //! \todo Implement this. This should return to the schematic document.
    }

    void SymbolDocument::popHierarchy()
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
        if (!m_cGraphicsScene->distributeElements(Qt::Horizontal)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

    void SymbolDocument::distributeVertical()
    {
        if (!m_cGraphicsScene->distributeElements(Qt::Vertical)) {
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
        m_cGraphicsScene->print(printer, fitInView);
    }

    void SymbolDocument::exportImage(QPaintDevice &device)
    {
        m_cGraphicsScene->exportImage(device);
    }

    QSizeF SymbolDocument::documentSize()
    {
        return m_cGraphicsScene->itemsBoundingRect().size();
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

            m_cGraphicsScene->undoStack()->clear();
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

    void SymbolDocument::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
    {
        // Launch the context of the current document
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();
        _menu->addAction(am->actionForName("select"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editPaste"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("schEdit"));
        _menu->addAction(am->actionForName("layEdit"));
        _menu->addAction(am->actionForName("propertiesDialog"));

        _menu->exec(e->screenPos());
    }

    void SymbolDocument::launchPropertiesDialog()
    {
        // Get a list of selected items
        QList<QGraphicsItem*> qItems = m_cGraphicsScene->selectedItems();
        QList<CGraphicsItem*> schItems = filterItems<CGraphicsItem>(qItems);

        // If there is any selection, launch corresponding properties dialog,
        // else launch the properties dialog corresponding to the current scene
        if(!schItems.isEmpty()) {
            foreach(CGraphicsItem *item, schItems) {
                item->launchPropertyDialog(Caneda::PushUndoCmd);
            }
        }
        else {
            m_cGraphicsScene->properties()->launchPropertyDialog();
        }
    }

    //! \brief Align selected elements appropriately based on \a alignment
    void SymbolDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_cGraphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }


    /*************************************************************************
     *                            TextDocument                               *
     *************************************************************************/
    //! \brief Constructor.
    TextDocument::TextDocument()
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

    QUndoStack* TextDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
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

    void TextDocument::intoHierarchy()
    {
        //! \todo Implement this. This should open currently selected file.
    }

    void TextDocument::popHierarchy()
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
     * \sa simulationReady(), simulationLog()
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
        simulationProcess->setStandardOutputFile(path + "/" + baseName + ".log", QIODevice::Append);  // Create a log file

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
     * \sa simulate(), simulationLog()
     */
    void TextDocument::simulationReady(int error)
    {
        // If there was any error during the process, do not display the waveforms
        if(error | simulationErrorStatus) {
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


    /*************************************************************************
     *                             WebDocument                               *
     *************************************************************************/
    //! \brief Constructor.
    WebDocument::WebDocument()
    {
        m_webUrl = new QUrl;
    }

    IContext* WebDocument::context()
    {
        return WebContext::instance();
    }

    QUndoStack* WebDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
    }

    void WebDocument::copy()
    {
        WebPage *page = activeWebPage();
        if (!page) {
            return;
        }
        page->copy();
    }

    void WebDocument::print(QPrinter *printer, bool fitInView)
    {
        Q_UNUSED(fitInView);

        WebPage *page = activeWebPage();
        if (!page) {
            return;
        }

        page->print(printer);
    }

    QSizeF WebDocument::documentSize()
    {
        // Return 0, as this method is only used for graphic documents.
        QSizeF size(0, 0);
        return size;
    }

    bool WebDocument::load(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty filename");
            }
            return false;
        }

        m_webUrl->setUrl(fileName());
        return true;
    }

    IView* WebDocument::createView()
    {
        return new WebView(this);
    }

    WebPage* WebDocument::activeWebPage()
    {
        IView *view = DocumentViewManager::instance()->currentView();
        WebView *wv = qobject_cast<WebView*>(view);

        WebPage *wp = qobject_cast<WebPage*>(wv->toWidget());

        if (wp) {
            return wp;
        }

        return 0;
    }

} // namespace Caneda
