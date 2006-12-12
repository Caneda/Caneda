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
#include <QtCore/QList>
#include <QtCore/QtAlgorithms>
#include <QtGui/QPainterPath>
#include <QtCore/QLineF>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtCore/QDebug>

Wire::Wire(QGraphicsScene *scene,Node *n1, Node *n2) : Component(0l,scene)
{
   m_ports.append(new ComponentPort(this,mapFromScene(n1->scenePos()),n1));
   m_ports.append(new ComponentPort(this,mapFromScene(n2->scenePos()),n2));
      
   rebuild();
   setFlags(ItemIsMovable | ItemIsSelectable);
}

Wire::~Wire()
{
   qDeleteAll(m_lines);
   m_lines.clear();
}

QString Wire::name() const
{
   return "W";
}

QString Wire::model() const
{
   return "W";
}

QString Wire::text() const
{
   return "W";
}

QString Wire::netlist() const
{
   return "W";
}

QRectF Wire::rectForLine(const QLineF& line) const
{
   qreal x = qMin(line.p1().x() , line.p2().x());
   qreal y = qMin(line.p1().y() , line.p2().y());
   qreal w = qAbs(line.p1().x() - line.p2().x());
   if(w  < 1.0)
      w = 1.0;
   qreal h = qAbs(line.p1().y() - line.p2().y());
   if(h < 1.0)
      h = 1.0;
   return QRectF(x,y,w,h);
}

QRectF Wire::boundingRect() const
{
   QRectF rect(0.0,0.0,0.0,0.0);
   foreach(QLineF* line, m_lines)
      rect |= rectForLine(*line);
   return rect.adjusted(-1.0,-1.0,1.0,1.0);
}

QPainterPath Wire::shape() const
{
   QPainterPath path;
   if(m_lines.isEmpty())
      return path;
   foreach(QLineF *line, m_lines)
      path.addRect(rectForLine(*line));
   return path;
}

void Wire::paint(QPainter *p, const QStyleOptionGraphicsItem * o, QWidget * w )
{
   Q_UNUSED(w);
   if(o->state & QStyle::State_Selected)
      p->setPen(Qt::red);
   else
      p->setPen(Qt::darkBlue);
   
   foreach(QLineF *line, m_lines)
      p->drawLine(*line);
}

bool Wire::contains(const QPointF& pt) const
{
   foreach(QLineF* line, m_lines)
   {
      if(rectForLine(*line).contains(pt))
	 return true;
   }
   return false;
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent * event )
{
   QGraphicsItem::mousePressEvent(event);
}

void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent * event )
{
   QGraphicsItem::mouseMoveEvent(event);
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent * event )
{
   QGraphicsItem::mouseReleaseEvent(event);
}

void Wire::rebuild()
{
   if(!m_lines.isEmpty())
   {
      qDeleteAll(m_lines);
      m_lines.clear();
   }
   
   Node *m_node1 = m_ports[0]->node();
   Node *m_node2 = m_ports[1]->node();
   Q_ASSERT(m_node1 != 0l && m_node2 != 0l);

   QPointF st = mapFromScene(m_node1->scenePos());
   QPointF en = mapFromScene(m_node2->scenePos());

   if(st.x() == en.x() || st.y() == en.y())
   {
      m_lines.append(new QLineF(st,en));
      prepareGeometryChange();
      return;
   }

   QPointF inter = QPointF(st.x(),en.y());
   m_lines.append(new QLineF(st,inter));
   m_lines.append(new QLineF(inter,en));
   prepareGeometryChange();
}

void Wire::setPathLines(QList<QLineF *> lines)
{
   qDeleteAll(m_lines);
   m_lines.clear();
   m_lines = lines;
   update();
}

int Wire::type() const
{
   return QucsItem::WireType;
}

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value)
{
   Q_ASSERT(scene());
   if(change == ItemPositionChange)
   {
      QPointF pos = value.toPointF();
      QPointF oldPos = scenePos();
      if(scene()->selectedItems().size() == 1)
	 return QVariant(oldPos);
      oldPos = value.toPointF();
      return QVariant(oldPos);
   }
   //rebuild();
   return QGraphicsItem::itemChange(change,value);
}

void Wire::rebuild(const QPointF& s, const QPointF& e)
{
   if(!m_lines.isEmpty())
   {
      qDeleteAll(m_lines);
      m_lines.clear();
   }
   
   
   QPointF st = mapFromScene(s);
   QPointF en = mapFromScene(e);

   if(st.x() == en.x() || st.y() == en.y())
   {
      m_lines.append(new QLineF(st,en));
      prepareGeometryChange();
      return;
   }

   QPointF inter = QPointF(st.x(),en.y());
   m_lines.append(new QLineF(st,inter));
   m_lines.append(new QLineF(inter,en));
   prepareGeometryChange();
}
