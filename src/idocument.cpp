/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include <QUndoStack>

namespace Caneda
{
    /*!
     * \class IDocument
     *
     * This class serves as interface for all documents that can be handled by
     * Caneda.
     */

    /*!
     * \fn IView::context()
     *
     * \brief Returns the context that handles documents, views of specific type.
     * \note It is enough to create the context object only once per new type.
     * \see IContext
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
     * \fn IDocument::load(QString *errorMessage = 0)
     *
     * \brief Loads the document from file IDocument::fileName()
     * \return true on success, false on failure.
     * \param errorMessage If load operation fails and errorMessage points to valid
     *                     location, the appropriate message corresponding for failure
     *                     is set.
     */

    /*!
     * \fn IDocument::save(QString *errorMessage = 0)
     *
     * \brief Saves the document into file IDocument::fileName()
     * \return true on success, false on failure.
     * \param errorMessage If save operation fails and errorMessage points to valid
     *                     location, the appropriate message corresponding for failure
     *                     is set.
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

} // namespace Caneda
