/***************************************************************************
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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
    #                          SvgPainter methods                            #
    ##########################################################################
    */

    //! \brief Constructs svg painter object.
    SvgPainter::SvgPainter(QObject *parent) : QObject(parent)
    {
    }

    /*! Destructor.
     *  \brief Deletes the data belonging to this.
     */
    SvgPainter::~SvgPainter()
    {
        QHash<QString, QSvgRenderer*>::iterator it = m_dataHash.begin(), end = m_dataHash.end();
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
        if(isSvgRegistered(svg_id)) {
            return;
        }

        m_dataHash[svg_id] = new QSvgRenderer(svg);
    }

    /*!
     * \brief Returns the registered status of svg_id.
     *
     * True if the svg_id data was previously registered,
     * false otherwise.
     */
    bool SvgPainter::isSvgRegistered(const QString& svg_id) const
    {
        return m_dataHash.contains(svg_id);
    }

    //! \brief Returns the bounding rect corresponding to svg_id.
    QRectF SvgPainter::boundingRect(const QString& svg_id) const
    {
        return m_dataHash[svg_id]->viewBox();
    }

    /*!
     * \brief This method paints (or renders) a registerd svg using \a painter.
     *
     * \param painter Painter with which svg should be rendered.
     * \param svg_id Svg id which should be rendered.
     */
    void SvgPainter::paint(QPainter *painter, const QString& svg_id)
    {
        QSvgRenderer *data = m_dataHash[svg_id];
        QMatrix m = painter->worldMatrix();
        QRect deviceRect = m.mapRect(boundingRect(svg_id)).toRect();

        // If there is transformation render without cache.
        if(painter->worldTransform().isScaling()) {
            data->render(painter, boundingRect(svg_id));
            return;
        }

        QPixmap pix;
        QPointF viewPoint = m.mapRect(boundingRect(svg_id)).topLeft();
        QPointF viewOrigo = m.map(QPointF(0, 0));

        if(!QPixmapCache::find(svg_id, pix)) {
            pix = QPixmap(deviceRect.size());
            pix.fill(Qt::transparent);

            QPainter p(&pix);
            QPointF offset = viewOrigo - viewPoint;
            p.translate(offset);
            p.setWorldMatrix(m, true);
            p.translate(m.inverted().map(QPointF(0, 0)));

            data->render(&p, boundingRect(svg_id));

            p.end();
            QPixmapCache::insert(svg_id,  pix);
        }

        const QTransform xformSave = painter->transform();

        painter->setWorldMatrix(QMatrix());
        painter->drawPixmap(viewPoint, pix);
        painter->setTransform(xformSave);
    }

    /*!
     * \brief Returns the component rendered to pixmap.
     *
     * \param component Component to be rendered.
     * \param symbol Symbol to be rendered.
     */
    QPixmap SvgPainter::renderedPixmap(QString component, QString symbol)
    {
        QString svgId = component + '/' + symbol;
        QRectF rect =  boundingRect(svgId);
        QPixmap pix;

        if(!QPixmapCache::find(svgId, pix)) {
            pix = QPixmap(rect.toRect().size());
            pix.fill(Qt::transparent);
            QPainter painter(&pix);
            painter.setWindow(rect.toRect());
            paint(&painter, svgId);
        }

        return pix;
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
