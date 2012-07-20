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

#include "singletonowner.h"

#include <QPainter>
#include <QPixmapCache>

namespace Caneda
{
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
        QHash<QString, SvgItemData*>::iterator it = m_dataHash.begin(), end = m_dataHash.end();
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
     * \brief This method paints (or renders) a registerd svg using \a painter.
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

        // If Caching disabled or if there is transformation render without cache.
        if(!isCachingEnabled() || painter->worldTransform().isScaling()) {
            data->renderer.render(painter, data->boundingRect());
            return;
        }
        // else when cache is enabled...

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

        QHash<QString, SvgItemData*>::iterator it = m_dataHash.begin(), end = m_dataHash.end();
        while(it != end) {
            it.value()->pixmapDirty = true;
            //force updation
            it.value()->renderer.load(it.value()->content);
            ++it;
        }
    }

    /*!
     * \brief Returns the default svg painter object, shared by default graphicsscenes
     */
    SvgPainter* SvgPainter::instance()
    {
        static SvgPainter *instance = 0;
        if (!instance) {
            instance = new SvgPainter(SingletonOwner::instance());
        }
        return instance;
    }

} // namespace Caneda
