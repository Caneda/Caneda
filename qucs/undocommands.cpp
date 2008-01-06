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

PropertyChangeCmd::PropertyChangeCmd(const QString& propertyName,
                                     const QVariant& newValue,
                                     const QVariant& oldValue,
                                     Component *const component,
                                     QUndoCommand *parent) :
   QUndoCommand(parent),
   m_property(propertyName), m_newValue(newValue), m_oldValue(oldValue),
   m_component(component)
{
}

void PropertyChangeCmd::undo()
{
   m_component->setProperty(m_property, m_oldValue);
   qDebug() << "PropertyChangeCmd::undo()\n";
}

void PropertyChangeCmd::redo()
{
   m_component->setProperty(m_property, m_newValue);
   qDebug() << "PropertyChangeCmd::redo()\n";
}

/*
  ##########################################################################
  #                                 MoveCmd                                #
  ##########################################################################
*/

MoveCmd::MoveCmd(QGraphicsItem *i,const QPointF& init,const QPointF& end,
                 QUndoCommand *parent) :
   QUndoCommand(parent),
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
   qDebug() << "MoveCmd::undo()\n";

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
   qDebug() << "MoveCmd::redo()\n";
}

/*
  ##########################################################################
  #                            ConnectCmd                                  #
  ##########################################################################
*/

struct WireConnectionInfo
{
      WireConnectionInfo(Wire *w, Port *p1 = 0, Port *p2 = 0) :
         wire(w), port1(p1), port2(p2)
      {
      }

      ~WireConnectionInfo()
      {
         if(!wire->scene()) {
            qDebug() << "WireConnectionInfo::destructor Deleted sceneless wire";
            delete wire;
         }
      }

      Wire *const wire;
      Port *port1;
      Port *port2;
      QPointF wirePos;
};

ConnectCmd::ConnectCmd(Port *p1, Port *p2, const QList<Wire*> &wires,
                       SchematicScene *scene, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_port1(p1), m_port2(p2), m_scene(scene)
{
   foreach(Wire *w, wires) {
      m_wireConnectionInfos << new WireConnectionInfo(w);
   }
}

ConnectCmd::~ConnectCmd()
{
   qDeleteAll(m_wireConnectionInfos);
}

void ConnectCmd::undo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   qDebug() << "ConnectCmd::undo() : Disconencting "
            << port1Name <<  " from " << port2Name << '\n';

   m_port1->disconnectFrom(m_port2);

   foreach(WireConnectionInfo *info, m_wireConnectionInfos) {
      m_scene->addItem(info->wire);
      info->wire->setPos(info->wirePos);

      info->wire->port1()->connectTo(info->port1);
      info->wire->port2()->connectTo(info->port2);
   }
}

void ConnectCmd::redo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   qDebug() << "ConnectCmd::redo() : Conencting "
            << port1Name <<  " to " << port2Name << '\n';
   foreach(WireConnectionInfo *info, m_wireConnectionInfos) {
      info->port1 = info->wire->port1()->getAnyConnectedPort();
      info->port2 = info->wire->port2()->getAnyConnectedPort();

      Q_ASSERT(info->port1);
      Q_ASSERT(info->port2);

      info->wire->port1()->removeConnections();
      info->wire->port2()->removeConnections();

      info->wirePos = info->wire->pos();
      m_scene->removeItem(info->wire);
   }

   m_port1->connectTo(m_port2);
}

/*
  ##########################################################################
  #                           DisconnectCmd                                #
  ##########################################################################
*/

DisconnectCmd::DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_port1(p1), m_port2(p2)
{
}

void DisconnectCmd::undo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   qDebug() << "DisconnectCmd::undo() : Conencting "
            << port1Name <<  " to " << port2Name << '\n';
   m_port1->connectTo(m_port2);
}

void DisconnectCmd::redo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   qDebug() << "DisconnectCmd::undo() : Disconencting "
            << port1Name <<  " from " << port2Name << '\n';
   m_port1->disconnectFrom(m_port2);
}


/*
  ##########################################################################
  #                              AddWireCmd                                #
  ##########################################################################
*/

AddWireCmd::AddWireCmd(Port *p1, Port *p2, QUndoCommand *parent) :
   QUndoCommand(parent), m_port1(p1), m_port2(p2)
{
   m_scene = m_port1->schematicScene();
   m_wire = new Wire(m_port1, m_port2, m_scene);
   m_pos = m_wire->scenePos();
}

