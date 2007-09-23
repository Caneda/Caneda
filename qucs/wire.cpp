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

   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);

   if(scene) {
      scene->insertWire(this);
      rebuild();
      deleteNullLines();
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

void Wire::replaceNode(Node *oldNode, Node *newNode)
{
   if(oldNode == m_node1)
      m_node1 = newNode;
   else if(oldNode == m_node2)
      m_node2 = newNode;
   else
      qFatal("Wire::replaceNode() : Some bug in node variable");
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
   for(; it != end; ++it) {
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

Wire* Wire::connectedWire(const Node* n1, const Node* n2)
{
   Node *n = const_cast<Node*>(n1);
   foreach(Wire *w, n->wires()) {
      if((w->node1() == n1 && w->node2() == n2) || (w->node2() == n1 && w->node1() == n2))
         return w;
   }
   return 0;
}

void Wire::startMoveAndResize()
{
   if(m_grabbedLineIndex == -1)
      m_grabbedLineIndex = m_lines.size()/2;
   int num_of_wires_in_beginning = -(m_grabbedLineIndex-2);
   int num_of_wires_in_end = -(m_lines.size()-1-m_grabbedLineIndex-2);
   QPointF p1 = m_lines.first().p1();
   QPointF p2 = m_lines.last().p2();
   for(int i=0; i < num_of_wires_in_beginning; ++i)
      m_lines.prepend(WireLine(p1, p1));
   for(int i=0; i < num_of_wires_in_end; ++i)
      m_lines.append(WireLine(p2, p2));

   if(num_of_wires_in_beginning > 0)
      m_grabbedLineIndex += num_of_wires_in_beginning;
   hide();
}

void Wire::moveAndResizeBy(qreal dx, qreal dy)
{
   Q_ASSERT(m_grabbedLineIndex >= 2 && (m_lines.size()-1-m_grabbedLineIndex) >= 2);

   QPointF node1Pos = mapFromScene(m_node1->scenePos());
   QPointF node2Pos = mapFromScene(m_node2->scenePos());
   QPointF refPos1 = m_grabbedLineIndex == 2 ? node1Pos : m_lines[m_grabbedLineIndex-3].p2();
   QPointF refPos2 = m_lines.size()-1-m_grabbedLineIndex == 2 ? node2Pos : m_lines[m_grabbedLineIndex+3].p1();
   QPointF inter1, inter2;

   WireLine& grabbedLine = m_lines[m_grabbedLineIndex];
   grabbedLine.translate(QPointF(dx, dy));

   if(grabbedLine.isHorizontal()) {
      inter1 = QPointF(grabbedLine.p1().x(), refPos1.y());
      inter2 = QPointF(grabbedLine.p2().x(), refPos2.y());
   }
   else {
      Q_ASSERT(grabbedLine.isVertical());
      inter1 = QPointF(refPos1.x(), grabbedLine.p1().y());
      inter2 = QPointF(refPos2.x(), grabbedLine.p2().y());
   }
   m_lines[m_grabbedLineIndex-1].setP2(grabbedLine.p1());
   m_lines[m_grabbedLineIndex-1].setP1(inter1);
   m_lines[m_grabbedLineIndex-2] = WireLine(refPos1, inter1);

   m_lines[m_grabbedLineIndex+1].setP1(grabbedLine.p2());
   m_lines[m_grabbedLineIndex+1].setP2(inter2);
   m_lines[m_grabbedLineIndex+2] = WireLine(inter2, refPos2);
   updateProxyWires();
}

void Wire::stopMoveAndResize()
{
   m_grabbedLineIndex = -1;
   deleteNullLines();
   clearProxyWires();
   show();
   prepareGeometryChange();
}

void Wire::setWireLines(const QList<WireLine>& lines)
{
   if(isVisible())
      prepareGeometryChange();
   m_lines = lines;

   if(!isVisible())
      updateProxyWires();
}

void Wire::deleteNullLines()
{
   prepareGeometryChange();
   QList<WireLine>::iterator it = m_lines.begin(), it1;
   while(it != m_lines.end()) {
      if(it->isNull()) {
         it = m_lines.erase(it);
      }
      else {
         ++it;
      }
   }

   if(m_lines.size() <= 1)
      return;
   it = m_lines.begin() + 1;
   while(it != m_lines.end()) {
      it1 = it - 1;
      if(it->isHorizontal() && it1->isHorizontal()) {
         Q_ASSERT(it1->p2() == it->p1());
         it1->setP2(it->p2());
         it = m_lines.erase(it);
      }
      else if(it->isVertical() && it1->isVertical()) {
         Q_ASSERT(it1->p2() == it->p1());
         it1->setP2(it->p2());
         it = m_lines.erase(it);
      }
      else {
         ++it;
      }
   }
}

QString Wire::saveString() const
{
   using Qucs::realToString;
   QString s  = "<" + realToString(m_node1->pos().x()) + " " + realToString(m_node1->pos().y());
   s += " "+realToString(m_node2->pos().x()) + " " + realToString(m_node2->pos().y());
   //TODO: Wire label
   s += " \"\" 0 0 0 \"\">";
   return s;
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

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value)
{
   if(change == ItemPositionChange) {
      QPointF delta = value.toPointF() - pos();

      if(m_node1->components().isEmpty() && m_node1->wires().size() == 1)
         m_node1->moveBy(delta.x(), delta.y());
      if(m_node2->components().isEmpty() && m_node2->wires().size() == 1)
         m_node2->moveBy(delta.x(), delta.y());
   }
   return QGraphicsSvgItem::itemChange(change,value);
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
   m_grabbedLineIndex = -1;
   QGraphicsSvgItem::mousePressEvent(event);

   m_grabbedLineIndex = indexForPos(mapFromScene(event->scenePos()));
}

void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
      event->ignore();

   schematicScene()->setGrabbedWire(this);
   startMoveAndResize();
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   QucsItem::mouseReleaseEvent(event);
}


void Wire::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
   Q_UNUSED(mouseEvent);
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

QRectF Wire::rectForLine(const WireLine& line) const
{
   qreal x = qMin(line.p1().x() , line.p2().x()) - 3.0;
   qreal y = qMin(line.p1().y() , line.p2().y()) - 3.0;
   qreal w = qAbs(line.p1().x() - line.p2().x()) + 6.0;
   qreal h = qAbs(line.p1().y() - line.p2().y()) + 6.0;
   return QRectF(x,y,w,h);
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
   QList<WireLine>::const_iterator lineIt = m_lines.constBegin();
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
