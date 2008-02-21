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
#include "xmlutilities.h"

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

/*!
 * \brief Constructs a text item.
 * \param text Item's text.
 * \param scene SchematicScene to which this item should be added.
 */
GraphicText::GraphicText(const QString &text, SchematicScene *scene)
   : Painting(scene)
{
   m_textItem = new QGraphicsSimpleTextItem(this);
   setText(text);
   setPen(Qt::NoPen);
   m_textItem->setAcceptedMouseButtons(0);
}

//! \brief Destructor.
GraphicText::~GraphicText()
{
}

/*!
 * \brief Sets the item's text to \a text.
 *
 * The text will be displayed as plain text. Newline characters ('\n') as well
 * as characters of type QChar::LineSeparator will cause item to break the
 * text into multiple lines.
 */
void GraphicText::setText(const QString &text)
{
   prepareGeometryChange();
   m_textItem->setText(text);
   setPaintingRect(m_textItem->boundingRect());
}

//! \brief Returns the item's text.
QString GraphicText::text() const
{
   return m_textItem->text();
}

/*!
 * \brief Sets the font that is used to draw the item's text to \a font.
*/
void GraphicText::setFont(const QFont &font)
{
   prepareGeometryChange();
   m_textItem->setFont(font);
   setPaintingRect(m_textItem->boundingRect());
}

/*!
 * \brief Set the text's pen to \a pen.
 */
void GraphicText::setPen(const QPen& pen)
{
   m_textItem->setPen(pen);
   Painting::setPen(pen);
}

/*!
 * \brief Returns the font that is used to draw the item's text.
*/
QFont GraphicText::font() const
{
   return m_textItem->font();
}

/*!
 * \brief Draw's hightlight rect if selected.
 */
void GraphicText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
   if(option->state & QStyle::State_Selected) {
      Qucs::drawHighlightRect(painter, boundingRect(), 1, option);
   }
}

//! \brief Returns a copy of this item.
QucsItem* GraphicText::copy(SchematicScene *scene) const
{
   GraphicText *text = new GraphicText(this->text(), scene);
   text->setFont(font());
   Painting::copyDataTo(text);
   return text;
}

//! \brief Save's data as xml.
void GraphicText::saveData(Qucs::XmlWriter *writer) const
{
   writer->writeStartElement("painting");
   writer->writeAttribute("name", "text");

   writer->writeEmptyElement("properties");
   writer->writeAttribute("text", text());
   writer->writePointAttribute(pos(), "pos");

   writer->writeFont(font());
   writer->writePen(pen());

   writer->writeEndElement(); // </painting>
}

//! \brief Loads text data from xml referred by \a reader.
void GraphicText::loadData(Qucs::XmlReader *reader)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
   Q_ASSERT(reader->attributes().value("name") == "text");

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement())
         break;

      if(reader->isStartElement()) {
         if(reader->name() == "properties") {

            setText(reader->attributes().value("text").toString());
            setPos(reader->readPointAttribute("pos"));

            reader->readUnknownElement(); //read till end tag
         }

         else if(reader->name() == "font") {
            setFont(reader->readFont());
         }

         else if(reader->name() == "pen") {
            setPen(reader->readPen());
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
