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

#ifndef __XMLUTILITIES_H
#define __XMLUTILITIES_H

#include <QtXml/QXmlStreamReader>
#include <QtCore/QMap>

class QXmlStreamWriter;

class QRectF;
class QSize;
class QTransform;

namespace Qucs
{
   enum Transformation {
      m11, m12, m13,
      m21, m22, m23,
      m31, m32, m33
   };

   const QMap<QString,Transformation>& transformMap();

   class XmlReader : public QXmlStreamReader
   {
      public:
         XmlReader(QIODevice * device) : QXmlStreamReader(device) {}
         XmlReader(const QByteArray & data) : QXmlStreamReader(data) {}
         XmlReader(const QString & data) : QXmlStreamReader(data) {}
         ~XmlReader() {}

         int readInt(const QString& tag = QString());
         double readDouble(const QString& tag = QString());
         QString readText(const QString& tag);

         QPointF readPoint(const QString& tag = QString("point"));
         QSize readSize(const QString& tag = QString("size"));
         QRectF readRect(const QString& tag = QString("rect"));
         QTransform readTransform(const QString& tag = QString("transform"));

         void readFurther();
         void readUnknownElement();
   };

   class XmlWriter : public QXmlStreamWriter
   {
      public:
         XmlWriter(QIODevice *device) : QXmlStreamWriter(device) {}
         XmlWriter(QByteArray *bytearray) : QXmlStreamWriter(bytearray) {}
         XmlWriter(QString *string) : QXmlStreamWriter(string) {}

         ~XmlWriter() {}
   };

   void writeElement(QXmlStreamWriter *writer, const QString& tag, const QString& value);
   void writeElement(QXmlStreamWriter *writer, const QString& tag, int value);
   void writeElement(QXmlStreamWriter *writer, const QString& tag, qreal value);
   void writeElement(QXmlStreamWriter *writer, const QString& tag, bool value);

   void writeRect(QXmlStreamWriter *writer, const QRectF& rect);
   void writeTransform(QXmlStreamWriter *writer, const QTransform& transform);
   void writeSize(QXmlStreamWriter *writer, const QSize& size);
   void writePoint(QXmlStreamWriter *writer, const QPointF& point);

}

#endif //__XMLUTILITIES_H
