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

#include "rs_flipflop.h"
#include "shapes.h"

RS_FlipFlop::RS_FlipFlop(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void RS_FlipFlop::initConstants()
{
   m_boundingRect = QRectF( -30, -24, 60, 48);

   model = "RSFF";
   name = "Y";
   description =  QObject::tr("RS flip flop");

   m_shapes.append(new Line(-20,-20, 20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20, 20, 20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-20,-20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 20,-20, 20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-10,-20,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 10,-20, 10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-10, 20,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 10, 20, 10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Text( -17, -17, "R", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( -17, 3, "S", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( 6, -17, "Q", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( 6, 3, "Q", Qt::darkBlue, 12.0));
   m_shapes.append(new Line(  7,   3, 15,   3, Component::getPen(Qt::darkBlue,1)));
}

void RS_FlipFlop::initPorts()
{
   addPort(QPointF(-30,-10));
   addPort(QPointF(-30,10));
   addPort(QPointF(30,-10));
   addPort(QPointF(30,10));
}

void RS_FlipFlop::initProperties()
{
   addProperty("t","0",QObject::tr("delay time"),false);
}

