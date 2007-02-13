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

#ifndef __CAPACITOR_H
#define __CAPACITOR_H

#include "component.h"

class Capacitor : public Component
{
   public:
      Capacitor(QGraphicsScene *scene);
      ~Capacitor(){}

      QString netlist() const;

      void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w = 0);
      inline QRectF boundingRect() const;
};

inline QRectF Capacitor::boundingRect() const
{
   qreal pw = 0.5;
   return QRectF(-27,-11,54,22).adjusted(-pw,-pw,pw,pw);
}

#endif //__CAPACITOR_H
