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

#include "attenuator.h"
#include "shapes.h"

Attenuator::Attenuator(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Attenuator::initConstants()
{
   m_boundingRect = QRectF( -30, -17, 60, 34);

   model = "Attenuator";
   name = "X";
   description =  QObject::tr("attenuator");

   m_shapes.append(new Line( -4, -6, -4,  6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -4, -6,  4, -6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  4, -6,  4,  6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -4,  6,  4,  6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,-11,  0, -6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,  6,  0, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14,-14, 14,-14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14, 14, 14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14,-14,-14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14,-14, 14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-14,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
}

void Attenuator::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void Attenuator::initProperties()
{
   addProperty("L","10 dB",QObject::tr("power attenuation"),true);
   addProperty("Zref","50 Ohm",QObject::tr("reference impedance"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

