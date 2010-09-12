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

#include "layer.h"

#include "settings.h"
#include "styledialog.h"

#include "xmlutilities/xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a rectangle layer painting item.
     * \param rect Rectangle in local coords.
     * \param layerName Phisical layer to recreate
     * \param scene Scene to which this item should be added.
     */
    Layer::Layer(const QRectF &rect, LayerName layerName, const QString &netLabel,
                 SchematicScene *scene) :
       Painting(scene),
       m_layerName(layerName),
       m_netLabel(netLabel)
    {
       setRect(rect);
       setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle |
                        Caneda::TopRightHandle| Caneda::BottomLeftHandle);

       updateBrush();
    }

    //! \brief Destructor.
    Layer::~Layer()
    {
    }

    //! \copydoc Painting::shapeForRect()
    QPainterPath Layer::shapeForRect(const QRectF& rect) const
    {
       QPainterPath path;
       path.addRect(boundForRect(rect));
       return path;
    }

    //! \copydoc Painting::boundForRect()
    QRectF Layer::boundForRect(const QRectF &rect) const
    {
       qreal adj = (pen().width() + 5) / 2;
       return rect.adjusted(-adj, -adj, adj, adj);
    }

    //! \brief Updates the brush according to current layer.
    void Layer::updateBrush()
    {
        Settings *settings = Settings::instance();
        QBrush _brush(Qt::transparent);
        if(layerName() == Layer::Metal1) {
            _brush.setColor(settings->currentValue("gui/layout/metal1").value<QColor>());
            setZValue(0);
        }
        else if(layerName() == Layer::Metal2) {
            _brush.setColor(settings->currentValue("gui/layout/metal2").value<QColor>());
            setZValue(1);
        }
        else if(layerName() == Layer::Poly1) {
            _brush.setColor(settings->currentValue("gui/layout/poly1").value<QColor>());
            setZValue(2);
        }
        else if(layerName() == Layer::Poly2) {
            _brush.setColor(settings->currentValue("gui/layout/poly2").value<QColor>());
            setZValue(3);
        }
        else if(layerName() == Layer::Active) {
            _brush.setColor(settings->currentValue("gui/layout/active").value<QColor>());
            setZValue(4);
        }
        else if(layerName() == Layer::Contact) {
            _brush.setColor(settings->currentValue("gui/layout/contact").value<QColor>());
            setZValue(5);
        }
        else if(layerName() == Layer::NWell) {
            _brush.setColor(settings->currentValue("gui/layout/nwell").value<QColor>());
            setZValue(6);
        }
        else if(layerName() == Layer::PWell) {
            _brush.setColor(settings->currentValue("gui/layout/pwell").value<QColor>());
            setZValue(7);
        }

        setBrush(_brush);
    }


    //! \brief Draw the layer item.
    void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
    {
       if(option->state & QStyle::State_Selected) {
          painter->setBrush(Qt::NoBrush);

          QPen _pen(pen());

          _pen.setColor(Qt::darkGray);
          _pen.setWidth(pen().width() + 5);

          painter->setPen(_pen);
          painter->drawRect(rect());

          _pen.setColor(Qt::white);
          _pen.setWidth(pen().width());
          painter->setPen(_pen);
       }
       else {
           painter->setPen(QPen(Qt::NoPen));
       }

       painter->setOpacity(0.5);
       painter->setBrush(brush());
       painter->drawRect(rect());

       // Call base method to draw resize handles.
       Painting::paint(painter, option, w);
    }

    //! \copydoc Painting::copy()
    Layer* Layer::copy(SchematicScene *scene) const
    {
       Layer *layerItem = new Layer(rect(), layerName(), netLabel(), scene);
       Painting::copyDataTo(layerItem);
       return layerItem;
    }

    //! \brief Saves layer data to xml using \a writer.
    void Layer::saveData(Caneda::XmlWriter *writer) const
    {
       writer->writeStartElement("painting");
       writer->writeAttribute("name", "layer");

       writer->writeEmptyElement("properties");
       writer->writeRectAttribute(rect(), QLatin1String("rect"));
       writer->writePointAttribute(pos(), "pos");
       writer->writeAttribute("layerName", QString::number(int(m_layerName)));
       writer->writeAttribute("netLabel", netLabel());
       writer->writeTransform(transform());

       writer->writeEndElement(); // </painting>
    }

    //! \brief Loads layer data from xml referred by \a reader.
    void Layer::loadData(Caneda::XmlReader *reader)
    {
       Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
       Q_ASSERT(reader->attributes().value("name") == "layer");

       while(!reader->atEnd()) {
          reader->readNext();

          if(reader->isEndElement()) {
             break;
          }

          if(reader->isStartElement()) {
             if(reader->name() == "properties") {
                QRectF rect = reader->readRectAttribute(QLatin1String("rect"));
                setRect(rect);

                QPointF pos = reader->readPointAttribute("pos");
                setPos(pos);

                int layer = reader->attributes().value("layerName").toString().toInt();
                setLayerName((LayerName)layer);

                QString label = reader->attributes().value("netLabel").toString();
                setNetLabel(label);

                reader->readUnknownElement(); //read till end tag
             }
             else if(reader->name() == "transform") {
                setTransform(reader->readTransform());
             }
             else {
                reader->readUnknownElement();
             }
          }
       }

       updateBrush();
    }

    int Layer::launchPropertyDialog(Caneda::UndoOption opt)
    {
       StyleDialog dia(this, opt);
       return dia.exec();
    }

} // namespace Caneda
