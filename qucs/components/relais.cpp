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

#include "relais.h"
#include "shapes.h"

Relais::Relais(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Relais::initConstants()
{
   m_boundingRect = QRectF( -48, -30, 93, 60);

   model = "Relais";
   name = "S";
   description =  QObject::tr("relay");

   m_shapes.append(new Line(-30,-30,-30, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  8,-30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-45, -8,-15, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-45,  8,-15,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-45, -8,-45,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-15, -8,-15,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-45,  8,-15, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-15,  0, 35,  0, Component::getPen(Qt::darkBlue,1,Qt::DotLine)));
   m_shapes.append(new Line( 30,-30, 30,-18, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 15, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 15, 45,-15, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 27,-18, 5, 5, 0, 16*360, Component::getPen(Qt::darkBlue,2)));
}

void Relais::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void Relais::initProperties()
{
   addProperty("Vt","0.5 V",QObject::tr("threshold voltage in Volts"),false);
   addProperty("Vh","0.1 V",QObject::tr("hysteresis voltage in Volts"),false);
   addProperty("Ron","0",QObject::tr("resistance of \"on\" state in Ohms"),false);
   addProperty("Roff","1e12",QObject::tr("resistance of \"off\" state in Ohms"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

