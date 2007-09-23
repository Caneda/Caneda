 /***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __ITEM_H
#define __ITEM_H

#include <QtSvg/QGraphicsSvgItem>
#include <QtGui/QPen>
#include <QtGui/QBrush>

class QGraphicsScene;
class QGraphicsView;
class SchematicScene;
class QucsMainWindow;
class QMenu;

namespace Qucs {
   class XmlReader;
   class XmlWriter;
}

class QucsItem : public QGraphicsSvgItem
{
   public:
      enum QucsItemTypes {
         QucsItemType = (UserType << 14),
         ComponentType = (UserType << 13) | QucsItemType,
         MultiSymbolComponentType = (UserType << 12) | ComponentType,
         NodeType = (UserType << 11) | QucsItemType,
         WireType = (UserType << 10) | QucsItemType,
         PaintingType = (UserType << 9) | QucsItemType,
         DisplayType = (UserType << 8) | QucsItemType
      };

      enum {
         Type = QucsItemType
      };

      using QGraphicsItem::rotate;

      QucsItem(QGraphicsItem* parent = 0, SchematicScene* scene = 0);
      virtual ~QucsItem();

      virtual void copyTo(QucsItem *_item) const;

      int type() const { return QucsItemType; }
      QRectF boundingRect() const { return m_boundingRect; }

      SchematicScene* schematicScene() const;
      QGraphicsView* activeView() const;
      QucsMainWindow* mainWindow() const;

      void setPenColor(QColor _color);
      QColor penColor() const { return m_penColor; }

      virtual QString saveString() const { return QString(""); }
      virtual bool loadFromString(QString ) { return true; }

      virtual void writeXml(Qucs::XmlWriter *writer);
      virtual void readXml(Qucs::XmlReader *reader);

      virtual void mirrorX();
      virtual void mirrorY();
      virtual void rotate();

      //TODO:Make this pure virtual
      virtual void invokePropertiesDialog() {}

      QMenu* defaultContextMenu() const;

   protected:
      void setBoundingRect(const QRectF& rect);
      QColor m_penColor;
      QRectF m_boundingRect;
};

#endif //__ITEM_H
