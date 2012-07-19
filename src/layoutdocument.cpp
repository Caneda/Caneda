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

#include "layoutdocument.h"

#include "cgraphicsscene.h"
#include "layoutcontext.h"
#include "layoutview.h"
#include "settings.h"
#include "statehandler.h"
#include "xmllayout.h"

#include "dialogs/exportdialog.h"

#include <QFileInfo>
#include <QPainter>
#include <QPrinter>

#include <cmath>

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

    LayoutDocument::~LayoutDocument()
    {
    }

    // Interface implementation
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

    bool LayoutDocument::canPaste() const
    {
        return true;
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

    bool LayoutDocument::printSupportsFitInPage() const
    {
        return true;
    }

    void LayoutDocument::print(QPrinter *printer, bool fitInView)
    {
        QPainter p(printer);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const bool fullPage = printer->fullPage();

        const bool viewGridStatus = Settings::instance()->currentValue("gui/gridVisible").value<bool>();
        Settings::instance()->setCurrentValue("gui/gridVisible", false);

        const QRectF diagramRect = m_cGraphicsScene->itemsBoundingRect();
        if(fitInView) {
            m_cGraphicsScene->render(&p,
                    QRectF(), // Dest rect
                    diagramRect, // Src rect
                    Qt::KeepAspectRatio);
        }
        else {
            //Printing on one or more pages
            QRectF printedArea = fullPage ? printer->paperRect() : printer->pageRect();

            const int horizontalPages =
                int(std::ceil(diagramRect.width() / printedArea.width()));
            const int verticalPages =
                int(std::ceil(diagramRect.height() / printedArea.height()));

            QList<QRectF> pagesToPrint;

            //The schematic is printed on a grid of sheets running from top-bottom, left-right.
            qreal yOffset = 0;
            for(int y = 0; y < verticalPages; ++y) {
                //Runs through the sheets of the line
                qreal xOffset = 0;
                for(int x = 0; x < horizontalPages; ++x) {
                    const qreal width = qMin(printedArea.width(), diagramRect.width() - xOffset);
                    const qreal height = qMin(printedArea.height(), diagramRect.height() - yOffset);
                    pagesToPrint << QRectF(xOffset, yOffset, width, height);
                    xOffset += printedArea.width();
                }

                yOffset += printedArea.height();
            }

            for (int i = 0; i < pagesToPrint.size(); ++i) {
                const QRectF rect = pagesToPrint.at(i);
                m_cGraphicsScene->render(&p,
                        rect.translated(-rect.topLeft()), // dest - topleft at (0, 0)
                        rect.translated(diagramRect.topLeft()), // src
                        Qt::KeepAspectRatio);
                if(i != (pagesToPrint.size() - 1)) {
                    printer->newPage();
                }
            }
        }

        Settings::instance()->setCurrentValue("gui/gridVisible", viewGridStatus);
    }

    bool LayoutDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "xlay") {
            XmlLayout *format = new XmlLayout(this);
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
            XmlLayout *format = new XmlLayout(this);
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

    void LayoutDocument::exportImage()
    {
        ExportDialog *d = new ExportDialog(this, m_cGraphicsScene);
        d->exec();
    }

    IView* LayoutDocument::createView()
    {
        return new LayoutView(this);
    }

    void LayoutDocument::updateSettingsChanges()
    {
    }

    // End of Interface implemention.
    CGraphicsScene* LayoutDocument::cGraphicsScene() const
    {
        return m_cGraphicsScene;
    }

} // namespace Caneda
