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

#include "tline_4port.h"
#include "shapes.h"

TLine_4Port::TLine_4Port(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void TLine_4Port::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -12, 60, 24).adjusted(-pw, -pw, pw, pw);

   model = "TLIN4P";
   name = "Line";
   description =  QObject::tr("ideal 4-terminal transmission line");

   m_shapes.append(new Arc(-28,-40, 18, 38,16*232, 16*33, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-28,  2, 18, 38, 16*95, 16*33, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-20,-2, 20,-2, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20, 2, 20, 2, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 10,-40, 18, 38,16*270, 16*40, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( 10,  2, 18, 38, 16*50, 16*40, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-38,-10, 16, 28, 16*45, 16*45, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-38,-18, 16, 28,16*270, 16*45, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( 22,-10, 16, 28, 16*90, 16*45, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( 22,-18, 16, 28,16*225, 16*45, Component::getPen(Qt::darkBlue,1)));
}

void TLine_4Port::initPorts()
{
   addPort(QPointF(-30,-10));
   addPort(QPointF(30,-10));
   addPort(QPointF(30,10));
   addPort(QPointF(-30,10));
}

void TLine_4Port::initProperties()
{
   addProperty("Z","50 Ohm",QObject::tr("characteristic impedance"),true);
   addProperty("L","1 mm",QObject::tr("electrical length of the line"),true);
   addProperty("Alpha","0 dB",QObject::tr("attenuation factor per length in 1/m"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

