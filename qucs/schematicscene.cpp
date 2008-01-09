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

#include "schematicscene.h"
#include "schematicview.h"
#include "wire.h"
#include "port.h"
#include "paintings/painting.h"
#include "diagrams/diagram.h"
#include "undocommands.h"
#include "qucsprimaryformat.h"
#include "xmlutilities.h"
#include "qucs-tools/global.h"
#include "component.h"
#include "library.h"
#include "svgtest.h"

#include "propertygroup.h"
#include <QtCore/QMimeData>
#include <QtCore/QtDebug>
#include <QtCore/QVarLengthArray>
#include <QtCore/QFileInfo>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QUndoStack>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QScrollBar>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QMessageBox>
#include <cmath>

SchematicScene::SchematicScene(QObject *parent) : QGraphicsScene(parent)
{
   init();
}

SchematicScene::SchematicScene ( qreal x, qreal y, qreal width, qreal height, QObject * parent ) : QGraphicsScene(x,y,width,height,parent)
{
   init();
}

void SchematicScene::init()
{
   m_undoStack = new QUndoStack(this);
   m_gridWidth = m_gridHeight = 10;
   m_currentMode = Qucs::SchematicMode;
   m_frameVisible = false;
   m_modified = false;
   m_snapToGrid = true;
   m_gridVisible = true;
   m_opensDataDisplay = true;
   m_frameTexts = QStringList() << tr("Title") << tr("Drawn By:") << tr("Date:") << tr("Revision:");
   m_macroProgress = false;
   m_areItemsMoving = false;
   setCurrentMouseAction(Normal);

}

SchematicScene::~SchematicScene()
{
}

void SchematicScene::test()
{
}

void SchematicScene::mirrorXItems(QList<QucsItem*> items, bool pushUndoCmds,
                                  QUndoCommand *parent)
{
   MirrorItemsCmd *cmd = new MirrorItemsCmd(items, Qt::XAxis, parent);
   if(pushUndoCmds && !parent) {
      m_undoStack->push(cmd);
   }
   else if(!parent) {
      cmd->redo();
      delete cmd;
   }
}

void SchematicScene::mirrorYItems(QList<QucsItem*> items, bool pushUndoCmds,
                                  QUndoCommand *parent)
{
   MirrorItemsCmd *cmd = new MirrorItemsCmd(items, Qt::YAxis, parent);
   if(pushUndoCmds && !parent) {
      m_undoStack->push(cmd);
   }
   else if(!parent) {
      cmd->redo();
      delete cmd;
   }
}

void SchematicScene::rotateItems(QList<QucsItem*> items, bool pushUndoCmds,
                                  QUndoCommand *parent)
{
   RotateItemsCmd *cmd = new RotateItemsCmd(items, parent);
   if(pushUndoCmds && !parent) {
      m_undoStack->push(cmd);
   }
   else if(!parent) {
      cmd->redo();
      delete cmd;
   }
}

void SchematicScene::deleteItems(QList<QucsItem*> items, bool pushUndoCmds,
                                  QUndoCommand *parent)
{
   foreach(QucsItem* item, items) {
      delete item;
   }
}

void SchematicScene::setItemsOnGrid(QList<QucsItem*> items, bool pushUndoCmds,
                                  QUndoCommand *parent)
{
   foreach(QucsItem* item, items) {
      QPointF pos = item->pos();
      pos = nearingGridPoint(pos);
      item->setPos(pos);
   }
}

void SchematicScene::toggleActiveStatus(QList<QucsItem*> components, bool pushUndoCmds,
                                  QUndoCommand *parent)
{
   Q_UNUSED(components);
   //TODO:
}

void SchematicScene::setFileName(const QString& name)
{
   if(name == m_fileName || name.isEmpty())
      return;
   m_fileName = name;
   QFileInfo info(m_fileName);
   m_dataSet = info.baseName() + ".dat";
   m_dataDisplay = info.baseName() + ".dpl";
   emit fileNameChanged(m_fileName);
   emit stateUpdated();
}

