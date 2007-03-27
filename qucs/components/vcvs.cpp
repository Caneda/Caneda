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

#include "vcvs.h"
#include "shapes.h"

VCVS::VCVS(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void VCVS::initConstants()
{
   m_boundingRect = QRectF( -30, -30, 60, 60);

   model = "VCVS";
   name = "SRC";
   description =  QObject::tr("voltage controlled voltage source");

   m_shapes.append(new Arc(0,-11, 22, 22,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-30,-12,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 30,-12, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11,-30, 30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11, 30, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,-30,-12,-23, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12, 30,-12, 23, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11,-30, 11,-11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11, 30, 11, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,-18,-12, 18, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-12, 18,-17,  9, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-12, 18, -7,  9, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 19,-21, 19,-15, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line( 16,-18, 22,-18, Component::getPen(Qt::red,1)));
   m_shapes.append(new Line( 16, 18, 22, 18, Component::getPen(Qt::black,1)));
   m_shapes.append(new Line(-25,-27, 25,-27, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 25,-27, 25, 27, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 25, 27,-25, 27, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line(-25, 27,-25,-27, Component::getPen(Qt::darkGray,1)));
}

void VCVS::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void VCVS::initProperties()
{
   addProperty("G","1",QObject::tr("forward transfer factor"),true);
   addProperty("T","0",QObject::tr("delay time"),false);
}

