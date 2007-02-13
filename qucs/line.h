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

#ifndef __LINE_UTILITIES_H
#define __LINE_UTILITIES_H

#include <QtCore/QLineF>

class Line
{
   public:
      inline Line();
      inline Line(const QLineF& line);
      inline Line(const QPointF& p2, const QPointF& p2);
      inline Line(qreal x1,qreal y1, qreal x2,qreal y2);
      inline bool isHorizontal() const;
      inline bool isVertical() const;
      inline bool isNull() const;

      inline void setP1(const QPointF& pt);
      inline void setP2(const QPointF& p2);

      inline void setX(qreal x);
      inline void setY(qreal y);

      inline void translate(const QPointF& delta);
      inline void translate(qreal dx,qreal dy);

      inline qreal x1() const;
      inline qreal y1() const;
      inline qreal x2() const;
      inline qreal y2() const;

      inline QPointF p1() const;
      inline QPointF p2() const;

      inline qreal x() const;
      inline qreal y() const;

      inline qreal length() const;

      inline operator const QLineF () const;// { return m_line; }
      friend inline bool operator==(const Line& l1, const Line& l2);
      friend inline bool operator!=(const Line& l1, const Line& l2);
   private:
      QLineF m_line;
};

inline Line::Line() {}

inline Line::Line(const QLineF& line) : m_line(line) {}

inline Line::Line(const QPointF& p1, const QPointF& p2) : m_line(p1,p2) {}

inline Line::Line(qreal x1,qreal y1,qreal x2,qreal y2) : m_line(x1,y1,x2,y2) {}

inline bool Line::isHorizontal() const
{
   return m_line.p1().y() == m_line.p2().y();
}

inline bool Line::isVertical() const
{
   return m_line.p1().x() == m_line.p2().x();
}

inline bool Line::isNull() const
{
   return m_line.isNull();
}

inline void Line::setP1(const QPointF& pt)
{
   m_line = QLineF(pt,m_line.p2());
}

inline void Line::setP2(const QPointF& pt)
{
   m_line = QLineF(m_line.p1(),pt);
}

inline void Line::setX(qreal x)
{
   QPointF p1 = m_line.p1();
   QPointF p2 = m_line.p2();
   p1.setX(x);
   p2.setX(x);
   m_line = QLineF(p1,p2);
}

inline void Line::setY(qreal y)
{
   QPointF p1 = m_line.p1();
   QPointF p2 = m_line.p2();
   p1.setY(y);
   p2.setY(y);
   m_line = QLineF(p1,p2);
}

inline void Line::translate(const QPointF& delta)
{
   m_line.translate(delta);
}

inline void Line::translate(qreal dx,qreal dy)
{
   m_line.translate(dx,dy);
}

inline qreal Line::x1() const
{
   return m_line.x1();
}

inline qreal Line::y1() const
{
   return m_line.y1();
}

inline qreal Line::x2() const
{
   return m_line.x2();
}

inline qreal Line::y2() const
{
   return m_line.y2();
}


inline QPointF Line::p1() const
{
   return m_line.p1();
}

inline QPointF Line::p2() const
{
   return m_line.p2();
}


inline qreal Line::x() const
{
   return m_line.x1();
}

inline qreal Line::y() const
{
   return m_line.y1();
}

inline qreal Line::length() const
{
   return m_line.length();
}

inline Line::operator const QLineF() const
{
   return m_line;
}

inline bool operator==(const Line& l1,const Line& l2)
{
   return QLineF(l1) == QLineF(l2);
}

inline bool operator!=(const Line& l1,const Line& l2)
{
   return QLineF(l1) != QLineF(l2);
}


#endif //__LINE_UTILITIES_H