QPointF SchematicScene::nearingGridPoint(const QPointF &pos)
{
   QPoint point = pos.toPoint();
   int x = point.x();
   int y = point.y();

   if(x<0) x -= (m_gridWidth >> 1) - 1;
   else x += m_gridWidth >> 1;
   x -= x % m_gridWidth;

   if(y<0) y -= (m_gridHeight >> 1) - 1;
   else y += m_gridHeight >> 1;
   y -= y % m_gridHeight;

   return QPointF(x,y);
}

void SchematicScene::setGridSize(uint width, uint height)
{
   if(m_gridWidth == width && m_gridHeight == height)
      return;
   m_gridWidth = width;
   m_gridHeight = height;

   if(isGridVisible())
      update();
}

void SchematicScene::setGridVisible(bool visibility)
{
   if(m_gridVisible == visibility) return;
   m_gridVisible = visibility;
   update();
}

void SchematicScene::setDataSet(const QString& _dataSet)
{
   m_dataSet = _dataSet;
}

void SchematicScene::setDataDisplay(const QString& display)
{
   m_dataDisplay = display;
}

void SchematicScene::setOpensDataDisplay(bool state)
{
   m_opensDataDisplay = state;
}

void SchematicScene::setFrameVisible(bool visibility)
{
   if(m_frameVisible == visibility) return;
   m_frameVisible = visibility;
   update();
}

void SchematicScene::setFrameTexts(const QStringList& texts)
{
   m_frameTexts = texts;
   if(isFrameVisible())
      update();
}

void SchematicScene::setMode(Qucs::Mode mode)
{
   if(m_currentMode == mode) return;
   m_currentMode = mode;
   update();
   //TODO:
}

void SchematicScene::setCurrentMouseAction(MouseAction action)
{
   if(m_currentMouseAction == action) return;

   m_areItemsMoving = false;
   m_currentMouseAction = action;

   QGraphicsView::DragMode dragMode = (action == Normal) ?
      QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag;
   foreach(QGraphicsView *view, views())
      view->setDragMode(dragMode);
   setFocusItem(0);
   clearSelection();
   //TODO: Implemement this appropriately for all mouse actions
}

void SchematicScene::cutItems(QList<QucsItem*> _items)
{
   copyItems(_items);
   deleteItems(_items);
}

void SchematicScene::copyItems(QList<QucsItem*> _items)
{
   if(_items.isEmpty())
      return;
   QString clipText;
   Qucs::XmlWriter *writer = new Qucs::XmlWriter(&clipText);
   writer->setAutoFormatting(true);
   writer->writeStartDocument();
   writer->writeDTD(QString("<!DOCTYPE qucs>"));
   writer->writeStartElement("qucs");
   writer->writeAttribute("version", Qucs::version);

   foreach(QucsItem *_item, _items)
      _item->saveData(writer);

   writer->writeEndDocument();

   QClipboard *clipboard =  QApplication::clipboard();
   clipboard->setText(clipText);
}

void SchematicScene::paste()
{
   //TODO:
}

void SchematicScene::setModified(bool m)
{
   if(m_modified != m) {
      m_modified = m;
      emit modificationChanged(m_modified);
      emit stateUpdated();
   }
}

void SchematicScene::drawBackground(QPainter *painter, const QRectF& rect)
{
   if(!isGridVisible())
      return QGraphicsScene::drawBackground(painter,rect);

   painter->setPen(QPen(Qt::black,0));
   painter->setBrush(Qt::NoBrush);
   painter->setRenderHint(QPainter::Antialiasing,false);

   if(rect.contains(QPointF(0.,0.))) {
      painter->drawLine(QLineF(-3.0, 0.0, 3.0, 0.0));
      painter->drawLine(QLineF(0.0, -3.0, 0.0, 3.0));
   }

   int gridWidth = m_gridWidth, gridHeight = m_gridHeight;

   // Adjust visual representation of grid to be multiple, if
   // grid sizes are very small
#if 0
   while(gridWidth < 20)
      gridWidth *= 2;
   while(gridHeight < 20)
      gridHeight *= 2;
   while(gridWidth > 60)
      gridWidth /= 2;
   while(gridHeight > 60)
      gridHeight /= 2;
#endif

   qreal left = int(rect.left()) + gridWidth - (int(rect.left()) % gridWidth);
   qreal top = int(rect.top()) + gridHeight - (int(rect.top()) % gridHeight);
   qreal right = int(rect.right()) - (int(rect.right()) % gridWidth);
   qreal bottom = int(rect.bottom()) - (int(rect.bottom()) % gridHeight);
   qreal x,y;

   for( x = left; x <= right; x += gridWidth)
      for( y = top; y <=bottom; y += gridHeight)
         painter->drawPoint(QPointF(x,y));

   painter->setRenderHint(QPainter::Antialiasing,true);
}

void SchematicScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
   if(event->mimeData()->formats().contains("application/qucs.sidebarItem"))
      event->acceptProposedAction();
   else
      event->ignore();
}

void SchematicScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
   if(event->mimeData()->formats().contains("application/qucs.sidebarItem"))
      event->acceptProposedAction();
   else
      event->ignore();
}

void SchematicScene::dropEvent(QGraphicsSceneDragDropEvent * event)
{
   if(event->mimeData()->formats().contains("application/qucs.sidebarItem")) {
      //HACK: The event generating active view is obtained indirectly so that
      //       the scrollbar's position can be restored since the framework
      //       scrolls the view to show component rect.
      QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
      // The scrollBars are never null as seen from source of qgraphicsview.
      int hor = v->horizontalScrollBar()->value();
      int ver = v->verticalScrollBar()->value();
      QByteArray encodedData = event->mimeData()->data("application/qucs.sidebarItem");
      QDataStream stream(&encodedData, QIODevice::ReadOnly);
      QString text;
      stream >> text;
      Component *c = LibraryLoader::defaultInstance()->newComponent(text, 0);
      if(c) {
         QPointF dest = m_snapToGrid ? nearingGridPoint(event->scenePos()) :
            event->scenePos();

         QUndoCommand *cmd = insertComponentCmd(c, this, dest);
         m_undoStack->push(cmd);

         v->horizontalScrollBar()->setValue(hor);
         v->verticalScrollBar()->setValue(ver);

         event->acceptProposedAction();
         setModified(true);
      }
   }
   else
      event->ignore();
}

void SchematicScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
   //Cache the mouse press position
   if((e->buttons() & Qt::MidButton) == Qt::MidButton)
      qDebug() << "pressed" << e->scenePos();
   if(m_snapToGrid) {
      lastPos = nearingGridPoint(e->scenePos());
   }
   sendMouseActionEvent(e);
}

void SchematicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
   if(m_snapToGrid) {
      //HACK: Fool the event receivers by changing event parameters with new grid position.
      QPointF point = nearingGridPoint(e->scenePos());
      if(point == lastPos && m_currentMouseAction != Normal) {
         e->accept();
         return;
      }
      e->setScenePos(point);
      e->setPos(point);
      e->setLastScenePos(lastPos);
      e->setLastPos(lastPos);
      //Now cache this point for next move
      lastPos = point;
   }
   sendMouseActionEvent(e);
}

void SchematicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
   sendMouseActionEvent(e);
}

void SchematicScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
   sendMouseActionEvent(e);

}

void SchematicScene::wiringEvent(MouseActionEvent *event)
{
}

void SchematicScene::deletingEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   if(!_list.isEmpty()) {
      QList<QucsItem*> _items;
      foreach(QGraphicsItem *item, _list) {
         if(qucsitem_cast<QucsItem*>(item)) {
            _items << static_cast<QucsItem*>(item);
            break;
         }
      }
      if(!_items.isEmpty()) {
         deleteItems(_items);
         setModified(true);
      }
   }
}

void SchematicScene::markingEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

void SchematicScene::rotatingEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      rotateItems(QList<QucsItem*>() << qItems.first());
      setModified(true);
   }
}

void SchematicScene::mirroringXEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      mirrorXItems(QList<QucsItem*>() << qItems.first());
      setModified(true);
   }
}

void SchematicScene::mirroringYEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      mirrorYItems(QList<QucsItem*>() << qItems.first());
      setModified(true);
   }
}

void SchematicScene::changingActiveStatusEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

void SchematicScene::settingOnGridEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   if(!_list.isEmpty()) {
      QList<QucsItem*> _items;
      foreach(QGraphicsItem *item, _list) {
         if(qucsitem_cast<QucsItem*>(item)) {
            _items << static_cast<QucsItem*>(item);
            break;
         }
      }
      if(!_list.isEmpty()) {
         setItemsOnGrid(_items);
         setModified(true);
      }
   }
}

