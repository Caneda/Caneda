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

#include "twistedpair.h"
#include "shapes.h"

TwistedPair::TwistedPair(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void TwistedPair::initConstants()
{
   m_boundingRect = QRectF( -30, -12, 60, 24);

   model = "TWIST";
   name = "Line";
   description =  QObject::tr("twisted pair transmission line");

   m_shapes.append(new Arc(-25,-36, 18, 38,16*230, 16*68, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-25, -2, 18, 38, 16*62, 16*68, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-17,-36, 18, 38,16*242, 16*56, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-17, -2, 18, 38, 16*62, 16*56, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( -9,-36, 18, 38,16*242, 16*56, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( -9, -2, 18, 38, 16*62, 16*56, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( -1,-36, 18, 38,16*242, 16*56, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( -1, -2, 18, 38, 16*62, 16*56, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(  7,-36, 18, 38,16*242, 16*68, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(  7, -2, 18, 38, 16*50, 16*68, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-40,-10, 20, 33, 16*32, 16*58, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-40,-23, 20, 33,16*270, 16*58, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 20,-10, 20, 33, 16*90, 16*58, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 20,-23, 20, 33,16*212, 16*58, Component::getPen(Qt::darkBlue,2)));
}

void TwistedPair::initPorts()
{
   addPort(QPointF(-30,-10));
   addPort(QPointF(30,-10));
   addPort(QPointF(30,10));
   addPort(QPointF(-30,10));
}

void TwistedPair::initProperties()
{
   addProperty("d","0.5 mm",QObject::tr("diameter of conductor"),true);
   addProperty("D","0.8 mm",QObject::tr("diameter of wire (conductor and insulator)"),true);
   addProperty("L","1.5",QObject::tr("physical length of the line"),true);
   addProperty("T","100",QObject::tr("twists per length in 1/m"),false);
   addProperty("er","4",QObject::tr("dielectric constant of insulator"),false);
   addProperty("mur","1",QObject::tr("relative permeability of conductor"),false);
   addProperty("rho","0.022e-6",QObject::tr("specific resistance of conductor"),false);
   addProperty("tand","4e-4",QObject::tr("loss tangent"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

