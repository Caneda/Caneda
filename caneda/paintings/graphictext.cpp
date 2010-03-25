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

#include "graphictext.h"

#include "graphictextdialog.h"
#include "mnemo.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>

/*!
 * \brief Constructs a text item.
 * \param text Item's text.
 * \param scene SchematicScene to which this item should be added.
 */
GraphicText::GraphicText(const QString &text, SchematicScene *scene) : Painting(scene)
{
    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setAcceptedMouseButtons(0);
    setText(text);
}

//! \brief Destructor.
GraphicText::~GraphicText()
{
}

QString GraphicText::plainText() const
{
    return m_textItem->toPlainText();
}

/*!
 * \brief Sets the item's text to \a text.
 *
 * The text will be displayed as plain text. Newline characters ('\n') as well
 * as characters of type QChar::LineSeparator will cause item to break the
 * text into multiple lines.
 */
void GraphicText::setPlainText(const QString &text)
{
    prepareGeometryChange();
    QString unicodeText = Qucs::latexToUnicode(text);
    m_textItem->setPlainText(unicodeText);
    setPaintingRect(m_textItem->boundingRect());
}

QString GraphicText::richText() const
{
    return m_textItem->toHtml();
}

void GraphicText::setRichText(const QString &text)
{
    prepareGeometryChange();
    QString unicodeText = Qucs::latexToUnicode(text);
    m_textItem->setHtml(unicodeText);
    setPaintingRect(m_textItem->boundingRect());
}

void GraphicText::setText(const QString &text)
{
    if(Qt::mightBeRichText(text)) {
        setRichText(text);
    }
    else {
        setPlainText(text);
    }
}

//! \brief Draw's hightlight rect if selected.
void GraphicText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if(option->state & QStyle::State_Selected) {
        Qucs::drawHighlightRect(painter, boundingRect(), 1, option);
    }
}

//! \brief Returns a copy of this item.
GraphicText* GraphicText::copy(SchematicScene *scene) const
{
    GraphicText *text = new GraphicText(richText(), scene);
    Painting::copyDataTo(text);
    return text;
}

//! \brief Save's data as xml.
void GraphicText::saveData(Qucs::XmlWriter *writer) const
{
    writer->writeStartElement("painting");
    writer->writeAttribute("name", "text");

    writer->writeEmptyElement("properties");
    writer->writeAttribute("text", richText());
    writer->writePointAttribute(pos(), "pos");

    writer->writeEndElement(); // </painting>
}

//! \brief Loads text data from xml referred by \a reader.
void GraphicText::loadData(Qucs::XmlReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
    Q_ASSERT(reader->attributes().value("name") == "text");

    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isEndElement()) {
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "properties") {

                setText(reader->attributes().value("text").toString());
                setPos(reader->readPointAttribute("pos"));

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
}

//! \brief Launch rich text edit dialog.
int GraphicText::launchPropertyDialog(Qucs::UndoOption opt)
{
    GraphicTextDialog dialog(this, opt);
    return dialog.exec();
}
