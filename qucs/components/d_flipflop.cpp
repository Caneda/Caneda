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

#include "d_flipflop.h"
#include "shapes.h"

D_FlipFlop::D_FlipFlop(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void D_FlipFlop::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -24, 60, 54).adjusted(-pw, -pw, pw, pw);

   model = "DFF";
   name = "Y";
   description =  QObject::tr("D flip flop with asynchron reset");

   m_shapes.append(new Line(-20,-20, 20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20, 20, 20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-20,-20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 20,-20, 20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-10,-20,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 10,-20, 10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-10, 20,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, 20,  0, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Text( -17, -17, "D", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( 6, -17, "Q", Qt::darkBlue, 12.0));
   m_shapes.append(new Text( -4, 5, "R", Qt::darkBlue, 12.0));
   m_shapes.append(new Line(-20,  6,-12, 10, Component::getPen(Qt::darkBlue,0)));
   m_shapes.append(new Line(-20, 14,-12, 10, Component::getPen(Qt::darkBlue,0)));
}

void D_FlipFlop::initPorts()
{
   addPort(QPointF(-30,-10));
   addPort(QPointF(-30,10));
   addPort(QPointF(30,-10));
   addPort(QPointF(0,30));
}

void D_FlipFlop::initProperties()
{
   addProperty("t","0",QObject::tr("delay time"),false);
}

