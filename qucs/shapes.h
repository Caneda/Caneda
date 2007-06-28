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

#ifndef __SHAPES_H
#define __SHAPES_H

#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QApplication>

#include "components/component.h"


struct Shape
{
      Shape(const QPen& _style) : style(_style) {}
      virtual ~Shape() {}
      virtual void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         if(o->state & QStyle::State_Open)
         {
            QPen _pen = p->pen();
            _pen.setWidth(style.width());
            p->setPen(_pen);
         }
         else if(o->state & QStyle::State_Selected)
            p->setPen(Component::getPen(Qt::darkGray,style.width()));
         else
            p->setPen(style);
      }
      const QPen style;
};


class Line : public Shape
{
   public:
      Line(qreal x1, qreal y1, qreal x2, qreal y2, QPen _style)
         : Shape(_style),
           line(x1, y1, x2, y2)
      {}

      inline void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         Shape::draw(p,o);
         p->drawLine(line);
      }

   private:
      const QLineF line;


};

class Arc : public Shape
{
   public:
      Arc(qreal x, qreal y, qreal w, qreal h, int _angle, int _arclen, QPen _style)
         : Shape(_style),
           rect(x, y, w, h),
           angle(_angle),
           arclen(_arclen)
      {}

      inline void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         Shape::draw(p,o);
         p->drawArc(rect,angle,arclen);
      }

   private:
      const QRectF rect;
      const int angle, arclen;
};

class Text : public Shape
{
   public:
      Text(qreal _x, qreal _y, const QString& t, QColor _color = Qt::black, qreal _size = 10.0)
         : Shape(QPen(_color)),
           pos(_x,_y+12),//HACK: +12 here is easier to do than changing in each component
           text(t),
           size(_size)
      {}

      inline void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         Shape::draw(p,o);
         //QFont f = qApp->font();
         //f.setPointSizeF(size);
         p->drawText(pos,text);
      }

   private:
      const QPointF pos;
      const QString text;
      const qreal size;
};

class Rectangle : public Shape
{
   public:
      Rectangle(qreal x, qreal y, qreal width, qreal height, QPen _style)
         : Shape(_style),
           rect(x,y,width,height)
      {}

      inline void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         Shape::draw(p,o);
         p->drawRect(rect);
      }

   private:
      const QRectF rect;
};


typedef QList<Shape*> ShapesList;

#endif //__SHAPES_H
