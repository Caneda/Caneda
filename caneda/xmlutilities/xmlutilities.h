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

#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include "QXmlStreamReaderExt.h"

#include <QMap>
#include <QPolygonF>
#include <QXmlStreamWriter>

// Forward declarations
class QBrush;
class QFont;
class QLineF;
class QPen;
class QRectF;
class QSize;
class QTransform;

namespace Qucs
{
    /*!
     * \brief This class adds a few more helper methods to qt's QXmlStreamReader class.
     * \warning QXmlStreamReader doesn't have virtual destructor. Don't delete any
     * instance of this class from base pointer.
     */
    class XmlReader : public QXmlStreamReaderExt
    {
    public:
        //! Constructs an xml stream reader acting on \a device.
        //XmlReader(QIODevice * device) : QXmlStreamReader(device) {}
        //! Constructs an xml stream reader acting on \a data.
        XmlReader(const QByteArray & data,
                const QRelaxNGvalidator * schema = NULL,
                const QXsltTransformer * xslt = NULL)
            : QXmlStreamReaderExt(data, schema, xslt) {}
        //! Constructs an xml stream reader acting on \a data.
        //XmlReader(const QString & data) : QXmlStreamReader(data) {}

        int readInt();
        double readDouble();

        QPointF readPoint();

        QPointF readPointAttribute(QString tag = "point");
        QLineF readLineAttribute(QString tag="line");
        QRectF readRectAttribute(QLatin1String tag = QLatin1String("rect"));
        qreal readDoubleAttribute(QString tag);

        QSize readSize();

        QRectF readRect();
        QTransform readTransform();

        QPen readPen();
        QBrush readBrush();
        QFont readFont();

        QString readLocaleText(const QString& localePrefix = QString("C"));

        void readFurther();
        void readUnknownElement();

        QString readXmlFragment();
    };

    /*!
     * \brief This class adds a few more helper methods to qt's
     * QXmlStreamWriter class.
     * \warning QXmlStreamWriter doesn't have virtual destructor. Don't delete any
     * instance of this class from base pointer.
     */
    class XmlWriter : public QXmlStreamWriter
    {
    public:
        //! Constructs an xml stream writer acting on \a device.
        XmlWriter(QIODevice *device) : QXmlStreamWriter(device) {}
        //! Constructs an xml stream writer acting on \a bytearray.
        XmlWriter(QByteArray *bytearray) : QXmlStreamWriter(bytearray) {}
        //! Constructs an xml stream writer acting on \a string.
        XmlWriter(QString *string) : QXmlStreamWriter(string) {}

        void writeElement(const QString& tag, const QString& value);
        void writeElement(const QString& tag, int value);
        void writeElement(const QString& tag, qreal value);
        void writeElement(const QString& tag, bool value);

        void writeRect(const QRectF& rect, QString tag = "rect");
        void writeTransform(const QTransform& transform);
        void writeSize(const QSize& size, QString tag = "size");
        void writePoint(const QPointF& point, QString tag = "point");

        void writePointAttribute(const QPointF& point, QString tag = "point");
        void writeLineAttribute(const QLineF& line, QLatin1String tag = QLatin1String("line"));
        void writeRectAttribute(const QRectF& rect, QLatin1String tag = QLatin1String("rect"));
        void writePolygonAttribute(const QPolygonF& polygon, QString tag="polygon");

        void writePen(const QPen& pen, QLatin1String tag = QLatin1String("pen"));
        void writeBrush(const QBrush& brush, QLatin1String tag = QLatin1String("brush"));
        void writeFont(const QFont& font, QLatin1String tag = QLatin1String("font"));

        void writeLocaleText(const QString &lang, const QString& value);
    };
}

#endif //XMLUTILITIES_H
