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
#include "node.h"
#include "wire.h"
#include "components/components.h"
#include "components/componentproperty.h"
#include "paintings/painting.h"
#include "diagrams/diagram.h"
#include "undocommands.h"
#include "qucsprimaryformat.h"

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

#include <cmath>

static Node* singlifyNodes(QSet<Node*> &nodeSet);


SchematicScene::SchematicScene(QObject *parent) : QGraphicsScene(parent)
{
   init();
}

SchematicScene::SchematicScene ( qreal x, qreal y, qreal width, qreal height, QObject * parent ) : QGraphicsScene(x,y,width,height,parent)
{
   init();
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
   foreach(QGraphicsItem *item, items(centre))
   {
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
   n->setPos(centre);
   return n;
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

   if(m_components.count(comp) == 0)
      m_components.append(comp);

   foreach(ComponentPort *port, comp->m_ports)
   {
      Node *n = port->node();
      n->removeComponent(comp);
      if(n->isEmpty()) delete n;
      QPointF pos = comp->mapToScene(port->centrePos());
      n = nodeAt(pos);
      if(!n) n = createNode(pos);
      n->addComponent(comp);
      port->setNode(n);
   }

   foreach(ComponentProperty *prop, comp->m_properties)
      if(prop->isVisible() && prop->item()->scene() != this)
         addItem(prop->item());

   //TODO: add number suffix to component name
}

void SchematicScene::removeComponent(Component *comp)
{
   m_components.removeAll(comp);
   foreach(ComponentPort *port, comp->m_ports) {
      Node *n = port->node();
      n->removeComponent(comp);
      if(n->isEmpty())
         removeItem(n);
   }

   foreach(ComponentProperty *prop, comp->m_properties)
      if(prop->item())
         removeItem(prop->item());

   removeItem(comp);
}

void SchematicScene::insertWire(Wire *w)
{
   if(m_wires.count(w) == 0)
      m_wires << w;
   //TODO: check labels and stuff
}

void SchematicScene::removeWire(Wire *w)
{
   m_wires.removeAll(w);
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
   //TODO
}

void SchematicScene::setFileName(const QString& name)
{
   if(name == m_fileName || name.isEmpty())
      return;
   m_fileName = name;
   QFileInfo info(m_fileName);
   m_dataSet = info.baseName() + ".dat";
   m_dataDisplay = info.baseName() + ".dpl";
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
   //TODO
}

void SchematicScene::setCurrentMouseAction(MouseAction action)
{
   if(m_currentMouseAction == action) return;

   m_movingNodes.clear();
   m_resizingWires.clear();
   m_moveResizingWires.clear();

   m_areItemsMoving = false;
   m_grabbedWire = 0;
   if(eventWire) {
      Node *n = 0;
      foreach(QGraphicsItem *item, helperNode->collidingItems()) {
         n = qucsitem_cast<Node*>(item);
         if(n) break;
      }
      if(!n)
         n = createNode(helperNode->pos());
      else
         helperNode->setPos(n->pos());
      eventWire->rebuild();
      eventWire->setNode2(n);
      eventWire->show();
   }
   eventWire = 0;
   helperNode = 0;

   m_currentMouseAction = action;

   QGraphicsView::DragMode dragMode = (action == Normal) ? QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag;
   foreach(QGraphicsView *view, views())
      view->setDragMode(dragMode);

   //TODO: Implemement this appropriately for all mouse actions
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
      //#HACK: The event generating active view is obtained indirectly so that
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
      Component *c = Component::componentFromName(text,this);
      if(c) {
         c->setPos(event->scenePos());
         v->horizontalScrollBar()->setValue(hor);
         v->verticalScrollBar()->setValue(ver);
         event->acceptProposedAction();
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
   //m_areItemsMoving = true;
   m_grabbedWire = 0;
   QGraphicsScene::mouseDoubleClickEvent(e);
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
               eventWire->rebuild();
               eventWire->show();
               eventWire->rebuild();
               insertWire(eventWire);
               eventWire = 0;
               break;
            }
            n = createNode(pos);
            eventWire->setNode2(n);
            eventWire->show();
            eventWire->rebuild();
            insertWire(eventWire);
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
         helperNode->hide();
         eventWire = new Wire(this,n,helperNode);
         insertWire(eventWire);
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
   //TODO
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
      mirrorXItems(_items);
   }
}

void SchematicScene::changingActiveStatusEvent(MouseActionEvent *event)
{
   //TODO
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
   //TODO
}

void SchematicScene::insertingEquationEvent(MouseActionEvent *event)
{
   //TODO
}

void SchematicScene::insertingGroundEvent(MouseActionEvent *event)
{
   //TODO
}

void SchematicScene::insertingPortEvent(MouseActionEvent *event)
{
   //TODO
}

void SchematicScene::insertingWireLabelEvent(MouseActionEvent *event)
{
   //TODO
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
            Q_ASSERT(node->isControllerSet());
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
               foreach(Component *component,node->connectedComponents()) {
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
            node->resetController();
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

      default:
         qDebug("SchematicScene::normalEvent() :Unknown event type");
   };
}


void SchematicScene::init()
{
   m_undoStack = new QUndoStack(this);

   m_gridWidth = m_gridHeight = 10;
   m_currentMode = Qucs::SchematicMode;
   m_frameVisible = false;
   m_gridVisible = true;
   m_opensDataDisplay = true;
   m_frameTexts = QStringList() << tr("Title") << tr("Drawn By:") << tr("Date:") << tr("Revision:");

   eventWire = 0;
   helperNode = 0;
   m_grabbedWire = 0;
   m_areItemsMoving = false;

   setCurrentMouseAction(Normal);
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

   foreach(Component *component, from->connectedComponents())
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
   foreach(Component *component, node->connectedComponents()) {
      if(!component->isSelected() || m_alreadyMoved.contains(component))
         continue;
      component->moveBy(delta.x(),delta.y());
      m_alreadyMoved << component;
      foreach(ComponentPort *port, component->componentPorts()) {
         Node *portNode = port->node();
         if(portNode->connectedComponents().size() > 1)
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
   const QList<Component*>& fnc = fn->connectedComponents();
   foreach(Node *n, nodeSet)
   {
      foreach(Component *c, n->connectedComponents())
      {
         if(!fnc.contains(c))
         {
            fn->addComponent(c);
            //find port containing dup node and modify its variable
            ComponentPort *pt = c->portWithNode(n);
            if(pt)
               pt->setNode(fn);
         }
      } //n->connectedComponents()
   } //ns

   qDeleteAll(nodeSet); //now safe to delete
   qDebug("Just for info: Found many nodes at same location. Singlified them");
   return fn;
}
