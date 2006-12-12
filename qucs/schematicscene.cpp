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
#include "components/resistor.h"
#include "components/node.h"
#include "components/wire.h"
#include "undocommands.h"

#include <QtCore/QMimeData>
#include <QtCore/QtDebug>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>


// Returns wire connected between two nodes
static Wire* connectedWire(const Node* n1, const Node* n2)
{
   foreach(Component *c, n1->connectedComponents())
   {
      Wire *w = qgraphicsitem_cast<Wire*>(c);
      if(w)
      {
	 const QList<ComponentPort*>& pl = w->componentPorts();
	 Q_ASSERT(pl.size() == 2);
	 if((pl[0]->node() == n1 && pl[1]->node() == n2) || (pl[1]->node() == n1 && pl[0]->node() == n2))
	    return w;
      }
   }
   return 0l;
}

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
      if(text == "Resistor")
      {
	 Resistor *r = new Resistor(this);
	 r->setPos(event->scenePos());
	 event->acceptProposedAction();
      }
   }
   else
      event->ignore();
}

void SchematicScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
   
   QGraphicsScene::mousePressEvent(e);
}

void SchematicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
   //This condition serves 2 purposes
   // (1) -> It detects whether event is generated because of moving items.
   // (2) -> If also helps to detect items before being moved for 1st time.
   if(!m_areItemsMoving && (e->buttons() & Qt::LeftButton) && !selectedItems().isEmpty())
   {
      //processBeforeMouseMove();
      m_areItemsMoving = true;
   }

   QGraphicsScene::mouseMoveEvent(e);

   //moveCommonNodes();
}

void SchematicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
   if(!m_areItemsMoving)
      return QGraphicsScene::mouseReleaseEvent(e);
   m_areItemsMoving = false;
   QSet<Node*> trash;
   foreach(QGraphicsItem *item, selectedItems())
   {
      Component *c = qgraphicsitem_cast<Component*>(item);
      if(!c)
	 continue;

      foreach(ComponentPort *p,c->componentPorts())
      {
	 if(trash.contains(p->node()))
	    continue;
	 QList<QGraphicsItem*> coll = p->node()->collidingItems();
	 foreach(QGraphicsItem *citem, coll)
	 {
	    Node *n = 0l;
	    if(citem->type() == QucsItem::NodeType)
	       n = (Node*)(citem);
	    if(!n)
	       continue;
	    if(trash.contains(n))
	       continue;
	    Q_ASSERT(n != p->node());
	    qreal dx = n->x() - p->node()->x();
	    qreal dy = n->y() - p->node()->y();
	    c->moveBy(dx,dy);
	    
	    
	    QSet<Component*>& connComp = const_cast<QSet<Component*>&>(n->connectedComponents());
	    connComp.unite(p->node()->connectedComponents());
	    
	    trash.insert(p->node());
	    m_circuitNodes.removeAll(p->node());
	    

	    foreach(Component *c, connComp)
	    {
	       foreach(ComponentPort *port,c->componentPorts())
	       {
		  if(port != p && port->node() == p->node())
		     port->m_node = n;
	       }
	    }
	    p->m_node = n;
	 }
      }
   }
   
   qDeleteAll(trash);
   
   
   QGraphicsScene::mouseReleaseEvent(e); // other behaviour by base
}

void SchematicScene::processBeforeMouseMove()
{
   foreach(QGraphicsItem *item,selectedItems())
   {

      Component *c = qgraphicsitem_cast<Component*>(item);
      if(c)
      {
	 c->determineHowToMove();
	 foreach(ComponentPort *port, c->componentPorts())
	 {
	    if(port->node()->selectedComponents().size() > 1)
	    {
	       bool shdBackup = true;
	       foreach(Component *sc, port->node()->connectedComponents())
	       {
		  if(sc->type() == QucsItem::WireType && !sc->isSelected())
		  {
		     qDebug("Here");
		     shdBackup = false;
		     break;
		  }
	       }
	       if(shdBackup)
		  port->node()->backupScenePos();
	       m_commonMovingNodes.insert(port->node());
	    }
		  
	 }
	 c->backupScenePos();
      }
      else
      {
	 QucsItem *qitem = qgraphicsitem_cast<QucsItem*>(item);
	 if(qitem)
	    qitem->backupScenePos();
      }
   }
}

