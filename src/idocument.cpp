/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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
#include "documentviewmanager.h"
#include "mainwindow.h"

#include <QUndoStack>

namespace Caneda
{
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

    IDocument::IDocument()
    {
    }

    IDocument::~IDocument()
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

    void IDocument::setNormalAction()
    {
        MainWindow::instance()->setNormalAction();
    }

} // namespace Caneda
