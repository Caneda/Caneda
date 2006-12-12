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

#ifndef __NODE_H
#define __NODE_H

#include "item.h"
#include <QtCore/QSet>

class Component;

class Node : public QucsItem
{
   public:
      static const qreal Radius;
      
      Node(const QString& name = QString(),QGraphicsScene *scene = 0l);
      ~Node();
      
      void addComponent(Component *comp);
      void removeComponent(Component *comp);
      const QSet<Component*>& connectedComponents() const;

      virtual void paint(QPainter *p,const QStyleOptionGraphicsItem *o, QWidget *w = 0l);
      virtual bool contains(const QPointF& pt) const;

      QString name() const;
      void setName(const QString& name);

      QRectF boundingRect() const;
      bool collidesWith(const Node* node) const;

      bool isOpen() const;
      bool areAllComponentsSelected() const;

      QSet<Component*> selectedComponents();
      QSet<Component*> unselectedComponents();
      void setNewPos(const QPointF& np);
      QPointF newPos() const;

      void setController(Component *c);
      void resetController();
      bool isControllerSet() const;

      int type() const;
      
   protected:
      QVariant itemChange(GraphicsItemChange change, const QVariant& val);
      
   private:
      QSet<Component*> m_connectedComponents;
      QString m_name;
      
      QPointF m_newPos; // to cache new pos before attaining it
      Component *m_controller;
};

#endif //__NODE_H
