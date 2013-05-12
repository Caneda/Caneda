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

#include "schematicdocument.h"

#include "cgraphicsscene.h"
#include "formatxmlschematic.h"
#include "schematiccontext.h"
#include "schematicview.h"
#include "statehandler.h"

#include <QFileInfo>
#include <QMessageBox>

namespace Caneda
{
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
        setNormalAction();
        //! \todo Implement this
    }

    void SchematicDocument::popHierarchy()
    {
        setNormalAction();
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

    void SchematicDocument::simulate()
    {
        setNormalAction();
        //! \todo Implement this
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

    void SchematicDocument::launchPropertiesDialog()
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
    void SchematicDocument::alignElements(Qt::Alignment alignment)
    {
        if (!m_cGraphicsScene->alignElements(alignment)) {
            QMessageBox::information(0, tr("Info"),
                    tr("At least two elements must be selected!"));
        }
    }

} // namespace Caneda
