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

#include "ampere_ac.h"
#include "shapes.h"

Ampere_ac::Ampere_ac(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate();
}

void Ampere_ac::initConstants()
{
   m_boundingRect = QRectF( -30, -14, 60, 30);

   model = "Iac";
   name = "I";
   description =  QObject::tr("ideal ac current source");

   m_shapes.append(new Arc(-12,-12, 24, 24,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7,  0,  7,  0, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  6,  0,  0, -4, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  6,  0,  0,  4, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Arc( 12,  5,  6,  6,16*270, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 12, 11,  6,  6, 16*90, 16*180, Component::getPen(Qt::darkBlue,2)));
}

void Ampere_ac::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void Ampere_ac::initProperties()
{
   addProperty("I","1 mA",QObject::tr("peak current in Ampere"),true);
   addProperty("f","1 GHz",QObject::tr("frequency in Hertz"),false);
   addProperty("Phase","0",QObject::tr("initial phase in degrees"),false);
   addProperty("Theta","0",QObject::tr("damping factor (transient simulation only)"),false);
}

