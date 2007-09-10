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
#include "qucs-tools/global.h"
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

#include "xmlutilities.h"

Wire::Wire(SchematicScene *scene,Node *n1,Node *n2) : QucsItem(0,scene),m_node1(n1),m_node2(n2)
{
   n1->addWire(this);
   n2->addWire(this);
   m_grabbedLineIndex = -1;
   m_proxyWiring = false;
   m_wasGrabbed = false;
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);

   if(scene) {
      scene->insertWire(this);
      rebuild();
   }
}

Wire::~Wire()
{
   clearProxyWires();
   m_node1->removeWire(this);
   m_node2->removeWire(this);
   if(schematicScene())
      schematicScene()->removeWire(this);
}

void Wire::rebuild()
{
   if(isVisible()) {
      clearProxyWires();
      prepareGeometryChange();
   }
   QPointF node1Pos = mapFromScene(m_node1->scenePos());
   QPointF node2Pos = mapFromScene(m_node2->scenePos());



   if(!m_lines.isEmpty()) {
      bool bothMoved = m_lines.first().p1() != node1Pos && m_lines.last().p2() != node2Pos;
      Q_ASSERT_X(!bothMoved, "Wire::rebuild()", "Against logical assumption that only one node is moved.");
      bool noneMoved = m_lines.first().p1() == node1Pos && m_lines.last().p2() == node2Pos;
      if(noneMoved)
         return; // Nothing to do.
   }


   // This is true when wire is created
   if(m_lines.isEmpty()) {
      QPointF referencePos = QPointF(node1Pos.x(), node2Pos.y());
      m_lines << WireLine(node1Pos, referencePos);
      m_lines << WireLine(referencePos, node2Pos);
   }
   else {
      bool node1Moved = m_lines.first().p1() != node1Pos;
      QPointF referencePos = (node1Moved ? node1Pos : node2Pos);

      if(m_lines.size() == 1) {
         if(node1Moved)
            m_lines.append(WireLine(m_lines.first().p2(), m_lines.first().p2()));
         else
            m_lines.prepend(WireLine(m_lines.last().p1(), m_lines.last().p1()));
      }
      QList<WireLine>::iterator it,it1;
      if(node1Moved) {
         it = m_lines.begin();
         it1 = it + 1;
      }
      else {
         it = m_lines.end()-1;
         it1 = it - 1;
      }

      bool notOblique = it->isNull() || it->isHorizontal() || it->isVertical();
      notOblique = notOblique && (it1->isNull() || it1->isHorizontal() || it1->isVertical());
      Q_ASSERT_X(notOblique, "Wire::rebuild()", "Oblique lines found!");
      if(it->isNull()) {
         if(it1->isNull() || it1->isHorizontal())
            it->setX(referencePos.x());
         else
            it->setY(referencePos.y());
      }
      else {
         if(it->isVertical())
            it->setX(referencePos.x());
         else
            it->setY(referencePos.y());
      }
      if(node1Moved) {
         it->setP1(referencePos);
         referencePos = it->p2();
      }
      else {
         it->setP2(referencePos);
         referencePos = it->p1();
      }

      if(it1->isNull()) {
         if(it->isNull() || it->isVertical())
            it1->setY(referencePos.y());
         else
            it1->setX(referencePos.x());
      }
      else {
         if(it1->isHorizontal())
            it1->setY(referencePos.y());
         else
            it1->setX(referencePos.x());
      }
      if(node1Moved)
         it1->setP1(referencePos);
      else
         it1->setP2(referencePos);
   }

   if(!isVisible())
      updateProxyWires();
}

