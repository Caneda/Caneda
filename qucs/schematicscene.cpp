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
#include "node.h"
#include "wire.h"
#include "components/components.h"
#include "components/componentproperty.h"
#include "paintings/painting.h"
#include "diagrams/diagram.h"
#include "undocommands.h"
#include "qucsprimaryformat.h"
#include "xmlutilities.h"
#include "qucs-tools/global.h"

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

static Node* singlifyNodes(QSet<Node*> &nodeSet);


SchematicScene::SchematicScene(QObject *parent) : QGraphicsScene(parent)
{
   init();
}

SchematicScene::SchematicScene ( qreal x, qreal y, qreal width, qreal height, QObject * parent ) : QGraphicsScene(x,y,width,height,parent)
{
   init();
//   testInsertingItems();
}

void SchematicScene::init()
{
   m_undoStack = new QUndoStack(this);
   m_gridWidth = m_gridHeight = 10;
   m_currentMode = Qucs::SchematicMode;
   m_frameVisible = false;
   m_modified = false;
   m_gridVisible = true;
   m_opensDataDisplay = true;
   m_frameTexts = QStringList() << tr("Title") << tr("Drawn By:") << tr("Date:") << tr("Revision:");

   eventWire = 0;
   helperNode = 0;
   m_grabbedWire = 0;
   m_areItemsMoving = false;
   setCurrentMouseAction(Normal);
}

SchematicScene::~SchematicScene()
{
}

Node* SchematicScene::nodeAt(qreal x, qreal y)
{
   return nodeAt(QPointF(x,y));
}

// Returns node at location given by centre
Node* SchematicScene::nodeAt(const QPointF& centre)
{
   QSet<Node*> ns;

   foreach(QGraphicsItem *item, items(centre)) {
      Node *node = qucsitem_cast<Node*>(item);
      if(!node)
         continue;
      if(node->pos() == centre)
         ns << node;
   }
   if(ns.size() == 1) //only one node found
      return *(ns.begin());
   else if(ns.isEmpty()) //no node found
      return 0;
   else
      return singlifyNodes(ns);
}

Node* SchematicScene::createNode(const QPointF& centre)
{
   Node *n = new Node(QString("FH"),this);
   //TODO: Implement node numbering/suffix
   n->setPos(centre);
   return n;
}

void SchematicScene::removeNode(Node *n)
{
   m_nodes.removeAll(n);
   removeItem(n);
}

void SchematicScene::setGrabbedWire(Wire *w)
{
   m_grabbedWire = w;
}

void SchematicScene::insertComponent(Component *comp)
{
   SchematicScene *old = comp->schematicScene();
   if(old != this) {
      if(old)
         old->removeComponent(comp);
      addItem(comp);
   }

   if(!m_components.contains(comp))
      m_components.append(comp);

   foreach(ComponentPort *port, comp->m_ports) {
      Node *n = port->node();
      n->removeComponent(comp);
      if(n->isEmpty()) delete n;
      QPointF pos = comp->mapToScene(port->centrePos());
      n = nodeAt(pos);
      if(!n) n = createNode(pos);
      n->addComponent(comp);
      port->setNode(n);
   }
   if(comp->propertyGroup()->scene() != this) {
      addItem(comp->propertyGroup());
      comp->propertyGroup()->realignItems();
   }

   //TODO: add number suffix to component name
}

void SchematicScene::removeComponent(Component *comp)
{
   m_components.removeAll(comp);
   foreach(ComponentPort *port, comp->m_ports) {
      Node *n = port->node();
      n->removeComponent(comp);
      if(n->isEmpty()) {
         removeNode(n);
         delete n;
      }
   }

   removeItem(comp->propertyGroup());
   removeItem(comp);
}

void SchematicScene::insertWire(Wire *w)
{
   if(!m_wires.contains(w) == 0)
      m_wires << w;
   //TODO: check labels and stuff
}

