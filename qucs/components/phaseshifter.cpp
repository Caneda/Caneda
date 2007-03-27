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

#include "phaseshifter.h"
#include "shapes.h"

Phaseshifter::Phaseshifter(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Phaseshifter::initConstants()
{
   m_boundingRect = QRectF( -30, -17, 60, 34);

   model = "PShift";
   name = "X";
   description =  QObject::tr("phase shifter");

   m_shapes.append(new Line(-14,-14, 14,-14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14, 14, 14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14,-14,-14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14,-14, 14, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -9, -9, 17, 17, 0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 10, 10,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-14,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 14,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
}

void Phaseshifter::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void Phaseshifter::initProperties()
{
   addProperty("phi","90",QObject::tr("phase shift in degree"),true);
   addProperty("Zref","50 Ohm",QObject::tr("reference impedance"),false);
}

