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
#include <QtCore/QDebug>

#include <QtGui/QTransform>

#include "xmlutilities.h"
#include "qucs-tools/global.h"

namespace Qucs
{
   //! Just skips through unknown tag by reading and discarding tokens parsed.
   void XmlReader::readUnknownElement()
   {
      Q_ASSERT(isStartElement());

      QString startName = name().toString();
      int depth = 1;

      while (!atEnd()) {
         readNext();

         if (isEndElement() && name() == startName && --depth <= 0)
            break;

         if (isStartElement() && name() == startName)
            ++depth;
      }

      Q_ASSERT(depth == 0);
   }

   /*! \brief Returns an xml fragment, as string. Thus embedded svgs for
    * example can be extracted from an xml file.
    */
   QString XmlReader::readXmlFragment()
   {
      Q_ASSERT(isStartElement());

      QString retVal;
      int depth = 1;
      Qucs::XmlWriter writer(&retVal);

      writer.setCodec("UTF-8");

      // Write the starting tag
      writer.writeCurrentToken(*this);
      QString startName = name().toString();

      // Loop through the tokens till the starting tag's end element is found.
      while(!atEnd()) {
         readNext();

         if(isEndElement() && name() == startName && --depth <= 0)
            break;

         else if(isStartElement() && name() == startName)
            depth++;

         // write the token
         writer.writeCurrentToken(*this);
      }
      // Make sure depth is 0
      Q_ASSERT(depth == 0);

      writer.writeCurrentToken(*this);
      return retVal;
   }

   int XmlReader::readInt()
   {
      bool ok;
      QString text = readElementText();
      int retVal = text.toInt(&ok);
      if(!ok) {
         raiseError(QObject::tr("Expected int but found %1").arg(text));
      }
      return retVal;
   }

   double XmlReader::readDouble()
   {
      bool ok;
      QString text = readElementText();
      double retVal = text.toDouble(&ok);
      if(!ok) {
         raiseError(QObject::tr("Expected double but found %1").arg(text));
      }
      return retVal;
   }

   qreal XmlReader::readDoubleAttribute(QString tag)
   {
      bool ok;
      qreal val = attributes().value(tag).toString().toDouble(&ok);
      Q_ASSERT(ok);
      return val;
   }

   QPointF XmlReader::readPoint()
   {
      Q_ASSERT(isStartElement());

      bool ok1, ok2;
      QXmlStreamAttributes attribs = attributes();

      qreal x = attribs.value("x").toString().toDouble(&ok1);
      qreal y = attribs.value("y").toString().toDouble(&ok2);

      if(!ok1 || !ok2) {
         raiseError(QObject::tr("String to double conversion failed"));
      }

      // read till end tag
      readUnknownElement();

      return QPointF(x, y);
   }

   QPointF XmlReader::readPointAttribute(QString tag)
   {
      Q_ASSERT(isStartElement());
      QString pointStr = attributes().value(tag).toString();
      int commaPos = pointStr.indexOf(',');
      Q_ASSERT(commaPos != -1);

      QPointF point;
      bool ok;
      point.setX(pointStr.left(commaPos).toDouble(&ok));
      Q_ASSERT(ok);
      point.setY(pointStr.mid(commaPos+1).toDouble(&ok));
      Q_ASSERT(ok);

      return point;
   }

   QSize XmlReader::readSize()
   {
      Q_ASSERT(isStartElement());

      bool ok1, ok2;
      QXmlStreamAttributes attribs = attributes();

      int w = attribs.value("width").toString().toInt(&ok1);
      int h = attribs.value("height").toString().toInt(&ok2);

      if(!ok1 || !ok2) {
         raiseError(QObject::tr("String to int conversion failed"));
      }

      // read till end tag
      readUnknownElement();

      return QSize(w, h);
   }

   QRectF XmlReader::readRect()
   {
      Q_ASSERT(isStartElement());

      bool ok1, ok2, ok3, ok4;
      QXmlStreamAttributes attribs = attributes();

      qreal x = attribs.value("x").toString().toDouble(&ok1);
      qreal y = attribs.value("y").toString().toDouble(&ok2);
      qreal w = attribs.value("width").toString().toDouble(&ok3);
      qreal h = attribs.value("height").toString().toDouble(&ok4);

      if(!ok1 || !ok2 || !ok3 || !ok4) {
         raiseError(QObject::tr("String to double conversion failed"));
      }

      // read till end tag
      readUnknownElement();

      return QRectF(x, y, w, h);
   }

