/***************************************************************************
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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
#include "symbolcontext.h"
#include "symbolview.h"
#include "settings.h"
#include "statehandler.h"
#include "xmlsymbol.h"

#include "dialogs/exportdialog.h"

#include <QFileInfo>
#include <QPainter>
#include <QPrinter>

#include <cmath>

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

    void SymbolDocument::print(QPrinter *printer, bool fitInView)
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

            //The symbol is printed on a grid of sheets running from top-bottom, left-right.
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

    bool SymbolDocument::load(QString *errorMessage)
    {
        QFileInfo info(fileName());

        if(info.suffix() == "xsym") {
            XmlSymbol *format = new XmlSymbol(this);
            return format->loadSymbol();
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
            XmlSymbol *format = new XmlSymbol(this);
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

    void SymbolDocument::exportImage()
    {
        ExportDialog *d = new ExportDialog(this, m_cGraphicsScene);
        d->exec();
    }

    IView* SymbolDocument::createView()
    {
        return new SymbolView(this);
    }

} // namespace Caneda
