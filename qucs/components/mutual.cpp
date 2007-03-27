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

#include "mutual.h"
#include "shapes.h"

Mutual::Mutual(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Mutual::initConstants()
{
   m_boundingRect = QRectF( -33, -34, 66, 68);

   model = "MUT";
   name = "Tr";
   description =  QObject::tr("two mutual inductors");

   m_shapes.append(new Arc(-16,-18,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16, -6,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16,  6,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4,-18,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4, -6,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4,  6,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-18,-10,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-30,-30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-18, 10,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-30, 30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 18,-10, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 30,-30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, 18, 10, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, 30, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Text( -21, -22, "1"));
   m_shapes.append(new Text( 15, -22, "2"));
   m_shapes.append(new Line(  0,-20,  0, 20, Component::getPen(Qt::darkBlue,1,Qt::DashLine)));
}

void Mutual::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void Mutual::initProperties()
{
   addProperty("L1","1 mH",QObject::tr("inductance of coil 1"),false);
   addProperty("L2","1 mH",QObject::tr("inductance of coil 2"),false);
   addProperty("k","0.9",QObject::tr("coupling factor between coil 1 and 2"),false);
}

