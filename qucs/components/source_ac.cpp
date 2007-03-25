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

#include "source_ac.h"
#include "shapes.h"

Source_ac::Source_ac(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate();
}

void Source_ac::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -14, 60, 28).adjusted(-pw, -pw, pw, pw);

   model = "Pac";
   name = "P";
   description =  QObject::tr("ac power source");

   m_shapes.append(new Line(-22,-11, 22,-11, Component::getPen(Qt::darkGray,0)));
   m_shapes.append(new Line(-22, 11, 22, 11, Component::getPen(Qt::darkGray,0)));
   m_shapes.append(new Line(-22,-11,-22, 11, Component::getPen(Qt::darkGray,0)));
   m_shapes.append(new Line( 22,-11, 22, 11, Component::getPen(Qt::darkGray,0)));
   m_shapes.append(new Arc(-19, -9, 18, 18,     0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-13, -6,  6,  6,16*270, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-13,  0,  6,  6, 16*90, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-19,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 19,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -1,  0,  3,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  3, -5, 19, -5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  3,  5, 19,  5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  3, -5,  3,  5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 19, -5, 19,  5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 25,  5, 25, 11, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line( 28,  8, 22,  8, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(-25,  5,-25, 11, Component::getPen(Qt::black,1)));
}

void Source_ac::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void Source_ac::initProperties()
{
   addProperty("Num","1",QObject::tr("number of the port"),true);
   addProperty("Z","50 Ohm",QObject::tr("port impedance"),true);
   addProperty("P","0 dBm",QObject::tr("(available) ac power in Watts"),false);
   addProperty("f","1 GHz",QObject::tr("frequency in Hertz"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

