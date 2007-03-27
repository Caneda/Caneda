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

#include "irect.h"
#include "shapes.h"

iRect::iRect(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate();
}

void iRect::initConstants()
{
   m_boundingRect = QRectF( -30, -14, 60, 34);

   model = "Irect";
   name = "I";
   description =  QObject::tr("ideal rectangle current source");

   m_shapes.append(new Arc(-12,-12, 24, 24,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7,  0,  7,  0, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  6,  0,  0, -4, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  6,  0,  0,  4, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( 19,  5, 19,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 13,  7, 19,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 13,  7, 13, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 13, 11, 19, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 19, 11, 19, 15, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 13, 15, 19, 15, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 13, 15, 13, 17, Component::getPen(Qt::darkBlue,2)));
}

void iRect::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void iRect::initProperties()
{
   addProperty("I","1 mA",QObject::tr("current at high pulse"),true);
   addProperty("TH","1 ms",QObject::tr("duration of high pulses"),true);
   addProperty("TL","1 ms",QObject::tr("duration of low pulses"),true);
   addProperty("Tr","1 ns",QObject::tr("rise time of the leading edge"),false);
   addProperty("Tf","1 ns",QObject::tr("fall time of the trailing edge"),false);
   addProperty("Td","0 ns",QObject::tr("initial delay time"),false);
}