void SchematicScene::zoomingAtPointEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
   SchematicView *sv = qobject_cast<SchematicView*>(v);
   if(sv) {
      QPointF viewPoint(sv->mapFromScene(event->scenePos()));
      sv->zoomIn();
      QPointF afterScalePoint(sv->mapFromScene(event->scenePos()));
      int dx = (afterScalePoint - viewPoint).toPoint().x();
      int dy = (afterScalePoint - viewPoint).toPoint().y();

      QScrollBar *hb = sv->horizontalScrollBar();
      QScrollBar *vb = sv->verticalScrollBar();

      hb->setValue(hb->value() + dx);
      vb->setValue(vb->value() + dy);
   }
   //TODO: Add zooming by drawing a rectangle for zoom area.
}

void SchematicScene::insertingItemsEvent(MouseActionEvent *event)
{
}

void SchematicScene::insertingEquationEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

void SchematicScene::insertingGroundEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

void SchematicScene::insertingPortEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

void SchematicScene::insertingWireLabelEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

void SchematicScene::normalEvent(MouseActionEvent *e)
{
   switch(e->type()) {
      case QEvent::GraphicsSceneMousePress:
      {
         QGraphicsScene::mousePressEvent(e);
         processForSpecialMove(selectedItems());
      }
      break;


      case QEvent::GraphicsSceneMouseMove:
      {
         if(!m_areItemsMoving) {
            if(e->buttons() & Qt::LeftButton && !selectedItems().isEmpty()) {
               m_areItemsMoving = true;
               if(!m_macroProgress) {
                  m_macroProgress = true;
                  m_undoStack->beginMacro(QString());
               }
            }
         }

         if(!m_areItemsMoving)
            return;
         disconnectDisconnectibles();
         QGraphicsScene::mouseMoveEvent(e);
         QPointF delta = e->scenePos() - e->lastScenePos();
         specialMove(delta.x(), delta.y());
      }
      break;


      case QEvent::GraphicsSceneMouseRelease:
      {
         if(m_areItemsMoving) {
            m_areItemsMoving = false;
            endSpecialMove();
         }
         if(m_macroProgress) {
            m_macroProgress = false;
            m_undoStack->endMacro();
         }
         QGraphicsScene::mouseReleaseEvent(e); // other behaviour by base
      }
      break;

      case QEvent::GraphicsSceneMouseDoubleClick:
         QGraphicsScene::mouseDoubleClickEvent(e);
         break;

      default:
         qDebug("SchematicScene::normalEvent() :  Unknown event type");
   };
}

void SchematicScene::processForSpecialMove(QList<QGraphicsItem*> _items)
{
   disconnectibles.clear();
   movingWires.clear();
   grabMovingWires.clear();

   foreach(QGraphicsItem *item, _items) {
      Component *c = qucsitem_cast<Component*>(item);
      storePos(item);
      if(c) {
         //check for disconnections and wire resizing.
         foreach(Port *port, c->ports()) {
            QList<Port*> *connections = port->connections();
            if(!connections) continue;

            foreach(Port *other, *connections) {
               if(other == port ) continue;


               Component *otherComponent = 0;
               if((otherComponent = other->owner()->component())
                  && !otherComponent->isSelected()) {
                  disconnectibles << c;
                  break;
               }


               Wire *wire = other->owner()->wire();
               if(wire) {
                  Port* otherPort = wire->port1() == other ? wire->port2() :
                     wire->port1();
                  if(!otherPort->areAllOwnersSelected()) {
                     movingWires << wire;
                     wire->storeState();
                  }
               }
            }
         }
      }

      Wire *wire = qucsitem_cast<Wire*>(item);
      if(wire && !movingWires.contains(wire)) {
         bool condition = wire->isSelected();
         condition = condition && (!wire->port1()->areAllOwnersSelected() ||
                                   !wire->port2()->areAllOwnersSelected());
         if(condition) {
            grabMovingWires << wire;
            wire->storeState();
         }
      }
   }

   qDebug() << "\n############Process special move"
            << "\n\tDisconnectibles.size() = " << disconnectibles.size()
            << "\n\tMoving wires.size() = " << movingWires.size()
            << "\n\tGrab moving wires.size() = " << grabMovingWires.size()
            <<"\n#############\n";
}

