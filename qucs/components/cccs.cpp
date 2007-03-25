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

#include "cccs.h"
#include "shapes.h"

CCCS::CCCS(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void CCCS::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -30, 60, 60).adjusted(-pw, -pw, pw, pw);

   model = "CCCS";
   name = "SRC";
   description =  QObject::tr("current controlled current source");

   m_shapes.append(new Arc(0,-11, 22, 22,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11, -7, 11,  7, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( 11,  6, 15,  1, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( 11,  6,  7,  1, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(-30,-30,-12,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 30,-12, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11,-30, 30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11, 30, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12,-30,-12, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11,-30, 11,-11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11, 30, 11, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12, 20,-17, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-12, 20, -8, 11, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25,-27, 25,-27, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 25,-27, 25, 27, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 25, 27,-25, 27, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line(-25, 27,-25,-27, Component::getPen(Qt::darkGray,1)));
}

void CCCS::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void CCCS::initProperties()
{
   addProperty("G","1",QObject::tr("forward transfer factor"),true);
   addProperty("T","0",QObject::tr("delay time"),false);
}