   QTransform XmlReader::readTransform()
   {
      Q_ASSERT(isStartElement());

      QString matrix = attributes().value("matrix").toString();
      QStringList eleStr = matrix.split(',');

      if(eleStr.size() != 6) {
         raiseError(QObject::tr("Invalid transform matrix %1").arg(matrix));
         return QTransform();
      }

      bool ok, finalOk = true;
      qreal ele[6];
      for(int i = 0; i < 6; ++i) {
         ele[i] = eleStr[i].toDouble(&ok);
         finalOk = finalOk && ok;
      }

      if(!finalOk) {
         raiseError(QObject::tr("Invalid transform matrix %1").arg(matrix));
         return QTransform();
      }

      // read till end tag
      readUnknownElement();

      QTransform svgLike
         (ele[0], ele[2], ele[4],
          ele[1], ele[3], ele[5],
          0,      0,          1);

      QTransform retVal = svgLike.inverted(&ok);

      if(!ok) {
         qWarning() << Q_FUNC_INFO << "Singular matrix found";
      }
      return retVal;
   }

   QString XmlReader::readLocaleText(const QString& localePrefix)
   {
      QString c, actual;
      Q_ASSERT(isStartElement());

      bool matchFound = false;
      while(!atEnd()) {
         readNext();

         if(isEndElement())
            break;

         if(isStartElement() && (matchFound || name() != "lang"))
            readUnknownElement();

         if(!matchFound && isStartElement() && name() == "lang") {
            QString lang = attributes().value("lang").toString();
            if(lang == "C") {
               c = readElementText();
               if(localePrefix == "C") {
                  matchFound = true;
               }
            }
            else if(lang == localePrefix) {
               actual = readElementText();
               matchFound = true;
            }
            else {
               readUnknownElement();
            }
         }
      }
      if(actual.isEmpty())
         return c;
      return actual;
   }

   void XmlReader::readFurther()
   {
      readNext();
      if(isWhitespace())
         readNext();
   }

   void XmlWriter::writeElement(const QString& tag, const QString& value)
   {
      writeTextElement(tag, value);
   }

   void XmlWriter::writeElement(const QString& tag, int value)
   {
      writeTextElement(tag, QString::number(value));
   }

   void XmlWriter::writeElement(const QString& tag, qreal value)
   {
      writeTextElement(tag, Qucs::realToString(value));
   }

   void XmlWriter::writeElement(const QString& tag, bool value)
   {
      writeTextElement(tag, Qucs::boolToString(value));
   }

   void XmlWriter::writeRect(const QRectF& rect, QString tag)
   {
      writeEmptyElement(tag);
      writeAttribute("x", QString::number(rect.x()));
      writeAttribute("y", QString::number(rect.y()));
      writeAttribute("width", QString::number(rect.width()));
      writeAttribute("height", QString::number(rect.height()));
   }

   void XmlWriter::writeTransform(const QTransform& transform)
   {
      writeEmptyElement("transform");
      bool ok;
      QTransform svgLike = transform.inverted(&ok);
      if(!ok) {
         qWarning() << Q_FUNC_INFO << "Singular matrix found";
      }

      QStringList svgVec;

      svgVec << QString::number(svgLike.m11())
             << QString::number(svgLike.m21())
             << QString::number(svgLike.m12())
             << QString::number(svgLike.m22())
             << QString::number(svgLike.m13())
             << QString::number(svgLike.m23());

      writeAttribute("matrix", svgVec.join(","));
   }

   void XmlWriter::writeSize(const QSize& size, QString tag)
   {
      writeEmptyElement(tag);
      writeAttribute("width", QString::number(size.width()));
      writeAttribute("height", QString::number(size.height()));
   }

   void XmlWriter::writePoint(const QPointF& point, QString tag)
   {
      writeEmptyElement(tag);
      writeAttribute("x", QString::number(point.x()));
      writeAttribute("y", QString::number(point.y()));
   }

   void XmlWriter::writePointAttribute(const QPointF& point, QString tag)
   {
      QString pointStr = QString("%1,%2").arg(point.x()).arg(point.y());
      writeAttribute(tag, pointStr);
   }
}
