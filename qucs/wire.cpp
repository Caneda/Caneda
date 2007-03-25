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

#include "wire.h"
#include "node.h"
#include "schematicscene.h"

#include <QtCore/QList>
#include <QtCore/QtAlgorithms>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsView>

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneEvent>
#include <QtCore/QDebug>
#include <QtGui/QRubberBand>
#include <QtCore/QVariant>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionGraphicsItem>


Wire::Wire(SchematicScene *scene,Node *n1,Node *n2) : QucsItem(0,scene),m_node1(n1),m_node2(n2)
{
   n1->addWire(this);
   n2->addWire(this);
   m_grabbedLineIndex = -1;
   m_proxyWiring = false;
   m_wasGrabbed = false;
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   rebuild();
}

Wire::~Wire()
{
   clearProxyWires();
   m_node1->removeWire(this);
   m_node2->removeWire(this);
}

void Wire::rebuild()
{
   if(isVisible())
   {
      clearProxyWires();
      prepareGeometryChange();
   }
   QPointF node1Pos = mapFromScene(m_node1->scenePos());
   QPointF node2Pos = mapFromScene(m_node2->scenePos());

   if(m_lines.size() < 3)
      m_lines = linesBetween(node1Pos,node2Pos);
   else if(node1Pos != m_lines.first().p1())
   {
      bool cont = false;
      do
      {
         int firstIndex = 0;
         int secondIndex = firstIndex + 1;
         cont = false;
         if(m_lines[firstIndex].isHorizontal())
            m_lines[firstIndex].setY(node1Pos.y());
         else
         {
            Q_ASSERT(m_lines[firstIndex].isVertical());
            m_lines[firstIndex].setX(node1Pos.x());
         }
         m_lines[firstIndex].setP1(node1Pos);
         m_lines[secondIndex].setP1(m_lines[firstIndex].p2());
         if(m_lines[firstIndex].isNull())
         {
            m_lines.removeFirst();
            --secondIndex;
         }
         
         if(m_lines[secondIndex].isNull())
         {
            m_lines.removeAt(secondIndex);
            if(secondIndex != firstIndex)
            {
               m_lines.removeFirst();
               cont = true;
            }
         }
         if(m_lines.isEmpty())
         {
            m_lines.append(WireLine(node1Pos,node2Pos));
            qDebug() << "Check this out!!";
         }
      }while(cont);
   }
   else if(node2Pos != m_lines.last().p2())
   {
      bool cont = false;
      do
      {
         int lastIndex = m_lines.size() - 1;
         int lastButIndex = lastIndex - 1;
         cont = false;
         if(m_lines[lastIndex].isHorizontal())
            m_lines[lastIndex].setY(node2Pos.y());
         else
         {
            Q_ASSERT(m_lines[lastIndex].isVertical());
            m_lines[lastIndex].setX(node2Pos.x());
         }
         m_lines[lastIndex].setP2(node2Pos);
         m_lines[lastButIndex].setP2(m_lines[lastIndex].p1());
         if(m_lines[lastIndex].isNull())
         {
            m_lines.removeLast();
            --lastIndex;
         }
         
         if(m_lines[lastButIndex].isNull())
         {
            m_lines.removeAt(lastButIndex);
            if(lastButIndex != lastIndex)
            {
               m_lines.removeLast();
               cont = true;
            }
         }
      }while(cont);
   }
   if(!isVisible())
      updateProxyWires();
}


void Wire::createLines(const QPointF& p1, const QPointF& p2)
{
   m_lines.clear();
   m_lines = linesBetween(p1,p2);
}

QList<WireLine> Wire::linesBetween(const QPointF& p1, const QPointF& p2) const
{
   QList<WireLine> lines;
   if(p1.x() == p2.x() || p1.y() == p2.y())
      lines << WireLine(p1,p2);
   else
   {
      QPointF inter = QPointF(p1.x(),p2.y());
      lines << WireLine(p1,inter);
      lines << WireLine(inter,p2);
   }
   return lines;
}

void Wire::createProxyWires()
{
   clearProxyWires();
   QGraphicsView *view = activeView();
   if(!view)
      return;
   foreach(WireLine line, m_lines)
   {
      QRubberBand *proxy = new QRubberBand(QRubberBand::Line,view->viewport());
      proxy->setGeometry(proxyRect(line));
      proxy->show();
      m_proxyWires.append(proxy);
   }
}

