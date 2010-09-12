/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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
     * This class allows user to draw a rectangle layer on schematic. The rectangles can
     * be filled by setting \a Painting::setBrush() .
     */
    class Layer : public Painting
    {
    public:
        enum {
            Type = Painting::LayerType
        };

        //! Represents the layer name type.
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

        Layer(const QRectF &rect, LayerName layerName = Metal1, SchematicScene *scene = 0);
        ~Layer();

        QPainterPath shapeForRect(const QRectF& rect) const;
        QRectF boundForRect(const QRectF &rect) const;

        void updateBrush();
        void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *);

        //! \brief Returns rectangle coords as QRectF.
        QRectF rect() const { return paintingRect(); }
        void setRect(const QRectF& rect) { setPaintingRect(rect); }

        LayerName layerName() const { return m_layerName; }
        void setLayerName(LayerName layerName) { m_layerName = layerName; }

        int type() const { return Layer::Type; }
        Layer* copy(SchematicScene *scene = 0) const;

        void saveData(Caneda::XmlWriter *writer) const;
        void loadData(Caneda::XmlReader *reader);

        int launchPropertyDialog(Caneda::UndoOption opt);

    private:
        LayerName m_layerName;
    };

} // namespace Caneda

#endif //LAYER_H
