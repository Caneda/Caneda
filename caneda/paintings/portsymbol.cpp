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

#include "portsymbol.h"

#include "port.h"

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

int portSymbolOffset = 3;

/*!
 * \brief Constructs a port symbol painting item.
 * \param nameStr_ Represents the name part of port id.
 * \param numberStr_ Represents the number part of port id.
 * \param scene SchematicScene on which this item should be added.
 */
PortSymbol::PortSymbol(const QString& nameStr_, const QString& numberStr_,
        SchematicScene *scene) :
    Painting(scene),
    m_mirrored(false),

    m_numberString(numberStr_),
    m_nameString(nameStr_)
{
    updateGeometry();
}

//! \brief Draw port ellipse and id.
void PortSymbol::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    painter->setPen(unconnectedPen);  // like open node
    painter->drawEllipse(portEllipse);

    QPointF textPos = m_mirrored ? portEllipse.bottomLeft() : portEllipse.bottomRight();
    if(m_mirrored) {
        textPos.rx() -= portSymbolOffset;
        textPos.rx() -= QFontMetrics(font()).width(text());
    }
    else {
        textPos.rx() += portSymbolOffset;
    }

    painter->setPen(pen());
    painter->setFont(font());

    painter->drawText(textPos, text());

    painter->setPen(Qt::lightGray);
    painter->drawRect(paintingRect());

    if(option->state & QStyle::State_Selected) {
        painter->setPen(Qt::darkGray);
        painter->drawRoundRect(boundingRect());
    }
}

//! \brief Set number part of port id to \a str.
void PortSymbol::setNumberString(QString str)
{
    prepareGeometryChange();
    m_numberString = str;
    updateGeometry();
}

//! \brief Set name part of port id to \a str.
void PortSymbol::setNameString(QString str)
{
    prepareGeometryChange();
    m_nameString = str;
    updateGeometry();
}

//! \brief Updates the geometry once a font is set or it is mirrored.
void PortSymbol::updateGeometry()
{
    QFontMetrics fm(font());
    qreal height = qMax(portEllipse.bottom(), qreal(fm.height() + fm.descent()));

    QPointF topLeft(0, -height/2.);
    QPointF bottomRight(0, height/2.);

    if(m_mirrored) {
        topLeft.rx() = portEllipse.left() - portSymbolOffset - fm.width(text());
        bottomRight.rx() = portEllipse.right();
    } else {
        topLeft.rx() = portEllipse.left();
        bottomRight.rx() = portEllipse.right() + portSymbolOffset + fm.width(text());
    }

    QRectF rect(topLeft, bottomRight);
    setPaintingRect(rect);
}

//! \brief Sets the font used to draw port id to \a font.
void PortSymbol::setFont(const QFont& font)
{
    m_font = font;
    updateGeometry();
}

//! \brief Reimplement mirror method to take care of mirror along Y axis.
void PortSymbol::mirrorAlong(Qt::Axis axis)
{
    if(axis == Qt::YAxis) {
        prepareGeometryChange();
        m_mirrored = !m_mirrored;
        updateGeometry();
    }
    //ignore other axis
}

//! \brief Returns a copy of port symbol item parented to scene \a scene.
PortSymbol* PortSymbol::copy(SchematicScene *scene) const
{
    PortSymbol *port = new PortSymbol(m_numberString, m_nameString, scene);
    port->m_mirrored = m_mirrored;
    port->m_font = m_font;
    port->updateGeometry();

    Painting::copyDataTo(port);

    return port;
}

//! \brief Saves data as xml.
void PortSymbol::saveData(Qucs::XmlWriter *writer) const
{
    writer->writeStartElement("painting");
    writer->writeAttribute("name", "portSymbol");

    writer->writeEmptyElement("properties");
    writer->writeAttribute("nameString", m_nameString);
    writer->writeAttribute("numberString", m_numberString);
    writer->writeAttribute("mirrored", Qucs::boolToString(m_mirrored));
    writer->writePointAttribute(pos(), "pos");

    writer->writeFont(font());
    writer->writeTransform(transform());

    writer->writeEndElement(); // < /painting>
}

//! \brief Loads portSymbol from xml referred by \a reader.
void PortSymbol::loadData(Qucs::XmlReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
    Q_ASSERT(reader->attributes().value("name") == "portSymbol");

    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isEndElement()) {
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "properties") {

                setNumberString(reader->attributes().value("numberString").toString());
                setNameString(reader->attributes().value("nameString").toString());

                setPos(reader->readPointAttribute("pos"));
                m_mirrored = Qucs::stringToBool(reader->attributes().value("mirrored").toString());
                updateGeometry();
                reader->readUnknownElement(); //read till end tag
            }

            else if(reader->name() == "font") {
                setFont(reader->readFont());
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
