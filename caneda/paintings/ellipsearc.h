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

#ifndef ELLIPSEARC_H
#define ELLIPSEARC_H

#include "painting.h"

//! \brief Represents an elliptic arc painting item.
class EllipseArc : public Painting
{
public:
    enum {
        Type = Painting::EllipseArcType
    };

    EllipseArc(QRectF rect = QRectF(), int startAngle = 20, int spanAngle = 180,
            SchematicScene *scene = 0);
    ~EllipseArc();

    QRectF boundForRect(const QRectF &rect) const;
    QPainterPath shapeForRect(const QRectF &rect) const;

    //! \brief Returns arc's startAngle of this item.
    int startAngle() const { return m_startAngle; }
    void setStartAngle(int angle);

    //! \brief Returns arc's spanAngle of this item.
    int spanAngle() const { return m_spanAngle; }
    void setSpanAngle(int angle);

    void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *);

    //! \brief Returns ellipse represented by this elliptic arc.
    QRectF ellipse() const { return paintingRect(); }
    void setEllipse(const QRectF& ellipse) { setPaintingRect(ellipse); }

    int type() const { return EllipseArc::Type; }
    EllipseArc* copy(SchematicScene *scene = 0) const;

    void saveData(Qucs::XmlWriter *writer) const;
    void loadData(Qucs::XmlReader *reader);

    int launchPropertyDialog(Qucs::UndoOption opt);

private:
    int m_startAngle;
    int m_spanAngle;
};
#endif //ELLIPSEARC_H