void SchematicScene::moveCommonNodes()
{
   if(m_commonMovingNodes.isEmpty())
      return;
   foreach(Node *n, m_commonMovingNodes)
   {
      QPointF oldPos = n->savedScenePosition();
      qDebug() << oldPos;
      n->setPos(n->newPos());
      if(n->areAllComponentsSelected())
	 continue;

      Node *sn = nodeAt(oldPos);
      Wire *w;
      if(sn && (w = ::connectedWire(n,sn)))
      	 w->rebuild();
      else
      {
	 if(!sn)
	 {
	    static int cnt = 0;
	    qDebug() << "Me here " << cnt++ << "   " << oldPos;
	    sn = createNode(oldPos);
	 }
	 QSet<Component*> unselected = n->unselectedComponents();
	 if(unselected.isEmpty())
	    continue;
	 foreach(Component *c, unselected)
	 {
	    foreach(ComponentPort *port, c->componentPorts())
	    {
	       if(port->node() == n)
	       {
		  n->removeComponent(c);
		  sn->addComponent(c);
		  port->m_node = sn;
	       }
	    }
	 }
	 static_cast<void*>(new Wire(this,n,sn));
      }
   }
}

Node* SchematicScene::nodeAt(qreal x, qreal y)
{
   return this->nodeAt(QPointF(x,y));
}

// Returns node at location given by centre
Node* SchematicScene::nodeAt(const QPointF& centre)
{
   if(!m_circuitNodes.isEmpty())
   {
      QSet<Node*> ns;
      foreach(Node *node, m_circuitNodes)
      {
	 Q_ASSERT(node != 0l);
	 QPointF mapped = node->mapFromScene(centre);
	 if(node->contains(mapped))
	    ns << node;
      }
      if(ns.size() == 1) //only one node found
	 return *(ns.begin());
      else if(ns.isEmpty()) //no node found
	 return 0l;
      // Many nodes found at same location. Hence remove all duplicate nodes
      // by adding all components to one node and deleting others
      Node *fn = *(ns.begin());
      ns.erase(ns.begin()); // remove first one from set and delete others
      const QSet<Component*>& fnc = fn->connectedComponents();
      foreach(Node *n, ns)
      {
	 foreach(Component *c, n->connectedComponents())
	 {
	    if(!fnc.contains(c))
	    {
	       fn->addComponent(c);
	       //find port containing dup node and modify its variable
	       foreach(ComponentPort *pt, c->componentPorts())
	       {
		  if(pt->node() == n)
		  {
		     pt->m_node = fn;
		     break;
		  }
	       }
	    }
	 } //n->connectedComponents()
      } //ns
      //foreach(Node *nd, ns)
      // removeNode(nd);
      qDeleteAll(ns); //now safe to delete
      qDebug("fn");
      return fn;
   }
   return 0l;
}

Node* SchematicScene::createNode(const QPointF& centre)
{
   Node *n = new Node(QString("FH"),(QGraphicsScene*)this);
   n->setPos(centre);
   m_circuitNodes.append(n);
   return n;
}

// Removes node from circuit nodes but doesn't delete
void SchematicScene::removeNode(Node *n)
{
   m_circuitNodes.removeAll(n);
}

// Returns a const reference to circuit nodes
const QList<Node*>& SchematicScene::circuitNodes() const
{
   return m_circuitNodes;
}

QUndoStack* SchematicScene::undoStack()
{
   return m_undoStack;
}

bool SchematicScene::areItemsMoving() const
{
   return m_areItemsMoving;
}

bool SchematicScene::isCommonMovingNode(Node *n) const
{
   return m_commonMovingNodes.contains(n);
}
