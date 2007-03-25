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

#include "mutual2.h"
#include "shapes.h"

Mutual2::Mutual2(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Mutual2::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -33, -74, 66, 148).adjusted(-pw, -pw, pw, pw);

   model = "MUT2";
   name = "Tr";
   description =  QObject::tr("three mutual inductors");

   m_shapes.append(new Arc(-16,-58,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16,-46,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16,-34,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16, 46,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16, 34,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16, 22,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4,-18,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4, -6,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4,  6,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-58,-10,-70, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-70,-30,-70, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-18, 10,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-30, 30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 58,-10, 70, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 70,-30, 70, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, 18, 10, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, 30, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-10,-30,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-22,-10,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 10,-30, 10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 10,-10, 22, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Text( -20, -61, "1"));
   m_shapes.append(new Text( -20, 18, "2"));
   m_shapes.append(new Text( 15, -22, "3"));
   m_shapes.append(new Line(  0,-57,  0, 57, Component::getPen(Qt::darkBlue,1,Qt::DashLine)));
}

void Mutual2::initPorts()
{
   addPort(QPointF(-30,-70));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,70));
   addPort(QPointF(-30,10));
   addPort(QPointF(-30,-10));
}

void Mutual2::initProperties()
{
   addProperty("L1","1 mH",QObject::tr("inductance of coil 1"),false);
   addProperty("L2","1 mH",QObject::tr("inductance of coil 2"),false);
   addProperty("L3","1 mH",QObject::tr("inductance of coil 3"),false);
   addProperty("k12","0.9",QObject::tr("coupling factor between coil 1 and 2"),false);
   addProperty("k13","0.9",QObject::tr("coupling factor between coil 1 and 3"),false);
   addProperty("k23","0.9",QObject::tr("coupling factor between coil 2 and 3"),false);
}

