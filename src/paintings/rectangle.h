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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "painting.h"

namespace Caneda
{
    /*!
     * \brief Represents rectangular painting item.
     *
     * This class allows user to draw rectangle on schematic. The rectangles can
     * be filled by setting \a Painting::setBrush() .
     */
    class Rectangle : public Painting
    {
    public:
        Rectangle(const QRectF &rect, CGraphicsScene *scene = 0);

        //! \copydoc CGraphicsItem::Type
        enum { Type = Painting::RectangleType };
        //! \copydoc CGraphicsItem::type()
        int type() const { return Type; }

        QPainterPath shapeForRect(const QRectF& rect) const;

        void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *);

        //! \brief Returns rectangle coords as QRectF.
        QRectF rect() const { return paintingRect(); }
        void setRect(const QRectF& rect) { setPaintingRect(rect); }

        Rectangle* copy(CGraphicsScene *scene = 0) const;

        void saveData(Caneda::XmlWriter *writer) const;
        void loadData(Caneda::XmlReader *reader);

        int launchPropertiesDialog();
    };

} // namespace Caneda

#endif //RECTANGLE_H
