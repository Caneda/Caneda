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

#include "qucssvgrenderer.h"
#include "qucssvgitem.h"

#include <QtSvg/QSvgRenderer>
#include <QtGui/QPixmapCache>
#include <QtGui/QPainter>

#include <QtCore/QDebug>

// Caching is enabled by default
bool QucsSvgRenderer::m_cachingEnabled = true;

QMap<QString, QSvgRenderer*> QucsSvgRenderer::m_renderers;
QMap<QString, bool> QucsSvgRenderer::m_dirtyFlags;
QMap<QString, bool> QucsSvgRenderer::m_reloadSvgFlags;

/*!\brief This static method renders the svg to painter.
 * \details This method first checks whether caching is enabled and if not
 * just renders using QSvgRenderer. If caching is enabled it checks whether
 * cache is dirty and takes required action.
 * \note Item should be explicitly registered.
 * \sa QucsSvgRenderer::registerItem()
 **/
void QucsSvgRenderer::render(QPainter *painter, QucsSvgItem *item)
{
   if(!isRegistered(item)) {
      qWarning("Trying to render an unregistered item which is not allowed");
      return;
   }

   //Check whether the svg is to be reloaded.
   if(shouldReload(item)) {
      reloadSvgFor(item);
   }

   QMatrix m = painter->worldMatrix();
   QRect deviceRect = m.mapRect(item->boundingRect()).toRect();

   // If Caching disabled or if there is transformation render without cache.
   if (!m_cachingEnabled || painter->worldTransform().isScaling()) {
      qDebug() << "Rendering without cache" << item->uniqueId();
      rendererFor(item)->render(painter, item->uniqueId(),
                                item->boundingRect());
      return;
   }
   // else when cache is enabled ..

   QPixmap pix;
   if (!QPixmapCache::find(item->uniqueId(), pix)) {
      pix = QPixmap(deviceRect.size());
      m_dirtyFlags[item->uniqueId()] = true;
   }

   QPointF viewPoint = m.mapRect(item->boundingRect()).topLeft();
   QPointF viewOrigo = m.map(QPointF(0, 0));

   if (isCacheDirtyFor(item)) {
      pix.fill(Qt::transparent);
      qDebug() << "Caching afresh";

      QPainter p(&pix);

      QPointF offset = viewOrigo - viewPoint;
      p.translate(offset);
      p.setWorldMatrix(m, true);
      p.translate(m.inverted().map(QPointF(0, 0)));

      rendererFor(item)->render(&p, item->uniqueId(), item->boundingRect());

      p.end();
      QPixmapCache::insert(item->uniqueId(),  pix);
      m_dirtyFlags[item->uniqueId()] = false;
   }

   const QTransform xformSave = painter->transform();

   painter->setWorldMatrix(QMatrix());
   painter->drawPixmap(viewPoint, pix);
   painter->setTransform(xformSave);
}

/*!\brief This static method registers the given item to class.
 * \details This method does nothing if item is already registered.
 * Basically this uses item->uniqueId() as a key for internal storage.
 * This is the main gateway method for items to use this class.
 **/
void QucsSvgRenderer::registerItem(QucsSvgItem *item)
{
   if(isRegistered(item)) {
      return;
   }
   QByteArray svgContent = item->svgContent();
   QSvgRenderer *renderer = new QSvgRenderer(svgContent);
   m_renderers.insert(item->uniqueId(), renderer);
   //Set cache to dirty initially.
   m_dirtyFlags.insert(item->uniqueId(), true);
   //No need to reload the svg as of now.
   m_reloadSvgFlags.insert(item->uniqueId(), false);
}

/*!\brief Method to enable/disable caching.
 * \details If the item's previous caching status is same as before, this
 * method simply returns. Otherwise if cache is enabled, it dirties the
 * whole cache.*/
void QucsSvgRenderer::setCachingEnabled(bool caching)
{
   if(m_cachingEnabled == caching)
      return;

   m_cachingEnabled = caching;

   if(m_cachingEnabled) {
      QMap<QString, bool>::iterator it = m_dirtyFlags.begin(),
         end = m_dirtyFlags.end();
      while (it != end) {
         it.value() = true;
         ++it;
      }
   }
}

/*!\brief Determines whether the cache is dirty or not for the item.
 * \note Make sure the item is registered.
 * \sa QucsSvgRenderer::registerItem()
 **/
bool QucsSvgRenderer::isCacheDirtyFor(QucsSvgItem *item)
{
   if(!isRegistered(item)) {
      qWarning("QucsSvgRenderer::isCacheDirtyFor(): Item not registered.");
      return true;
   }
   Q_ASSERT(m_dirtyFlags.contains(item->uniqueId()));
   return m_dirtyFlags[item->uniqueId()];
}

/*!\brief Returns the QSvgRenderer corresponding to item based on
 * item->uniqueId()
 * \note Make sure the item is registered.
 * \sa QucsSvgRenderer::registerItem()
 **/
QSvgRenderer* QucsSvgRenderer::rendererFor(QucsSvgItem *item)
{
   if(!isRegistered(item)) {
      qWarning("QucsSvgRenderer::rendererFor() : Item not registered."
               " Returning invalid renderer.");
      //Note also causes memory leak. But prevents segfault.
      return new QSvgRenderer();
   }
   return m_renderers[item->uniqueId()];
}

//!\brief Determines whether item is registered or not.
bool QucsSvgRenderer::isRegistered(QucsSvgItem *item) {
   return m_renderers.contains(item->uniqueId());
}


/*!\brief Determines whether the svg correspoding to item should be reloaded or
 * not.
 * \note Make sure the item is registered.
 * \sa QucsSvgRenderer::registerItem()
 **/
bool QucsSvgRenderer::shouldReload(QucsSvgItem *item)
{
   if(!isRegistered(item)) {
      qWarning("QucsSvgRenderer::shouldReload(): Item not registered.");
      return false;
   }
   return m_reloadSvgFlags[item->uniqueId()];
}

/*!\brief Reloads svg corresponding to item into renderer.
 * \note Make sure the item is registered.
 * \sa QucsSvgRenderer::registerItem()
 **/
void QucsSvgRenderer::reloadSvgFor(QucsSvgItem *item)
{
   qDebug() << "QucsSvgRenderer::reloadSvgFor() for " << item->uniqueId();
   if(!isRegistered(item)) {
      qWarning("QucsSvgRenderer::reloadSvgFor(): Item not registered!");
      return;
   }
   QSvgRenderer *renderer = rendererFor(item);
   if(!renderer->load(item->svgContent())) {
      qWarning("QucsSvgRenderer::reloadSvgFor(): Svg parsing failed.");
   }
   else {
      qDebug() << "reload successful";
   }
   m_dirtyFlags[item->uniqueId()] = true;
   m_reloadSvgFlags[item->uniqueId()] = false;
}

/*!\brief Schedules reloading of all svgs.
 * \details This doesn't reload spontaneously. It just sets the flags.
 * When render method is called, the item's svg is reloaded.
 * So after reloadAllSvgs(), update() the whole scene to see the effect.
 **/
void QucsSvgRenderer::reloadAllSvgs()
{
   qDebug() << "Scheduling reloading of all svgs";
   QMap<QString, bool>::iterator it = m_reloadSvgFlags.begin(),
      end = m_reloadSvgFlags.end();
   while (it != end) {
      it.value() = true;
      ++it;
   }
}
