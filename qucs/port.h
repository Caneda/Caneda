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

//Forward declarations.
namespace Qucs
{
   class Wire;
   class Component;
}
class SchematicScene;

//! Thin class used to abstract owner of port.
class PortOwner
{
   public:
      PortOwner(Qucs::Wire *wire);
      PortOwner(Qucs::Component *comp);

      //! Return the wire if stored, or null otherwise.
      Qucs::Wire* wire() const { return m_wire; }
      //! Return the component if stored, or null otherwise.
      Qucs::Component* component() const { return m_component; }
      //! Returns the owner item as graphicsitem.
      QGraphicsItem* item() const;

   private:
      Qucs::Wire *const m_wire;
      Qucs::Component *const m_component;
      //Disable copy
      PortOwner(const PortOwner& other);
};

//! Class used to represend sharable port's data.
struct PortData : public QSharedData
{
   PortData(QPointF _pos, QString _name) : pos(_pos), name(_name) {}

   QPointF pos;
   QString name;
};

class Port
{
   public:
      Port(Qucs::Wire *owner, const QSharedDataPointer<PortData> &data);
      Port(Qucs::Wire *owner, QPointF _pos, QString portName = QString());

      Port(Qucs::Component *owner, const QSharedDataPointer<PortData> &data);
      Port(Qucs::Component *oner, QPointF _pos, QString portName = QString());

      ~Port();

      //! Returns the position relative to owner - usually constant.
      QPointF pos() const { return d->pos; }

      QPointF scenePos(bool *ok = 0) const;
      //! Returns the port's name.
      QString name() const { return d->name; }
      //! Returns the owner.
      PortOwner* owner() const { return m_owner; }
      //! Shorthand method for owner->item()
      QGraphicsItem* ownerItem() const { return m_owner->item(); }
      //! Returns a pointer to list of connected ports(null if unconnected).
      QList<Port*> * connections() const { return m_connections; }

      SchematicScene* schematicScene() const;

      static void connect(Port* port1, Port *port2);
      static void disconnect(Port *port, Port *from);

      static bool isConnected(Port *port1, Port* port2);

      void connectTo(Port *other);
      void disconnectFrom(Port *from);

      Port* findIntersectingPort() const;
      Port* findCoincidingPort() const;

      bool areAllOwnersSelected() const;
      void paint(QPainter *painter, const QStyleOptionGraphicsItem* option);

   private:
      Port* findIntersectingPort(const QList<Port*> &ports) const;
      Port* findCoincidingPort(const QList<Port*> &ports) const;

      void setPos(const QPointF& newPos);

      friend class Qucs::Wire;

      QSharedDataPointer<PortData> d;
      PortOwner *m_owner;
      QList<Port*> *m_connections;
      /*! This represents the connection name.
       * \todo Implement this feature.
       */
      QString *m_nodeName;
};

void drawPorts(const QList<Port*> & ports, QPainter *painter,
               const QStyleOptionGraphicsItem* option);

QRectF portsRect(const QList<Port*> &ports, const QRectF& rect);

void addPortEllipses(const QList<Port*> &ports, QPainterPath &path);

#endif //__PORT_H
