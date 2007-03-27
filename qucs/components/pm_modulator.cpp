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

#include "pm_modulator.h"
#include "shapes.h"

PM_Modulator::PM_Modulator(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void PM_Modulator::initConstants()
{
   m_boundingRect = QRectF( -30, -30, 44, 60);

   model = "PM_Mod";
   name = "V";
   description =  QObject::tr("ac voltage source with phase modulator");

   m_shapes.append(new Arc(-12,-12, 24, 24,     0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -7, -4,  7,  7,     0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  0, -4,  7,  7,16*180, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, 30,  0, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,-30,  0,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  5,-18, 11,-18, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(  8,-21,  8,-15, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(  5, 18, 11, 18, Component::getPen(Qt::black,1)));
   m_shapes.append(new Line(-12,  0,-30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,  0,-17,  5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,  0,-17, -5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Text( -30, -22, QObject::tr("PM"), Qt::black, 10.0));
}

void PM_Modulator::initPorts()
{
   addPort(QPointF(0,-30));
   addPort(QPointF(0,30));
   addPort(QPointF(-30,0));
}

void PM_Modulator::initProperties()
{
   addProperty("U","1 V",QObject::tr("peak voltage in Volts"),true);
   addProperty("f","1 GHz",QObject::tr("frequency in Hertz"),false);
   addProperty("Phase","0",QObject::tr("initial phase in degrees"),false);
   addProperty("M","1.0",QObject::tr("modulation index"),false);
}

