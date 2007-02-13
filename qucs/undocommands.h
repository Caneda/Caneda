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

class UndoCommand : public QUndoCommand
{
   public:
      enum CommandIds
      {
	 Move = 0,
	 Connect,
	 Disconnect,
	 AddWire,
	 RemoveWire
      };
      
      UndoCommand();

      void undo();
      void redo();

   protected:
      virtual void undoIt()=0;
      virtual void redoIt()=0;
   private:
      bool firstTime;
};

class MoveCommand : public UndoCommand
{
   public:
      MoveCommand(QucsItem *i,const QPointF& init,const QPointF& end);
      int id() const;
   protected:
      void undoIt();
      void redoIt();
   private:
      QucsItem * const item;
      QPointF initialPoint;
      QPointF endPoint;
};

class ComponentPort;

class ConnectCommand : public UndoCommand
{
   public:
      ConnectCommand(ComponentPort *p1,ComponentPort *p2);
      ~ConnectCommand();
      int id() const;
   protected:
      void undoIt();
      void redoIt();

   private:
      ComponentPort * const port1;
      ComponentPort * const port2;
};

class DisconnectCommand : public UndoCommand
{
   public:
      DisconnectCommand(ComponentPort *p1,ComponentPort *p2);
      //~ConnectCommand();
      int id() const;
   protected:
      void undoIt();
      void redoIt();

   private:
      ComponentPort * const port1;
      ComponentPort * const port2;
};

class Wire;

class AddWireCommand : public UndoCommand
{
   public:
      AddWireCommand(ComponentPort *p1, ComponentPort *p2, Wire *w);
      ~AddWireCommand(){};
      int id() const;
   protected:
      void undoIt();
      void redoIt();

   private:
      
      ComponentPort * const port1;
      ComponentPort * const port2;
      Wire *wire;
};

class RemoveWireCommand : public UndoCommand
{
   public:
      RemoveWireCommand(ComponentPort *p1, ComponentPort *p2);
      ~RemoveWireCommand(){};
      int id() const;
   protected:
      void undoIt();
      void redoIt();

   private:
      Wire *wire;
      ComponentPort * const port1;
      ComponentPort * const port2; 
};

#endif