void SchematicScene::removeWire(Wire *w)
{
   w->node1()->removeWire(w);
   if(w->node1()->isEmpty()) {
      removeNode(w->node1());
      delete w->node1();
   }
   w->node2()->removeWire(w);
   if(w->node2()->isEmpty()) {
      removeNode(w->node2());
      delete w->node2();
   }
   m_wires.removeAll(w);
   removeItem(w);
}

void SchematicScene::mirrorXItems(QList<QucsItem*>& items)
{
   foreach(QucsItem* item, items)
      item->mirrorX();
}

void SchematicScene::mirrorYItems(QList<QucsItem*>& items)
{
   foreach(QucsItem* item, items)
      item->mirrorY();
}

void SchematicScene::rotateItems(QList<QucsItem*>& items)
{
   foreach(QucsItem* item, items)
      item->rotate();
}

void SchematicScene::deleteItems(QList<QucsItem*>& items)
{
   foreach(QucsItem* item, items) {
      if(qucsitem_cast<Node*>(item))
         continue;
      delete item;
   }
}

void SchematicScene::setItemsOnGrid(QList<QucsItem*>& items)
{
   foreach(QucsItem* item, items) {
      if(qucsitem_cast<Wire*>(item) ||
         qucsitem_cast<Node*>(item))
         continue;
      QPointF pos = item->pos();
      pos = nearingGridPoint(pos);
      item->setPos(pos);
   }
}

void SchematicScene::toggleActiveStatus(QList<QucsItem*>& components)
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
   int x = int(pos.x());
   int y = int(pos.y());
   int xr = x % m_gridWidth;
   int yr = y % m_gridHeight;
   x = xr > int(m_gridWidth/2) ? x - xr + m_gridWidth : x - xr;
   y = yr > int(m_gridHeight/2) ? y - yr + m_gridHeight : y - yr;
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

   m_movingNodes.clear();
   m_resizingWires.clear();
   m_moveResizingWires.clear();
   if(!m_insertingItems.isEmpty() && action != InsertingItems) {
      qDeleteAll(m_insertingItems);
      m_insertingItems.clear();
   }

   m_areItemsMoving = false;
   m_grabbedWire = 0;

   if(eventWire) {
      Node *n = eventWire->node1();
      n->show();
      delete eventWire;
      delete helperNode;
      if(n->wires().isEmpty() && n->components().isEmpty())
         delete n;
      else if(!createdWires.isEmpty()) {
         Wire *finalWire = createdWires.takeFirst();
         //n = createdWires.last()->node2();
         QList<WireLine> wLines = finalWire->wireLines();
         QSet<Node*> delNodes;
         foreach(Wire *w, createdWires) {
            foreach(WireLine line, w->wireLines()) {
               QPointF p1 = finalWire->mapFromItem(w, line.p1());
               QPointF p2 = finalWire->mapFromItem(w, line.p2());
               wLines << WireLine(p1, p2);
            }
            Node *n1 = w->node1();
            Node *n2 = w->node2();
            delete w;
            if(n1 != n) delNodes << n1;
            if(n2 != n) delNodes << n2;
         }

         qDeleteAll(delNodes);
         createdWires.clear();
         finalWire->setNode2(n);
         n->show();
         n->addWire(finalWire);
         finalWire->show();
         finalWire->setWireLines(wLines);
      }
   }
   eventWire = 0;
   helperNode = 0;
   createdWires.clear();
   m_currentMouseAction = action;

   QGraphicsView::DragMode dragMode = (action == Normal) ? QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag;
   foreach(QGraphicsView *view, views())
      view->setDragMode(dragMode);
   setFocusItem(0);
   clearSelection();
   if(action == InsertingGround) {
      m_currentMouseAction = InsertingItems;
      Component *g = Component::componentFromModel("GND", this);
      m_insertingItems << g;
      g->setSelected(true);
      QPoint m = QCursor::pos();
      m = g->activeView()->mapFromGlobal(m);
      QPointF p = g->activeView()->mapToScene(m);
      g->setPos(p);
   }
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
      _item->writeXml(writer);
   writer->writeEndDocument();

   QClipboard *clipboard =  QApplication::clipboard();
   clipboard->setText(clipText);
}

