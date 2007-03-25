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

#include "substrate.h"
#include "shapes.h"

Substrate::Substrate(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Substrate::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -34, -44, 118, 64).adjusted(-pw, -pw, pw, pw);

   model = "SUBST";
   name = "Subst";
   description =  QObject::tr("substrate definition");

   m_shapes.append(new Line(-30,-16, 30,-16, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-12, 30,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 16, 30, 16, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 12, 30, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-16,-30, 16, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-16, 30, 16, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-16, 16,-40, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-16, 80,-40, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-12, 80,-36, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 12, 80,-16, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 16, 80,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 16,-40, 80,-40, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 80,-40, 80,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-18,-12, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-22, 12,  2,-12, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( -2, 12, 22,-12, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 18, 12, 30,  0, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 30,  1, 37,  8, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 37,-15, 52,  0, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 52,-22, 66, -8, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 66,-30, 80,-16, Component::getPen(Qt::darkBlue,1)));
}

void Substrate::initPorts()
{
}

void Substrate::initProperties()
{
   addProperty("er","9.8",QObject::tr("relative permittivity"),true);
   addProperty("h","1 mm",QObject::tr("thickness in meters"),true);
   addProperty("t","35 um",QObject::tr("thickness of metalization"),true);
   addProperty("tand","2e-4",QObject::tr("loss tangent"),true);
   addProperty("rho","0.022e-6",QObject::tr("specific resistance of metal"),true);
   addProperty("D","0.15e-6",QObject::tr("rms substrate roughness"),true);
}

