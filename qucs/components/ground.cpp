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

#include "ground.h"
#include "shapes.h"

Ground::Ground(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Ground::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -12, 0, 24, 25).adjusted(-pw, -pw, pw, pw);

   model = "GND";
   name = "";
   description =  QObject::tr("ground (reference potential)");

   m_shapes.append(new Line(  0,  0,  0, 10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-11, 10, 11, 10, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( -7, 16,  7, 16, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( -3, 22,  3, 22, Component::getPen(Qt::darkBlue,3)));
}

void Ground::initPorts()
{
   addPort(QPointF(0,0));
}

void Ground::initProperties()
{
}

