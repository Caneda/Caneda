/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef ARROW_H
#define ARROW_H

#include "painting.h"

/*!
 * \brief This class is used to represent resizable arrow item on schematic.
 *
 * This class supports two different styles of arrow. See \a HeadStyle
 */
class Arrow : public Painting
{
public:
    enum {
        Type = Painting::ArrowType
    };

    //! Represents the arrow head style.
    enum HeadStyle {
        //! This represents an ordinary head style for arrow.
        TwoLineArrow,
        //! This represents a filled triangle for arrow head.
        FilledArrow
    };

    Arrow(const QLineF &line = QLineF(), HeadStyle style = FilledArrow,
            qreal headWidth = 12, qreal headHeight = 20,
            SchematicScene *scene = 0);
    ~Arrow();

    QPainterPath shapeForRect(const QRectF &rect) const;
    QRectF boundForRect(const QRectF &rect) const;

    void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *);

    int type() const { return Arrow::Type; }
    Arrow* copy(SchematicScene *scene = 0) const;

    void saveData(Qucs::XmlWriter *writer) const;
    void loadData(Qucs::XmlReader *reader);

    HeadStyle headStyle() const { return m_headStyle; }
    void setHeadStyle(HeadStyle style);

    //! Returns the base triangle width of arrow head.
    qreal headWidth() const { return m_headWidth; }
    void setHeadWidth(qreal width);

    //! Returns the triangle height of arrow head.
    qreal headHeight() const { return m_headHeight; }
    void setHeadHeight(qreal width);

    //! Returns the line of the arrow.
    QLineF line() const { return lineFromRect(paintingRect()); }
    void setLine(const QLineF &line);

    int launchPropertyDialog(Qucs::UndoOption opt);

protected:
    void geometryChange();

private:
    void calcHeadPoints();
    QLineF lineFromRect(const QRectF &rect) const;
    void drawHead(QPainter *painter);

    HeadStyle m_headStyle;
    qreal m_headWidth;
    qreal m_headHeight;

    //the head's tip is always at index 1
    QPolygonF m_head;
};
#endif //ARROW_H
