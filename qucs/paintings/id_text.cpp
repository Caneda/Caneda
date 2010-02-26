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

#include "id_text.h"

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

//! \brief Returns the parameter name and value as combined string.
QString SubParameter::text() const
{
    return QString("%1=%2").arg(name).arg(defaultValue);
}

//! \brief Constructs IdText item with default prefix set to SUB.
IdText::IdText(SchematicScene *scene) :
    Painting(scene),
    m_prefix("SUB")
{
}

//! \brief Destructs parameters.
IdText::~IdText()
{
    qDeleteAll(m_parameters);
}

//! \brief Set the subckt netlist prefix to \a prefix.
void IdText::setPrefix(const QString &prefix)
{
    prepareGeometryChange();
    m_prefix = prefix;
    updateGeometry();
}

//! \brief Set the font to \a font.
void IdText::setFont(const QFont &font)
{
    prepareGeometryChange();
    m_font = font;
    updateGeometry();
}

/*!
 * \brief Adds a parameter to subckt.
 * \param display Whether this parameter should be displayed by default or not.
 * \param name Name of the property.
 * \param description A short description of item.
 * \param defVal The default value of property.
 */
void IdText::addParameter(bool display, QString name,
        QString description, QString defVal)
{
    prepareGeometryChange();
    m_parameters << new SubParameter(display, name, description, defVal);
    updateGeometry();
}

/*!
 * \brief Removes the parameters named \a name.
 */
void IdText::removeParameter(QString name)
{
    QList<SubParameter*>::iterator it = m_parameters.begin();
    while(it != m_parameters.end()) {
        if((*it)->name == name) {
            break;
        }
        it++;
    }
    if(it != m_parameters.end()) {
        prepareGeometryChange();
        m_parameters.erase(it);
        updateGeometry();
    }
}

//! \brief Returns a pointer to parameter object named \a name.
SubParameter* IdText::parameter(const QString &name) const
{
    QList<SubParameter*>::const_iterator it = m_parameters.constBegin();
    while(it != m_parameters.constEnd()) {
        if((*it)->name == name) {
            return *it;
        }
        it++;
    }
    return 0;
}

//! \brief Draws this item taking care of alignment as well.
void IdText::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    painter->setPen(pen());
    painter->setBrush(Qt::NoBrush);

    painter->setFont(font());
    int lineSpacing = QFontMetrics(m_font).lineSpacing();

    QPointF drawPos;
    drawPos.ry() += lineSpacing;

    painter->drawText(drawPos, prefix());

    drawPos.ry() += lineSpacing;
    foreach(SubParameter *param, m_parameters) {
        if(param->display) {

            painter->drawText(drawPos, param->text());
            drawPos.ry() += lineSpacing;
        }
    }

    if(option->state & QStyle::State_Selected) {
        painter->setPen(Qt::darkGray);
        painter->drawRect(boundingRect());
    }
}

//! \brief Updates geometry of this item (paintingRect).
void IdText::updateGeometry()
{
    qreal width, height;
    QFontMetrics fm(m_font);

    int lineSpacing = fm.lineSpacing();
    int fontHeight = fm.height();

    height = lineSpacing;
    width = fm.width(m_prefix);
    foreach(SubParameter *param, m_parameters) {
        if(param->display) {
            height += fontHeight;
            width = qMax((qreal)fm.width(param->text()), width);
        }
    }
    height += fm.descent() + 1;
    setPaintingRect(QRectF(0, 0, width, height));
}

//! \brief Returns a copy of this item parented to scene \a scene.
IdText* IdText::copy(SchematicScene *scene) const
{
    IdText *id = new IdText(scene);

    id->setPrefix(prefix());
    id->setFont(font());
    id->m_parameters = m_parameters;
    id->updateGeometry();

    Painting::copyDataTo(id);

    return id;
}

//! \brief Saves data as xml referred by \a writer.
void IdText::saveData(Qucs::XmlWriter *writer) const
{
    writer->writeStartElement("painting");
    writer->writeAttribute("name", "idText");

    writer->writeEmptyElement("properties");
    writer->writeAttribute("prefix", prefix());
    writer->writePointAttribute(pos(), "pos");

    writer->writeFont(font());

    writer->writeStartElement("subParameters");
    foreach(SubParameter *sub, m_parameters) {
        writer->writeEmptyElement("param");
        writer->writeAttribute("name", sub->name);
        writer->writeAttribute("description", sub->description);
        writer->writeAttribute("default", sub->defaultValue);
        writer->writeAttribute("display", Qucs::boolToString(sub->display));
    }
    writer->writeEndElement();

    writer->writeEndElement(); // </painting>
}

//! \brief Loads IdText data from xml referred by \a reader.
void IdText::loadData(Qucs::XmlReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
    Q_ASSERT(reader->attributes().value("name") == "idText");

    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isEndElement()) {
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "properties") {
                setPrefix(reader->attributes().value("prefix").toString());

                setPos(reader->readPointAttribute("pos"));
                reader->readUnknownElement(); //read till end tag
            }

            else if(reader->name() == "font") {
                setFont(reader->readFont());
            }

            else if(reader->name() == "subParameters") {
                while(!reader->atEnd()) {
                    reader->readNext();

                    if(reader->isEndElement()) {
                        break;
                    }

                    if(reader->isStartElement()) {
                        if(reader->name() == "param") {
                            QXmlStreamAttributes attribs = reader->attributes();
                            QString name = attribs.value("name").toString();
                            QString des = attribs.value("description").toString();
                            QString def = attribs.value("default").toString();
                            bool dis = Qucs::stringToBool(attribs.value("display").toString());

                            m_parameters << new SubParameter(dis, name, des, def);

                            reader->readUnknownElement(); //read till end tag
                        }
                    }
                }
            }

            else {
                reader->readUnknownElement();
            }
        }
    }
    updateGeometry();
}
