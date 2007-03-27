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

#include "jk_flipflop.h"
#include "shapes.h"

JK_FlipFlop::JK_FlipFlop(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void JK_FlipFlop::initConstants()
{
   m_boundingRect = QRectF( -30, -40, 60, 80);

   model = "JKFF";
   name = "Y";
   description =  QObject::tr("JK flip flop with asynchron set and reset");

   m_shapes.append(new Line(-20,-30, 20,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20, 30, 20, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-30,-20, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 20,-30, 20, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-20,-20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 20,-20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-20, 20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 20, 20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-20,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,-30,  0,-40, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, 30,  0, 40, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Text( -4, -27, "S", Qt::darkBlue, 9.0));
   m_shapes.append(new Text( -4, 16, "R", Qt::darkBlue, 9.0));
   m_shapes.append(new Text( -17, -27, "J", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( -17, 13, "K", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( 6, -27, "Q", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( 6, 13, "Q", Qt::darkBlue, 12.0));
   m_shapes.append(new Line(  7, 13, 15, 13, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-20, -4,-12,  0, Component::getPen(Qt::darkBlue,0)));
   m_shapes.append(new Line(-20,  4,-12,  0, Component::getPen(Qt::darkBlue,0)));
}

void JK_FlipFlop::initPorts()
{
   addPort(QPointF(-30,-20));
   addPort(QPointF(-30,20));
   addPort(QPointF(30,-20));
   addPort(QPointF(30,20));
   addPort(QPointF(-30,0));
   addPort(QPointF(0,-40));
   addPort(QPointF(0,40));
}

void JK_FlipFlop::initProperties()
{
   addProperty("t","0",QObject::tr("delay time"),false);
}