QRect Wire::proxyRect(const WireLine& line) const
{
   QRectF rect; 
   if(!line.isNull())
   {
      WireLine sceneLine(mapToScene(line.p1()),mapToScene(line.p2()));
      QGraphicsView *view = activeView();
      if(!view)
         return rect.toRect();
      rect.setTopLeft(view->mapFromScene(sceneLine.p1()));
      rect.setBottomRight(view->mapFromScene(sceneLine.p2()));
      //normalize dimension of rect
      rect = rect.normalized();
      if(rect.height() < 1)
         rect.setHeight(1);
      if(rect.width() < 1)
         rect.setWidth(1);
   }
   return rect.toRect();
}

void Wire::updateProxyWires()
{
   QGraphicsView *view = activeView();
   Q_ASSERT(view);
   QWidget *viewport = view->viewport();
   if(m_proxyWires.size() > m_lines.size())
   {
      int size = m_proxyWires.size() - m_lines.size() ;
      for(int i=0; i < size; i++)
         delete m_proxyWires.takeAt(0);
   }
   
   if(m_proxyWires.size() < m_lines.size())
   {
      int size = m_lines.size() - m_proxyWires.size() ;
      for(int i=0; i < size; ++i)
         m_proxyWires.prepend(new QRubberBand(QRubberBand::Line,viewport));
   }
   
   QList<QRubberBand*>::iterator proxyIt = m_proxyWires.begin();
   QList<WireLine>::iterator lineIt = m_lines.begin();
   for(; proxyIt != m_proxyWires.end(); ++proxyIt,++lineIt)
   {
      (*proxyIt)->setParent(viewport);
      const QRect& geometry = proxyRect(*lineIt);
      (*proxyIt)->setGeometry(geometry);
      (*proxyIt)->show();
   }
}

void Wire::clearProxyWires()
{
   qDeleteAll(m_proxyWires);
   m_proxyWires.clear();
}

void Wire::replaceNode(Node *oldNode,Node *newNode)
{
   if(oldNode == m_node1)
      m_node1 = newNode;
   else if(oldNode == m_node2)
      m_node2 = newNode;
   else
      qFatal("Wire::replaceNode() : Some bug in node variable");
}

Wire* Wire::connectedWire(const Node* n1, const Node* n2)
{
   Node *n = const_cast<Node*>(n1);
   foreach(Wire *w, n->wires())
   {
      if((w->node1() == n1 && w->node2() == n2) || (w->node2() == n1 && w->node1() == n2))
         return w;
   }
   return 0;
}

QRectF Wire::boundingRect() const
{
   QRectF rect;
   
   QList<WireLine>::const_iterator it = m_lines.constBegin();
   QList<WireLine>::const_iterator end = m_lines.constEnd();
   for(; it != end; ++it)
      rect |= rectForLine(*it);
   
   qreal adjust = 1.0;
   return rect.adjusted(-adjust,-adjust,+adjust,+adjust);
}

QPainterPath Wire::shape() const
{
   QPainterPath path;
   if(m_lines.isEmpty())
      return path;
   QList<WireLine>::const_iterator it = m_lines.constBegin();
   QList<WireLine>::const_iterator end = m_lines.constEnd();
   for(; it != end; ++it)
      path.addRect(rectForLine(*it));
   return path;
}

bool Wire::contains ( const QPointF & point ) const
{
   QList<WireLine>::const_iterator it = m_lines.constBegin();
   QList<WireLine>::const_iterator end = m_lines.constEnd();
   for(; it != end; ++it)
   {
      if(rectForLine(*it).contains(point))
         return true;
   }
   return false;
}

void Wire::paint (QPainter * p, const QStyleOptionGraphicsItem * o, QWidget * w)
{
   Q_UNUSED(w);
   if(o->state & QStyle::State_Selected)
      p->setPen(Qt::red);
   else
      p->setPen(Qt::blue);
   QList<WireLine>::const_iterator it = m_lines.constBegin();
   QList<WireLine>::const_iterator end = m_lines.constEnd();
   for(; it != end; ++it)
      p->drawLine(*it);
}

QRectF Wire::rectForLine(const WireLine& line) const
{
   qreal x = qMin(line.p1().x() , line.p2().x()) - 3.0;
   qreal y = qMin(line.p1().y() , line.p2().y()) - 3.0;
   qreal w = qAbs(line.p1().x() - line.p2().x()) + 3.0;
   qreal h = qAbs(line.p1().y() - line.p2().y()) + 3.0;
   return QRectF(x,y,w,h);
}

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value)
{
   if(change == ItemPositionChange && isSelected() && schematicScene()->selectedItems().size() == 1)
      return QVariant(pos());
   return QGraphicsItem::itemChange(change,value);
}

