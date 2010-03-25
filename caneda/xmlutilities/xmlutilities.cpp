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

#include "xmlutilities.h"

#include "qucs-tools/global.h"

#include <QBrush>
#include <QDebug>
#include <QLineF>
#include <QPen>
#include <QRectF>
#include <QSize>
#include <QTransform>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Qucs
{
    //! Just skips through unknown tag by reading and discarding tokens parsed.
    void XmlReader::readUnknownElement()
    {
        Q_ASSERT(isStartElement());

        QString startName = name().toString();
        int depth = 1;

        while(!atEnd()) {
            readNext();

            if(isEndElement() && name() == startName && --depth <= 0) {
                break;
            }

            if(isStartElement() && name() == startName) {
                ++depth;
            }
        }

        Q_ASSERT(depth == 0);
    }

    /*!
     * \brief Returns an xml fragment, as string. Thus embedded svgs for
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

            if(isEndElement() && name() == startName && --depth <= 0) {
                break;
            }

            else if(isStartElement() && name() == startName) {
                depth++;
            }

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

    QLineF XmlReader::readLineAttribute(QString tag)
    {
        Q_ASSERT(isStartElement());
        QString errorString = QObject::tr("Invalid line attribute");
        QStringList lineCoordsStr = attributes().value(tag).toString().
            split(",",QString::SkipEmptyParts);

        if(lineCoordsStr.size() != 4) {
            raiseError(errorString);
            return QLineF();
        }

        qreal lineCoords[4];
        bool ok;
        for(int i=0; i < 4; ++i) {
            lineCoords[i] = lineCoordsStr.at(i).trimmed().toDouble(&ok);
            if(!ok) {
                raiseError(errorString);
                return QLineF();
            }
        }

        return QLineF(lineCoords[0], lineCoords[1], lineCoords[2], lineCoords[3]);
    }

    QRectF XmlReader::readRectAttribute(QLatin1String tag)
    {
        Q_ASSERT(isStartElement());
        QString errorString = QObject::tr("Invalid rect attribute");
        QStringList rectCoordsStr = attributes().value(tag).toString().
            split(",",QString::SkipEmptyParts);

        if(rectCoordsStr.size() != 4) {
            raiseError(errorString);
            return QRectF();
        }

        qreal rectCoords[4];
        bool ok;
        for(int i=0; i < 4; ++i) {
            rectCoords[i] = rectCoordsStr.at(i).trimmed().toDouble(&ok);
            if(!ok) {
                raiseError(errorString);
                return QRectF();
            }
        }

        return QRectF(rectCoords[0], rectCoords[1], rectCoords[2], rectCoords[3]);
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

    QPen XmlReader::readPen()
    {
        Q_ASSERT(isStartElement());

        bool ok1, ok2;
        QColor color(attributes().value("color").toString());
        int width(attributes().value("width").toString().toInt(&ok1));
        int style(attributes().value("style").toString().toInt(&ok2));

        QPen retVal;
        if(color.isValid() && ok1 && ok2) {
            retVal =  QPen(color, width, (Qt::PenStyle)style);
        }
        else {
            raiseError(QObject::tr("Invalid pen attribute"));
        }

        readUnknownElement();//read till end tag

        return retVal;
    }

    QBrush XmlReader::readBrush()
    {
        Q_ASSERT(isStartElement());

        bool ok;
        QColor color(attributes().value("color").toString());
        int style(attributes().value("style").toString().toInt(&ok));

        QBrush brush;
        if(color.isValid() && ok) {
            brush = QBrush(color, (Qt::BrushStyle)style);
        }
        else {
            raiseError(QObject::tr("Invalid brush attribute"));
        }

        readUnknownElement(); //read till end tag

        return brush;
    }

    QFont XmlReader::readFont()
    {
        Q_ASSERT(isStartElement());

        QXmlStreamAttributes attribs = attributes();
        QFont font;
        font.setFamily(attribs.value("family").toString());

        bool ok1, ok2, ok3;

        int pointSize(attribs.value("pointSize").toString().toInt(&ok1));
        int pixelSize(attribs.value("pixelSize").toString().toInt(&ok2));
        if(pointSize == -1) {
            font.setPixelSize(pixelSize);
        }
        else {
            font.setPointSize(pointSize);
        }

        font.setWeight(attribs.value("weight").toString().toInt(&ok3));

        readUnknownElement(); //read till end tag

        if(!ok1 || !ok2 || !ok3) {
            raiseError(QObject::tr("Invalid font attribute"));
        }

        return font;
    }

    QString XmlReader::readLocaleText(const QString& localePrefix)
    {
        QString c, actual;
        Q_ASSERT(isStartElement());

        bool matchFound = false;
        while(!atEnd()) {
            readNext();

            if(isEndElement()) {
                break;
            }

            if(isStartElement() && (matchFound || name() != "lang")) {
                readUnknownElement();
            }

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
        if(actual.isEmpty()) {
            return c;
        }
        return actual;
    }

    void XmlReader::readFurther()
    {
        readNext();
        if(isWhitespace()) {
            readNext();
        }
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

    void XmlWriter::writeLineAttribute(const QLineF& line, QLatin1String tag)
    {
        QString lineStr = QString("%1,%2,%3,%4")
            .arg(line.x1())
            .arg(line.y1())
            .arg(line.x2())
            .arg(line.y2());
        writeAttribute(tag, lineStr);
    }

    void XmlWriter::writeRectAttribute(const QRectF& rect, QLatin1String tag)
    {
        QString rectStr = QString("%1,%2,%3,%4")
            .arg(rect.x())
            .arg(rect.y())
            .arg(rect.width())
            .arg(rect.height());
        writeAttribute(tag, rectStr);
    }

    void XmlWriter::writePen(const QPen& pen, QLatin1String tag)
    {
        writeEmptyElement(tag);
        writeAttribute("width", QString::number(pen.width()));
        writeAttribute("color", pen.color().name());
        writeAttribute("style", QString::number((int)pen.style()));
    }

    void XmlWriter::writeBrush(const QBrush& brush, QLatin1String tag)
    {
        writeEmptyElement(tag);
        writeAttribute("color", brush.color().name());
        writeAttribute("style", QString::number((int)brush.style()));
    }

    void XmlWriter::writeFont(const QFont& font, QLatin1String tag)
    {
        writeEmptyElement(tag);
        writeAttribute("family", font.family());
        writeAttribute("pixelSize", QString::number(font.pixelSize()));
        writeAttribute("pointSize", QString::number(font.pointSize()));
        writeAttribute("weight", QString::number(font.weight()));
    }

    void XmlWriter::writeLocaleText(const QString &lang, const QString& value)
    {
        writeStartElement("lang");
        writeAttribute("lang", lang);
        writeCharacters(value);
        writeEndElement();
    }
}
