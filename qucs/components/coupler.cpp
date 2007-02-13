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

#include "coupler.h"
#include "propertytext.h"
#include "schematicscene.h"

#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionGraphicsItem>

Coupler::Coupler(QGraphicsScene *s) : Component(0,s)
{
   initComponentStrings();
   m_ports.append(new ComponentPort(this,QPointF(-30,-20)));
   m_ports.append(new ComponentPort(this,QPointF( 30,-20)));
   m_ports.append(new ComponentPort(this,QPointF( 30, 20)));
   m_ports.append(new ComponentPort(this,QPointF(-30, 20)));
}

void Coupler::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
   Q_UNUSED(w);
   initPainter(p,o);
   p->drawLine(-23,-24, 23,-24);
   p->drawLine( 23,-24, 23, 24);
   p->drawLine( 23, 24,-23, 24);
   p->drawLine(-23, 24,-23,-24);

   p->drawLine(-27,-20,-20,-20);
   p->drawLine( 27,-20, 20,-20);
   p->drawLine(-20,-20, 20,-20);
   p->drawLine(-27, 20,-20, 20);
   p->drawLine( 27, 20, 20, 20);
   p->drawLine(-20, 20, 20, 20);

   p->drawLine( 14, 14,-14,-14);
   p->drawLine(-14,-14, -9,-14);
   p->drawLine(-14,-14,-14, -9);
   p->drawLine(  9, 14, 14, 14);
   p->drawLine( 14,  9, 14, 14);

   p->drawLine( 14,-14,-14, 14);
   p->drawLine( 14,-14,  9,-14);
   p->drawLine( 14,-14, 14, -9);
   p->drawLine(-14, 14, -9, 14);
   p->drawLine(-14, 14,-14,  9);

   if(o->state & QStyle::State_Open)
      drawNodes(p);
}

void Coupler::initComponentStrings()
{
   model = "Coupler";
   name  = "X";
   description = QObject::tr("ideal coupler");
}
