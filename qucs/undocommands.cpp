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
#include "component.h"
#include "schematicscene.h"
#include "port.h"
#include <QtCore/QDebug>

/*
  ##########################################################################
  #                            PropertyChangeCmd                           #
  ##########################################################################
*/

static QString debugString;

static QDebug debug()
{

   return qDebug();
}

PropertyChangeCmd::PropertyChangeCmd(const QString& propertyName,
                                     const QVariant& newValue,
                                     const QVariant& oldValue,
                                     Qucs::Component *const component) :
   m_property(propertyName), m_newValue(newValue), m_oldValue(oldValue),
   m_component(component)
{
}

void PropertyChangeCmd::undo()
{
   m_component->setProperty(m_property, m_oldValue);
}

void PropertyChangeCmd::redo()
{
   m_component->setProperty(m_property, m_newValue);
   qDebug() << "\n\n" << debugString;
}

/*
  ##########################################################################
  #                                 MoveCmd                                #
  ##########################################################################
*/

MoveCmd::MoveCmd(QGraphicsItem *i,const QPointF& init,const QPointF& end) :
   m_item(i), m_initialPos(init), m_finalPos(end)
{
}

void MoveCmd::undo()
{
   if(m_item->parentItem()) {
      QPointF p = m_item->mapFromScene(m_initialPos);
      p = m_item->mapToParent(p);
      m_item->setPos(p);
   }
   else {
      m_item->setPos(m_initialPos);
   }
   debug() << "\n######################  MoveCmd::undo()  #######\n"
            << "Moving from " << m_finalPos << "to " << m_initialPos
            <<"\n#######################\n";

}

void MoveCmd::redo()
{
   if(m_item->parentItem()) {
      QPointF p = m_item->mapFromScene(m_finalPos);
      p = m_item->mapToParent(p);
      m_item->setPos(p);
   }
   else {
      m_item->setPos(m_finalPos);
   }
   debug() << "\n######################  MoveCmd::redo()  #######\n"
            << "Moving from " << m_initialPos << "to " << m_finalPos
            <<"\n##################################\n";
}

/*
  ##########################################################################
  #                            ConnectCmd                                  #
  ##########################################################################
*/

ConnectCmd::ConnectCmd(Port *p1, Port *p2) : m_port1(p1), m_port2(p2)
{
}

void ConnectCmd::undo()
{
   debug() << "\n######################  ConnectCmd::undo()  ############################";
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   debug() << "Disconencting "
            << port1Name <<  " from " << port2Name
            <<"\n##################################\n";
   m_port1->disconnectFrom(m_port2);
}

void ConnectCmd::redo()
{
   debug() << "\n######################  ConnectCmd::redo()  ############################";
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   debug() << "Conencting "
            << port1Name <<  " to " << port2Name
            <<"\n##################################\n";
   m_port1->connectTo(m_port2);
}

/*
  ##########################################################################
  #                           DisconnectCmd                                #
  ##########################################################################
*/

DisconnectCmd::DisconnectCmd(Port *p1, Port *p2) : m_port1(p1), m_port2(p2)
{
}

void DisconnectCmd::undo()
{
   debug() << "\n######################  DisConnectCmd::undo()  ############################";
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   debug() << "Conencting "
            << port1Name <<  " to " << port2Name
            <<"\n##################################\n";
   m_port1->connectTo(m_port2);
}

void DisconnectCmd::redo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   debug() << "\n######################  DisConnectCmd::redo()  ############################";
   debug() << "Disconencting "
            << port1Name <<  " from " << port2Name
            <<"\n##################################\n";
   m_port1->disconnectFrom(m_port2);
}


/*
  ##########################################################################
  #                              AddWireCmd                                #
  ##########################################################################
*/

AddWireCmd::AddWireCmd(Port *p1, Port *p2) : m_port1(p1), m_port2(p2)
{
   m_scene = m_port1->schematicScene();
   m_wire = new Qucs::Wire(m_port1, m_port2, m_scene);
   m_pos = m_wire->scenePos();
}

void AddWireCmd::undo()
{
   debug() << "\n######################  AddWireCmd::undo()  ############################";
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   debug() << "Removing wire between "
            << port1Name <<  "and"  << port2Name
            <<"\n##################################\n";
   m_wire->port1()->disconnectFrom(m_port1);
   m_wire->port2()->disconnectFrom(m_port2);
   m_scene->removeItem(m_wire);
}

void AddWireCmd::redo()
{
   debug() << "\n######################  AddWireCmd::redo()  ############################";
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   debug() << "Adding wire between "
            << port1Name <<  "and"  << port2Name
            <<"\n##################################\n";

   m_scene->addItem(m_wire);
   m_wire->setPos(m_pos);
   m_wire->port1()->connectTo(m_port1);
   m_wire->port2()->connectTo(m_port2);

   Q_ASSERT(m_wire->port1()->connections() == m_port1->connections());
   Q_ASSERT(m_wire->port2()->connections() == m_port2->connections());
}

/*
  ##########################################################################
  #                           WireStateChangeCmd                           #
  ##########################################################################
*/

WireStateChangeCmd::WireStateChangeCmd(Qucs::Wire *wire, WireStore initState,
                                       WireStore finalState) :
   m_wire(wire), m_initState(initState), m_finalState(finalState)
{
}

void WireStateChangeCmd::undo()
{
   debug() << "\n######################  WireStateChangeCmd::undo()  ############################";
   debug() << "Setting intial state"
            << "\nWirelines is " << m_initState.wLines
            << "\nPort1 pos = " << m_initState.port1Pos
            << "\nPort2 pos = " << m_initState.port2Pos
            <<"\n##################################\n";
   m_wire->setState(m_initState);
   m_wire->/*schematicScene()->*/update();
}

void WireStateChangeCmd::redo()
{
   debug() << "\n######################  WireStateChangeCmd::redo()  ############################";
   debug() << "Setting fina state"
            << "\nWirelines is " << m_finalState.wLines
            << "\nPort1 pos = " << m_finalState.port1Pos
            << "\nPort2 pos = " << m_finalState.port2Pos
            <<"\n##################################\n";
   m_wire->setState(m_finalState);
   m_wire->/*schematicScene()->*/update();
}
