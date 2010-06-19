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

#include "schematicdocument.h"

#include "schematiccontext.h"
#include "schematicscene.h"
#include "schematicstatehandler.h"
#include "schematicview.h"
#include "settings.h"
#include "xmlsymbolformat.h"

#include <QFileInfo>
#include <QPrinter>

#include <cmath>

namespace Caneda
{
    SchematicDocument::SchematicDocument()
    {
        m_schematicScene = new SchematicScene(this);
        connect(m_schematicScene, SIGNAL(changed()), this,
                SLOT(emitDocumentChanged()));
        connect(m_schematicScene->undoStack(), SIGNAL(canUndoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_schematicScene->undoStack(), SIGNAL(canRedoChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_schematicScene, SIGNAL(selectionChanged()), this,
                SLOT(emitDocumentChanged()));
    }

    SchematicDocument::~SchematicDocument()
    {
    }

    // Interface implementation
    IContext* SchematicDocument::context()
    {
        return SchematicContext::instance();
    }

    bool SchematicDocument::isModified() const
    {
        return m_schematicScene->isModified();
    }

    bool SchematicDocument::canUndo() const
    {
        return m_schematicScene->undoStack()->canUndo();
    }

    bool SchematicDocument::canRedo() const
    {
        return m_schematicScene->undoStack()->canRedo();
    }

    void SchematicDocument::undo()
    {
        m_schematicScene->undoStack()->undo();
    }

    void SchematicDocument::redo()
    {
        m_schematicScene->undoStack()->redo();
    }

    bool SchematicDocument::canCut() const
    {
        QList<QGraphicsItem*> qItems = m_schematicScene->selectedItems();
        QList<SchematicItem*> schItems = filterItems<SchematicItem>(qItems);

        return schItems.isEmpty() == false;
    }

    bool SchematicDocument::canCopy() const
    {
        return SchematicDocument::canCut();
    }

    bool SchematicDocument::canPaste() const
    {
        return true;
    }

    void SchematicDocument::cut()
    {
        QList<QGraphicsItem*> qItems = m_schematicScene->selectedItems();
        QList<SchematicItem*> schItems = filterItems<SchematicItem>(qItems);

        if(!schItems.isEmpty()) {
            m_schematicScene->cutItems(schItems);
        }
    }

    void SchematicDocument::copy()
    {
        QList<QGraphicsItem*> qItems = m_schematicScene->selectedItems();
        QList<SchematicItem*> schItems = filterItems<SchematicItem>(qItems);

        if(!schItems.isEmpty()) {
            m_schematicScene->copyItems(schItems);
        }
    }

    void SchematicDocument::paste()
    {
        SchematicStateHandler::instance()->slotHandlePaste();
    }

    void SchematicDocument::selectAll()
    {
        QPainterPath path;
        path.addRect(m_schematicScene->sceneRect());
        m_schematicScene->setSelectionArea(path);
    }

    bool SchematicDocument::printSupportsFitInPage() const
    {
        return true;
    }

    void SchematicDocument::print(QPrinter *printer, bool fitInView)
    {
        QPainter p(printer);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const bool fullPage = printer->fullPage();

        const bool viewGridStatus = Settings::instance()->currentValue("gui/gridVisible").value<bool>();
        Settings::instance()->setCurrentValue("gui/gridVisible", false);

        const QRectF diagramRect = m_schematicScene->imageBoundingRect();
        if(fitInView) {
            m_schematicScene->render(&p,
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
                m_schematicScene->render(&p,
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

    void SchematicDocument::exportToPaintDevice(QPaintDevice *device,
            const QVariantMap &configuration)
    {

    }

    bool SchematicDocument::load(QString *errorMessage)
    {
        FileFormatHandler *format =
            FileFormatHandler::handlerFromSuffix(QFileInfo(fileName()).suffix(), this);

        if(!format) {
            if (errorMessage) {
                *errorMessage = tr("Unknown file format!");
            }
            return false;
        }

        return format->load();
    }

    bool SchematicDocument::save(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty file name");
            }
            return false;
        }

        QFileInfo info(fileName());

        // Correct the extension.
        if(QString(info.suffix()).isEmpty()) {
            setFileName(fileName() + ".xsch");
            info = QFileInfo(fileName());
        }

        FileFormatHandler *format =
            FileFormatHandler::handlerFromSuffix(info.suffix(), this);

        if(!format) {
            if (errorMessage) {
                *errorMessage = tr("Unknown file format!");
            }
            return false;
        }

        if(!format->save()) {
            return false;
        }

        m_schematicScene->undoStack()->clear();
        //If we are editing the symbol, and svg (and xsym) files were previously created, we must update them.

        //FIXME: The current way of using one method to save both symbol and schematic isn't
        //correct. Infact its better to separate concerns by creating a dedicated SchematicScene and
        //thus dedicated SchematicDocument to symbol. Then here if we could ensure the presence of
        //SchematicDocument for .xsch, and invoke save on that too.


#if 0
        if(schematicScene()->currentMode() == Caneda::SymbolMode) {
            info = QFileInfo(fileName().replace(".xsch",".xsym"));
            if(info.exists()) {
                setFileName(fileName().replace(".xsch",".xsym"));
                format = FileFormatHandler::handlerFromSuffix(info.suffix(), schematicScene());
                format->save();
                setFileName(fileName().replace(".xsym",".xsch"));
            }
        }
#endif

        return true;
    }

    IView* SchematicDocument::createView()
    {
        return new SchematicView(this);
    }

    void SchematicDocument::updateSettingsChanges()
    {
    }

    // End of Interface implemention.
    SchematicScene* SchematicDocument::schematicScene() const
    {
        return m_schematicScene;
    }
} // namespace Caneda
