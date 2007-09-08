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
#include "components/component.h"
#include "node.h"
#include "wire.h"
#include "schematicscene.h"

#include <QtCore/QDebug>

UndoCommand::UndoCommand() : firstTime(true)
{
}

void UndoCommand::undo()
{
   this->undoIt();
}

void UndoCommand::redo()
{
   if(firstTime == true)
      firstTime = false;
   else
      this->redoIt();
}

MoveCommand::MoveCommand(QucsItem *i,const QPointF& init,const QPointF& end) : item(i), initialPoint(init),endPoint(end)
{
   setText(QObject::tr("Move Component"));
}

void MoveCommand::undoIt()
{
   item->setPos(initialPoint);
   item->scene()->clearSelection();
}

void MoveCommand::redoIt()
{
   item->setPos(endPoint);
}

int MoveCommand::id() const
{
   return UndoCommand::Move;
}

/*ConnectCommand::ConnectCommand(ComponentPort *p1,ComponentPort *p2) : port1(p1),port2(p2)
{
   setText("Connect component");
}

ConnectCommand::~ConnectCommand()
{

}

int ConnectCommand::id() const
{
   return UndoCommand::Connect;
}

void ConnectCommand::undoIt()
{
   Node *n1 = 0;
   if(port1->node() == port2->node())
   {
      n1 = port1->owner()->schematicScene()->createNode(port1->node()->pos());
      n1->addComponent(port1->owner());
      port1->setNode(n1);
   }
   else
      Q_ASSERT(0);
   {
      //Wire *w = Wire::connectedWire(port1->node(),port2->node());
      //if(w)
      //{
	// port1->node()->removeComponent(w);
	// port2->node()->removeComponent(w);
	// delete w;
	// }
   }
   port2->node()->removeComponent(port1->owner());
   port1->node()->removeComponent(port2->owner());
}

void ConnectCommand::redoIt()
{
   Q_ASSERT(port1->node() != port2->node());
   port2->node()->connectedComponents().unite(port1->node()->connectedComponents());
   Node *n = port1->node();
   foreach(Component *c, n->connectedComponents())
   {
      ComponentPort *p = c->portWithNode(n);
      if(p)
	 p->setNode(port2->node());
   }
   delete n;
   qDebug("ConnectCommand::redoIt()  port1Size : %d,   port2Size: %d",port1->node()->connectedComponents().size(),port2->node()->connectedComponents().size());
}


DisconnectCommand::DisconnectCommand(ComponentPort *p1,ComponentPort *p2) : port1(p1),port2(p2)
{
   setText("Disconnect component");
}


int DisconnectCommand::id() const
{
   return UndoCommand::Disconnect;
}

void DisconnectCommand::undoIt()
{
   Q_ASSERT(port1->node() != port2->node());
   port2->node()->connectedComponents().unite(port1->node()->connectedComponents());
   Node *n = port1->node();
   foreach(Component *c, n->connectedComponents())
   {
      ComponentPort *p = c->portWithNode(n);
      if(p)
	 p->setNode(port2->node());
   }
   delete n;
   qDebug("DisconnectCommand::undoIt()  port1Size : %d,   port2Size: %d",port1->node()->connectedComponents().size(),
	  port2->node()->connectedComponents().size());
}

void DisconnectCommand::redoIt()
{
   Q_ASSERT(port1->node() == port2->node());
   Node *n1 = 0;
   n1 = port1->owner()->schematicScene()->createNode(port1->node()->pos());
   n1->addComponent(port1->owner());
   port1->setNode(n1);

   port2->node()->removeComponent(port1->owner());
   port1->node()->removeComponent(port2->owner());
   qDebug() << "DisconnectCommand::redoIt()";
}



AddWireCommand::AddWireCommand(ComponentPort *p1, ComponentPort *p2,Wire *w) :port1(p1),port2(p2),wire(w)
{
   setText(QObject::tr("Add wire"));
}

int AddWireCommand::id() const
{
   return AddWire;
}

void AddWireCommand::undoIt()
{
   Q_ASSERT(wire != 0);
   qDebug() << "AddWireCommand::undoIt()";

   delete wire;
   wire = 0;
}

void AddWireCommand::redoIt()
{
   Q_ASSERT(port1->node() != port2->node());
   Q_ASSERT(wire == 0);
   wire = new Wire(port1->owner()->schematicScene(),port1->node(),port2->node());
   qDebug() << "AddWireCommand::redoIt()";
}

RemoveWireCommand::RemoveWireCommand(ComponentPort *p1, ComponentPort *p2) :port1(p1),port2(p2)
{
   wire = 0;
   setText(QObject::tr("Remove wire"));
}

int RemoveWireCommand::id() const
{
   return RemoveWire;
}

void RemoveWireCommand::undoIt()
{
   Q_ASSERT(port1->node() != port2->node());
   Q_ASSERT(wire == 0);
   wire = new Wire(port1->owner()->schematicScene(),port1->node(),port2->node());
}

void RemoveWireCommand::redoIt()
{
   Q_ASSERT(wire != 0);
   Q_ASSERT(port1->node() != port2->node());
   delete wire;
   wire = 0;
}
*/
