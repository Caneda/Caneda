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

/* forward declaration */
class QGraphicsScene;
class QGraphicsView;
class SchematicScene;
class QucsMainWindow;
class QMenu;

namespace Qucs {
   class XmlReader;
   class XmlWriter;
}

/*!\brief Qucs item - The base class for components, wires, nodes..
    \todo Better documentation
*/
class QucsItem : public QGraphicsSvgItem
{
   public:
      /*!\brief Component type
	 \todo Document each type
       */
      enum QucsItemTypes {
         QucsItemType = (UserType << 14),
         ComponentType = (UserType << 13) | QucsItemType,
         MultiSymbolComponentType = (UserType << 12) | ComponentType,
         NodeType = (UserType << 11) | QucsItemType,
         WireType = (UserType << 10) | QucsItemType,
         PaintingType = (UserType << 9) | QucsItemType,
         DisplayType = (UserType << 8) | QucsItemType
      };

      /*!\brief XXXX
	 \todo Document
      */
      enum {
         Type = QucsItemType
      };

      //Pull the base implementation to be accesible here.
      using QGraphicsItem::rotate;

      QucsItem(QGraphicsItem* parent = 0, SchematicScene* scene = 0);
      virtual ~QucsItem();

      virtual void copyTo(QucsItem *_item) const;

      /*!\brief Return type of item */
      int type() const { return QucsItemType; }
      /*!\brief Return bounding box
	 \todo Why not inline
       */
      QRectF boundingRect() const { return m_boundingRect; }

      SchematicScene* schematicScene() const;
      QGraphicsView* activeView() const;
      QucsMainWindow* mainWindow() const;

      /*!\brief set pen color */
      void setPenColor(QColor _color);
      /*!\brief return pen color */
      QColor penColor() const { return m_penColor; }

      /*!\brief XXXX
	\todo Document
      */
      virtual QString saveString() const { return QString(""); }
      /*!\brief XXXX
	 \todo Document
      */
      virtual bool loadFromString(QString ) { return true; }

      virtual void writeXml(Qucs::XmlWriter *writer);
      virtual void readXml(Qucs::XmlReader *reader);

      virtual void mirrorX();
      virtual void mirrorY();
      virtual void rotate();

      /*!\brief Invoke properties dialog of item
	\todo Make this pure virtual
      */
      virtual void invokePropertiesDialog() {}

      /*!\brief Context menu of item */
      QMenu* defaultContextMenu() const;

   protected:
      /*!\brief Set bounding box */
      void setBoundingRect(const QRectF& rect);
      /*!\brief pen color of item */
      QColor m_penColor;
      /*!\brief Bounding box */
      QRectF m_boundingRect;
};

#endif //__ITEM_H
