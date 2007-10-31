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

#include "qucssvgitem.h"
#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <QtXml>

static bool isRealNumber(char ch)
{
   return ch == '.' || (ch >= '0' && ch <= '9');
}

static qreal parseStrokeWidth(const QByteArray& styleSheet)
{
   static const QByteArray strokeText("stroke-width:");
   const int styleTextSize = styleSheet.size();
   qreal penWidth = 0.5; //default pen width

   int numBeg = styleSheet.indexOf(strokeText);
   if(numBeg != -1) {
      numBeg += strokeText.size();
      while(numBeg <= styleTextSize &&
            !isRealNumber(styleSheet.at(numBeg))) {
         ++numBeg;
      }
      if(numBeg > styleTextSize) {
         qWarning() << "Svg style sheet seems to be corrupted" << styleSheet;
         return penWidth;
      }

      int numEnd = numBeg;
      while(numEnd <= styleTextSize &&
            isRealNumber(styleSheet.at(numEnd))) {
         ++numEnd;
      }
      if(numEnd > styleTextSize) {
         qWarning() << "Svg style sheet seems to be corrupted" << styleSheet;
         return penWidth;
      }
      bool myOk;
      penWidth = styleSheet.mid(numBeg, numEnd - numBeg).toDouble(&myOk);
      if(!myOk) {
         penWidth = 0.5; //reset to default
         qWarning() << "Svg style sheet seems to be corrupted" << styleSheet;
         return penWidth;
      }
   }

   return penWidth;
}

static QRectF parseViewBox(const QByteArray& content)
{
   static const QByteArray viewBoxText("viewBox");
   //Default rect size
   QRectF viewBox(-2, -2, 4, 4);
   int s = content.indexOf(viewBoxText);
   if(s == -1) {
      qWarning() << "Parsing svg content's viewbox error" << content;
      return viewBox;
   }
   s += viewBoxText.size();
   s = content.indexOf("\"", s);
   int e = content.indexOf("\"", s+1);
   if(s == -1 || e == -1) {
      qWarning() << "Parsing svg content's viewbox error" << content;
      return viewBox;
   }

   int commaIndex = content.indexOf(',', s);
   char splitChar = ' ';
   if(commaIndex > s && commaIndex < e)
      splitChar = ',';

   QList<QByteArray> rectCoords(content.mid(s+1, e-s-1).split(splitChar));
   rectCoords.removeAll(QByteArray());
   rectCoords.removeAll(QByteArray(" "));
   rectCoords.removeAll(QByteArray(","));

   if(rectCoords.size() != 4) {
      qWarning() << "Parsing svg content's viewbox error" << content;
      return viewBox;
   }
   qreal coords[4];
   bool ok = true;
   bool myOk;
   qDebug() << "Printing rect coords" << content.mid(s+1, e-s);
   for(int i=0; i < 4; ++i) {
      qDebug() << rectCoords.at(i);
      coords[i] = rectCoords.at(i).toDouble(&myOk);
      ok &= myOk;
   }
   if(!ok) {
      qWarning() << "Parsing svg content's viewbox error" << content;
      return viewBox;
   }
   viewBox.setRect(coords[0], coords[1], coords[2], coords[3]);
   return viewBox;
}


static bool parseStyleSheet(const QByteArray &content, const QByteArray& stylesheet,
                            QByteArray &changedContent)//, QByteArray& currStyleSheet)
{
   QDomDocument doc("svg");
   QString errorMsg;
   if(!doc.setContent(content, &errorMsg)) {
      qWarning() << "SVG parse failed" << errorMsg << "\n" << content;
      return false;
   }
   QDomNode docEle = doc.documentElement();
   QDomNode defs = docEle.firstChildElement("defs");
   if(defs.isNull()) {
      defs = doc.createElement("defs");
      defs = docEle.insertBefore(defs, docEle.firstChild());
   }
   QDomNode style = defs.firstChildElement("style");
   if(style.isNull()) {
      style = doc.createElement("style");
      QDomElement ele = style.toElement();
      ele.setAttribute("type", "text/css");
      style = ele;
      style = defs.appendChild(style);
   }
   QDomNode n = style.firstChild();
   if(!n.isNull() && !n.isCDATASection()) {
      qWarning() << "Please use cdata section to specify style in svg"
                 << content;
      return false;
   }
   QDomCDATASection cdata = n.toCDATASection();
   if(cdata.isNull()) {
      cdata = doc.createCDATASection("g{\nstroke: blue; fill: lightgreen; stroke-width: 0.5}");
      cdata = style.appendChild(cdata).toCDATASection();
   }
   else {
      cdata.setData("g{\nstroke: blue; fill: lightgreen; stroke-width: 0.5}");
   }
   changedContent = doc.toByteArray();
   return true;
}

/*!\brief This method parses and returns the appropriate new bytearray.
 * \details
 */

/*static bool parseStyleSheet(const QByteArray &content, const QByteArray& _stylesheet,
                             QByteArray &changedContent, QByteArray& currStyleSheet)
{
   QDomDocument doc("svg");
   if(doc.setContent(content))
      return false;

   if(_stylesheet.isEmpty())
      return;
      }*/

QucsSvgItem::QucsSvgItem(const QString& filename, const QByteArray& _stylesheet,
                         SchematicScene *scene) : QucsItem(0, scene)
{
   m_styleSheet = _stylesheet;
   QFile file(filename);
   if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "QucsSvgItem::QucsSvgItem() : Couldn't open "
                 << filename;
   }
   else {
      m_content = file.readAll();
      file.close();
   }
   calcBoundingRect();
}

QucsSvgItem::QucsSvgItem(const QByteArray& contents,
                         const QByteArray& _stylesheet, SchematicScene *scene)
   : QucsItem(0, scene)
{
   m_styleSheet = _stylesheet;
   m_content = contents;
   calcBoundingRect();
}

QucsSvgItem::~QucsSvgItem()
{
}

QByteArray QucsSvgItem::svgContentWithStyleSheet() const
{
   //TODO:
   return m_content;
}

void QucsSvgItem::setSvgContent(const QByteArray& content)
{
   m_content = content;
   calcBoundingRect();
}

void QucsSvgItem::setSvgContent(const QString& filename)
{
   QFile file(filename);
   if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "QucsSvgItem::setSvgContent() : Couldn't open "
                 << filename;
   }
   else {
      m_content = file.readAll();
      file.close();
   }
   calcBoundingRect();
}

void QucsSvgItem::setStyleSheet(const QByteArray& _stylesheet)
{
   m_styleSheet = _stylesheet;
   calcBoundingRect();
}

void QucsSvgItem::calcBoundingRect()
{
   qreal strokeWidth = parseStrokeWidth(m_styleSheet);
   QRectF viewBox = parseViewBox(m_content);
   setShapeAndBoundRect(QPainterPath(), viewBox, strokeWidth);
}
