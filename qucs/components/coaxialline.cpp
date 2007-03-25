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

#include "coaxialline.h"
#include "shapes.h"

CoaxialLine::CoaxialLine(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void CoaxialLine::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -12, 60, 24).adjusted(-pw, -pw, pw, pw);

   model = "COAX";
   name = "Line";
   description =  QObject::tr("coaxial transmission line");

   m_shapes.append(new Arc(-20, -9, 8, 18,     0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 11, -9, 8, 18,16*270, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-16,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 19,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16, -9, 16, -9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16,  9, 16,  9, Component::getPen(Qt::darkBlue,2)));
}

void CoaxialLine::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void CoaxialLine::initProperties()
{
   addProperty("er","2.29",QObject::tr("relative permittivity of dielectric"),true);
   addProperty("rho","0.022e-6",QObject::tr("specific resistance of conductor"),false);
   addProperty("mur","1",QObject::tr("relative permeability of conductor"),false);
   addProperty("D","2.95 mm",QObject::tr("inner diameter of shield"),false);
   addProperty("d","0.9 mm",QObject::tr("diameter of inner conductor"),false);
   addProperty("L","1500 mm",QObject::tr("mechanical length of the line"),true);
   addProperty("tand","4e-4",QObject::tr("loss tangent"),false);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

