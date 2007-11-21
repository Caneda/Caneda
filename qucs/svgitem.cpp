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

#include "svgitem.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <QtSvg/QSvgRenderer>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtXml>

static const QByteArray defaultStyle = "g[id]{stroke: black; fill: none; stroke-width: 0.5}";

/*****************************************************************************
 *                            HELPER METHODS                                 *
 *****************************************************************************/

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
   if(ok) {
      *ok = false;
      qWarning() << "Couldn't obtain pen width from the stylesheet:\n"
                 << stylesheet;
   }
   return 0.0;
}

/*!\internal
 * \brief This code draws the highlighted rect around the item
 * \note This code is stolen from source of qt ;-)
 */
static void highlightSelectedSvgItem(
   SvgItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
{
   const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
    if (qFuzzyCompare(qMax(murect.width(), murect.height()), qreal(0.0)))
        return;

    const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
    if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
        return;

    qreal itemPenWidth = item->strokeWidth();
    const qreal pad = itemPenWidth / 2;
    const qreal penWidth = 0; // cosmetic pen

    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( // ensure good contrast against fgcolor
        fgcolor.red()   > 127 ? 0 : 255,
        fgcolor.green() > 127 ? 0 : 255,
        fgcolor.blue()  > 127 ? 0 : 255);

    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}

/*****************************************************************************
 *                          SvgItemData methods                              *
 *****************************************************************************/

SvgItemData::SvgItemData(const QString& _groupId, const QByteArray& _content) :
   groupId(_groupId),
   content(_content),
   cachedStrokeWidth(getPenWidth(content)),
   renderer(content),
   pixmapDirty(true)
{
   Q_ASSERT(renderer.isValid());
   Q_ASSERT(!content.isEmpty());
}

void SvgItemData::setStyleSheet(const QByteArray& stylesheet)
{
   if(getStyleSheet(content).isEmpty()) {
      qWarning() << "SvgItemData::setStyleSheet()  :  "
                 << "Cannot apply stylesheet to svg without stylesheet present"
                 << " beforehand in the svg";
      return;
   }
   content = insertStyleSheet(content, stylesheet);
   pixmapDirty = true;
   cachedStrokeWidth = getPenWidth(content);

   renderer.load(content);
   Q_ASSERT(renderer.isValid());
}

QByteArray SvgItemData::styleSheet() const
{
   return getStyleSheet(content);
}

/*****************************************************************************
 *                           SvgPainter methods                              *
 *****************************************************************************/

SvgPainter::SvgPainter()
{
   m_cachingEnabled = true;
}

SvgPainter::~SvgPainter()
{
   DataHash::iterator it = m_dataHash.begin(), end = m_dataHash.end();
   while(it != end) {
      delete it.value();
      it.value() = 0;
      ++it;
   }
}

void SvgPainter::registerSvg(const QString& groupId, const QByteArray& svg)
{
   Q_ASSERT(!groupId.isEmpty());

   if(isSvgRegistered(groupId)) {
      return;
   }

   m_dataHash[groupId] = new SvgItemData(groupId, svg);
}

QSvgRenderer* SvgPainter::rendererFor(const QString& gid) const
{
   return &(svgData(gid)->renderer);
}

QRectF SvgPainter::boundingRect(const QString& gid) const
{
   return svgData(gid)->boundingRect();
}

void SvgPainter::paint(QPainter *painter, const QString& gid)
{
   SvgItemData *data = svgData(gid);
   QMatrix m = painter->worldMatrix();
   QRect deviceRect = m.mapRect(data->boundingRect()).toRect();

   // If Caching disabled or if there is transformation render without cache.
   if (!isCachingEnabled() || painter->worldTransform().isScaling()) {
      qDebug() << "Rendering without cache" << gid;
      data->renderer.render(painter, gid, data->boundingRect());
      return;
   }
   // else when cache is enabled ..

   QPixmap pix;
   if (!QPixmapCache::find(gid, pix)) {
      pix = QPixmap(deviceRect.size());
      data->pixmapDirty = true;
   }

   QPointF viewPoint = m.mapRect(data->boundingRect()).topLeft();
   QPointF viewOrigo = m.map(QPointF(0, 0));

   if (data->pixmapDirty) {
      pix.fill(Qt::transparent);
      qDebug() << "Caching afresh";

      QPainter p(&pix);

      QPointF offset = viewOrigo - viewPoint;
      p.translate(offset);
      p.setWorldMatrix(m, true);
      p.translate(m.inverted().map(QPointF(0, 0)));

      data->renderer.render(&p, gid, data->boundingRect());

      p.end();
      QPixmapCache::insert(gid,  pix);
      data->pixmapDirty = false;
   }

   const QTransform xformSave = painter->transform();

   painter->setWorldMatrix(QMatrix());
   painter->drawPixmap(viewPoint, pix);
   painter->setTransform(xformSave);
}

SvgItemData* SvgPainter::svgData(const QString& gid) const
{
   Q_ASSERT(isSvgRegistered(gid));
   return m_dataHash[gid];
}

QByteArray SvgPainter::svgContent(const QString& gid) const
{
   return svgData(gid)->content;
}

qreal SvgPainter::strokeWidth(const QString& gid) const
{
   return svgData(gid)->cachedStrokeWidth;
}

void SvgPainter::setCachingEnabled(bool caching)
{
   if(m_cachingEnabled == caching)
      return;
   m_cachingEnabled = caching;

   DataHash::iterator it = m_dataHash.begin(), end = m_dataHash.end();
   while(it != end) {
      it.value()->pixmapDirty = true;
      //force updation
      it.value()->renderer.load(it.value()->content);
      ++it;
   }
}

void SvgPainter::setStyleSheet(const QString& gid, const QByteArray& stylesheet)
{
   svgData(gid)->setStyleSheet(stylesheet);
}

QByteArray SvgPainter::styleSheet(const QString& gid) const
{
   return svgData(gid)->styleSheet();
}


SvgItem::SvgItem(SchematicScene *_scene)
   : QucsItem(0, _scene)
{
   m_svgPainter = 0;
}

SvgItem::~SvgItem()
{
}

void SvgItem::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem * option, QWidget *)
{
   SvgPainter *svg = svgPainter();
   if(!svg) {
      qWarning() << "SvgItem::paint() : called when unregistered";
      return;
   }
   if(m_groupId.isEmpty()) {
      qWarning() << "SvgItem::paint() : called when group id is null";
      return;
   }
   svg->paint(painter, m_groupId);

   if(option->state & QStyle::State_Selected)
      highlightSelectedSvgItem(this, painter, option);
}

