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

#include "graphicline.h"

#include "styledialog.h"

#include "xmlutilities/xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

/*!
 * \brief Constructs a line item.
 * \param line Line in local coords.
 * \param scene SchematicScene to which this item should be added.
 */
GraphicLine::GraphicLine(const QLineF &line, SchematicScene *scene) :
      Painting(scene)
{
   setLine(line);
   setResizeHandles(Qucs::TopLeftHandle | Qucs::BottomRightHandle);
}

//! \brief Destructor.
GraphicLine::~GraphicLine()
{
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
   painter->setBrush(Qt::NoBrush);

   QLineF line = lineFromRect(paintingRect());

   if(option->state & QStyle::State_Selected) {
      QPen _pen(pen());
      _pen.setColor(Qt::darkGray);
      _pen.setWidth(pen().width() + 5);

      painter->setPen(_pen);

      painter->drawLine(line);

      _pen.setWidth(pen().width());
      _pen.setColor(Qt::white);

      painter->setPen(_pen);
   }
   else {
      painter->setPen(pen());
   }

   painter->drawLine(line);

   Painting::paint(painter, option, w);
}

//! \brief Returns copy of this line item.
GraphicLine* GraphicLine::copy(SchematicScene *scene) const
{
   GraphicLine *lineItem = new GraphicLine(line(), scene);
   Painting::copyDataTo(lineItem);
   return lineItem;
}

//! \brief Saves data as xml.
void GraphicLine::saveData(Qucs::XmlWriter *writer) const
{
   writer->writeStartElement("painting");
   writer->writeAttribute("name", "line");

   writer->writeEmptyElement("properties");
   writer->writeLineAttribute(line());
   writer->writePointAttribute(pos(), "pos");

   writer->writePen(pen());
   writer->writeTransform(transform());

   writer->writeEndElement(); // </painting>
}

//! \brief Loads xml data referred by \a reader.
void GraphicLine::loadData(Qucs::XmlReader *reader)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
   Q_ASSERT(reader->attributes().value("name") == "line");

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "properties") {
            setLine(reader->readLineAttribute("line"));
            setPos(reader->readPointAttribute("pos"));

            reader->readUnknownElement(); //read till end tag
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

//! \brief Set's line to \a line (local coords).
void GraphicLine::setLine(const QLineF& line)
{
   setPaintingRect(QRectF(line.p1(), line.p2()));
}

int GraphicLine::launchPropertyDialog(Qucs::UndoOption opt)
{
   StyleDialog dia(this, opt);
   return dia.exec();
}
