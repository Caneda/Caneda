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

#include <QtGui/QGraphicsItem>
#include <limits>

/*!\brief This macro determines the pattern for derived class.
 * \details The macro accepts two parameter base and shift and returns the
 * the pattern corresponding to derived class.*/
#define PATTERN(base,shift) (((base) >> (shift)) | (base))

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

//!\brief Qucs item - The base class for components, wires, nodes..
class QucsItem : public QGraphicsItem
{
   public:
      /*!\brief This enum helps in polymorphic cast without using dynamic_cast.
        \details Actually a bitpattern is used to determine whether the cast
        is valid or not. The cast function is approximately defined like this
        cast(a,b) {
        return (a&b) == a;
        }
        \sa qucsitem_cast and PATTERN.
      */
      enum QucsItemTypes {
         //!Recognizes all classes derived from QucsItem
         QucsItemType = (1 << (std::numeric_limits<int>::digits-1)),
         //!Recognizes classes derived from Component
         ComponentType = PATTERN(QucsItemType,1),
         //!Recognizes classes derived from MultiSymbolComponent
         MultiSymbolComponentType = PATTERN(ComponentType, 1),
         //!Recognizes classes derived from Node
         NodeType = PATTERN(QucsItemType,3),
         //!Recognizes classes derived from Wire
         WireType = PATTERN(QucsItemType, 4),
         //!Recognizes classes derived from Painting
         PaintingType = PATTERN(QucsItemType, 5),
         //!Recognizes classes derived from Display
         DisplayType = PATTERN(QucsItemType, 6)
      };

      /*!\brief Item identifier
        \sa QucsItemTypes
      */
      enum {
         Type = QucsItemType
      };

      QucsItem(QGraphicsItem* parent = 0, SchematicScene* scene = 0);
      virtual ~QucsItem();

      //!\brief Return type of item
      int type() const { return QucsItemType; }
      //!\brief Return bounding box
      QRectF boundingRect() const { return m_boundingRect; }

      SchematicScene* schematicScene() const;
      QGraphicsView* activeView() const;
      QucsMainWindow* mainWindow() const;

      //! Pure virtual method to write item's properties to writer
      virtual void writeXml(Qucs::XmlWriter *writer) = 0;
      //! Pure virtual method to read item's properties from reader
      virtual void readXml(Qucs::XmlReader *reader) = 0;

      virtual void mirrorAlong(Qt::Axis axis);
      virtual void rotate90();

      virtual void invokePropertiesDialog() = 0;

      QMenu* defaultContextMenu() const;

   protected:
      void setShapeAndBoundRect(const QPainterPath& path,
                                const QRectF& rect, qreal penWidth = 1.0);
      void init();

      //!\brief Bounding box cache
      QRectF m_boundingRect;
      //!\brief Shape cache
      QPainterPath m_shape;
};

#endif //__ITEM_H
