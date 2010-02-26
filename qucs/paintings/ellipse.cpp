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

#include "ellipse.h"

#include "styledialog.h"

#include "xmlutilities/xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

/*!
 * \brief Constructs an Ellipse item.
 * \param rect Ellipse rect
 * \param scene SchematicScene to which this item should be added.
 */
Ellipse::Ellipse(QRectF rect, SchematicScene *scene) : Painting(scene)
{
    setEllipse(rect);
    setResizeHandles(Qucs::TopLeftHandle | Qucs::BottomRightHandle |
            Qucs::TopRightHandle| Qucs::BottomLeftHandle);
}

//! \brief Destructor.
Ellipse::~Ellipse()
{
}

//! \copydoc Painting::boundForRect()
QRectF Ellipse::boundForRect(const QRectF &rect) const
{
    qreal adj = (pen().width() + 5) / 2;
    return rect.adjusted(-adj, -adj, adj, adj);
}

//! \copydoc Painting::shapeForRect()
QPainterPath Ellipse::shapeForRect(const QRectF &rect) const
{
    QPainterPath path;
    path.addEllipse(boundForRect(rect));
    return path;
}

//! \brief Draws ellipse.
void Ellipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    if(option->state & QStyle::State_Selected) {
        painter->setBrush(Qt::NoBrush);

        QPen _pen(pen());

        _pen.setColor(Qt::darkGray);
        _pen.setWidth(pen().width() + 5);

        painter->setPen(_pen);
        painter->drawEllipse(ellipse());

        _pen.setColor(Qt::white);
        _pen.setWidth(pen().width());
        painter->setPen(_pen);
    }
    else {
        painter->setPen(pen());
    }
    painter->setBrush(brush());
    painter->drawEllipse(ellipse());
    //call base method to draw resize handles.
    Painting::paint(painter, option, w);
}

//! \brief Returns a copy of this Ellipse item.
Ellipse* Ellipse::copy(SchematicScene *scene) const
{
    Ellipse *ell = new Ellipse(ellipse(), scene);
    Painting::copyDataTo(ell);
    return ell;
}

//! \brief Saves ellipse data as xml.
void Ellipse::saveData(Qucs::XmlWriter *writer) const
{
    writer->writeStartElement("painting");
    writer->writeAttribute("name", "ellipse");

    writer->writeEmptyElement("properties");
    writer->writeRectAttribute(ellipse(), QLatin1String("ellipse"));
    writer->writePointAttribute(pos(), "pos");

    writer->writePen(pen());
    writer->writeBrush(brush());
    writer->writeTransform(transform());

    writer->writeEndElement(); // </painting>
}

//! \brief Loads ellipse data from xml refered by \a reader.
void Ellipse::loadData(Qucs::XmlReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
    Q_ASSERT(reader->attributes().value("name") == "ellipse");

    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isEndElement()) {
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "properties") {
                QRectF ellipse = reader->readRectAttribute(QLatin1String("ellipse"));
                setEllipse(ellipse);

                QPointF pos = reader->readPointAttribute("pos");
                setPos(pos);

                reader->readUnknownElement(); //read till end tag
            }

            else if(reader->name() == "pen") {
                setPen(reader->readPen());
            }

            else if(reader->name() == "brush") {
                setBrush(reader->readBrush());
            }

            else if(reader->name() == "transform") {
                setTransform(reader->readTransform());
            }

            else {
                reader->readUnknownElement();
            }
        }
    }
}

int Ellipse::launchPropertyDialog(Qucs::UndoOption opt)
{
    StyleDialog dia(this, opt);
    return dia.exec();
}
