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

#include "graphictext.h"

#include "global.h"
#include "graphictextdialog.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>

namespace Caneda
{
    /*!
     * \brief Constructs a text item.
     *
     * \param text Item's text.
     * \param parent Parent of the GraphicText item.
     */
    GraphicText::GraphicText(const QString &text, QGraphicsItem *parent) :
        Painting(parent)
    {
        m_textItem = new QGraphicsTextItem(this);
        m_textItem->setAcceptedMouseButtons(0);
        setText(text);
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
        QString unicodeText = Caneda::latexToUnicode(text);
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
        QString unicodeText = Caneda::latexToUnicode(text);
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
            // Save pen
            const QPen savePen = painter->pen();

            // Draw selection rectangle
            Settings *settings = Settings::instance();
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt(), Qt::DashLine));
            painter->drawRect(boundingRect());

            // Restore pen
            painter->setPen(savePen);
        }
    }

    //! \copydoc GraphicsItem::copy()
    GraphicText* GraphicText::copy() const
    {
        GraphicText *text = new GraphicText(richText(), parentItem());
        Painting::copyDataTo(text);
        return text;
    }

    //! \brief Save's data as xml.
    void GraphicText::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "text");

        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(sceneTransform());

        writer->writeEmptyElement("properties");
        writer->writeAttribute("text", richText());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads text data from xml referred by \a reader.
    void GraphicText::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "text");

        setPos(reader->readPointAttribute("pos"));
        setTransform(reader->readTransformAttribute("transform"));

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {

                if(reader->name() == "properties") {

                    setText(reader->attributes().value("text").toString());

                    reader->readUnknownElement(); //read till end tag
                }

                else {
                    reader->readUnknownElement();
                }

            }
        }
    }

    //! \copydoc GraphicsItem::launchPropertiesDialog()
    int GraphicText::launchPropertiesDialog()
    {
        GraphicTextDialog dialog(this, true);
        return dialog.exec();
    }

} // namespace Caneda
