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

#include "opamp.h"
#include "shapes.h"

OpAmp::OpAmp(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void OpAmp::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -38, 60, 76).adjusted(-pw, -pw, pw, pw);

   model = "OpAmp";
   name = "OP";
   description =  QObject::tr("operational amplifier");

   m_shapes.append(new Line(-30,-20,-20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 20,-20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 40,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-35,-20, 35, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-35, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20, 35, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16, 19, -9, 19, Component::getPen(Qt::black,1)));
   m_shapes.append(new Line(-16,-19, -9,-19, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line(-13,-22,-13,-15, Component::getPen(Qt::red,1)));
}

void OpAmp::initPorts()
{
   addPort(QPointF(-30,20));
   addPort(QPointF(-30,-20));
   addPort(QPointF(40,0));
}

void OpAmp::initProperties()
{
   addProperty("G","1e6",QObject::tr("voltage gain"),true);
   addProperty("Umax","15 V",QObject::tr("absolute value of maximum and minimum output voltage"),false);
}

