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

#include "dcblock.h"
#include "shapes.h"

dcBlock::dcBlock(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void dcBlock::initConstants()
{
   m_boundingRect = QRectF( -30, -16, 60, 33);

   model = "DCBlock";
   name = "C";
   description =  QObject::tr("dc block");

   m_shapes.append(new Line(- 4,-11, -4, 11, Component::getPen(Qt::darkBlue,4)));
   m_shapes.append(new Line(  4,-11,  4, 11, Component::getPen(Qt::darkBlue,4)));
   m_shapes.append(new Line(-30,  0, -4,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  4,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,-14, 23,-14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-23, 14, 23, 14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-23,-14,-23, 14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 23,-14, 23, 14, Component::getPen(Qt::darkBlue,1)));
}

void dcBlock::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void dcBlock::initProperties()
{
   addProperty("C","1 uF",QObject::tr("for transient simulation: capacitance in Farad"),false);
}

