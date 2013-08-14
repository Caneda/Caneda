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

#include "ellipse.h"

#include "settings.h"
#include "styledialog.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs an Ellipse item.
     *
     * \param rect Ellipse rect
     * \param scene CGraphicsScene to which this item should be added.
     */
    Ellipse::Ellipse(QRectF rect, CGraphicsScene *scene) : Painting(scene)
    {
        setEllipse(rect);
        setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle |
                Caneda::TopRightHandle| Caneda::BottomLeftHandle);
    }

    //! \copydoc Painting::shapeForRect()
    QPainterPath Ellipse::shapeForRect(const QRectF &rect) const
    {
        QPainterPath path;
        path.addEllipse(rect);
        return path;
    }

    //! \brief Draws ellipse.
    void Ellipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
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

        painter->drawEllipse(ellipse());

        // Call base method to draw resize handles.
        Painting::paint(painter, option, w);
    }

    //! \brief Returns a copy of this Ellipse item.
    Ellipse* Ellipse::copy(CGraphicsScene *scene) const
    {
        Ellipse *ell = new Ellipse(ellipse(), scene);
        Painting::copyDataTo(ell);
        return ell;
    }

    //! \brief Saves ellipse data as xml.
    void Ellipse::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "ellipse");

        writer->writeRectAttribute(ellipse(), QLatin1String("ellipse"));
        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(transform());

        writer->writePen(pen());
        writer->writeBrush(brush());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads ellipse data from xml refered by \a reader.
    void Ellipse::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "ellipse");

        setEllipse(reader->readRectAttribute(QLatin1String("ellipse")));
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

    int Ellipse::launchPropertyDialog(Caneda::UndoOption opt)
    {
        StyleDialog dia(this, opt);
        return dia.exec();
    }

} // namespace Caneda
