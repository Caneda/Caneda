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

#include "undocommands.h"
#include "item.h"
#include <QtGui/QGraphicsScene>

MoveItemCommand::MoveItemCommand(QucsItem *i,const QPointF& init,const QPointF& end) : item(i), initialPoint(init),endPoint(end)
{
   firstTime = true;
   setText("Move Component");
}

MoveItemCommand::~MoveItemCommand()
{
}
      
void MoveItemCommand::undo()
{
   item->setPos(initialPoint);
   item->scene()->clearSelection();
}

void MoveItemCommand::redo()
{
   if(firstTime)
      firstTime = false;
   else
      item->setPos(endPoint);
}

int MoveItemCommand::id() const
{
   return 5;
}