void SchematicScene::paste()
{
   QString clipText = QApplication::clipboard()->text();
   Qucs::XmlReader *reader = new Qucs::XmlReader(clipText);
   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isStartElement()) {
         if(reader->name() == "qucs" &&
            Qucs::checkVersion(reader->attributes().value("version").toString()))
            break;
         else {
            reader->raiseError(QObject::tr("Cannot paste the non qucs items here"));
         }
      }
   }

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isStartElement()) {
         if(reader->name() == "component") {
            QString model = reader->attributes().value("model").toString();
            Component *c = Component::componentFromModel(model, this);
            if(!c)
               reader->raiseError(QObject::tr("Component %1 doesn't exist").arg(model));
            else {
               c->readXml(reader);
               c->checkForConnections();
            }
         }
         else if(reader->name() == "wire") {
            reader->readFurther();
            QPointF n1Pos, n2Pos;
            if(reader->isStartElement() && reader->name() == "node1pos") {
               reader->readFurther();
               n1Pos = reader->readPoint();
               reader->readFurther();
               Q_ASSERT(reader->isEndElement() && reader->name() == "node1pos");
            }
            else {
               reader->raiseError(QObject::tr("Couldn't read node 1 position of wire"));
               break;
            }

            reader->readFurther();

            if(reader->isStartElement() && reader->name() == "node2pos") {
               reader->readFurther();
               n2Pos = reader->readPoint();
               reader->readFurther();
               Q_ASSERT(reader->isEndElement() && reader->name() == "node2pos");
            }
            else {
               reader->raiseError(QObject::tr("Couldn't read node 2 position of wire"));
               break;
            }



            Node *n1 = nodeAt(n1Pos);
            if(!n1)
               n1 = createNode(n1Pos);

            Node *n2 = nodeAt(n2Pos);
            if(!n2)
               n2 = createNode(n2Pos);

            Wire *wire = new Wire(this, n1, n2);
            reader->readFurther();
            Q_ASSERT(reader->isStartElement() && reader->name() == "wirelines");
            wire->readXml(reader);
            if(reader->hasError()) {
               delete wire;
               break;
            }

            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "wire");
         }
      }
   }


   if(reader->hasError()) {
      QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
      delete reader;
   }
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

   int gridWidth = m_gridWidth,gridHeight = m_gridHeight;

   // Adjust visual representation of grid to be multiple, if
   // grid sizes are very small

   while(gridWidth < 20)
      gridWidth = int(gridWidth * 1.5);
   while(gridHeight < 20)
      gridHeight = int(gridHeight * 1.5);

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
      Component *c = Component::componentFromModel(text,this);
      insertComponent(c);
      if(c) {
         c->setPos(event->scenePos());
         c->checkForConnections(false);
         c->setInitialPropertyPosition();
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
   sendMouseActionEvent(e);
}

void SchematicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
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
   Node *n = 0;

   switch(event->type()) {
      case QEvent::GraphicsSceneMousePress:
         if(eventWire) {
            QPointF pos = helperNode->pos();

            foreach(QGraphicsItem *item, helperNode->collidingItems()) {
               n = qucsitem_cast<Node*>(item);
               if(n) break;
            }
            delete helperNode;
            helperNode = 0;
            if(n) {
               eventWire->setNode2(n);
               n->addWire(eventWire);
               createdWires << eventWire;
               Wire *finalWire = createdWires.takeFirst();
               QList<WireLine> wLines = finalWire->wireLines();
               QSet<Node*> delNodes;
               foreach(Wire *w, createdWires) {
                  foreach(WireLine line, w->wireLines()) {
                     QPointF p1 = finalWire->mapFromItem(w, line.p1());
                     QPointF p2 = finalWire->mapFromItem(w, line.p2());
                     wLines << WireLine(p1, p2);
                  }
                  Node *n1 = w->node1();
                  Node *n2 = w->node2();
                  delete w;
                  if(n1 != n) delNodes << n1;
                  if(n2 != n) delNodes << n2;
               }
               qDeleteAll(delNodes);
               createdWires.clear();
               finalWire->setNode2(n);
               n->addWire(finalWire);
               finalWire->setWireLines(wLines);

               eventWire = 0;
               break;
            }
            n = createNode(pos);
            eventWire->setNode2(n);
            n->addWire(eventWire);
            eventWire->show();
            eventWire->rebuild();
            createdWires << eventWire;
         }

         n = 0;
         foreach(QGraphicsItem *item, items(event->scenePos())) {
            n = qucsitem_cast<Node*>(item);
            if(n) break;
         }
         if(!n) n = createNode(event->scenePos());
         if(eventWire)
            n->hide();
         helperNode = createNode(event->scenePos());
         eventWire = new Wire(this,n,helperNode);
         eventWire->hide();

         break;

      case QEvent::GraphicsSceneMouseMove:
         if(eventWire) {
            helperNode->setPos(event->scenePos());
            eventWire->rebuild();
         }
         break;

      default:;
   };
}

