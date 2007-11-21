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
#include "schematicscene.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <QtSvg/QSvgRenderer>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtXml>

/*
  ##########################################################################
  #                            HELPER METHODS                              #
  ##########################################################################
*/

/*!\internal
 * \brief This method hooks stylesheet to raw svg \a content.
 *
 * \param svgContent The raw svg to which stylesheet should be hooked to.
 * \param stylesheet The stylesheet which is to be hooked.
 */
static void hookStyleSheetTo(QByteArray &svgContent,
                                   const QByteArray& stylesheet)
{
   // No new stylesheet to insert.
   if(stylesheet.isEmpty()) {
      return;
   }

   QDomDocument doc("svg");
   QString errorMsg;
   if(!doc.setContent(svgContent, &errorMsg)) {
      qWarning() << "SVG parse failed" << errorMsg << "\n" << svgContent;
      return;
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
                 << svgContent;
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

   svgContent = doc.toByteArray();
}

/*!\internal
 * \brief This method extracts stylesheet content from given svg \a content.
 *
 * This also parses and checks for any violations of svg related to css
 * styling.
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
 * \param content The actual svg file with style info in it.
 * \param ok If non null, this will indicate success state of this method.
 * \return Returns the parsed stroke width if found, else return 0.
 */
static qreal getStrokeWidth(const QByteArray &content, bool *ok = 0)
{
   QByteArray stylesheet = getStyleSheet(content).simplified();
   stylesheet.replace(QByteArray(" "),"");

   int indStart = stylesheet.indexOf("{");
   int indEnd = stylesheet.indexOf("}", indStart);

   if(indStart == -1 || indEnd == -1 || indEnd < indStart) {
      qWarning() << "getStrokeWidth() : Parse error.";
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

    qreal itemStrokeWidth = item->strokeWidth();
    const qreal pad = itemStrokeWidth / 2;
    const qreal strokeWidth = 0; // cosmetic pen

    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( // ensure good contrast against fgcolor
        fgcolor.red()   > 127 ? 0 : 255,
        fgcolor.green() > 127 ? 0 : 255,
        fgcolor.blue()  > 127 ? 0 : 255);

    painter->setPen(QPen(bgcolor, strokeWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}

/*
  ##########################################################################
  #                          SvgItemData methods                           #
  ##########################################################################
 */

/*! \brief Constructs an SvgItemData with given group id and raw svg content.
 *
 * \param _groupId Represents the group id of the svg to which this corresponds
 * \param _content Raw svg content.
 */
SvgItemData::SvgItemData(const QString& _groupId, const QByteArray& _content) :
   groupId(_groupId),
   content(_content),
   cachedStrokeWidth(getStrokeWidth(content)),
   renderer(content),
   pixmapDirty(true)
{
   Q_ASSERT(renderer.isValid());
   Q_ASSERT(!content.isEmpty());
}

/*!\brief Hooks in the stylesheet to raw svg and updates the renderer and the
 * cached values.
 */
void SvgItemData::setStyleSheet(const QByteArray& stylesheet)
{
   if(getStyleSheet(content).isEmpty()) {
      qWarning() << "SvgItemData::setStyleSheet()  :  "
                 << "Cannot apply stylesheet to svg without stylesheet present"
                 << " beforehand in the svg";
      return;
   }
   hookStyleSheetTo(content, stylesheet);
   pixmapDirty = true;
   cachedStrokeWidth = getStrokeWidth(content);

   /*NOTE: This load should be called only after all the state variables are
           updated as the repaintNeeded signal is emitted requiring immediate
           availability of new state variables.
   */
   renderer.load(content);
   Q_ASSERT(renderer.isValid());
}

//! Returns the style sheet associated with the svg.
QByteArray SvgItemData::styleSheet() const
{
   return getStyleSheet(content);
}

/*
  ##########################################################################
  #                          SvgPainter methods                            #
  ##########################################################################
 */

//! Constructs svg painter object.
SvgPainter::SvgPainter()
{
   m_cachingEnabled = true;
}

//! Destructor. Deletes the data belonging to this.
SvgPainter::~SvgPainter()
{
   DataHash::iterator it = m_dataHash.begin(), end = m_dataHash.end();
   while(it != end) {
      delete it.value();
      it.value() = 0;
      ++it;
   }
}

/*! \brief Registers svg with group id \a groupId with this instance.
 *
 * Registering is required for rendering any svg with the instance of this
 * class. If the \a groupId is already registered does nothing.
 */
void SvgPainter::registerSvg(const QString& groupId, const QByteArray& svg)
{
   Q_ASSERT(!groupId.isEmpty());

   if(isSvgRegistered(groupId)) {
      return;
   }

   m_dataHash[groupId] = new SvgItemData(groupId, svg);
}

//! Returns QSvgRenderer corresponding to group id \a gid.
QSvgRenderer* SvgPainter::rendererFor(const QString& gid) const
{
   return &(svgData(gid)->renderer);
}

//! Returns bound rect corresponding to group id \a gid.
QRectF SvgPainter::boundingRect(const QString& gid) const
{
   return svgData(gid)->boundingRect();
}

/*! \brief This method paints( or renders) a registerd svg using \a painter.
 *
 * This also takes care of updating the cache if caching is enabled.
 * \param painter Painter with which svg should be rendered.
 * \param gid Group id which should be rendered.
 */
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

//! Returns the SvgItemData* corresponding to group id \a gid.
SvgItemData* SvgPainter::svgData(const QString& gid) const
{
   Q_ASSERT(isSvgRegistered(gid));
   return m_dataHash[gid];
}

//! Returns svg contentcorresponding to group id \a gid.
QByteArray SvgPainter::svgContent(const QString& gid) const
{
   return svgData(gid)->content;
}

//! Returns stroke width corresponding to group id \a gid.
qreal SvgPainter::strokeWidth(const QString& gid) const
{
   return svgData(gid)->cachedStrokeWidth;
}

//! Enables/Disables caching based on \a caching.
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

//! Hooks the stylesheet to svg. \sa SvgItemData::setStyleSheet()
void SvgPainter::setStyleSheet(const QString& gid, const QByteArray& stylesheet)
{
   svgData(gid)->setStyleSheet(stylesheet);
}

//! Returns stylesheet of svg corresponding to group id \a gid.
QByteArray SvgPainter::styleSheet(const QString& gid) const
{
   return svgData(gid)->styleSheet();
}

/*
  ##########################################################################
  #                            SvgItem methods                             #
  ##########################################################################
 */

/*! \brief Constructs an unregistered, initially unrenderable svg item.
 *
 * To render this item it should first be connected to SvgPainter and the
 * group id should already be registered with SvgPainter.
 * \sa registerConnections, SvgPainter::registerSvg()
 */
SvgItem::SvgItem(SchematicScene *_scene)
   : QucsItem(0, _scene),
     m_svgPainter(0)
{
}

//! Destructor.
SvgItem::~SvgItem()
{
}

/*! \brief Paints the item using SvgPainter::paint method, peforms sanity
 * checks and takes care of drawing selection rectangle.
 */
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

//! Returns stroke width used in svg.
qreal SvgItem::strokeWidth() const
{
   if(svgPainter() && svgPainter()->isSvgRegistered(m_groupId)) {
      return svgPainter()->strokeWidth(m_groupId);
   }
   return 0.;
}

/*! \brief Registers connections of this item with SvgPainter \a painter.
 *
 * Unless this item is connected this way, it won't be rendered. The svg
 * corresponding to group id \a gid should already be registered with
 * SvgPainter \a painter using SvgPainter::registerSvg.
 * \sa SvgPainter::registerSvg()
 */
void SvgItem::registerConnections(const QString& gid, SvgPainter *painter)
{
   Q_ASSERT(!gid.isEmpty());
   Q_ASSERT(painter);

   if(!painter->isSvgRegistered(gid)) {
      qWarning() << "SvgItem::registerConnections()  :  "
                 << "Cannot register for ungregisted svgs. Register svg first";
      return;
   }

   // Disconnect if this was connected to a different SvgPainter before.
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

//! Returns svg corresponding to this item.
QByteArray SvgItem::svgContent() const
{
   if(m_svgPainter && m_svgPainter->isSvgRegistered(m_groupId)) {
      return m_svgPainter->svgContent(m_groupId);
   }
   return QByteArray();
}

/*! \brief Updates the bounding rect of this item.
 *
 * This is public slot which is connected to QSvgRenderer::repaintNeeded()
 * signal.
 */
void SvgItem::updateBoundingRect()
{
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

/*! \brief This handles the updation of connections when this item is cut and
 * pasted in different scene.
 */
QVariant SvgItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
   if(change == QGraphicsItem::ItemSceneChange) {
      // Disconnect if this was connected to a different SvgPainter before.
      if(m_svgPainter) {
         Q_ASSERT(m_svgPainter->isSvgRegistered(m_groupId));

         disconnect(m_svgPainter->rendererFor(m_groupId), SIGNAL(repaintNeeded()),
                    this, SLOT(updateBoundingRect()));
      }

      SchematicScene *newScene = qobject_cast<SchematicScene*>(
         qvariant_cast<QGraphicsScene*>(value));
      if(newScene) {
         registerConnections(m_groupId, newScene->svgPainter());
      }
   }
   return QucsItem::itemChange(change, value);
}