QList<WireLine> Wire::linesBetween(const QPointF& p1, const QPointF& p2) const
{
   QList<WireLine> lines;
   if(p1.x() == p2.x() || p1.y() == p2.y())
      lines << WireLine(p1,p2);
   else {
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
   foreach(WireLine line, m_lines) {
      QRubberBand *proxy = new QRubberBand(QRubberBand::Line,view->viewport());
      proxy->setGeometry(proxyRect(line));
      proxy->show();
      m_proxyWires.append(proxy);
   }
}

QRect Wire::proxyRect(const WireLine& line) const
{
   QRectF rect;
   if(!line.isNull()) {
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
   if(m_proxyWires.size() > m_lines.size()) {
      int size = m_proxyWires.size() - m_lines.size() ;
      for(int i=0; i < size; i++)
         delete m_proxyWires.takeAt(0);
   }

   if(m_proxyWires.size() < m_lines.size()) {
      int size = m_lines.size() - m_proxyWires.size() ;
      for(int i=0; i < size; ++i)
         m_proxyWires.prepend(new QRubberBand(QRubberBand::Line,viewport));
   }
   Q_ASSERT(m_proxyWires.size() == m_lines.size());
   QList<QRubberBand*>::iterator proxyIt = m_proxyWires.begin();
   QList<WireLine>::iterator lineIt = m_lines.begin();
   for(; proxyIt != m_proxyWires.end(); ++proxyIt,++lineIt) {
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
   foreach(Wire *w, n->wires()) {
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

bool Wire::contains( const QPointF & point ) const
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

void Wire::paint(QPainter * p, const QStyleOptionGraphicsItem * o, QWidget * w)
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
   if(change == ItemPositionChange) {
      QPointF delta = value.toPointF() - pos();

      if(m_node1->components().isEmpty() && m_node1->wires().size() == 1)
         m_node1->moveBy(delta.x(), delta.y());
      if(m_node2->components().isEmpty() && m_node2->wires().size() == 1)
         m_node2->moveBy(delta.x(), delta.y());
   }
   return QGraphicsItem::itemChange(change,value);
}

void Wire::startMoveAndResize()
{
   m_wasGrabbed = true;
   m_grabbedLineIndex = m_lines.size()/2;
   hide();
   rebuild();
}

void Wire::moveAndResizeBy(qreal dx, qreal dy)
{
   if(m_grabbedLineIndex < 0)
      return;
   QPointF delta(dx,dy);
   int prevIndex = m_grabbedLineIndex - 1;
   int nextIndex = m_grabbedLineIndex + 1;
   QPointF node1Pos = mapFromScene(m_node1->scenePos());
   QPointF node2Pos = mapFromScene(m_node2->scenePos());

   WireLine& grabbedLine = m_lines[m_grabbedLineIndex];
   grabbedLine.translate(delta);

   if(prevIndex < 0) {
      QList<WireLine> begin = linesBetween(node1Pos,grabbedLine.p1());
      m_lines = begin + m_lines;
      m_grabbedLineIndex += begin.size();
   }
   else {
      while(prevIndex >= 0) {
         WireLine& prevLine = m_lines[prevIndex];
         WireLine& nextLine = m_lines[prevIndex + 1];
         if(prevLine.isVertical()) {
            prevLine.setX(nextLine.p1().x());
            prevLine.setP2(nextLine.p1());
         }
         else {
            Q_ASSERT(prevLine.isHorizontal());
            prevLine.setY(nextLine.p1().y());
            prevLine.setP2(nextLine.p1());
         }
         --prevIndex;
      }
      QPointF startLineP1 = m_lines.at(0).p1();
      if(node1Pos != startLineP1) {
         QList<WireLine> begin = linesBetween(node1Pos,startLineP1);
         m_lines = begin + m_lines;
         m_grabbedLineIndex += begin.size();
      }
   }
   deleteNullLines();

//--------------------------------------------------------------------------------------
   nextIndex = m_grabbedLineIndex + 1;
   if(nextIndex == m_lines.size()) {
      QList<WireLine> end = linesBetween(grabbedLine.p2(),node2Pos);
      m_lines += end;
   }
   else {
      while(nextIndex < m_lines.size()) {
         WireLine& nextLine = m_lines[nextIndex];
         WireLine& prevLine = m_lines[nextIndex-1];
         if(nextLine.isVertical()) {
            nextLine.setX(prevLine.p2().x());
            nextLine.setP1(prevLine.p2());
         }
         else {
            Q_ASSERT(nextLine.isHorizontal());
            nextLine.setY(prevLine.y2());
            nextLine.setP1(prevLine.p2());
         }
         ++nextIndex;
      }

      QPointF endLineP2 = m_lines.last().p2();
      if(node2Pos != endLineP2) {
         QList<WireLine> end = linesBetween(endLineP2,node2Pos);
         m_lines += end;
      }
   }

   deleteNullLines();
   updateProxyWires();
}

void Wire::stopMoveAndResize()
{
   m_wasGrabbed = false;
   m_grabbedLineIndex = -1;
   clearProxyWires();
   show();
   prepareGeometryChange();
}

void Wire::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
   m_grabbedLineIndex = -1;
   QGraphicsItem::mousePressEvent(event);

   if(!isSelected())
      return;

   m_grabbedLineIndex = indexForPos(event->scenePos());
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
   for( ; it != end; ++it,++i) {
      if(QLineF(*it).isNull()) {
         if(i < m_grabbedLineIndex)
            --m_grabbedLineIndex;
         m_lines.erase(it);
      }
   }
}

void Wire::delNullLines()
{
   prepareGeometryChange();
   qDebug() << "Wire::delNullLines() enter";
   QList<WireLine>::iterator it = m_lines.begin(),it1;
   int cnt = 0;
   while(it != m_lines.end()) {
      if(it->isNull()) {
         it = m_lines.erase(it);
         cnt++;
      }
      else {
         ++it;
      }
   }
   qDebug() << "Deleted" << cnt << "wirelines";
   cnt = 0;
   if(m_lines.size() <= 1)
      return;
   it = m_lines.begin() + 1;
   while(it != m_lines.end()) {
      it1 = it - 1;
      if(it->isHorizontal() && it1->isHorizontal()) {
         Q_ASSERT(it->y() == it1->y());
         Q_ASSERT(it1->p2() == it->p1());
         it1->setP2(it->p2());
         it = m_lines.erase(it);
         cnt++;
      }
      else if(it->isVertical() && it1->isVertical()) {
         Q_ASSERT(it->x() == it1->x());
         Q_ASSERT(it1->p2() == it->p1());
         it1->setP2(it->p2());
         it = m_lines.erase(it);
         cnt++;
      }
      else {
         ++it;
      }
   }
   qDebug() << "Singlified" << cnt << "wirelines";
}

void Wire::grabMoveEvent( QGraphicsSceneMouseEvent * event )
{
   if(m_grabbedLineIndex < 0)
      return;
   QPointF delta = event->scenePos() - event->lastScenePos();
   int prevIndex = m_grabbedLineIndex - 1;
   int nextIndex = m_grabbedLineIndex + 1;
   QPointF node1Pos = mapFromScene(m_node1->scenePos());
   QPointF node2Pos = mapFromScene(m_node2->scenePos());

   WireLine& grabbedLine = m_lines[m_grabbedLineIndex];
   grabbedLine.translate(delta);

   if(prevIndex < 0) {
      QList<WireLine> begin = linesBetween(node1Pos,grabbedLine.p1());
      m_lines = begin + m_lines;
      m_grabbedLineIndex += begin.size();
   }
   else {
      while(prevIndex >= 0)
      {
         WireLine& prevLine = m_lines[prevIndex];
         WireLine& nextLine = m_lines[prevIndex + 1];
         if(prevLine.isVertical()) {
            prevLine.setX(nextLine.p1().x());
            prevLine.setP2(nextLine.p1());
         }
         else {
            Q_ASSERT(prevLine.isHorizontal());
            prevLine.setY(nextLine.p1().y());
            prevLine.setP2(nextLine.p1());
         }
         --prevIndex;
      }
      QPointF startLineP1 = m_lines.at(0).p1();
      if(node1Pos != startLineP1) {
         QList<WireLine> begin = linesBetween(node1Pos,startLineP1);
         m_lines = begin + m_lines;
         m_grabbedLineIndex += begin.size();
      }
   }
   deleteNullLines();

//--------------------------------------------------------------------------------------
   nextIndex = m_grabbedLineIndex + 1;
   if(nextIndex == m_lines.size()) {
      QList<WireLine> end = linesBetween(grabbedLine.p2(),node2Pos);
      m_lines += end;
   }
   else {
      while(nextIndex < m_lines.size()) {
         WireLine& nextLine = m_lines[nextIndex];
         WireLine& prevLine = m_lines[nextIndex-1];
         if(nextLine.isVertical()) {
            nextLine.setX(prevLine.p2().x());
            nextLine.setP1(prevLine.p2());
         }
         else {
            Q_ASSERT(nextLine.isHorizontal());
            nextLine.setY(prevLine.y2());
            nextLine.setP1(prevLine.p2());
         }
         ++nextIndex;
      }

      QPointF endLineP2 = m_lines.last().p2();
      if(node2Pos != endLineP2) {
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

   for( ; it != end; ++it , ++retVal) {
      if(rectForLine(*it).contains(pos))
         return retVal;
   }

   return -1;
}

QString Wire::saveString() const
{
   using Qucs::realToString;
   QString s  = "<" + realToString(m_node1->pos().x()) + " " + realToString(m_node1->pos().y());
   s += " "+realToString(m_node2->pos().x()) + " " + realToString(m_node2->pos().y());
   //TODO: Wire label
//    if(0 && Label) {
//       s += " \""+Label->Name+"\" ";
//       s += QString::number(Label->x1)+" "+QString::number(Label->y1)+" ";
//       s += QString::number(Label->cx-x1 + Label->cy-y1);
//       s += " \""+Label->initValue+"\">";
//    }
//    else
   s += " \"\" 0 0 0 \"\">";
   return s;
}

void Wire::setWireLines(const QList<WireLine>& lines)
{
   if(isVisible())
      prepareGeometryChange();
   m_lines = lines;

   if(!isVisible())
      updateProxyWires();
}

void Wire::writeXml(Qucs::XmlWriter *writer)
{
   Q_ASSERT(!m_lines.isEmpty());
   writer->writeStartElement("wire");

   writer->writeStartElement("node1pos");
   writer->writePoint(m_lines.first().p1());
   writer->writeEndElement(); //</node1pos>

   writer->writeStartElement("node2pos");
   writer->writePoint(m_lines.last().p2());
   writer->writeEndElement(); //</node2pos>

   writer->writeStartElement("wirelines");
   foreach(WireLine wireline, m_lines) {
      writer->writeStartElement("wireline");

      writer->writeStartElement("p1");
      writer->writePoint(wireline.p1());
      writer->writeEndElement(); //</p1>

      writer->writeStartElement("p2");
      writer->writePoint(wireline.p2());
      writer->writeEndElement(); //</p2>

      writer->writeEndElement(); //</wireline>
   }
   writer->writeEndElement(); //</wirelines>
   writer->writeEndElement(); //</wire>
   //TODO: Wirelabel
}

void Wire::readXml(Qucs::XmlReader *reader)
{
   //NOTE: This assumes initial part is parsed already
   if(!reader->isStartElement() || reader->name() != "wirelines") {
      reader->raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(reader->name().toString()).arg("wire"));
   }
   QList<WireLine> lines;
   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         Q_ASSERT(reader->name() == "wirelines");
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "wireline") {
            QPointF p1;
            QPointF p2;

            while(!reader->atEnd()) {
               reader->readNext();

               if(reader->isEndElement()) {
                  lines << WireLine(p1, p2);
                  Q_ASSERT(reader->name() == "wireline");
                  break;
               }
               if(reader->name() == "p1") {
                  reader->readFurther();
                  p1 = reader->readPoint();
                  reader->readFurther();
                  Q_ASSERT(reader->isEndElement() && reader->name() == "p1");
               }

               else if(reader->name() == "p2") {
                  reader->readFurther();
                  p2 = reader->readPoint();
                  reader->readFurther();
                  Q_ASSERT(reader->isEndElement() && reader->name() == "p2");
               }
            }
         }
      }
   }
   if(!reader->hasError()) {
      setWireLines(lines);
   }
}