qreal SvgItem::strokeWidth() const
{
   if(svgPainter() && svgPainter()->isSvgRegistered(m_groupId)) {
      return svgPainter()->strokeWidth(m_groupId);
   }
   return -1;
}

void SvgItem::registerConnections(const QString& gid, SvgPainter *painter)
{
   Q_ASSERT(!gid.isEmpty());
   Q_ASSERT(painter);

   if(!painter->isSvgRegistered(gid)) {
      qWarning() << "SvgItem::registerConnections()  :  "
                 << "Cannot register for ungregisted svgs. Register svg first";
      return;
   }

   if(m_svgPainter) {
      Q_ASSERT(m_svgPainter->isSvgRegistered(m_groupId));

      disconnect(m_svgPainter->rendererFor(m_groupId), SIGNAL(repaintNeeded()),
                 this, SLOT(updateBoundingRect()));
   }


   m_groupId = gid;
   m_svgPainter = painter;

   connect(painter->rendererFor(gid), SIGNAL(repaintNeeded()), this,
           SLOT(updateBoundingRect()));
   updateBoundingRect();
}

QByteArray SvgItem::svgContent() const
{
   if(m_svgPainter && m_svgPainter->isSvgRegistered(m_groupId)) {
      return m_svgPainter->svgContent(m_groupId);
   }
   return QByteArray();
}

void SvgItem::updateBoundingRect()
{
   qDebug() << "SvgItem::updateBoundingRect() : " << m_svgPainter->boundingRect(m_groupId);

   if(!m_svgPainter || !m_svgPainter->isSvgRegistered(m_groupId)) {
      qWarning() << "SvgItem::updateBoundingRect()  : Cant update"
                 << "unregistered items";
      return;
   }

   QRectF bound = m_svgPainter->boundingRect(m_groupId);
   if(bound.isNull()) {
      qWarning() << "SvgItem::calcBoundingRect() : Data parse error";
   }
   // Now call this virtual function to get an adjusted rect preferably for
   // accomodating extra stuff like ports.
   QRectF adjustedRect = adjustedBoundRect(bound);

   setShapeAndBoundRect(QPainterPath(), adjustedRect, strokeWidth());
}
