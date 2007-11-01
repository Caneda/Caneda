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
#include "qucssvgrenderer.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <QtSvg/QSvgRenderer>
#include <QtXml>

static const QByteArray defaultStyle = "g[id]{stroke: black; fill: none; stroke-width: 0.5}";

static bool isRealNumber(char ch)
{
   return ch == '.' || (ch >= '0' && ch <= '9');
}

/*!\internal
 * \brief This method returns a new bytearray with its stylesheet content
 * set to \a stylesheet.
 * \details If the \a content already has stylesheet tag in it, then this
 * method modifies the value of that to \a stylesheet.
 * \param content The bytearray consisting of svg content.
 * \param stylesheet The stylesheet which is to be inserted.
 * \return bytearray consisting of svg with style info set to stylesheet,
 * Returns empty bytearray if error.
 */
static QByteArray insertStyleSheet(const QByteArray &content,
                                   const QByteArray& stylesheet)
{
   // No new stylesheet to insert.
   if(stylesheet.isEmpty()) {
      return content;
   }
   QDomDocument doc("svg");
   QString errorMsg;
   if(!doc.setContent(content, &errorMsg)) {
      qWarning() << "SVG parse failed" << errorMsg << "\n" << content;
      return QByteArray();
   }

   QDomNode docEle = doc.documentElement();
   //style tag should be in defs tag
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
      style = defs.insertBefore(ele, defs.firstChild());
   }
   QDomNode n = style.firstChild();
   QDomCDATASection cdata = n.toCDATASection();

   if(!n.isNull() && !n.isCDATASection()) {
      qWarning() << "Please use cdata section to specify style in svg"
                 << content;
      //reset cdata to null node to force insertion as cdata.
      cdata = QDomCDATASection();
   }

   if(cdata.isNull()) {
      cdata = doc.createCDATASection(stylesheet);
      cdata = style.insertBefore(cdata, style.firstChild()).toCDATASection();
   }
   else {
      cdata.setData(stylesheet);
   }

   return doc.toByteArray();
}

/*!\internal
 * \brief This method returns the bounding rect of given svg \a content.
 * \param content Bytearray corresponding to svg.
 * \param id The node with id whose rect is required.
 */
static QRectF getBounds(const QByteArray& content, const QString& id)
{
   QSvgRenderer renderer(content);
   if(renderer.isValid()) {
      return renderer.boundsOnElement(id);
   }
   return QRectF();
}

/*!\internal
 * \brief This method extracts stylesheet content from given svg \a content.
 * \details This also parses and checks for any violations of svg related
 * to css styling.
 * \param content Bytearray corresponding to svg.
 * \return An empty byte array if any error occurs, else stylesheet.
 */
static QByteArray getStyleSheet(const QByteArray &content)
{
   QDomDocument doc("svg");
   QString errorMsg;
   if(!doc.setContent(content, &errorMsg)) {
      qWarning() << "SVG parse failed" << errorMsg << "\n" << content;
      return QByteArray();
   }

   QDomNode docEle = doc.documentElement();
   //style tag should be in defs tag
   QDomNode defs = docEle.firstChildElement("defs");
   if(defs.isNull()) {
      qWarning() << "getStyleSheet() : " << "No defs tag found"
                 << "\n" << content;
      return QByteArray();
   }

   QDomNode style = defs.firstChildElement("style");
   if(style.isNull()) {
      qWarning() << "getStyleSheet() : " << "No style tag found"
                 << "\n" << content;
      return QByteArray();
   }

   QDomNode n = style.firstChild();
   QDomCDATASection cdata = n.toCDATASection();

   if(n.isNull() || !n.isCDATASection()) {
      qWarning() << "Please use cdata section to specify style in svg"
                 << "\n" << content;
      return QByteArray();
   }

   return cdata.data().toAscii();
}

