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

#include "isolator.h"
#include "shapes.h"

Isolator::Isolator(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Isolator::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -17, 60, 34).adjusted(-pw, -pw, pw, pw);

   model = "Isolator";
   name = "X";
   description =  QObject::tr("isolator");

   m_shapes.append(new Line( -8,  0,  8,  0, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  8,  0,  0, -5, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  8,  0,  0,  5, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(-14,-14, 14,-14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14, 14, 14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14,-14,-14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14,-14, 14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-14,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
}

void Isolator::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void Isolator::initProperties()
{
   addProperty("Z1","50 Ohm",QObject::tr("reference impedance of input port"),false);
   addProperty("Z2","50 Ohm",QObject::tr("reference impedance of output port"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

