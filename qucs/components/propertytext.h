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
#include <QtCore/QtDebug>

class ComponentProperty;
class SchematicScene;
class SchematicView;

class PropertyText : public QGraphicsTextItem
{
      Q_OBJECT;
   public:
      PropertyText(ComponentProperty *prop,SchematicScene *scene);

      QRectF boundingRect() const;
      QPainterPath shape() const;
      bool contains(const QPointF &point) const { return boundingRect().contains(point); }

      void setFont(const QFont& f);

      void validateText();
      void updateValue();

      SchematicView* activeView() const;

      bool eventFilter(QObject* object, QEvent* event);

   protected:
      void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

      bool sceneEvent(QEvent *event);

      void mousePressEvent ( QGraphicsSceneMouseEvent * event );
      void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
      void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
      void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

      void focusInEvent(QFocusEvent *event);
      void focusOutEvent(QFocusEvent *event);

      void keyPressEvent(QKeyEvent *event);

   private:
      void calculatePos();
      bool isSendable(QGraphicsSceneMouseEvent *event) const;
      void updateGroupGeometry();

      QString m_staticText;
      QPointF staticPos;
      ComponentProperty *property;
};

#endif //__PROPERTYTEXT_H
