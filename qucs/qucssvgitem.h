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

#ifndef __QUCSSVGITEM_H
#define __QUCSSVGITEM_H

#include "item.h"

/*!\brief Base class for items which can be rendered using svg.
 * \details This class implements an interface needed by QucsSvgRenderer
 * to render svg's.
 * \sa QucsSvgRenderer
 **/
class QucsSvgItem : public QucsItem
{
   public:
      enum {
         Type = QucsItemType+1
      };

      QucsSvgItem(const QString& file, const QByteArray& _stylesheet,
                  SchematicScene *scene);
      QucsSvgItem(const QByteArray& contents, const QByteArray& _stylesheet,
                  SchematicScene *scene);
      ~QucsSvgItem();

      int type() const { return QucsSvgItem::Type; }

      virtual QString uniqueId() const = 0;

      QByteArray svgContent() const { return m_content; }
      QByteArray svgContentWithStyleSheet() const;

      void setSvgContent(const QByteArray& content);
      void setSvgContent(const QString& file);

      void setStyleSheet(const QByteArray& _stylesheet);
      QByteArray styleSheet() const { return m_styleSheet; }

   private:
      void calcBoundingRect();

      QByteArray m_styleSheet;
      QByteArray m_content;
      QByteArray m_styleSheetWithContent;

      qreal m_strokeWidth;
};

#endif //__QUCSSVGITEM_H
