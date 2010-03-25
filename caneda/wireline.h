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

#ifndef WIRE_LINE_UTILITIES_H
#define WIRE_LINE_UTILITIES_H

#include <QLineF>
#include <QRectF>

/*!
 * \brief Wire class helper
 * \details This class transform a line to something more usable for wiring purpose.
 * For instance you can test horizontallity.
 */
class WireLine
{
public:
    inline WireLine();
    inline WireLine(const QLineF& line);
    inline WireLine(const QPointF& p1, const QPointF& p2);
    inline WireLine(qreal x1,qreal y1, qreal x2,qreal y2);

    inline bool isHorizontal() const;
    inline bool isVertical() const;
    inline bool isOblique() const;
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

    inline operator const QLineF () const; // { return m_line; }
    friend inline bool operator==(const WireLine& l1, const WireLine& l2);

    friend inline bool operator!=(const WireLine& l1, const WireLine& l2);

    QRectF boundingRect () const;

    //! Get adjust value
    static unsigned int getAdjust() { return m_adjust;};
    static void setAdjust(unsigned int adjust) { m_adjust = adjust; };
    private:
    //! \brief Line object
    QLineF m_line;

    /*!
     * \brief Bounding rectangle adjustement
     *
     * Normally a wireline has a height of zero quantity to add in order
     * to have a true rectangle
     * \todo Made Configurable
     */
    static qreal m_adjust;
};


//! \brief Default constructor
inline WireLine::WireLine() {}

/*!
 * \brief Construct a wire from a line
 * \param line: line to use
 */
inline WireLine::WireLine(const QLineF& line) : m_line(line) {}

/*!
 * \brief Construct a wire from two point
 * \param p1 origin point
 * \param p2 end point
 */
inline WireLine::WireLine(const QPointF& p1, const QPointF& p2) : m_line(p1,p2) {}

/*!
 * \brief Construct a wire from two tupple of coordinate
 * \param x1: origin  abscissa
 * \param x2: end abscissa
 * \param y1: origin ordinate
 * \param y2: end ordinate
 */
inline WireLine::WireLine(qreal x1,qreal y1,qreal x2,qreal y2) : m_line(x1,y1,x2,y2) {}

//! \brief return true if line is horizontal
inline bool WireLine::isHorizontal() const
{
    return m_line.p1().y() == m_line.p2().y();
}

//! \brief return true if line is vertical
inline bool WireLine::isVertical() const
{
    return m_line.p1().x() == m_line.p2().x();
}

inline bool WireLine::isOblique() const
{
    return !isNull() && !isHorizontal() && !isVertical();
}

//! \brief return true if line is Null ie length zero
inline bool WireLine::isNull() const
{
    return m_line.isNull();
}

/*!
 * \brief Set orign point
 * \param pt new orign point
 */
inline void WireLine::setP1(const QPointF& pt)
{
    m_line = QLineF(pt,m_line.p2());
}

/*!
 * \brief Set end point
 * \param pt new end point
 */
inline void WireLine::setP2(const QPointF& pt)
{
    m_line = QLineF(m_line.p1(),pt);
}

/*!
 * \brief set abscissa of both end points
 * \param x new abscissa
 */
inline void WireLine::setX(qreal x)
{
    QPointF p1 = m_line.p1();
    QPointF p2 = m_line.p2();
    p1.setX(x);
    p2.setX(x);
    m_line = QLineF(p1,p2);
}

/*!
 * \brief set ordinate of both end points
 * \param y new ordinate
 */
inline void WireLine::setY(qreal y)
{
    QPointF p1 = m_line.p1();
    QPointF p2 = m_line.p2();
    p1.setY(y);
    p2.setY(y);
    m_line = QLineF(p1,p2);
}

/*!
 * \brief Translate line
 * \param delta translation vector
 */
inline void WireLine::translate(const QPointF& delta)
{
    m_line.translate(delta);
}

/*!
 * \brief Translate line
 * \param dx transalation on abscissa coordinate
 * \param dy transalation on ordinate coordinate
 */
inline void WireLine::translate(qreal dx,qreal dy)
{
    m_line.translate(dx,dy);
}

/*!
 * \brief Return origin abscissa
 * \return origin abscissa
 */
inline qreal WireLine::x1() const
{
    return m_line.x1();
}

/*!
 * \brief Return origin ordinate
 * \return origin ordinate
 */
inline qreal WireLine::y1() const
{
    return m_line.y1();
}

/*!
 * \brief Return end abscissa
 * \return end abscissa
 */
inline qreal WireLine::x2() const
{
    return m_line.x2();
}

/*!
 * \brief Return end ordinate
 * \return end ordinate
 */
inline qreal WireLine::y2() const
{
    return m_line.y2();
}

/*!
 * \brief Return origin affix
 * \return origin affix
 */
inline QPointF WireLine::p1() const
{
    return m_line.p1();
}

/*!
 * \brief Return end affix
 * \return end affix
 */
inline QPointF WireLine::p2() const
{
    return m_line.p2();
}

/*!
 * \brief Return origin abscissa
 * \return origin abscissa
 * \todo Delete it! not clear semantic of x()
 */
inline qreal WireLine::x() const
{
    return m_line.x1();
}

/*!
 * \brief Return origin ordinate
 * \return origin ordinate
 * \todo Delete it! not clear semantic of y()
 */
inline qreal WireLine::y() const
{
    return m_line.y1();
}

/*!
 * \brief Return length of line
 * \return length of line
 */
inline qreal WireLine::length() const
{
    return m_line.length();
}

/*!
 * \brief Cast operation
 * \return line
 */
inline WireLine::operator const QLineF() const
{
    return m_line;
}

//! \brief Equality test
inline bool operator==(const WireLine& l1,const WireLine& l2)
{
    return QLineF(l1) == QLineF(l2);
}

//! \brief Inequality test
inline bool operator!=(const WireLine& l1,const WireLine& l2)
{
    return QLineF(l1) != QLineF(l2);
}


#endif //LINE_UTILITIES_H
