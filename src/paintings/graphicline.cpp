/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

#include "graphicline.h"

#include "settings.h"
#include "styledialog.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a line item.
     *
     * \param line Line in local coords.
     * \param scene CGraphicsScene to which this item should be added.
     */
    GraphicLine::GraphicLine(const QLineF &line, CGraphicsScene *scene) :
          Painting(scene)
    {
        setLine(line);
        setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle);
    }

    //! \copydoc Painting::shapeForRect()
    QPainterPath GraphicLine::shapeForRect(const QRectF &rect) const
    {
        QPainterPath path;
        path.moveTo(rect.topLeft());
        path.lineTo(rect.bottomRight());
        return path;
    }

    //! \brief Draws line.
    void GraphicLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
    {
        if(option->state & QStyle::State_Selected) {
            Settings *settings = Settings::instance();
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 pen().width()));
        }
        else {
            painter->setPen(pen());
        }

        painter->setBrush(Qt::NoBrush);
        QLineF line = lineFromRect(paintingRect());
        painter->drawLine(line);

        Painting::paint(painter, option, w);
    }

    //! \copydoc CGraphicsItem::copy()
    GraphicLine* GraphicLine::copy(CGraphicsScene *scene) const
    {
        GraphicLine *lineItem = new GraphicLine(line(), scene);
        Painting::copyDataTo(lineItem);
        return lineItem;
    }

    //! \brief Saves data as xml.
    void GraphicLine::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "line");

        writer->writeLineAttribute(line());
        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(transform());

        writer->writePen(pen());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads xml data referred by \a reader.
    void GraphicLine::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "line");

        setLine(reader->readLineAttribute("line"));
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

                else {
                    reader->readUnknownElement();
                }

            }
        }
    }

    //! \brief Set's line to \a line (local coords).
    void GraphicLine::setLine(const QLineF& line)
    {
        setPaintingRect(QRectF(line.p1(), line.p2()));
    }

    //! \copydoc CGraphicsItem::launchPropertiesDialog()
    int GraphicLine::launchPropertiesDialog(Caneda::UndoOption opt)
    {
        StyleDialog dia(this);
        return dia.exec();
    }

} // namespace Caneda
