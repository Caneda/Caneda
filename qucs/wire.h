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

#include "item.h"
#include "wireline.h"
#include "node.h"

#include <QtCore/QList>

class SchematicScene;
class QGraphicsLineItem;
class QRubberBand;

class Wire : public QucsItem
{
   public:
      enum {
         Type = QucsItem::WireType
      };

      Wire(SchematicScene *scene, Node *n1,Node *n2);
      ~Wire();
      void rebuild();

      Node* node1() const { return m_node1; }
      Node* node2() const { return m_node2; }
      void setNode1(Node *n1) { m_node1 = n1; }
      void setNode2(Node *n2) { m_node2 = n2; }
      void replaceNode(Node *oldNode,Node *newNode);

      QRectF boundingRect() const;
      int type() const { return Wire::Type; }
      void paint(QPainter * p, const QStyleOptionGraphicsItem * o, QWidget * w = 0 );
      QPainterPath shape() const;
      bool contains ( const QPointF & point ) const;

      static Wire* connectedWire(const Node *n1,const Node *n2);

      void grabMoveEvent( QGraphicsSceneMouseEvent * event );
      void grabReleaseEvent ( QGraphicsSceneMouseEvent * event );
      void startMoveAndResize();
      void moveAndResizeBy(qreal dx, qreal dy);
      void stopMoveAndResize();

      void setWireLines(const QList<WireLine>& lines);
      QList<WireLine> wireLines() const { return m_lines; }

      QString saveString() const;

      // Reimplemented virtuals to not to react
      void rotate() {}
      void mirrorX() {}
      void mirrorY() {}

      void writeXml(Qucs::XmlWriter *writer);
      void readXml(Qucs::XmlReader *reader);

   protected:
      QVariant itemChange(GraphicsItemChange change, const QVariant &value);
      void mousePressEvent ( QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
      void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

   private:
      void createProxyWires();
      QList<WireLine> linesBetween(const QPointF& p1, const QPointF& p2) const;
      QRect proxyRect(const WireLine& line) const;
      void updateProxyWires();
      void clearProxyWires();
      QRectF rectForLine(const WireLine& line) const;
      int indexForPos(const QPointF& pos) const;
      void deleteNullLines();
      void delNullLines();

      friend class SchematicScene;

      bool m_proxyWiring;
      QList<WireLine> m_lines;
      QList<QRubberBand*> m_proxyWires;
      Node *m_node1;
      Node *m_node2;
      int m_grabbedLineIndex;
      bool m_wasGrabbed;
};

#endif //__WIRE_H
