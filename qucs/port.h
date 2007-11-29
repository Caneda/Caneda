/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __PORT_H
#define __PORT_H

#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QList>

#include <QtGui/QGraphicsItem>
#include <QtCore/QSharedData>

class Wire;
class Component;

class Element
{
   public:
      Element(Wire *wire);
      Element(Component *comp);

      Wire* wire() const { return m_wire; }
      Component* component() const { return m_component; }

      QGraphicsItem* item() const {
         return m_wire != NULL ? (QGraphicsItem*)m_wire : (QGraphicsItem*)m_component;
      }
   private:
      Wire *const m_wire;
      Component *const m_component;
      //Disable copy
      Element(const Element& other);
};

struct PortData : public QSharedData
{
   PortData(QPointF _pos, QString _name) : pos(_pos), name(_name) {}
   QPointF pos;
   QString name;
};

class Port
{
   public:
      Port(Wire *owner, const QSharedDataPointer<PortData> &data);
      Port(Wire *owner, QPointF _pos, QString portName = QString());

      Port(Component *owner, const QSharedDataPointer<PortData> &data);
      Port(Component *owner, QPointF _pos, QString portName = QString());

      ~Port();

      inline void copyDataTo(Port *other) { other->d = d; }
      inline void copyDataFrom(Port *other) { d = other->d; }

      QPointF pos() const { return d->pos; }
      QPointF scenePos(bool *ok = 0) const;

      QString name() const { return d->name; }

      Wire* wireOwner() const { return m_owner->wire(); }
      Component* componentOwner() const { return m_owner->component(); }
      Element* owner() const { return m_owner; }
      QGraphicsItem* ownerItem() const { return m_owner->item(); }

      QList<Port*> * connections() const { return m_connections; }

      static void connect(Port* port1, Port *port2);
      static void disconnect(Port *port, Port *from);
      static bool isConnected(Port *port1, Port* port2);

      void connect(Port *other);
      void disconnect(Port *from);

      Port* findIntersectingPort() const;
      Port* findCoincidingPort() const;

      void paint(QPainter *painter, const QStyleOptionGraphicsItem* option);

   private:
      Port* findIntersectingPort(const QList<Port*> &ports) const;
      Port* findCoincidingPort(const QList<Port*> &ports) const;

      QSharedDataPointer<PortData> d;
      Element *m_owner;
      QList<Port*> *m_connections;
};

void drawPorts(const QList<Port*> & ports, QPainter *painter,
               const QStyleOptionGraphicsItem* option);

#endif //__PORT_H
