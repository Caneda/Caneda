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
namespace Qucs {
   class Component;
}
class Port;

class PropertyChangeCmd : public QUndoCommand
{
   public:
      PropertyChangeCmd(const QString& propertyName,
                        const QVariant& newValue,
                        const QVariant& oldValue,
                        Qucs::Component *const component);

      virtual void undo();
      virtual void redo();

   private:
      const QString m_property;
      const QVariant m_newValue;
      const QVariant m_oldValue;
      Qucs::Component *const m_component;
};

class MoveCmd : public QUndoCommand
{
   public:
      MoveCmd(QGraphicsItem *i,const QPointF& init, const QPointF& final);
      void undo();
      void redo();

   private:
      QGraphicsItem * const m_item;
      QPointF m_initialPos;
      QPointF m_finalPos;
};

class ConnectCmd : public QUndoCommand
{
   public:
      ConnectCmd(Port *p1, Port *p2);
      void undo();
      void redo();

   private:
      Port * const m_port1;
      Port * const m_port2;
};

class DisconnectCmd : public QUndoCommand
{
   public:
      DisconnectCmd(Port *p1, Port *p2);
      void undo();
      void redo();

   private:
      Port * const m_port1;
      Port * const m_port2;
};

class AddWireCmd : public QUndoCommand
{
   public:
      AddWireCmd(Port *p1, Port* p2);
      void undo();
      void redo();

      Qucs::Wire* wire() const { return m_wire; }

   private:
      Port * const m_port1;
      Port * const m_port2;
      SchematicScene *m_scene;
      Qucs::Wire *m_wire;
      QPointF m_pos;
};

class WireStateChangeCmd : public QUndoCommand
{
   public:
      typedef Qucs::Wire::Store WireStore;
      WireStateChangeCmd(Qucs::Wire *wire,WireStore initState, WireStore finalState);

      void undo();
      void redo();

   private:
      Qucs::Wire *m_wire;
      Qucs::Wire::Store m_initState, m_finalState;
};

#endif
