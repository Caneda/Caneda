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

#ifndef __PROPERTYITEM_H
#define __PROPERTYITEM_H

#include "property.h"
#include <QtGui/QGraphicsTextItem>

//Forwasrd declarations.
class SchematicScene;

class PropertyItem : public QGraphicsTextItem
{
   Q_OBJECT;
   public:
      PropertyItem(const QString& name, PropertyMap& propMap,
                   SchematicScene *scene);

      QRectF boundingRect() const;
      QPainterPath shape() const;
      //FIXME: Check whether contains method implementation is needed.

      void setFont(const QFont& f);

      void validateText();
      void updateValue();

      bool eventFilter(QObject* object, QEvent* event);
      void paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
                 QWidget * widget = 0 );

   protected:
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
      void updateGroupGeometry() const;

      QString m_staticText;
      QPointF m_staticPos;
      const QString m_propertyName;
      PropertyMap &m_propertyMapRef;
};

#endif //__PROPERTYITEM_H
