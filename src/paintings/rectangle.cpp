/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "rectangle.h"

#include "settings.h"
#include "styledialog.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a rectangle painting item.
     *
     * \param rect Rectangle in local coords.
     * \param parent Parent of the Rectangle item.
     */
    Rectangle::Rectangle(const QRectF& rect, QGraphicsItem *parent) :
        Painting(parent)
    {
        setRect(rect);
        setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle |
                         Caneda::TopRightHandle| Caneda::BottomLeftHandle);
    }

    //! \copydoc Painting::shapeForRect()
    QPainterPath Rectangle::shapeForRect(const QRectF& rect) const
    {
        QPainterPath path;
        path.addRect(rect);
        return path;
    }

    //! \brief Draw the rectangle.
    void Rectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
    {
        if(option->state & QStyle::State_Selected) {
            Settings *settings = Settings::instance();
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 pen().width()));

            painter->setBrush(Qt::NoBrush);
        }
        else {
            painter->setPen(pen());
            painter->setBrush(brush());
        }

        painter->drawRect(rect());

        // Call base method to draw resize handles.
        Painting::paint(painter, option, w);
    }

    //! \copydoc GraphicsItem::copy()
    Rectangle* Rectangle::copy() const
    {
        Rectangle *rectItem = new Rectangle(rect(), parentItem());
        Painting::copyDataTo(rectItem);

        return rectItem;
    }

    //! \brief Saves rectangle data to xml using \a writer.
    void Rectangle::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "rectangle");

        writer->writeRectAttribute(rect(), QLatin1String("rectangle"));
        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(sceneTransform());

        writer->writePen(pen());
        writer->writeBrush(brush());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads data from xml referred by \a reader.
    void Rectangle::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "rectangle");

        setRect(reader->readRectAttribute(QLatin1String("rectangle")));
        setPos(reader->readPointAttribute("pos"));
        setTransform(reader->readTransformAttribute("transform"));

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {

                if(reader->name() == "pen") {
                    setPen(reader->readPen());
                }

                else if(reader->name() == "brush") {
                    setBrush(reader->readBrush());
                }

                else {
                    reader->readUnknownElement();
                }
            }
        }
    }

    //! \copydoc GraphicsItem::launchPropertiesDialog()
    void Rectangle::launchPropertiesDialog()
    {
        StyleDialog dialog(this);
        dialog.exec();
    }

} // namespace Caneda