void SchematicScene::disconnectDisconnectibles()
{
   QSet<Component*> remove;
   foreach(Component *c, disconnectibles) {
      int disconnections = 0;
      foreach(Port *port, c->ports()) {
         if(!port->connections()) continue;

         Port *fromPort = 0;
         foreach(Port *other, *port->connections()) {
            if(other->owner()->component() && other->owner()->component() != c &&
               !other->owner()->component()->isSelected()) {
               fromPort = other;
               break;
            }
         }

         if(fromPort) {
            m_undoStack->push(new DisconnectCmd(port, fromPort));
            ++disconnections;
            AddWireCmd *wc = new AddWireCmd(port, fromPort);
            Wire *wire = wc->wire();
            m_undoStack->push(wc);
            movingWires << wire;
         }
      }
      if(disconnections) {
         remove << c;
      }
   }
   foreach(Component *c, remove)
      disconnectibles.removeAll(c);
}

void SchematicScene::specialMove(qreal dx, qreal dy)
{
   foreach(Wire *wire, movingWires) {
      wire->hide();
      if(wire->port1()->connections()) {
         Port *other = 0;
         foreach(Port *o, *(wire->port1()->connections())) {
            if(o != wire->port1()) {
               other = o;
               break;
            }
         }
         if(other) {
            wire->movePort(wire->port1()->connections(), other->scenePos());
         }
      }
      if(wire->port2()->connections()) {
         Port *other = 0;
         foreach(Port *o, *(wire->port2()->connections())) {
            if(o != wire->port2()) {
               other = o;
               break;
            }
         }
         if(other) {
            wire->movePort(wire->port2()->connections(), other->scenePos());
         }
      }
   }

   foreach(Wire *wire, grabMovingWires) {
      wire->hide();
      wire->grabMoveBy(dx, dy);
   }
}

void SchematicScene::endSpecialMove()
{
   disconnectibles.clear();
   foreach(QGraphicsItem *item, selectedItems()) {
      m_undoStack->push(new MoveCmd(item, storedPos(item), item->scenePos()));
      Component * comp = qucsitem_cast<Component*>(item);
      if(comp) {
         comp->checkAndConnect();
      }
      Wire *wire = qucsitem_cast<Wire*>(item);
      if(wire) {
         wire->checkAndConnect();
      }

   }

   foreach(Wire *wire, movingWires) {
      wire->removeNullLines();
      wire->show();
      wire->movePort1(wire->port1()->pos());
      m_undoStack->push(new WireStateChangeCmd(wire, wire->storedState(),
                                               wire->currentState()));
      wire->checkAndConnect();
   }
   foreach(Wire *wire, grabMovingWires) {
      wire->removeNullLines();
      wire->show();
      wire->movePort(wire->port1()->connections(), wire->port1()->scenePos());
      m_undoStack->push(new WireStateChangeCmd(wire, wire->storedState(),
                                               wire->currentState()));
   }

   grabMovingWires.clear();
   movingWires.clear();
}

void SchematicScene::sendMouseActionEvent(MouseActionEvent *e)
{
   switch(m_currentMouseAction) {
      case Wiring:
         wiringEvent(e);
         break;

      case Deleting:
         deletingEvent(e);
         break;

      case Marking:
         markingEvent(e);
         break;

      case Rotating:
         rotatingEvent(e);
         break;

      case MirroringX:
         mirroringXEvent(e);
         break;

      case MirroringY:
         mirroringYEvent(e);
         break;

      case ChangingActiveStatus:
         changingActiveStatusEvent(e);
         break;

      case SettingOnGrid:
         settingOnGridEvent(e);
         break;

      case ZoomingAtPoint:
         zoomingAtPointEvent(e);
         break;

      case InsertingItems:
         insertingItemsEvent(e);
         break;

      case InsertingEquation:
         insertingEquationEvent(e);
         break;

      case InsertingGround:
         insertingGroundEvent(e);
         break;

      case InsertingPort:
         insertingPortEvent(e);
         break;

      case InsertingWireLabel:
         insertingWireLabelEvent(e);
         break;

      case Normal:
         normalEvent(e);
         break;

      default:;
   };
}
