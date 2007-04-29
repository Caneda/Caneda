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
#include "undocommands.h"

#include <QtCore/QMimeData>
#include <QtCore/QtDebug>
#include <QtCore/QVarLengthArray>

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QUndoStack>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

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
   m_undoStack = new QUndoStack();
   m_areItemsMoving = false;
   m_xGridSize = m_yGridSize = 10;
   m_currentMode = Qucs::SchematicMode;
   m_frameShown = false;
   m_gridShown = true;
   m_simOpenDpl = true;
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
   if(event->mimeData()->formats().contains("application/qucs.sidebarItem"))
   {
      QByteArray encodedData = event->mimeData()->data("application/qucs.sidebarItem");
      QDataStream stream(&encodedData, QIODevice::ReadOnly);
      QString text;
      stream >> text;
      Component *c = Component::componentFromName(text,this);
      if(c)
      {
         c->setPos(event->scenePos());
         event->acceptProposedAction();
      }
   }
   else
      event->ignore();
}

void SchematicScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
   if(e->button() == Qt::MidButton)
   {
      qDebug() << "SchematicScene::mousePressEvent()" << e->pos();
      foreach(QGraphicsItem *item, items(e->pos()))
      {
         Node *node = qgraphicsitem_cast<Node*>(item);
         if(node)
         {
            qDebug() << "Node wire size is" << node->wires().size();
            qDebug() << "Node's Components size is" << node->connectedComponents().size();
            qDebug() << "Are all components selected is" << node->areAllComponentsSelected();
         }
      }
   }

   // This has to be before Scene::mousePress since, wire sets this variable if it is grabbed
   m_grabbedWire = 0;
   QGraphicsScene::mousePressEvent(e);
   m_movingNodes.clear();
   m_resizingWires.clear();
   m_moveResizingWires.clear();
   foreach(QGraphicsItem *item, selectedItems())
   {
      Component *component = qgraphicsitem_cast<Component*>(item);
      Wire *theWire = qgraphicsitem_cast<Wire*>(item);
      if(component)
      {
         foreach(ComponentPort *port, component->componentPorts())
         {
            Node *portNode = port->node();
            portNode->setController(component);

            m_movingNodes.insert(portNode);
            if(portNode->areAllComponentsSelected())
            {
               foreach(Wire *wire, portNode->wires())
                  m_resizingWires.insert(wire);
            }
         }
      }
      else if(theWire && theWire->isSelected() &&
              theWire != m_grabbedWire &&
              theWire->node1()->selectedComponents().size() == 0 &&
              theWire->node2()->selectedComponents().size() == 0)
      {
      	 m_moveResizingWires << theWire;

      }
   }
}

void SchematicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
   if(!m_areItemsMoving)
   {
      if(e->buttons()& Qt::LeftButton && !selectedItems().isEmpty())
         m_areItemsMoving = true;
   }

   QGraphicsScene::mouseMoveEvent(e);

   if(!m_areItemsMoving)
      return;

   if(m_grabbedWire)
   {
      QGraphicsSceneMouseEvent *mouseEvent = new QGraphicsSceneMouseEvent();
      mouseEvent->setButtons(e->buttons());
      mouseEvent->setButton(e->button());
      mouseEvent->setScenePos(e->scenePos());
      mouseEvent->setLastScenePos(e->lastScenePos());
      mouseEvent->setPos(m_grabbedWire->mapFromScene(e->scenePos()));
      mouseEvent->setLastPos(m_grabbedWire->mapFromScene(e->lastScenePos()));
      m_grabbedWire->grabMoveEvent(mouseEvent);
      delete mouseEvent;
   }
   QPointF delta;

   foreach(Node *node, m_movingNodes)
   {
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
      else
      {
         Node *newNode = createNode(oldPos);
         foreach(Component *component,node->connectedComponents())
         {
            if(!component->isSelected())
            {
               component->replaceNode(node,newNode);
               node->removeComponent(component);
               newNode->addComponent(component);
            }
         }
         foreach(Wire *wire, node->wires())
         {
            if(!wire->isSelected())
            {
               wire->replaceNode(node,newNode);
               node->removeWire(wire);
               newNode->addWire(wire);
            }
         }

         Wire *newWire = new Wire(this,node,newNode);
         m_resizingWires.insert(newWire);
      }
   }

   foreach(Wire *wire, m_resizingWires)
   {
      if(wire->isVisible())
         wire->hide();
      wire->rebuild();
   }

   qreal dx = e->scenePos().x() - e->lastScenePos().x();
   qreal dy = e->scenePos().y() - e->lastScenePos().y();

   foreach(Wire *wire, m_moveResizingWires)
   {
      if(wire->isVisible())
         wire->startMoveAndResize();
      wire->moveAndResizeBy(dx,dy);
   }
}

void SchematicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
   if(m_grabbedWire)
   {
      m_grabbedWire->grabReleaseEvent(e);
      m_grabbedWire = 0;
   }
   if(!m_areItemsMoving)
      return QGraphicsScene::mouseReleaseEvent(e);

   m_areItemsMoving = false;

   QSet<Node*> deletions;

   foreach(Node *node, m_movingNodes)
   {
      if(deletions.contains(node))
         continue;
      node->resetController();
      foreach(QGraphicsItem *item, node->collidingItems())
      {
         Node *otherNode = qgraphicsitem_cast<Node*>(item);
         if(!otherNode || deletions.contains(node))
            continue;

         QPointF delta = otherNode->pos() - node->pos();
         m_areItemsMoving = true;
         m_alreadyMoved.clear();
         adjustPositions(node,delta);
         m_alreadyMoved.clear();
         m_areItemsMoving = false;
         connect(node,otherNode);

         deletions.insert(node);
         break;
      }
   }

   foreach(Wire *wire,m_resizingWires)
   {
      wire->show();
      wire->rebuild();
   }

   foreach(Wire *wire, m_moveResizingWires)
   {
      wire->stopMoveAndResize();
      wire->setSelected(true);
   }

   qDeleteAll(deletions);
   deletions.clear();

   m_movingNodes.clear();
   m_resizingWires.clear();

   QGraphicsScene::mouseReleaseEvent(e); // other behaviour by base
}

void SchematicScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
   m_areItemsMoving = true;
   m_grabbedWire = 0;
   QGraphicsScene::mouseDoubleClickEvent(e);
}

void SchematicScene::connect(Node *from, Node *to)
{
   Wire *w =  Wire::connectedWire(from,to);
   if(w)
   {
      m_resizingWires.remove(w);
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
   if(!m_alreadyMoved.contains(node))
   {
      node->moveBy(delta.x(),delta.y());
      m_alreadyMoved << node;
   }
   foreach(Component *component, node->connectedComponents())
   {
      Q_ASSERT(component->isSelected());
      if(m_alreadyMoved.contains(component))
         continue;;
      component->moveBy(delta.x(),delta.y());
      m_alreadyMoved << component;
      foreach(ComponentPort *port, component->componentPorts())
      {
         Node *portNode = port->node();
         if(portNode->connectedComponents().size() > 1)
            adjustPositions(portNode,delta);
         else if(!m_alreadyMoved.contains(portNode))
         {
            portNode->moveBy(delta.x(),delta.y());
            m_alreadyMoved << portNode;
         }
      }
   }
}

void SchematicScene::setGrabbedWire(Wire *w)
{
   m_grabbedWire = w;
}

void SchematicScene::setMode(Qucs::Mode mode)
{
   if(m_currentMode == mode) return;
}


Node* SchematicScene::nodeAt(qreal x, qreal y)
{
   return nodeAt(QPointF(x,y));
}

static Node* singlifyNodes(QSet<Node*> &nodeSet)
{
   // Many nodes found at same location. Hence remove all duplicate nodes
   // by adding all components to one node and deleting others
   Node *fn = *(nodeSet.begin());
   nodeSet.erase(nodeSet.begin()); // remove first one from set and delete others
   const QSet<Component*>& fnc = fn->connectedComponents();
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

// Returns node at location given by centre
Node* SchematicScene::nodeAt(const QPointF& centre)
{
   QSet<Node*> ns;
   foreach(QGraphicsItem *item, items(centre))
   {
      Node *node = qgraphicsitem_cast<Node*>(item);
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

QUndoStack* SchematicScene::undoStack()
{
   return m_undoStack;
}

bool SchematicScene::areItemsMoving() const
{
   return m_areItemsMoving;
}

void SchematicScene::drawBackground(QPainter *painter, const QRectF& rect)
{
   painter->setPen(QPen(Qt::black,0));
   painter->setBrush(Qt::NoBrush);
   painter->setRenderHint(QPainter::Antialiasing,false);

   qreal left = int(rect.left()) - (int(rect.left()) % m_xGridSize);
   qreal top = int(rect.top()) - (int(rect.top()) % m_yGridSize);
   qreal bottom = rect.bottom();
   qreal right = rect.right();

   QVarLengthArray<QPointF, 800> points;
   for( qreal x = left; x < right; x+=m_xGridSize)
      for( qreal y = top; y < bottom; y+=m_yGridSize)
         points.append(QPointF(x,y));
   painter->drawPoints(points.data(),points.size());
   painter->setRenderHint(QPainter::Antialiasing,true);
}

void SchematicScene::insertComponent(Component *comp)
{
   SchematicScene *old = comp->schematicScene();
   if(old != this)
   {
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
      port->setNode(n);
   }

   foreach(ComponentProperty *prop, comp->m_properties)
      if(!prop->isVisible() || prop->item()->scene() != this)
         addItem(prop->item());
}

void SchematicScene::removeComponent(Component *comp)
{
   m_components.removeAll(comp);
   foreach(ComponentPort *port, comp->m_ports)
   {
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
