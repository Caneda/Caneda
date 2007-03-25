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

#include "volt_dc.h"
#include "shapes.h"

Volt_dc::Volt_dc(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate();
}

void Volt_dc::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -14, 60, 28).adjusted(-pw, -pw, pw, pw);

   model = "Vdc";
   name = "V";
   description =  QObject::tr("ideal dc voltage source");

   m_shapes.append(new Line(  4,-13,  4, 13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -4, -6, -4,  6, Component::getPen(Qt::darkBlue,4)));
   m_shapes.append(new Line( 30,  0,  4,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -4,  0,-30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11,  5, 11, 11, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line( 14,  8,  8,  8, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(-11,  5,-11, 11, Component::getPen(Qt::black,1)));
}

void Volt_dc::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void Volt_dc::initProperties()
{
   addProperty("U","1 V",QObject::tr("voltage in Volts"),true);
}

