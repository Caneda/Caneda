/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef SCHEMATIC_DOCUMENT_H
#define SCHEMATIC_DOCUMENT_H

#include "idocument.h"

namespace Caneda
{
    // Forward declations
    class CGraphicsScene;

    /*!
     * \brief This class represents the schematic document interface
     * implementation.
     *
     * This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture.
     *
     * This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself is included as a pointer to
     * CGraphicsScene, that contains all the scene specific methods.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SchematicContext, SchematicView
     */
    class SchematicDocument : public IDocument
    {
        Q_OBJECT

    public:
        SchematicDocument();

        // IDocument interface methods
        virtual IContext* context();

        virtual bool isModified() const;

        virtual bool canUndo() const;
        virtual bool canRedo() const;

        virtual void undo();
        virtual void redo();

        virtual QUndoStack* undoStack();

        virtual bool canCut() const;
        virtual bool canCopy() const;
        virtual bool canPaste() const { return true; }

        virtual void cut();
        virtual void copy();
        virtual void paste();

        virtual void selectAll();

        virtual void intoHierarchy();
        virtual void popHierarchy();

        virtual void alignTop();
        virtual void alignBottom();
        virtual void alignLeft();
        virtual void alignRight();
        virtual void distributeHorizontal();
        virtual void distributeVertical();
        virtual void centerHorizontal();
        virtual void centerVertical();

        virtual void simulate();

        virtual bool printSupportsFitInPage() const { return true; }
        virtual void print(QPrinter *printer, bool fitInView);
        virtual void exportImage(QPaintDevice &device);
        virtual QSizeF documentSize();

        virtual bool load(QString *errorMessage = 0);
        virtual bool save(QString *errorMessage = 0);

        virtual IView* createView();

        virtual void launchPropertiesDialog();
        // End of IDocument interface methods

        CGraphicsScene* cGraphicsScene() const { return m_cGraphicsScene; }

    private:
        CGraphicsScene *m_cGraphicsScene;

        void alignElements(Qt::Alignment alignment);
    };

} // namespace Caneda

#endif //SCHEMATIC_DOCUMENT_H
