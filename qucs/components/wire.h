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

#ifndef __WIRE_H
#define __WIRE_H

#include "component.h"

class Node;
class QLineF;

class Wire : public Component
{
   public:
      Wire(QGraphicsScene *scene,Node *n1,Node *n2);
      ~Wire();
      
      QString name() const;
      QString model() const;
      QString text() const;
      QString netlist() const;

      QRectF boundingRect() const;
      QPainterPath shape() const;

      void paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

      Node* node1() const;
      Node* node2() const;

      bool contains(const QPointF& point) const;
      void setPathLines(QList<QLineF*> lines);

      void rebuild();

      int type() const;
      
   protected:
      void mousePressEvent(QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent(QGraphicsSceneMouseEvent * event );
      void mouseReleaseEvent(QGraphicsSceneMouseEvent * event );
      QVariant itemChange(GraphicsItemChange change, const QVariant &value);
      
   private:

      QRectF rectForLine(const QLineF& line) const;
      QList<QLineF*> m_lines;
};

#endif //__WIRE_H
