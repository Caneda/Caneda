/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "schematicview.h"

#include "item.h"
#include "schematicscene.h"
#include "xmlformat.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QWheelEvent>

const qreal SchematicView::zoomFactor = 1.2f;

//! Constructor
SchematicView::SchematicView(SchematicScene *sc, QWidget *parent) :
    QGraphicsView(sc, parent),
    m_horizontalScroll(0),
    m_verticalScroll(0)
{
    if(sc == 0) {
        sc = new SchematicScene;
        sc->setSceneRect(0, 0, 1024, 768);
        setScene(sc);
        DragMode dragMode = (sc->currentMouseAction() == SchematicScene::Normal) ?
            RubberBandDrag : NoDrag;
        setDragMode(dragMode);
    }

    setAcceptDrops(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(SmartViewportUpdate);
    setCacheMode(CacheBackground);

    viewport()->setMouseTracking(true);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);

    connect(sc, SIGNAL(modificationChanged(bool)), SIGNAL(modificationChanged(bool)));
    connect(sc, SIGNAL(fileNameChanged(const QString&)),
            SIGNAL(fileNameChanged(const QString&)));
    connect(sc, SIGNAL(titleToBeUpdated()), SIGNAL(titleToBeUpdated()));
}

//! Destructor
SchematicView::~SchematicView()
{
    //HACK: For now delete the scene if this view is its only viewer.
    QList<QGraphicsView*> views;
    if (scene()) {
        views = scene()->views();
    }

    if (views.size() == 1 && views.first() == this) {
        delete scene();
    }
}

SchematicScene* SchematicView::schematicScene() const
{
    SchematicScene* s = qobject_cast<SchematicScene*>(scene());
    Q_ASSERT(s);// This should never fail!
    return s;
}

void SchematicView::setFileName(const QString& name)
{
    schematicScene()->setFileName(name);
}

QString SchematicView::fileName() const
{
    return schematicScene()->fileName();
}

bool SchematicView::load()
{
    //Assumes file name is set
    FileFormatHandler *format =
        FileFormatHandler::handlerFromSuffix(QFileInfo(fileName()).suffix(), schematicScene());

    if(!format) {
        QMessageBox::critical(0, tr("Error"), tr("Unknown file format!"));
        return false;
    }

    if(!format->load()) {
        return false;
    }
    return true;
}

bool SchematicView::save()
{
    //Assumes filename is set before the call
    QFileInfo info(fileName());

    if(QString(info.suffix()).isEmpty()) {
        setFileName(fileName()+".xsch");
        info = QFileInfo(fileName());
    }

    FileFormatHandler *format =
        FileFormatHandler::handlerFromSuffix(info.suffix(), schematicScene());

    if(!format) {
        QMessageBox::critical(0, tr("Error"), tr("Unknown file format!"));
        return false;
    }

    if(!format->save()) {
        return false;
    }
    else {
        schematicScene()->undoStack()->clear();
        //If we are editing the symbol, and svg (and xsym) files were previously created, we must update them.
        if(schematicScene()->currentMode() == Qucs::SymbolMode) {
            info = QFileInfo(fileName().replace(".xsch",".xsym"));
            if(info.exists()) {
                setFileName(fileName().replace(".xsch",".xsym"));
                format = FileFormatHandler::handlerFromSuffix(info.suffix(), schematicScene());
                format->save();
                setFileName(fileName().replace(".xsym",".xsch"));
            }
        }
    }

    return true;
}

void SchematicView::zoomIn()
{
    scale(zoomFactor, zoomFactor);
}

void SchematicView::zoomOut()
{
    qreal zf = 1.0/zoomFactor;
    scale(zf, zf);
}

void SchematicView::showAll()
{
    fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void SchematicView::showNoZoom()
{
    resetMatrix();
}

QWidget* SchematicView::toWidget() const
{
    SchematicView* self = const_cast<SchematicView*>(this);
    QGraphicsView* view = qobject_cast<QGraphicsView*>(self);
    return static_cast<QWidget*>(view);
}

SchematicView* SchematicView::toSchematicView() const
{
    SchematicView* self = const_cast<SchematicView*>(this);
    QGraphicsView* view = qobject_cast<QGraphicsView*>(self);
    return qobject_cast<SchematicView*>(view);
}

bool SchematicView::isModified() const
{
    return schematicScene()->isModified();
}

void SchematicView::copy() const
{
    QList<QGraphicsItem*> items = scene()->selectedItems();
    QList<QucsItem*> qItems = filterItems<QucsItem>(items);
    schematicScene()->copyItems(qItems);
}

void SchematicView::cut()
{
    QList<QGraphicsItem*> items = scene()->selectedItems();
    QList<QucsItem*> qItems = filterItems<QucsItem>(items);

    if(!qItems.isEmpty()) {
        schematicScene()->cutItems(qItems);
    }
}

void SchematicView::paste()
{
    emit pasteInvoked();
}

void SchematicView::saveScrollState()
{
    m_horizontalScroll = horizontalScrollBar()->value();
    m_verticalScroll  = verticalScrollBar()->value();
}

void SchematicView::restoreScrollState()
{
    horizontalScrollBar()->setValue(m_horizontalScroll);
    verticalScrollBar()->setValue(m_verticalScroll);
}

void SchematicView::setModified(bool m)
{
    schematicScene()->setModified(m);
}

void SchematicView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint newCursorPos = mapToScene(event->pos()).toPoint();
    QString str = QString("%1 : %2")
        .arg(newCursorPos.x())
        .arg(newCursorPos.y());
    emit cursorPositionChanged(str);
    QGraphicsView::mouseMoveEvent(event);
}

void SchematicView::focusInEvent(QFocusEvent *event)
{
    QGraphicsView::focusInEvent(event);
    if (hasFocus()) {
        emit focussed(this);
    }
}
