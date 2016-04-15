/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

namespace Caneda
{
    //! \brief Represents an elliptic arc painting item.
    class EllipseArc : public Painting
    {
    public:
        EllipseArc(QRectF rect = QRectF(), int startAngle = 20, int spanAngle = 180,
                CGraphicsScene *scene = 0);

        //! \copydoc CGraphicsItem::Type
        enum { Type = Painting::EllipseArcType };
        //! \copydoc CGraphicsItem::type()
        int type() const { return Type; }

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

        EllipseArc* copy(CGraphicsScene *scene = 0) const;

        void saveData(Caneda::XmlWriter *writer) const;
        void loadData(Caneda::XmlReader *reader);

        int launchPropertiesDialog(Caneda::UndoOption opt);

    private:
        int m_startAngle;
        int m_spanAngle;
    };

} // namespace Caneda

#endif //ELLIPSEARC_H
