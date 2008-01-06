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

#ifndef __UNDOCMDS_H
#define __UNDOCMDS_H

#include "wire.h"

#include <QtGui/QUndoCommand>
#include <QtCore/QPointF>
#include <QtCore/QVariant>

class QucsItem;
class Component;
class Port;

static const QPointF InvalidPoint(-30000, -30000);

class PropertyChangeCmd : public QUndoCommand
{
   public:
      PropertyChangeCmd(const QString& propertyName,
                        const QVariant& newValue,
                        const QVariant& oldValue,
                        Component *const component,
                        QUndoCommand *parent = 0);

      virtual void undo();
      virtual void redo();

   private:
      const QString m_property;
      const QVariant m_newValue;
      const QVariant m_oldValue;
      Component *const m_component;
};

class MoveCmd : public QUndoCommand
{
   public:
      MoveCmd(QGraphicsItem *i,const QPointF& init, const QPointF& final,
              QUndoCommand *parent = 0);
      void undo();
      void redo();

   private:
      QGraphicsItem * const m_item;
      QPointF m_initialPos;
      QPointF m_finalPos;
};

class WireConnectionInfo;
class ConnectCmd : public QUndoCommand
{
   public:
      ConnectCmd(Port *p1, Port *p2, const QList<Wire*> &wires,
                 SchematicScene *scene, QUndoCommand *parent = 0);
      ~ConnectCmd();

      void undo();
      void redo();

   private:
      Port * const m_port1;
      Port * const m_port2;
      QList<WireConnectionInfo*> m_wireConnectionInfos;

      SchematicScene *const m_scene;
};

class DisconnectCmd : public QUndoCommand
{
   public:
      DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent = 0);
      void undo();
      void redo();

   private:
      Port * const m_port1;
      Port * const m_port2;
};

class AddWireCmd : public QUndoCommand
{
   public:
      AddWireCmd(Port *p1, Port* p2, QUndoCommand *parent = 0);
      void undo();
      void redo();

      Wire* wire() const { return m_wire; }

   private:
      Port * const m_port1;
      Port * const m_port2;
      SchematicScene *m_scene;
      Wire *m_wire;
      QPointF m_pos;
};

class WireStateChangeCmd : public QUndoCommand
{
   public:
      typedef Wire::Store WireStore;
      WireStateChangeCmd(Wire *wire,WireStore initState, WireStore finalState,
                         QUndoCommand *parent = 0);

      void undo();
      void redo();

   private:
      Wire *m_wire;
      Wire::Store m_initState, m_finalState;
};

class InsertItemCmd : public QUndoCommand
{
   public:
      InsertItemCmd(QGraphicsItem *const item, SchematicScene *scene,
                    QPointF pos = InvalidPoint, QUndoCommand *parent = 0);
      ~InsertItemCmd();

      void undo();
      void redo();

   protected:
      QGraphicsItem *const m_item;
      SchematicScene *const m_scene;
      QPointF m_pos;
};

QUndoCommand* insertComponentCmd(Component *const item, SchematicScene *scene,
                            QPointF pos = InvalidPoint, QUndoCommand *parent = 0);

class RotateItemsCmd : public QUndoCommand
{
   public:
      RotateItemsCmd(QList<QucsItem*> items, QUndoCommand *parent = 0);
      RotateItemsCmd(QucsItem *item, QUndoCommand *parent = 0);

      void undo();
      void redo();

   protected:
      QList<QucsItem*> m_items;
};

class MirrorItemsCmd : public QUndoCommand
{
   public:
      MirrorItemsCmd(QList<QucsItem*> items, Qt::Axis axis, QUndoCommand *parent = 0);
      MirrorItemsCmd(QucsItem *item, Qt::Axis axis, QUndoCommand *parent = 0);

      void undo();
      void redo();

   protected:
      QList<QucsItem*> m_items;
      Qt::Axis m_axis;
};

#endif
