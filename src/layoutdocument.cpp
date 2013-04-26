/***************************************************************************
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

#include "layoutdocument.h"

#include "cgraphicsscene.h"
#include "formatxmllayout.h"
#include "layoutcontext.h"
#include "layoutview.h"
#include "statehandler.h"

#include <QFileInfo>
#include <QMessageBox>

namespace Caneda
{
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
        setNormalAction();
        //! \todo Implement this
    }

    void LayoutDocument::popHierarchy()
    {
        setNormalAction();
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
        setNormalAction();
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

    void LayoutDocument::launchPropertiesDialog()
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
    void LayoutDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_cGraphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

} // namespace Caneda
