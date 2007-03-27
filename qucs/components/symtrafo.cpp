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

#include "symtrafo.h"
#include "shapes.h"

symTrafo::symTrafo(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void symTrafo::initConstants()
{
   m_boundingRect = QRectF( -33, -74, 66, 148);

   model = "sTr";
   name = "Tr";
   description =  QObject::tr("ideal symmetrical transformer");

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
   m_shapes.append(new Line( -1,-57, -1, 57, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(  1,-57,  1, 57, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Text( -23, -57, "T1"));
   m_shapes.append(new Text( -23, 22, "T2"));
   m_shapes.append(new Arc(-21,-64,  5,  5,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-21, 15,  5,  5,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 15,-24,  5,  5,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
}

void symTrafo::initPorts()
{
   addPort(QPointF(-30,-70));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,70));
   addPort(QPointF(-30,10));
   addPort(QPointF(-30,-10));
}

void symTrafo::initProperties()
{
   addProperty("T1","1",QObject::tr("voltage transformation ratio of coil 1"),true);
   addProperty("T2","1",QObject::tr("voltage transformation ratio of coil 2"),true);
}