void AddWireCmd::undo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   qDebug() << "AddWireCmd::undo() : Removing wire between "
            << port1Name <<  "and"  << port2Name << '\n';
   m_wire->port1()->disconnectFrom(m_port1);
   m_wire->port2()->disconnectFrom(m_port2);
   m_scene->removeItem(m_wire);
}

void AddWireCmd::redo()
{
   QString port1Name = m_port1->owner()->component() ? m_port1->owner()->component()->name() : "Wire";
   QString port2Name = m_port2->owner()->component() ? m_port2->owner()->component()->name() : "Wire";
   qDebug() << "AddWireCmd::redo() : Adding wire between "
            << port1Name <<  "and"  << port2Name << '\n';

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

WireStateChangeCmd::WireStateChangeCmd(Wire *wire, WireStore initState,
                                       WireStore finalState, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_wire(wire), m_initState(initState), m_finalState(finalState)
{
}

void WireStateChangeCmd::undo()
{
   qDebug() << "WireStateChangeCmd::undo() : Setting intial state"
            << "\nWirelines is " << m_initState.wLines
            << "\nPort1 pos = " << m_initState.port1Pos
            << "\nPort2 pos = " << m_initState.port2Pos
            <<"\n\n";
   m_wire->setState(m_initState);
   m_wire->update();
}

void WireStateChangeCmd::redo()
{
   qDebug() << "WireStateChangeCmd::redo() : Setting final state"
            << "\nWirelines is " << m_finalState.wLines
            << "\nPort1 pos = " << m_finalState.port1Pos
            << "\nPort2 pos = " << m_finalState.port2Pos
            <<"\n\n";
   m_wire->setState(m_finalState);
   m_wire->update();
}

/*
  ##########################################################################
  #                           InsertItemCmd                                #
  ##########################################################################
*/

InsertItemCmd::InsertItemCmd(QGraphicsItem *const item, SchematicScene *scene,
                             QPointF pos, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_item(item), m_scene(scene)
{
   Q_ASSERT(scene);
   if(pos == InvalidPoint) {
      m_pos = m_item->scenePos();
   }
   else {
      m_pos = pos;
   }
}

InsertItemCmd::~InsertItemCmd()
{
   if(!m_item->scene()) {
      qDebug() << "InsertItemCmd destructor: Destructing item";
      delete m_item;
   }
}

void InsertItemCmd::undo()
{
   qDebug() << "InsertItemCmd::undo()\n";
   m_scene->removeItem(m_item);
}

void InsertItemCmd::redo()
{
   qDebug() << "InsertItemCmd::redo()\n";
   if(m_scene != m_item->scene()) {
      m_scene->addItem(m_item);
   }
   m_item->setPos(m_pos);
   Component *comp = qucsitem_cast<Component*>(m_item);
   if(comp) {
      comp->updatePropertyGroup();
   }
}

QUndoCommand* insertComponentCmd(Component *const item, SchematicScene *scene,
                                 QPointF pos, QUndoCommand *parent)
{
   QUndoCommand *complexCmd = new QUndoCommand(parent);
   InsertItemCmd *cmd = new InsertItemCmd(item, scene, pos, complexCmd);
   cmd->redo();
   item->checkAndConnect(true, complexCmd);
   return complexCmd;
}

/*
  ##########################################################################
  #                          RotateItemsCmd                                #
  ##########################################################################
*/

RotateItemsCmd::RotateItemsCmd(QList<QucsItem*> items, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_items(items)
{
}

RotateItemsCmd::RotateItemsCmd(QucsItem *item, QUndoCommand *parent) :
   QUndoCommand(parent)
{
   m_items << item;
}

void RotateItemsCmd::undo()
{
   foreach(QucsItem *item, m_items) {
      item->rotate90(Qucs::Clockwise);
   }
}

void RotateItemsCmd::redo()
{
   foreach(QucsItem *item, m_items) {
      item->rotate90(Qucs::AntiClockwise);
   }
}

/*
  ##########################################################################
  #                          MirrorItemsCmd                                #
  ##########################################################################
*/

MirrorItemsCmd::MirrorItemsCmd(QList<QucsItem*> items, Qt::Axis axis, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_items(items),
   m_axis(axis)
{
}

MirrorItemsCmd::MirrorItemsCmd(QucsItem *item, Qt::Axis axis, QUndoCommand *parent) :
   QUndoCommand(parent),
   m_axis(axis)
{
   m_items << item;
}

void MirrorItemsCmd::undo()
{
   foreach(QucsItem *item, m_items) {
      item->mirrorAlong(m_axis);
   }
}

void MirrorItemsCmd::redo()
{
   foreach(QucsItem *item, m_items) {
      item->mirrorAlong(m_axis);
   }
}
