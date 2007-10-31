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

#ifndef __QUCSSVGRENDERER_H
#define __QUCSSVGRENDERER_H

#include <QtCore/QMap>

/* Forward declarations */
class QPainter;
class QucsSvgItem;
class QSvgRenderer;

/*!\brief QucsSvgRenderer is a singleton class used to render svg.
 * \details The purpose of this class is to maintain the cache as
 * well as the renderer at same place.
 **/
class QucsSvgRenderer
{
   public:
      static void render(QPainter *painter, QucsSvgItem *item);
      static void registerItem(QucsSvgItem *item);
      static void setCachingEnabled(bool caching);

      //!\brief Determines whether caching the svgs is enabled or not.
      static bool isCachingEnabled() {
         return m_cachingEnabled;
      }

      static bool isRegistered(QucsSvgItem *item);
      static void reloadSvgFor(QucsSvgItem *item);
      static void reloadAllSvgs();

   private:
      static bool isCacheDirtyFor(QucsSvgItem *item);
      static QSvgRenderer* rendererFor(QucsSvgItem *item);
      static bool shouldReload(QucsSvgItem *item);

      //!\brief Determines whether the svgs are to be cached or not.
      static bool m_cachingEnabled;
      //!\brief Store renderers corresponding to item ids.
      static QMap<QString, QSvgRenderer*> m_renderers;
      //!\brief Store cache's dirty status.
      static QMap<QString, bool> m_dirtyFlags;
      //!\brief Flags to indicate whether svg's are to be reloaded.
      static QMap<QString, bool> m_reloadSvgFlags;
};

#endif //__QUCSSVGRENDERER_H
