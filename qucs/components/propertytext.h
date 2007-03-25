/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __PROPERTYTEXT_H
#define __PROPERTYTEXT_H

#include <QtGui/QGraphicsTextItem>
#include <QtGui/QPainterPath>

class ComponentProperty;
class SchematicScene;

class PropertyText : public QGraphicsTextItem
{
Q_OBJECT
   public:
      PropertyText(ComponentProperty *prop,SchematicScene *scene);
		
      inline QRectF boundingRect() const
      {
         QRectF br = QGraphicsTextItem::boundingRect();
         br.setLeft((staticPos.x()));
         return br;
      }
	
      inline QPainterPath shape() const
      {
         QPainterPath p;
         p.addRect(boundingRect());
         return p;
      }
		
      inline bool contains(const QPointF &point) const
      {
         return boundingRect().contains(point);
      }
		
      inline void setFont(const QFont& f)
      {
         QGraphicsTextItem::setFont(f);
         calculatePos();
      }
		
      void trimText();
      void updateValue();
		
   protected:
      void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
      void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
      void mousePressEvent ( QGraphicsSceneMouseEvent * event );
      void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
      void focusOutEvent(QFocusEvent *event);
      void keyPressEvent(QKeyEvent *event);
      QVariant itemChange(GraphicsItemChange change, const QVariant &value);

   private slots:
      void resetFlags();

   private:
      void calculatePos();
      bool clickedIn;
      QString m_staticText;
      QPointF staticPos;
      ComponentProperty *property;
};

#endif //__PROPERTYTEXT_H
