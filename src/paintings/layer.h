/***************************************************************************
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef LAYER_H
#define LAYER_H

#include "painting.h"

namespace Caneda
{
    /*!
     * \brief Represents rectangular layer painting item.
     *
     * This class allows user to draw a rectangle layer. The rectangles can
     * be filled by setting \a Painting::setBrush() .
     */
    class Layer : public Painting
    {
    public:
        //! \brief Represents the layer name type.
        enum LayerName {
            Metal1,
            Metal2,
            Poly1,
            Poly2,
            Active,
            Contact,
            NWell,
            PWell
        };

        explicit Layer(const QRectF &rect,
                       LayerName layerName = Metal1,
                       const QString &netLabel = QString(),
                       QGraphicsItem *parent = 0);

        //! \copydoc GraphicsItem::Type
        enum { Type = Painting::LayerType };
        //! \copydoc GraphicsItem::type()
        int type() const { return Type; }

        QPainterPath shapeForRect(const QRectF& rect) const;

        void updateBrush();
        void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *);

        //! \brief Returns rectangle coords as QRectF.
        QRectF rect() const { return paintingRect(); }
        void setRect(const QRectF& rect) { setPaintingRect(rect); }

        LayerName layerName() const { return m_layerName; }
        void setLayerName(LayerName layerName) { m_layerName = layerName; }

        QString netLabel() const { return m_netLabel; }
        void setNetLabel(QString netLabel) { m_netLabel = netLabel; }

        Layer* copy() const;

        void saveData(Caneda::XmlWriter *writer) const;
        void loadData(Caneda::XmlReader *reader);

        int launchPropertiesDialog();

    private:
        LayerName m_layerName;
        QString m_netLabel;
    };

} // namespace Caneda

#endif //LAYER_H