void Wire::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
   m_grabbedLineIndex = -1;
   QGraphicsItem::mousePressEvent(event);
   
   if(!isSelected())
      return;
   
   m_grabbedLineIndex = indexForPos(event->pos());
   Q_ASSERT(m_grabbedLineIndex != -1);
   m_wasGrabbed = true;
}

void Wire::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
   if(!m_wasGrabbed)
      event->ignore();
   
   schematicScene()->setGrabbedWire(this);
   hide();
}

void Wire::deleteNullLines()
{
   int i = 0;
   QList<WireLine>::iterator it(m_lines.begin());
   QList<WireLine>::iterator end(m_lines.end());
   for( ; it != end; ++it,++i)
   {
      if(QLineF(*it).isNull())
      {
         if(i < m_grabbedLineIndex)
            --m_grabbedLineIndex;
         m_lines.erase(it);
      }
   }
}

void Wire::grabMoveEvent( QGraphicsSceneMouseEvent * event )
{
   if(m_grabbedLineIndex < 0)
      return;
   QPointF delta = event->pos() - event->lastPos();
   int prevIndex = m_grabbedLineIndex - 1;
   int nextIndex = m_grabbedLineIndex + 1;
   QPointF node1Pos = mapFromScene(m_node1->scenePos());
   QPointF node2Pos = mapFromScene(m_node2->scenePos());
   
   WireLine& grabbedLine = m_lines[m_grabbedLineIndex];
   grabbedLine.translate(delta);
   
   if(prevIndex < 0)
   {
      QList<WireLine> begin = linesBetween(node1Pos,grabbedLine.p1());
      m_lines = begin + m_lines;
      m_grabbedLineIndex += begin.size();
   }
   else
   {
      while(prevIndex >= 0)
      {
         WireLine& prevLine = m_lines[prevIndex];
         WireLine& nextLine = m_lines[prevIndex + 1];
         if(prevLine.isVertical())
         {
            prevLine.setX(nextLine.p1().x());
            prevLine.setP2(nextLine.p1());
         }
         else
         {
            Q_ASSERT(prevLine.isHorizontal());
            prevLine.setY(nextLine.p1().y());
            prevLine.setP2(nextLine.p1());
         }
         --prevIndex;
      }
      QPointF startLineP1 = m_lines.at(0).p1();
      if(node1Pos != startLineP1)
      {
         QList<WireLine> begin = linesBetween(node1Pos,startLineP1);
         m_lines = begin + m_lines;
         m_grabbedLineIndex += begin.size();
      }
   }
   deleteNullLines();
   
//--------------------------------------------------------------------------------------
   nextIndex = m_grabbedLineIndex + 1;
   if(nextIndex == m_lines.size())
   {
      QList<WireLine> end = linesBetween(grabbedLine.p2(),node2Pos);
      m_lines += end;
   }
   else
   {
      while(nextIndex < m_lines.size())
      {
         WireLine& nextLine = m_lines[nextIndex];
         WireLine& prevLine = m_lines[nextIndex-1];
         if(nextLine.isVertical())
         {
            nextLine.setX(prevLine.p2().x());
            nextLine.setP1(prevLine.p2());
         }
         else
         {
            Q_ASSERT(nextLine.isHorizontal());
            nextLine.setY(prevLine.y2());
            nextLine.setP1(prevLine.p2());
         }
         ++nextIndex;
      }
      
      QPointF endLineP2 = m_lines.last().p2();
      if(node2Pos != endLineP2)
      {
         QList<WireLine> end = linesBetween(endLineP2,node2Pos);
         m_lines += end;
      }
   }

   deleteNullLines();
   updateProxyWires();
}

void Wire::grabReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
   Q_UNUSED(event);
   m_wasGrabbed = false;
   m_grabbedLineIndex = -1;
   clearProxyWires();
   show();
   prepareGeometryChange();
}

void Wire::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
   Q_UNUSED(mouseEvent);
   m_wasGrabbed = true;
}

int Wire::indexForPos(const QPointF& pos) const
{
   int retVal = 0;
   QList<WireLine>::const_iterator it = m_lines.begin();
   const QList<WireLine>::const_iterator end = m_lines.end();

   for( ; it != end; ++it , ++retVal)
   {
      if(rectForLine(*it).contains(pos))
         return retVal;
   }

   return -1;
}
