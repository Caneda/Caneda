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

#include "tline.h"
#include "shapes.h"

TLine::TLine(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void TLine::initConstants()
{
   m_boundingRect = QRectF( -30, -4, 60, 20);

   model = "TLIN";
   name = "Line";
   description =  QObject::tr("ideal transmission line");

   m_shapes.append(new Line(-30,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-28,  7, 28,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-28, 14,-21,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-21, 14,-14,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14, 14, -7,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7, 14,  0,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, 14,  7,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  7, 14, 14,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14, 14, 21,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 21, 14, 28,  7, Component::getPen(Qt::darkBlue,2)));
}

void TLine::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void TLine::initProperties()
{
   addProperty("Z","50 Ohm",QObject::tr("characteristic impedance"),true);
   addProperty("L","1 mm",QObject::tr("electrical length of the line"),true);
   addProperty("Alpha","0 dB",QObject::tr("attenuation factor per length in 1/m"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

