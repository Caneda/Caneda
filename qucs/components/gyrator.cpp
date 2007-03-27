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

#include "gyrator.h"
#include "shapes.h"

Gyrator::Gyrator(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Gyrator::initConstants()
{
   m_boundingRect = QRectF( -30, -30, 60, 60);

   model = "Gyrator";
   name = "X";
   description =  QObject::tr("gyrator (impedance inverter)");

   m_shapes.append(new Arc(  3, -9, 18, 18, 16*90, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-21, -9, 18, 18,16*270, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-30,-12,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 30,-12, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 12,-30, 30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 12, 30, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 12,-30, 12, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,-30,-12, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-22,-22, 22,-22, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 22,-22, 22, 22, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 22, 22,-22, 22, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line(-22, 22,-22,-22, Component::getPen(Qt::darkGray,1)));
}

void Gyrator::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void Gyrator::initProperties()
{
   addProperty("R","50 Ohm",QObject::tr("gyrator ratio"),true);
   addProperty("Zref","50 Ohm",QObject::tr("reference impedance"),false);
}

