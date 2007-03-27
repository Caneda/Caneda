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

#include "biast.h"
#include "shapes.h"

BiasT::BiasT(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void BiasT::initConstants()
{
   m_boundingRect = QRectF( -30, -13, 60, 43);

   model = "BiasT";
   name = "X";
   description =  QObject::tr("bias t");

   m_shapes.append(new Arc( -3,  2, 6, 6, 16*270, 16*180, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( -3,  8, 6, 6, 16*270, 16*180, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc( -3, 14, 6, 6, 16*270, 16*180, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-22,-10, 22,-10, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-22,-10,-22, 22, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-22, 22, 22, 22, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 22,-10, 22, 22, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-13, -6,-13,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -9, -6, -9,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -9,  0, 22,  0, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-22,  0,-13,  0, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-30,  0,-22,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 22,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,  0,  0,  2, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(  0, 20,  0, 22, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(  0, 22,  0, 30, Component::getPen(Qt::darkBlue,2)));
}

void BiasT::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
   addPort(QPointF(0,30));
}

void BiasT::initProperties()
{
   addProperty("L","1 uH",QObject::tr("for transient simulation: inductance in Henry"),false);
   addProperty("C","1 uF",QObject::tr("for transient simulation: capacitance in Farad"),false);
}

