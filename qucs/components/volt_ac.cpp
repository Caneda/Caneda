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

#include "volt_ac.h"
#include "shapes.h"

Volt_ac::Volt_ac(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate90();
}

void Volt_ac::initConstants()
{
   m_boundingRect = QRectF( -30, -14, 60, 28);

   model = "Vac";
   name = "V";
   description =  QObject::tr("ideal ac voltage source");

   m_shapes.append(new Arc(-12,-12, 24, 24,     0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -3, -7,  7,  7,16*270, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -3,  0,  7,  7, 16*90, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 18,  5, 18, 11, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line( 21,  8, 15,  8, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(-18,  5,-18, 11, Component::getPen(Qt::black,1)));
}

void Volt_ac::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void Volt_ac::initProperties()
{
   addProperty("U","1 V",QObject::tr("peak voltage in Volts"),true);
   addProperty("f","1 GHz",QObject::tr("frequency in Hertz"),false);
   addProperty("Phase","0",QObject::tr("initial phase in degrees"),false);
   addProperty("Theta","0",QObject::tr("damping factor (transient simulation only)"),false);
}

