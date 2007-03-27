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
      Shape() {}
      virtual ~Shape() {}
      virtual void draw(QPainter *p,const QStyleOptionGraphicsItem *o) = 0;
};
      

class Line : public Shape
{
   public:
      Line(int _x1, int _y1, int _x2, int _y2, QPen _style)
         : x1(_x1), y1(_y1), x2(_x2), y2(_y2), style(_style) {};

      void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         if(o->state & QStyle::State_Open)
         {
            QPen _pen = p->pen();
            _pen.setWidth(style.width()+1);
            p->setPen(_pen);
         }
         else if(o->state & QStyle::State_Selected)
            p->setPen(Component::getPen(Qt::darkGray,style.width()));
         else
            p->setPen(style);
         p->drawLine(x1,y1,x2,y2);
      }

   private:
      
      const int   x1, y1, x2, y2;
      const QPen  style;
     
};

class Arc : public Shape
{
   public:
      Arc(int _x, int _y, int _w, int _h, int _angle, int _arclen, QPen _style)
         : x(_x), y(_y), w(_w), h(_h), angle(_angle),
           arclen(_arclen), style(_style) {};

      inline void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         if(o->state & QStyle::State_Open)
            p->setPen(QPen(p->pen().color(),style.width()+1));
         else if(o->state & QStyle::State_Selected)
            p->setPen(Component::getPen(Qt::darkGray,style.width()));
         else
            p->setPen(style);
         p->drawArc(x,y,w,h,angle,arclen);
      }

   private:
      
      const int   x, y, w, h, angle, arclen;
      const QPen  style;
};

class Text : public Shape
{
   public:
      Text(int _x, int _y, const QString& t, QColor _color = Qt::black, qreal _size = 10.0)
         : x(_x), y(_y), text(t), color(_color), size(_size)
      {
      }

      inline void draw(QPainter *p,const QStyleOptionGraphicsItem *o)
      {
         if(o->state & QStyle::State_Open)
            p->setPen(color);
         else if(o->state & QStyle::State_Selected)
            p->setPen(Component::getPen(Qt::darkGray));
         else
            p->setPen(color);
         QFont f = qApp->font();
         f.setPointSizeF(size);
         p->drawText(x,y+12,text);
      }

   private:
      const int x,y;
      const QString text;
      const QColor color;
      const qreal size;
};

typedef QList<Shape*> ShapesList;

#endif //__SHAPES_H
