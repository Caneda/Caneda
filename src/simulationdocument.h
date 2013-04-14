/***************************************************************************
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef SIMULATION_DOCUMENT_H
#define SIMULATION_DOCUMENT_H

#include "idocument.h"

namespace Caneda
{
    // Forward declations
    class SimulationScene;

    class SimulationDocument : public IDocument
    {
        Q_OBJECT

    public:
        SimulationDocument();
        ~SimulationDocument() {}

        // IDocument interface methods
        virtual IContext* context();

        virtual bool isModified() const;

        virtual bool canUndo() const;
        virtual bool canRedo() const;

        virtual void undo();
        virtual void redo();

        virtual QUndoStack* undoStack();

        virtual bool canCut() const { return false; }
        virtual bool canCopy() const { return false; }
        virtual bool canPaste() const { return false; }

        virtual void cut() {}
        virtual void copy() {}
        virtual void paste() {}

        virtual void selectAll() {}

        virtual void intoHierarchy() {}
        virtual void popHierarchy() {}

        virtual void alignTop() {}
        virtual void alignBottom() {}
        virtual void alignLeft() {}
        virtual void alignRight() {}
        virtual void distributeHorizontal();
        virtual void distributeVertical();
        virtual void centerHorizontal();
        virtual void centerVertical();

        virtual void simulate() {}

        virtual bool printSupportsFitInPage() const { return true; }
        virtual void print(QPrinter *printer, bool fitInView);
        virtual void exportImage();

        virtual bool load(QString *errorMessage = 0);
        virtual bool save(QString *errorMessage = 0);

        virtual IView* createView();

        virtual void launchPropertiesDialog() {}
        virtual void updateSettingsChanges() {}
        // End of IDocument interface methods

        SimulationScene* simulationScene() const { return m_simulationScene; }

    private:
        SimulationScene *m_simulationScene;
    };

} // namespace Caneda

#endif //SIMULATION_DOCUMENT_H