void SchematicScene::deletingEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   if(!_list.isEmpty()) {
      QList<QucsItem*> _items;
      foreach(QGraphicsItem *item, _list) {
         if(qucsitem_cast<QucsItem*>(item) && !qucsitem_cast<Node*>(item)) {
            _items << static_cast<QucsItem*>(item);
            break;
         }
      }
      deleteItems(_items);
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
   if(!_list.isEmpty()) {
      QList<QucsItem*> _items;
      foreach(QGraphicsItem *item, _list) {
         if(qucsitem_cast<QucsItem*>(item)) {
            _items << static_cast<QucsItem*>(item);
            break;
         }
      }
      rotateItems(_items);
   }
}

void SchematicScene::mirroringXEvent(MouseActionEvent *event)
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
      mirrorXItems(_items);
   }
}

void SchematicScene::mirroringYEvent(MouseActionEvent *event)
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
      mirrorYItems(_items);
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
         if(qucsitem_cast<QucsItem*>(item) && !qucsitem_cast<Node*>(item)
            && !qucsitem_cast<Wire*>(item)) {
            _items << static_cast<QucsItem*>(item);
            break;
         }
      }
      setItemsOnGrid(_items);
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
   if(event->type() == QEvent::GraphicsSceneMousePress) {
      QList<QucsItem*> newList;
      foreach(QucsItem *item, m_insertingItems) {
         if(qucsitem_cast<Component*>(item)) {
            Component *c = static_cast<Component*>(item);
            insertComponent(c);
            Component *nc = c->newOne();
            c->copyTo(nc);
            c->setSelected(false);
            newList << nc;
            nc->setSelected(true);
         }
         else if(qucsitem_cast<Wire*>(item)) {
            insertWire(static_cast<Wire*>(item));
         }
      }
      m_insertingItems = newList;
      //m_insertingItems.clear();
      //setCurrentMouseAction(Normal);
   }
   else if(event->type() == QEvent::GraphicsSceneMouseMove) {
      bool b =  m_areItemsMoving;
      m_areItemsMoving = false;
      QPointF delta = event->scenePos() - event->lastScenePos();
      foreach(QucsItem *item, m_insertingItems) {
         item->moveBy(delta.x(), delta.y());
      }
      m_areItemsMoving = b;
   }
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
         // This has to be before Scene::mousePress since, wire sets this variable if it is grabbed
         m_grabbedWire = 0;
         QGraphicsScene::mousePressEvent(e);
         // Clear the containers from previous press
         m_movingNodes.clear();
         m_resizingWires.clear();
         m_moveResizingWires.clear();

         // Navigate through selected items and add them to the
         // corresponding containers for further processing.
         foreach(QGraphicsItem *item, selectedItems()) {
            Component *component = qucsitem_cast<Component*>(item);
            Wire *theWire = qucsitem_cast<Wire*>(item);
            if(component) {
               foreach(ComponentPort *port, component->componentPorts()) {
                  Node *portNode = port->node();
                  portNode->setController(component);

                  if(!m_movingNodes.contains(portNode))
                     m_movingNodes << portNode;
                  if(portNode->areAllComponentsSelected()) {
                     foreach(Wire *wire, portNode->wires()) {
                        if(!m_resizingWires.contains(wire))
                           m_resizingWires << wire;
                     }
                  }
               }
            }
            else if(theWire && theWire->isSelected() &&
                    theWire != m_grabbedWire &&
                    theWire->node1()->components().size() != 0 &&
                    theWire->node2()->components().size() != 0 &&
                    theWire->node1()->selectedComponents().size() == 0 &&
                    theWire->node2()->selectedComponents().size() == 0 &&
                    !m_moveResizingWires.contains(theWire)) {
               m_moveResizingWires << theWire;
            }
         }
      }
      break;


      case QEvent::GraphicsSceneMouseMove:
      {
         if(!m_areItemsMoving) {
            if(e->buttons() & Qt::LeftButton && !selectedItems().isEmpty())
               m_areItemsMoving = true;
         }

         QGraphicsScene::mouseMoveEvent(e);

         if(!m_areItemsMoving) return;

         // Send event to hidden wire since the framework doesn't do that
         if(m_grabbedWire)
            m_grabbedWire->grabMoveEvent(e);
         QPointF delta;
         foreach(Node *node, m_movingNodes) {
            Q_ASSERT(node->controller());
            Component* controller = node->controller();
            ComponentPort *port = controller->portWithNode(node);
            Q_ASSERT(controller && port);
            QPointF oldPos = node->pos();
            QPointF newPos = controller->mapToScene(port->centrePos());
            if(delta.isNull())
               delta = newPos - oldPos;
            //Note: Here wires selection isn't taken into account
            if(node->areAllComponentsSelected())
               node->setPos(newPos);
            else {
               Node *newNode = createNode(oldPos);
               foreach(Component *component,node->components()) {
                  if(!component->isSelected()) {
                     component->replaceNode(node,newNode);
                     node->removeComponent(component);
                     newNode->addComponent(component);
                  }
               }
               foreach(Wire *wire, node->wires()) {
                  if(!wire->isSelected()) {
                     wire->replaceNode(node,newNode);
                     node->removeWire(wire);
                     newNode->addWire(wire);
                  }
               }

               Wire *newWire = new Wire(this,node,newNode);
               m_resizingWires << newWire;
            }
         }

         foreach(Wire *wire, m_resizingWires) {
            if(wire != m_grabbedWire && wire->isVisible())
               wire->hide();
            wire->rebuild();
         }
         if(m_moveResizingWires.count() && m_grabbedWire)
            m_moveResizingWires.removeAll(m_grabbedWire);
         qreal dx = e->scenePos().x() - e->lastScenePos().x();
         qreal dy = e->scenePos().y() - e->lastScenePos().y();

         foreach(Wire *wire, m_moveResizingWires) {
            if(wire != m_grabbedWire) {
               if(wire->isVisible())
                  wire->startMoveAndResize();
               wire->moveAndResizeBy(dx,dy);
            }
         }
      }
      break;


      case QEvent::GraphicsSceneMouseRelease:
      {
         if(m_grabbedWire) {
            m_grabbedWire->grabReleaseEvent(e);
            m_grabbedWire = 0;
         }
         if(!m_areItemsMoving)
            return QGraphicsScene::mouseReleaseEvent(e);

         m_areItemsMoving = false;

         QSet<Node*> deletions;

         foreach(Node *node, m_movingNodes) {
            if(deletions.contains(node))
               continue;
            node->setController(0);
            foreach(QGraphicsItem *item, node->collidingItems()) {
               Node *otherNode = qucsitem_cast<Node*>(item);
               if(!otherNode || deletions.contains(node))
                  continue;

               QPointF delta = otherNode->pos() - node->pos();
               m_areItemsMoving = true;
               m_alreadyMoved.clear();
               adjustPositions(node,delta);
               m_alreadyMoved.clear();
               m_areItemsMoving = false;
               connectNodes(node,otherNode);

               deletions.insert(node);
               break;
            }
         }

         foreach(Wire *wire,m_resizingWires) {
            wire->show();
            wire->rebuild();
            wire->delNullLines();
         }

         foreach(Wire *wire, m_moveResizingWires) {
            wire->stopMoveAndResize();
            wire->setSelected(true);
         }

         qDeleteAll(deletions);
         deletions.clear();

         m_movingNodes.clear();
         m_resizingWires.clear();

         QGraphicsScene::mouseReleaseEvent(e); // other behaviour by base
      }
      break;

      case QEvent::GraphicsSceneMouseDoubleClick:
         m_grabbedWire = 0;
         QGraphicsScene::mouseDoubleClickEvent(e);
         break;

      default:
         qDebug("SchematicScene::normalEvent() :Unknown event type");
   };
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

