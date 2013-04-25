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

#include "symboldocument.h"

#include "cgraphicsscene.h"
#include "formatxmlsymbol.h"
#include "symbolcontext.h"
#include "symbolview.h"
#include "statehandler.h"

#include <QFileInfo>
#include <QMessageBox>

namespace Caneda
{
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
        setNormalAction();
        //! \todo Implement this. This should return to the schematic document.
    }

    void SymbolDocument::popHierarchy()
    {
        setNormalAction();
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

    void SymbolDocument::exportImage(QPaintDevice &device, qreal width, qreal height,
                                     Qt::AspectRatioMode aspectRatioMode)
    {
        m_cGraphicsScene->exportImage(device, width, height, aspectRatioMode);
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
            m_cGraphicsScene->launchPropertyDialog();
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

} // namespace Caneda
