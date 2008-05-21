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
#include <QtGui/QPainter>
#include <QtCore/QSharedData>

//Forward declarations.
class Wire;
class Component;
class SchematicScene;

//! \todo Make these constants settings.
static const QPen connectedPen(Qt::red, 0);
//! \todo Make these constants settings.
static const QBrush connectedBrush(Qt::cyan);
//! \todo Make these constants settings.
static const QPen unconnectedPen(Qt::darkRed, 0);
//! \todo Make these constants settings.
static const QBrush unconnectedBrush(Qt::NoBrush);
//! \todo Make these constants settings.
static const qreal portRadius(3.0);
//! \todo Make these constants settings.
static const QRectF portEllipse(-portRadius, -portRadius, 2*portRadius,
                                2*portRadius);


//! Thin class used to abstract owner of port.
class PortOwner
{
   public:
      PortOwner(Wire *wire);
      PortOwner(Component *comp);

      //! Return the wire if stored, or null otherwise.
      Wire* wire() const { return m_wire; }
      //! Return the component if stored, or null otherwise.
      Component* component() const { return m_component; }
      //! Returns the owner item as graphicsitem.
      QGraphicsItem* item() const;

      bool isWire() const { return m_wire != 0; }
      bool isComponent() const { return m_component != 0; }

   private:
      Wire *const m_wire;
      Component *const m_component;
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
      Port(Wire *owner, const QSharedDataPointer<PortData> &data);
      Port(Wire *owner, QPointF _pos, QString portName = QString());

      Port(Component *owner, const QSharedDataPointer<PortData> &data);
      Port(Component *oner, QPointF _pos, QString portName = QString());

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

      bool hasConnection() const;

      void connectTo(Port *other);
      /*! Shorthand for Port::disconnect(this, from)
	\note from == NULL is allowed 
      */  
      void disconnectFrom(Port *from) {
	Port::disconnect(this, from);
      }

      

      Port* getAnyConnectedPort();
      void removeConnections();

      static QList<Wire*> wiresBetween(Port* port1, Port* port2);

      Port* findIntersectingPort() const;
      Port* findCoincidingPort() const;

      bool areAllOwnersSelected() const;
      void paint(QPainter *painter, const QStyleOptionGraphicsItem* option);

   private:
      Port* findIntersectingPort(const QList<Port*> &ports) const;
      Port* findCoincidingPort(const QList<Port*> &ports) const;

      void setPos(const QPointF& newPos);

      friend class Wire;

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
