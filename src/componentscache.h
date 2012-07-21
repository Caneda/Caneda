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

#ifndef COMPONENTS_CACHE_H
#define COMPONENTS_CACHE_H

#include <QHash>
#include <QSvgRenderer>

namespace Caneda
{
    /*!
     * \brief A class used to take care of rendering components.
     *
     * This class renders a component, given the symbol id. The component to be
     * rendered should be first registered with the instance of this class. This
     * way, a cache of components is created an data needed for painting components
     * is created only once (independently of the number of components used by the
     * user in the final schematic).
     */
    struct ComponentsCache : public QObject
    {
    public:
        static ComponentsCache* instance();
        ~ComponentsCache();

        void registerComponent(const QString& symbol_id, const QByteArray& content);
        bool isComponentRegistered(const QString& symbol_id) const;

        QRectF boundingRect(const QString& symbol_id) const;

        void paint(QPainter *painter, const QString& symbol_id);
        QPixmap renderedPixmap(QString component, QString symbol);

    private:
        ComponentsCache(QObject *parent = 0);

        //! Hash table to hold Svg renderer (wich has raw svg content cached).
        QHash<QString, QSvgRenderer*> m_dataHash;
    };

} // namespace Caneda

#endif //COMPONENTS_CACHE_H
