/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#ifndef WEBDOCUMENT_H
#define WEBDOCUMENT_H

#include "idocument.h"

// Forward declarations.
class QUrl;

namespace Caneda
{
    // Forward declarations.
    class WebPage;

    class WebDocument : public IDocument
    {
        Q_OBJECT

    public:
        WebDocument();
        virtual ~WebDocument();

        // IDocument interface methods
        virtual IContext* context();

        virtual bool isModified() const { return false; }

        virtual bool canUndo() const { return false; }
        virtual bool canRedo() const { return false; }

        virtual void undo() {}
        virtual void redo() {}

        virtual QUndoStack* undoStack();

        virtual bool canCut() const { return false; }
        virtual bool canCopy() const { return false; }
        virtual bool canPaste() const { return false; }

        virtual void cut() {}
        virtual void copy();
        virtual void paste() {}

        virtual void selectAll() {}

        virtual bool printSupportsFitInPage() const { return false; }
        virtual void print(QPrinter *printer, bool fitInView);

        virtual bool load(QString *errorMessage = 0);
        virtual bool save(QString *errorMessage = 0) {}

        virtual void exportImage() {}

        virtual IView* createView();

        virtual void updateSettingsChanges() {}
        // End of IDocument interface methods

        QUrl* webUrl() const;

    private:
        WebPage* activeWebPage();
        QUrl *m_webUrl;
    };

} // namespace Caneda

#endif //WEBDOCUMENT_H