void SchematicScene::connectNodes(Node *from, Node *to)
{
   Wire *w =  Wire::connectedWire(from,to);
   if(w) {
      int index = m_resizingWires.indexOf(w);
      if(index != -1)
         m_resizingWires.removeAt(index);
      index = m_moveResizingWires.indexOf(w);
      if(index != -1)
         m_moveResizingWires.removeAt(index);
      delete w;
   }

   to->addAllComponentsFrom(from);
   to->addAllWiresFrom(from);

   foreach(Component *component, from->components())
      component->replaceNode(from,to);
   foreach(Wire *wire,from->wires())
      wire->replaceNode(from,to);
}

void SchematicScene::adjustPositions(Node *node,const QPointF& delta)
{
   if(!m_alreadyMoved.contains(node)) {
      node->moveBy(delta.x(),delta.y());
      m_alreadyMoved << node;
   }
   foreach(Component *component, node->components()) {
      if(!component->isSelected() || m_alreadyMoved.contains(component))
         continue;
      component->moveBy(delta.x(),delta.y());
      m_alreadyMoved << component;
      foreach(ComponentPort *port, component->componentPorts()) {
         Node *portNode = port->node();
         if(portNode->components().size() > 1)
            adjustPositions(portNode,delta);
         else if(!m_alreadyMoved.contains(portNode)) {
            portNode->moveBy(delta.x(),delta.y());
            m_alreadyMoved << portNode;
         }
      }
   }
}

