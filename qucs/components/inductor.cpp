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

#include "inductor.h"
#include "shapes.h"

Inductor::Inductor(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Inductor::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -10, 60, 16).adjusted(-pw, -pw, pw, pw);

   model = "L";
   name = "L";
   description =  QObject::tr("inductor");

   m_shapes.append(new Arc(-18, -6, 12, 12,  0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -6, -6, 12, 12,  0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  6, -6, 12, 12,  0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 18,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
}

void Inductor::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void Inductor::initProperties()
{
   addProperty("L","1 nH",QObject::tr("inductance in Henry"),true);
   addProperty("I","",QObject::tr("initial current for transient simulation"),false);
}

