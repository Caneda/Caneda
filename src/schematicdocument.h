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

#ifndef SCHEMATIC_DOCUMENT_H
#define SCHEMATIC_DOCUMENT_H

#include "idocument.h"

namespace Caneda
{
    // Forward declations
    class CGraphicsScene;

    class SchematicDocument : public IDocument
    {
        Q_OBJECT

    public:
        SchematicDocument();
        ~SchematicDocument();

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
        virtual bool canPaste() const;

        virtual void cut();
        virtual void copy();
        virtual void paste();

        virtual void selectAll();

        virtual bool printSupportsFitInPage() const;
        virtual void print(QPrinter *printer, bool fitInView);

        virtual bool load(QString *errorMessage = 0);
        virtual bool save(QString *errorMessage = 0);

        virtual void exportImage();

        virtual IView* createView();

        virtual void updateSettingsChanges();
        // End of IDocument interface methods

        CGraphicsScene* cGraphicsScene() const;

    private:
        CGraphicsScene *m_cGraphicsScene;
    };

} // namespace Caneda

#endif //SCHEMATIC_DOCUMENT_H
