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

#ifndef __SVGITEM_H
#define __SVGITEM_H

#include "item.h"

#include <QtSvg/QSvgRenderer>
#include <QtCore/QHash>

class SvgPainter;

class SvgItemData
{
   public:
      SvgItemData(const QString& _groupId, const QByteArray& _content);

      void setStyleSheet(const QByteArray& stylesheet);
      QByteArray styleSheet() const;

      QRectF boundingRect() const { return renderer.boundsOnElement(groupId); }

   private:
      friend class SvgPainter;

      const QString groupId;
      QByteArray content;
      qreal cachedStrokeWidth;
      QSvgRenderer renderer;
      bool pixmapDirty;
};

typedef QHash<QString, SvgItemData*> DataHash;

struct SvgPainter : public QObject
{
      Q_OBJECT;
   public:
      SvgPainter();
      ~SvgPainter();

      void registerSvg(const QString& _groupId, const QByteArray& content);
      bool isSvgRegistered(const QString& _groupId) const {
         return m_dataHash.contains(_groupId);
      }

      QSvgRenderer *rendererFor(const QString& _groupId) const;
      QRectF boundingRect(const QString& _groupId) const;

      void paint(QPainter *painter, const QString& _groupId);
      SvgItemData* svgData(const QString& _groupId) const;

      QByteArray svgContent(const QString& _groupId) const;
      qreal strokeWidth(const QString& _groupId) const;

      bool isCachingEnabled() const { return m_cachingEnabled; }
      void setCachingEnabled(bool caching);

      void setStyleSheet(const QString& _groupId, const QByteArray& stylesheet);
      QByteArray styleSheet(const QString& _groupId) const;

   private:
      QHash<QString, SvgItemData*> m_dataHash;
      bool m_cachingEnabled;
};

/*!\brief Base class for items which can be rendered using svg.
 * \details This class implements an interface needed by SvgRenderer
 * to render svg's.
 * \sa SvgRenderer
 **/
class SvgItem : public QObject, public QucsItem
{
      Q_OBJECT;
   public:
      enum {
         Type = QucsItemType+1
      };

      explicit SvgItem(SchematicScene *scene = 0);
      ~SvgItem();

      int type() const { return SvgItem::Type; }

      void paint(QPainter *p, const QStyleOptionGraphicsItem* o, QWidget *w);

      qreal strokeWidth() const;

      void registerConnections(const QString& id, SvgPainter *painter);
      QString groupId() const { return m_groupId; }

      QByteArray svgContent() const;
      SvgPainter* svgPainter() const { return m_svgPainter; }

   public slots:
      void updateBoundingRect();

   protected:
      virtual QRectF adjustedBoundRect(const QRectF &rect) { return rect; }

   private:
      SvgPainter *m_svgPainter;
      QString m_groupId;
};

#endif //__SVGITEM_H