static Node* singlifyNodes(QSet<Node*> &nodeSet)
{
   // Many nodes found at same location. Hence remove all duplicate nodes
   // by adding all components to one node and deleting others
   Node *fn = *(nodeSet.begin());
   nodeSet.erase(nodeSet.begin()); // remove first one from set and delete others
   const QList<Component*>& fnc = fn->components();
   const QList<Wire*>& fnw = fn->wires();
   foreach(Node *n, nodeSet) {
      foreach(Component *c, n->components()) {
         if(!fnc.contains(c)) {
            fn->addComponent(c);
            //find port containing dup node and modify its variable
            ComponentPort *pt = c->portWithNode(n);
            if(pt)
               pt->setNode(fn);
         }
      } //n->components()
      foreach(Wire *w, n->wires()) {
         if(!fnw.contains(w)) {
            fn->addWire(w);
            if(w->node1() == n)
               w->setNode1(fn);
            else {
               Q_ASSERT(w->node2() == n);
               w->setNode2(fn);
            }
         }
      }
   } //ns

   qDeleteAll(nodeSet); //now safe to delete
   return fn;
}

void SchematicScene::testInsertingItems()
{
   Component *r = Component::componentFromModel("R", this);
   m_insertingItems << r;
   r->setPos(0,0);
   r = Component::componentFromModel("R", this);
   m_insertingItems << r;
   r->setPos(92,0);
   Node *n1 = createNode(QPointF(90,0));
   Node *n2 = createNode(QPointF(150,5));
   Wire *w = new Wire(this, n1, n2);
   w->setSelected(true);
   m_insertingItems << w;
   //m_insertingItems << n1 << n2;
   setCurrentMouseAction(InsertingItems);
   foreach(QucsItem *i, m_insertingItems)
      i->setSelected(true);
}
