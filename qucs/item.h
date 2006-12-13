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

class QGraphicsScene;
class MoveItemCommand;
class SchematicScene;

class QucsItem : public QGraphicsItem
{
   public:
      enum QucsItemTypes {
	 QucsItemType = UserType+5,
	 PaintingType,
	 WireType,
	 PortType,
	 NodeType,
	 ComponentType,
	 DisplayType
      };

      QucsItem(QGraphicsItem* parent = 0l, QGraphicsScene* scene = 0l);
      virtual ~QucsItem() {};

      int type() const;
      QRectF boundingRect() const {return QRectF(); }

      MoveItemCommand* createMoveItemCommand();
      void backupUndoPosition();
      QPointF savedUndoPosition() const;

      SchematicScene* schematicScene() const;
      
   protected:
      QPointF m_savedUndoPosition;

};

#endif //__ITEM_H
