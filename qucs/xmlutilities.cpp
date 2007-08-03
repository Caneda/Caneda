/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include <QtXml/QXmlStreamWriter>
#include <QtXml/QXmlStreamReader>

#include <QtCore/QRectF>
#include <QtCore/QSize>

#include <QtGui/QTransform>

#include "xmlutilities.h"
#include "qucs-tools/global.h"

namespace Qucs
{
   const QMap<QString,Transformation>& transformMap()
   {
      static QMap<QString,Transformation> _map;
      if(_map.isEmpty()) {
         _map[QString("m11")] = Qucs::m11;
         _map[QString("m12")] = Qucs::m12;
         _map[QString("m13")] = Qucs::m13;
         _map[QString("m21")] = Qucs::m21;
         _map[QString("m22")] = Qucs::m22;
         _map[QString("m23")] = Qucs::m23;
         _map[QString("m31")] = Qucs::m31;
         _map[QString("m32")] = Qucs::m32;
         _map[QString("m33")] = Qucs::m33;
      }
      return _map;
   }

   void XmlReader::readUnknownElement()
   {
      Q_ASSERT(isStartElement());

      while (!atEnd()) {
         readNext();

         if (isEndElement())
            break;

         if (isStartElement())
            readUnknownElement();
      }
   }

   int XmlReader::readInt(const QString& tag)
   {
      int retVal = 0;
      bool ok = false;
      retVal = readText(tag).toInt(&ok);
      if(!ok && !hasError()) {
         raiseError(QObject::tr("Couldn't read int element. Text malformatted."));
      }
      return retVal;
   }

   double XmlReader::readDouble(const QString& tag)
   {
      double retVal = 0;
      bool ok = false;
      retVal = readText(tag).toDouble(&ok);
      if(!ok && !hasError()) {
         raiseError(QObject::tr("Couldn't read double element. Text malformatted."));
      }
      return retVal;
   }

   QString XmlReader::readText(const QString& tag)
   {
      QString retVal;
      if(!isStartElement() || (!tag.isEmpty() && name().toString() != tag)) {
         raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(name().toString()).arg(tag));
      }
      else
         retVal = readElementText();
      return retVal;
   }

   QPointF XmlReader::readPoint(const QString& tag)
   {
      QPointF retVal;
      if(!isStartElement() || tag != name()) {
         raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(name().toString()).arg(tag));
      }

      while(!atEnd()) {
         readNext();

         if(isEndElement())
            break;

         if(isStartElement()) {
            if(name() == "x")
               retVal.setX(readDouble("x"));
            else if(name() == "y")
               retVal.setY(readDouble("y"));
            else
               readUnknownElement();
         }
      }

      return retVal;
   }

   QSize XmlReader::readSize(const QString& tag)
   {
      QSize retVal;
      if(!isStartElement() || tag != name()) {
         raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(name().toString()).arg(tag));
      }

      while(!atEnd()) {
         readNext();

         if(isEndElement())
            break;

         if(isStartElement()) {
            if(name() == "width")
               retVal.setWidth(readInt("width"));
            else if(name() == "height")
               retVal.setHeight(readInt("height"));
            else
               readUnknownElement();
         }
      }

      return retVal;
   }

   QRectF XmlReader::readRect(const QString& tag)
   {
      QRectF rect;
      if(!isStartElement() || tag != name()) {
         raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(name().toString()).arg(tag));
      }

      while(!atEnd()) {
         readNext();

         if(isEndElement())
            break;

         if(isStartElement()) {
            if(name() == "x")
               rect.setX(readDouble("x"));
            else if(name() == "y")
               rect.setY(readDouble("y"));
            else if(name() == "width")
               rect.setWidth(readDouble("width"));
            else if(name() == "height")
               rect.setHeight(readDouble("height"));
            else
               readUnknownElement();
         }
      }

      return rect;
   }

   QTransform XmlReader::readTransform(const QString& tag)
   {
      if(!isStartElement() || tag != name()) {
         raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(name().toString()).arg(tag));
      }

      const QMap<QString, Transformation>& _map = Qucs::transformMap();
      QMap<Transformation,qreal> retVal;
      retVal[m11] = 1.;
      retVal[m12] = 0.;
      retVal[m13] = 1.;
      retVal[m21] = 0.;
      retVal[m22] = 1.;
      retVal[m23] = 0.;
      retVal[m31] = 0.;
      retVal[m32] = 0.;
      retVal[m33] = 0.;

      while(!atEnd()) {
         readNext();

         if(isEndElement())
            break;

         if(isStartElement()) {
            if(_map.contains(name().toString())) {
               Transformation t = _map[name().toString()];
               retVal[t] = readDouble(name().toString());
            }
            else
               readUnknownElement();
         }
      }

      return QTransform(retVal[m11], retVal[m12], retVal[m13],
                        retVal[m21], retVal[m22], retVal[m23],
                        retVal[m31], retVal[m32], retVal[m33]);

   }


   void XmlReader::readFurther()
   {
      readNext();
      if(isWhitespace())
         readNext();
   }

   void writeElement(QXmlStreamWriter *writer, const QString& tag, const QString& value)
   {
      writer->writeTextElement(tag, value);
   }

   void writeElement(QXmlStreamWriter *writer, const QString& tag, int value)
   {
      writeElement(writer, tag, QString::number(value));
   }

   void writeElement(QXmlStreamWriter *writer, const QString& tag, qreal value)
   {
      writeElement(writer, tag, Qucs::realToString(value));
   }

   void writeElement(QXmlStreamWriter *writer, const QString& tag, bool value)
   {
      writeElement(writer, tag, Qucs::boolToString(value));
   }

   void writeRect(QXmlStreamWriter *writer, const QRectF& rect)
   {
      writer->writeStartElement("rect");
      writeElement(writer, "x", rect.x());
      writeElement(writer, "y", rect.y());
      writeElement(writer, "width", rect.width());
      writeElement(writer, "height", rect.height());
      writer->writeEndElement(); //</rect>
   }

   void writeTransform(QXmlStreamWriter *writer, const QTransform& transform)
   {
      writer->writeStartElement("transform");
      writeElement(writer, "m11", transform.m11());
      writeElement(writer, "m12", transform.m12());
      writeElement(writer, "m13", transform.m13());
      writeElement(writer, "m21", transform.m21());
      writeElement(writer, "m22", transform.m22());
      writeElement(writer, "m23", transform.m23());
      writeElement(writer, "m31", transform.m31());
      writeElement(writer, "m32", transform.m32());
      writeElement(writer, "m33", transform.m33());
      writer->writeEndElement(); //</transform>
   }

   void writeSize(QXmlStreamWriter *writer, const QSize& size)
   {
      writer->writeStartElement("size");
      writeElement(writer, "width", size.width());
      writeElement(writer, "height", size.height());
      writer->writeEndElement(); //</size>
   }

   void writePoint(QXmlStreamWriter *writer, const QPointF& point)
   {
      writer->writeStartElement("point");
      writeElement(writer, "x", point.x());
      writeElement(writer, "y", point.y());
      writer->writeEndElement(); //</point>
   }
}