/*!\internal
 * \brief This method returns the stroke width of the svg \a content.
 * \param content The actual svg file with style info in it.
 * \param ok If non null, this will indicate success of this method.
 * \return stroke width.
 */
static qreal getPenWidth(const QByteArray &content, bool *ok = 0)
{
   QByteArray stylesheet = getStyleSheet(content).simplified();
   stylesheet.replace(QByteArray(" "),"");

   int indStart = stylesheet.indexOf("{");
   int indEnd = stylesheet.indexOf("}", indStart);

   if(indStart == -1 || indEnd == -1 || indEnd < indStart) {
      if(ok) *ok = false;
      return 0.0;
   }
   QList<QByteArray> attrList = stylesheet.mid(indStart+1, indEnd - indStart)
      .split(';');
   foreach(QByteArray attr, attrList) {
      int index = attr.indexOf("stroke-width");
      if(index != -1) {
         index = attr.indexOf(':', index);
         if(index == -1) {
            if(ok) *ok = false;
            return 0.0;
         }
         return attr.right(attr.size() - index -1).toDouble(ok);
      }
   }
   if(ok)
      *ok = false;
   return 0.0;
}


QucsSvgItem::QucsSvgItem(const QString& filename, const QString& id,
                         SchematicScene *scene)
   : QucsItem(0, scene),
     m_uniqueId(id)
{
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

QucsSvgItem::QucsSvgItem(const QString& filename, const QString& id,
                         const QByteArray& _stylesheet,
                         SchematicScene *scene)
   : QucsItem(0, scene),
     m_uniqueId(id)
{
   QFile file(filename);
   if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "QucsSvgItem::QucsSvgItem() : Couldn't open "
                 << filename;
   }
   else {
      m_content = file.readAll();
      if(!_stylesheet.isEmpty()) {
         m_content = insertStyleSheet(m_content, _stylesheet);
      }
      file.close();
   }
   calcBoundingRect();

}

QucsSvgItem::QucsSvgItem(const QByteArray& contents, const QString& id,
                         SchematicScene *scene)
   : QucsItem(0, scene),
     m_uniqueId(id)
{
   m_content = contents;
   calcBoundingRect();
}

QucsSvgItem::QucsSvgItem(const QByteArray& contents, const QString& id,
                         const QByteArray& _stylesheet, SchematicScene *scene)
   : QucsItem(0, scene),
     m_uniqueId(id)
{
   m_content = contents;
   if(!_stylesheet.isEmpty()) {
      m_content = insertStyleSheet(m_content, _stylesheet);
   }
   calcBoundingRect();
}

QucsSvgItem::~QucsSvgItem()
{
}

void QucsSvgItem::setSvgContent(const QByteArray& content)
{
   m_content = content;
   calcBoundingRect();
   QucsSvgRenderer::reloadSvgFor(this);
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
   QucsSvgRenderer::reloadSvgFor(this);
}

QByteArray QucsSvgItem::styleSheet() const
{
   return getStyleSheet(m_content);
}

void QucsSvgItem::setStyleSheet(const QByteArray& _stylesheet)
{
   if(!_stylesheet.isEmpty()) {
      m_content = insertStyleSheet(m_content, _stylesheet);
      //qDebug() << m_content;
   }
   calcBoundingRect();
   QucsSvgRenderer::reloadSvgFor(this);
}

void QucsSvgItem::calcBoundingRect()
{
   bool ok;
   if(getStyleSheet(m_content).isEmpty())
      m_content = insertStyleSheet(m_content, defaultStyle);
   m_strokeWidth = getPenWidth(m_content, &ok);
   QRectF bound = getBounds(m_content, m_uniqueId);
   if(!ok || bound.isNull()) {
      qWarning() << "QucsSvgItem::calcBoundingRect() : Data parse error";
   }
   setShapeAndBoundRect(QPainterPath(), bound, m_strokeWidth);
}

void QucsSvgItem::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem * option, QWidget *)
{
   QucsSvgRenderer::render(painter, this);
}
