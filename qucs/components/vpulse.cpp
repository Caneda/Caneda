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

#include "vpulse.h"
#include "shapes.h"

vPulse::vPulse(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate();
}

void vPulse::initConstants()
{
   m_boundingRect = QRectF( -30, -14, 60, 28);

   model = "Vpulse";
   name = "V";
   description =  QObject::tr("ideal voltage pulse source");

   m_shapes.append(new Arc(-12,-12, 24, 24,     0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 18,  5, 18, 11, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line( 21,  8, 15,  8, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(-18,  5,-18, 11, Component::getPen(Qt::black,1)));
   m_shapes.append(new Line(  6, -3,  6,  3, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -6, -7, -6, -3, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -6,  3, -6,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -6, -3,  6, -3, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -6,  3,  6,  3, Component::getPen(Qt::darkBlue,2)));
}

void vPulse::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void vPulse::initProperties()
{
   addProperty("U1","0 V",QObject::tr("voltage before and after the pulse"),true);
   addProperty("U2","1 V",QObject::tr("voltage of the pulse"),true);
   addProperty("T1","0",QObject::tr("start time of the pulse"),true);
   addProperty("T2","1 ms",QObject::tr("ending time of the pulse"),true);
   addProperty("Tr","1 ns",QObject::tr("rise time of the leading edge"),false);
   addProperty("Tf","1 ns",QObject::tr("fall time of the trailing edge"),false);
}

