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

#ifndef __PROPERTYTEXT_H
#define __PROPERTYTEXT_H

#include <QtGui/QGraphicsTextItem>

class PropertyTextValue : public QGraphicsTextItem
{
   public:
      PropertyTextValue(const QString& text,QGraphicsItem *p=0,QGraphicsScene *s=0);
      ~PropertyTextValue() {}
      void formatText();

   protected:
      void mousePressEvent(QGraphicsSceneMouseEvent *e);
      void keyPressEvent(QKeyEvent *e);
      void focusOutEvent(QFocusEvent *e);
};

class PropertyText : public QGraphicsTextItem
{
   public:
      PropertyText(const QString& name, const QString& initialValue = "nil",
		   const QString& description = "nil", QGraphicsItem* par = 0, QGraphicsScene *scene = 0);
      ~PropertyText() {}

      QString name() const { return m_name; }
      QString value() const { return m_valueItem->toPlainText(); }
      QString description() const { return m_description; }

   protected:
      QVariant itemChange(GraphicsItemChange c,const QVariant& value);
      void mousePressEvent(QGraphicsSceneMouseEvent *e);

   private:
      PropertyTextValue *m_valueItem;
      QString m_description;
      QString m_name;
};
#endif //__PROPERTYTEXT_H
