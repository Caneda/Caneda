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
#include "singletonmanager.h"

#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>

#include <memory>

/*
##########################################################################
#                            HELPER METHODS                              #
##########################################################################
*/

/*!
 * \brief Item stroke width
 * \todo should be cnfigurable
 */
static const double itemstrokewidth = 1.0;


/*!
 * \brief This code draws the highlighted rect around the item
 * \note This code is stolen from source of qt ;-)
 */
static void highlightSelectedSvgItem(
        SvgItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
    if(qFuzzyCompare(qMax(murect.width(), murect.height()), qreal(0.0))) {
        return;
    }

    const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
    if(qMin(mbrect.width(), mbrect.height()) < qreal(1.0)) {
        return;
    }

    qreal itemStrokeWidth = itemstrokewidth;
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

/*!
 * \brief Constructs an SvgItemData with raw svg content.
 *
 * \param _content Raw svg content.
 */
SvgItemData::SvgItemData(const QByteArray& _content) :
    content(_content),
    renderer(content),
    pixmapDirty(true)
{
    Q_ASSERT(renderer.isValid());
    Q_ASSERT(!content.isEmpty());
}


/*!
 * \brief Returns the bounding rect of the svg element.
 *
 * Use viewBox (ifexist) and as a fallback use boundsOnElement
 */
QRectF SvgItemData::boundingRect() const
{
    QRectF viewbox;
    viewbox = renderer.viewBox();

    if(viewbox.isNull()) {
        return renderer.boundsOnElement("svg");
    }
    return viewbox;
}

/*
##########################################################################
#                          SvgPainter methods                            #
##########################################################################
*/

//! \brief Constructs svg painter object.
SvgPainter::SvgPainter(QObject *parent) : QObject(parent)
{
    m_cachingEnabled = true;
}

/*! Destructor.
 *  \brief Deletes the data belonging to this.
 */
SvgPainter::~SvgPainter()
{
    DataHash::iterator it = m_dataHash.begin(), end = m_dataHash.end();
    while(it != end) {
        delete it.value();
        it.value() = 0;
        ++it;
    }
}

/*!
 * \brief Registers svg with svg id \a svg_id with this instance.
 *
 * Registering is required for rendering any svg with the instance of this
 * class. If the \a svg_id is already registered does nothing.
 */
void SvgPainter::registerSvg(const QString& svg_id, const QByteArray& svg)
{
    Q_ASSERT(!svg_id.isEmpty());
    if(isSvgRegistered(svg_id)) {
        return;
    }

    m_dataHash[svg_id] = new SvgItemData(svg);
}

//! \brief Returns QSvgRenderer corresponding to id \a svg_id.
QSvgRenderer* SvgPainter::rendererFor(const QString& svg_id) const
{
    return &(svgData(svg_id)->renderer);
}

//! \brief Returns bound rect corresponding to id \a svg_id.
QRectF SvgPainter::boundingRect(const QString& svg_id) const
{
    return svgData(svg_id)->boundingRect();
}

/*!
 * \brief This method paints( or renders) a registerd svg using \a painter.
 *
 * This also takes care of updating the cache ifcaching is enabled.
 *
 * \param painter Painter with which svg should be rendered.
 * \param svg_id Svg id which should be rendered.
 */
void SvgPainter::paint(QPainter *painter, const QString& svg_id)
{
    SvgItemData *data = svgData(svg_id);
    QMatrix m = painter->worldMatrix();
    QRect deviceRect = m.mapRect(data->boundingRect()).toRect();

    // If Caching disabled or ifthere is transformation render without cache.
    if(!isCachingEnabled() || painter->worldTransform().isScaling()) {
        data->renderer.render(painter, data->boundingRect());
        return;
    }
    // else when cache is enabled ..

    QPixmap pix;
    if(!QPixmapCache::find(svg_id, pix)) {
        pix = QPixmap(deviceRect.size());
        data->pixmapDirty = true;
    }

    QPointF viewPoint = m.mapRect(data->boundingRect()).topLeft();
    QPointF viewOrigo = m.map(QPointF(0, 0));

    if(data->pixmapDirty) {
        pix.fill(Qt::transparent);

        QPainter p(&pix);

        QPointF offset = viewOrigo - viewPoint;
        p.translate(offset);
        p.setWorldMatrix(m, true);
        p.translate(m.inverted().map(QPointF(0, 0)));

        data->renderer.render(&p, data->boundingRect());

        p.end();
        QPixmapCache::insert(svg_id,  pix);
        data->pixmapDirty = false;
    }

    const QTransform xformSave = painter->transform();

    painter->setWorldMatrix(QMatrix());
    painter->drawPixmap(viewPoint, pix);
    painter->setTransform(xformSave);
}

//! \brief Returns the SvgItemData* corresponding to svg id \a svg_id.
SvgItemData* SvgPainter::svgData(const QString& svg_id) const
{
    Q_ASSERT(isSvgRegistered(svg_id));
    return m_dataHash[svg_id];
}

//! \brief Returns svg content corresponding to svg id \a svg_id.
QByteArray SvgPainter::svgContent(const QString& svg_id) const
{
    return svgData(svg_id)->content;
}

//! \brief Enables/Disables caching based on \a caching.
void SvgPainter::setCachingEnabled(bool caching)
{
    if(m_cachingEnabled == caching) {
        return;
    }
    m_cachingEnabled = caching;

    DataHash::iterator it = m_dataHash.begin(), end = m_dataHash.end();
    while(it != end) {
        it.value()->pixmapDirty = true;
        //force updation
        it.value()->renderer.load(it.value()->content);
        ++it;
    }
}

/*!
 * \brief Returns the default svg painter object, shared by default schematics
 */
SvgPainter* SvgPainter::instance()
{
    return SingletonManager::instance()->svgPainter();
}


/*
##########################################################################
#                            SvgItem methods                             #
##########################################################################
*/

/*!
 * \brief Constructs an unregistered, initially unrenderable svg item.
 *
 * To render this item it should first be connected to SvgPainter and the
 * svg id should already be registered with SvgPainter.
 * \sa SvgItem::registerConnections, SvgPainter::registerSvg()
 */
SvgItem::SvgItem(SvgPainter *svgP, SchematicScene *_scene) : QucsItem(0, _scene),
    m_svgPainter(svgP)
{
}

//! Destructor.
SvgItem::~SvgItem()
{
}

/*!
 * \brief Paints the item using SvgPainter::paint method, peforms sanity
 * checks and takes care of drawing selection rectangle.
 */
void SvgItem::paint(QPainter *painter,
        const QStyleOptionGraphicsItem * option, QWidget *)
{
    if(!isRegistered()) {
        qWarning() << "SvgItem::paint() : called when unregistered";
        return;
    }

    svgPainter()->paint(painter, m_svgId);

    if(option->state & QStyle::State_Selected) {
        highlightSelectedSvgItem(this, painter, option);
    }
}


/*!
 * \brief Registers connections of this item with SvgPainter \a painter.
 *
 * Unless this item is connected this way, it won't be rendered. The svg
 * corresponding to svg id \a svg_id should already be registered with
 * SvgPainter \a painter using SvgPainter::registerSvg. This also unregisters
 * with previously connected SvgPainter ifit was connected.
 *
 * \sa SvgPainter::registerSvg()
 */
void SvgItem::registerConnections(const QString& svg_id, SvgPainter *painter)
{
    Q_ASSERT(!svg_id.isEmpty());
    Q_ASSERT(painter);

    if(!painter->isSvgRegistered(svg_id)) {
        qWarning() << "SvgItem::registerConnections()  :  "
            << "Cannot register for ungregisted svgs. Register svg first";
        return;
    }

    // Disconnect ifthis was connected to a different SvgPainter before.
    if(isRegistered()) {
        disconnect(m_svgPainter->rendererFor(m_svgId), SIGNAL(repaintNeeded()),
                this, SLOT(updateBoundingRect()));
    }


    m_svgId = svg_id;
    m_svgPainter = painter;

    connect(painter->rendererFor(svg_id), SIGNAL(repaintNeeded()), this,
            SLOT(updateBoundingRect()));
    updateBoundingRect();
}

//! \brief Returns svg corresponding to this item.
QByteArray SvgItem::svgContent() const
{
    if(isRegistered()) {
        return m_svgPainter->svgContent(m_svgId);
    }
    return QByteArray();
}

/*!
 * \brief Updates the bounding rect of this item.
 *
 * This is public slot which is connected to QSvgRenderer::repaintNeeded()
 * signal.
 */
void SvgItem::updateBoundingRect()
{

    if(!isRegistered()) {
        qWarning() << "SvgItem::updateBoundingRect()  : Cant update"
                                                        << "unregistered items";
        return;
    }

    QRectF bound = m_svgPainter->boundingRect(m_svgId);
    if(bound.isNull()) {
        qWarning() << "SvgItem::calcBoundingRect() : Data parse error";
    }
    // Now call this virtual function to get an adjusted rect preferably for
    // accomodating extra stuff like ports.
    QRectF adjustedRect = adjustedBoundRect(bound);

    setShapeAndBoundRect(QPainterPath(), adjustedRect, itemstrokewidth);
}

//! \brief Returns whether item is registered to an svg or not.
bool SvgItem::isRegistered() const
{
    return svgPainter() && !m_svgId.isEmpty() && svgPainter()->isSvgRegistered(m_svgId);
}
