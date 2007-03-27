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

#include "circulator.h"
#include "shapes.h"

Circulator::Circulator(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Circulator::initConstants()
{
   m_boundingRect = QRectF( -30, -16, 60, 46);

   model = "Circulator";
   name = "X";
   description =  QObject::tr("circulator");

   m_shapes.append(new Arc(-14,-14, 28, 28,  0,16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-14,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 14,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, 14,  0, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -8, -6, 16, 16,16*20,16*150, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  8,  0,  9, -7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  8,  0,  2, -1, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-22, -4,-26,  4, Component::getPen(Qt::darkBlue,2)));   // marks por));
}

void Circulator::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
   addPort(QPointF(0,30));
}

void Circulator::initProperties()
{
   addProperty("Z1","50 Ohm",QObject::tr("reference impedance of port 1"),false);
   addProperty("Z2","50 Ohm",QObject::tr("reference impedance of port 2"),false);
   addProperty("Z3","50 Ohm",QObject::tr("reference impedance of port 3"),false);
}

