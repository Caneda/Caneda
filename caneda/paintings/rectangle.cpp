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

#include "rectangle.h"

#include "styledialog.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

/*!
 * \brief Constructs a rectangle painting item.
 * \param rect Rectangle in local coords.
 * \param scene Scene to which this item should be added.
 */
Rectangle::Rectangle(const QRectF& rect, SchematicScene *scene) :
   Painting(scene)
{
   setRect(rect);
   setResizeHandles(Qucs::TopLeftHandle | Qucs::BottomRightHandle |
                    Qucs::TopRightHandle| Qucs::BottomLeftHandle);
}

//! \brief Destructor.
Rectangle::~Rectangle()
{
}

//! \copydoc Painting::shapeForRect()
QPainterPath Rectangle::shapeForRect(const QRectF& rect) const
{
   QPainterPath path;
   path.addRect(boundForRect(rect));
   return path;
}

//! \copydoc Painting::boundForRect()
QRectF Rectangle::boundForRect(const QRectF &rect) const
{
   qreal adj = (pen().width() + 5) / 2;
   return rect.adjusted(-adj, -adj, adj, adj);
}

//! \brief Draw the rectangle.
void Rectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
{
   if(option->state & QStyle::State_Selected) {
      painter->setBrush(Qt::NoBrush);

      QPen _pen(pen());

      _pen.setColor(Qt::darkGray);
      _pen.setWidth(pen().width() + 5);

      painter->setPen(_pen);
      painter->drawRect(rect());

      _pen.setColor(Qt::white);
      _pen.setWidth(pen().width());
      painter->setPen(_pen);
   }
   else {
      painter->setPen(pen());
   }
   painter->setBrush(brush());
   painter->drawRect(rect());

   //call base method to draw resize handles.
   Painting::paint(painter, option, w);
}

//! \copydoc Painting::copy()
Rectangle* Rectangle::copy(SchematicScene *scene) const
{
   Rectangle *rectItem = new Rectangle(rect(), scene);
   Painting::copyDataTo(rectItem);
   return rectItem;
}

//! \brief Saves rectangle data to xml using \a writer.
void Rectangle::saveData(Qucs::XmlWriter *writer) const
{
   writer->writeStartElement("painting");
   writer->writeAttribute("name", "rectangle");

   writer->writeEmptyElement("properties");
   writer->writeRectAttribute(rect(), QLatin1String("rectangle"));
   writer->writePointAttribute(pos(), "pos");

   writer->writePen(pen());
   writer->writeBrush(brush());
   writer->writeTransform(transform());

   writer->writeEndElement(); // </painting>
}

//! \brief Loads data from xml referred by \a reader.
void Rectangle::loadData(Qucs::XmlReader *reader)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
   Q_ASSERT(reader->attributes().value("name") == "rectangle");

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "properties") {
            QRectF rectangle = reader->readRectAttribute(QLatin1String("rectangle"));
            setRect(rectangle);

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

int Rectangle::launchPropertyDialog(Qucs::UndoOption opt)
{
   StyleDialog dia(this, opt);
   return dia.exec();
}
