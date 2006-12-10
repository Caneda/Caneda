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

#ifndef __UNDOCOMMANDS_H
#define __UNDOCOMMANDS_H

#include <QtGui/QUndoCommand>
#include <QtCore/QPointF>

class QucsItem;

class MoveItemCommand : public QUndoCommand
{
   public:
      MoveItemCommand(QucsItem *i,const QPointF& init,const QPointF& end);
      ~MoveItemCommand();

      void undo();
      void redo();
      int id() const;
      
   private:
      QucsItem *item;
      QPointF initialPoint;
      QPointF endPoint;
      bool firstTime;
};

#endif
