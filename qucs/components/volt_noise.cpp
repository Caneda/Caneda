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

#include "volt_noise.h"
#include "shapes.h"

Volt_noise::Volt_noise(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate90();
}

void Volt_noise::initConstants()
{
   m_boundingRect = QRectF( -30, -15, 60, 30);

   model = "Vnoise";
   name = "V";
   description =  QObject::tr("noise voltage source");

   m_shapes.append(new Arc(-12,-12, 24, 24,     0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,  1,  1,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,  6,  6,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7, 10, 10, -7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -2, 12, 12, -2, Component::getPen(Qt::darkBlue,2)));
}

void Volt_noise::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void Volt_noise::initProperties()
{
   addProperty("u","1e-6",QObject::tr("voltage power spectral density in V^2/Hz"),true);
   addProperty("e","0",QObject::tr("frequency exponent"),false);
   addProperty("c","1",QObject::tr("frequency coefficient"),false);
   addProperty("a","0",QObject::tr("additive frequency term"),false);
}

