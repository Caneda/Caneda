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

//!\brief forward declaration.
class SchematicScene;
class QGraphicsLineItem;
class QRubberBand;

/*!\brief Wire class

  This class implement wiring system
*/
class Wire : public QucsItem
{
   public:
      /*!\brief GraphicsView framework requirement for casting.

        \sa qucsitem_cast */
      enum {
         Type = QucsItem::WireType
      };

      Wire(SchematicScene *scene, Node *n1,Node *n2);
      ~Wire();

      void rebuild();

      /*!\brief Return node 1
	\todo Why not access directly
      */
      Node* node1() const { return m_node1; }
      /*!\brief Set node 1
	\todo Why not access directly
      */
      void setNode1(Node *n1) { m_node1 = n1; }

      /*!\brief Return node 2
	\todo Why not access directly
      */
      Node* node2() const { return m_node2; }
      /*!\brief Set node 2
	\todo Why not access directly
      */
      void setNode2(Node *n2) { m_node2 = n2; }

      void replaceNode(Node *oldNode,Node *newNode);

      QRectF boundingRect() const;
      QPainterPath shape() const;
      bool contains ( const QPointF & point ) const;

      /*!\brief qucsitem_cast identifier
	 \sa qucsitem_cast
      */
      int type() const { return Wire::Type; }

      void paint(QPainter * p, const QStyleOptionGraphicsItem * o, QWidget * w = 0 );

      static Wire* connectedWire(const Node *n1,const Node *n2);

      void startMoveAndResize();
      void moveAndResizeBy(qreal dx, qreal dy);
      void stopMoveAndResize();

      /*!\brief Return all the line constituing a wire */
      QList<WireLine> wireLines() const { return m_lines; }
      void setWireLines(const QList<WireLine>& lines);
      void deleteNullLines();

      QString saveString() const;
      void writeXml(Qucs::XmlWriter *writer);
      void readXml(Qucs::XmlReader *reader);

      /*!\brief Rotate
         \details Wire are not allowed to rotate
	 \todo Implement not allowed to rotate (block command)
       */
      void rotate() {}
      /*!\brief mirrorX
         \details Wires are not allowed to be mirrored
	 \todo Block command
       */
      void mirrorX() {}
      /*!\brief mirrorY
         \details Wires are not allowed to be mirrored
	 \todo Block command
       */
      void mirrorY() {}

   protected:
      QVariant itemChange(GraphicsItemChange change, const QVariant &value);
      void mousePressEvent(QGraphicsSceneMouseEvent *event);
      void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
      void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
      void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

   private:
      QRect proxyRect(const WireLine& line) const;
      QRectF rectForLine(const WireLine& line) const;

      void updateProxyWires();
      void clearProxyWires();

      int indexForPos(const QPointF& pos) const;

      /*!\brief List of line constituing a wire */
      QList<WireLine> m_lines;
      /*!\brief They represent wires visually when wires are moved or resized.

        This is used because, modifying the shape of a QGraphicsItem very often
        continously results in frequent changes in QGraphicsScene's bsp indices
        Hence resizing will be very slow.
        So the wire is hidden and the lines are represented by QRubberBand
        widgets and only the rubberband widget is updated until the resizing or
        moving is going on. RubberBand widgets being toplevel widgets doesn't
        belong to scene. Therefore they won't affect Bsp indexing and thus
        speed is obtained. Finally when the wire resizing stops, RubberBands
        are destroyed and the item is reshown resulting in change of index
        only once.
      */
      QList<QRubberBand*> m_proxyWires;
      /*!\brief first Node */
      Node *m_node1;
      /*!\brief second node */
      Node *m_node2;
      /*!\brief Represents the index of m_lines list corresponding to mouse
	 click.

	 This is used to caculate the exact wire movement when it is dragged.
      */
      int m_grabbedLineIndex;
};

#endif //__WIRE_H
