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

#include "componentscache.h"

#include "singletonowner.h"

#include <QPainter>
#include <QPixmapCache>

namespace Caneda
{
    /*
    ##########################################################################
    #                     ComponentsCache methods                            #
    ##########################################################################
    */

    //! \brief Constructs ComponentsCache object.
    ComponentsCache::ComponentsCache(QObject *parent) : QObject(parent)
    {
    }

    /*! Destructor.
     *  \brief Deletes the data belonging to this.
     */
    ComponentsCache::~ComponentsCache()
    {
        QHash<QString, QSvgRenderer*>::iterator it = m_dataHash.begin(), end = m_dataHash.end();
        while(it != end) {
            delete it.value();
            it.value() = 0;
            ++it;
        }
    }

    /*!
     * \brief Registers a component symbol with symbol_id in this instance.
     *
     * Registering is required for rendering any component with the instance of this
     * class. If the symbol_id is already registered does nothing.
     */
    void ComponentsCache::registerComponent(const QString& symbol_id, const QByteArray& svg)
    {
        if(isComponentRegistered(symbol_id)) {
            return;
        }

        m_dataHash[symbol_id] = new QSvgRenderer(svg);
    }

    /*!
     * \brief Returns the registered status of symbol_id.
     *
     * True if the symbol_id data was previously registered, false otherwise.
     */
    bool ComponentsCache::isComponentRegistered(const QString& symbol_id) const
    {
        return m_dataHash.contains(symbol_id);
    }

    //! \brief Returns the bounding rect corresponding to symbol_id.
    QRectF ComponentsCache::boundingRect(const QString& symbol_id) const
    {
        return m_dataHash[symbol_id]->viewBox();
    }

    /*!
     * \brief This method paints (or renders) a registerd component using \a painter.
     *
     * \param painter Painter to be used to render the component.
     * \param symbol_id Symbol id which should be rendered.
     */
    void ComponentsCache::paint(QPainter *painter, const QString& symbol_id)
    {
        QRect deviceRect = boundingRect(symbol_id).toRect();

        // In the case of zooming, the paint is performed without the pixmap cache.
        if(painter->worldTransform().isScaling()) {
            QSvgRenderer *data = m_dataHash[symbol_id];
            data->render(painter, boundingRect(symbol_id));
            return;
        }

        // Else, a pixmap cached is used.
        QPixmap pix = pixmapCache(symbol_id);
        painter->drawPixmap(deviceRect, pix);
    }

    /*!
     * \brief Returns the cached pixmap of a component.
     *
     * \param symbol_id Symbol id of the component to be rendered.
     */
    const QPixmap ComponentsCache::pixmapCache(const QString& symbol_id)
    {
        QRect rect =  boundingRect(symbol_id).toRect();
        QPixmap pix;

        if(!QPixmapCache::find(symbol_id, pix)) {
            pix = QPixmap(rect.size());
            pix.fill(Qt::transparent);

            QPainter painter(&pix);

            QPointF viewPoint = boundingRect(symbol_id).topLeft();
            QPointF viewOrigen = QPointF(0, 0);
            QPointF offset = viewOrigen - viewPoint;
            painter.translate(offset);

            QSvgRenderer *data = m_dataHash[symbol_id];
            data->render(&painter, boundingRect(symbol_id));

            painter.end();
            QPixmapCache::insert(symbol_id,  pix);
        }

        return pix;
    }

    /*!
     * \brief Returns the default ComponentsCache painter object, shared by default graphicsscenes
     */
    ComponentsCache* ComponentsCache::instance()
    {
        static ComponentsCache *instance = 0;
        if (!instance) {
            instance = new ComponentsCache(SingletonOwner::instance());
        }
        return instance;
    }

} // namespace Caneda
