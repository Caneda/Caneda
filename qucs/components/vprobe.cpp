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

#include "vprobe.h"
#include "shapes.h"

vProbe::vProbe(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void vProbe::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -24, -35, 48, 55).adjusted(-pw, -pw, pw, pw);

   model = "VProbe";
   name = "Pr";
   description =  QObject::tr("voltage probe");

   m_shapes.append(new Line(-20,-31, 20,-31, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,  9, 20,  9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-31,-20,  9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 20,-31, 20,  9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16,-27, 16,-27, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16, -9, 16, -9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16,-27,-16, -9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 16,-27, 16, -9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-20,-23, 39, 39, 16*50, 16*80, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-11,-24, -2, -9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,  0,-10,  6, Component::getPen(Qt::red,2)));
   m_shapes.append(new Line(-13,  3, -7,  3, Component::getPen(Qt::red,2)));
   m_shapes.append(new Line(  7,  3, 13,  3, Component::getPen(Qt::black,2)));
   m_shapes.append(new Line(-10,  9,-10, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,  9, 10, 20, Component::getPen(Qt::darkBlue,2)));
}

void vProbe::initPorts()
{
   addPort(QPointF(-10,20));
   addPort(QPointF(10,20));
}

void vProbe::initProperties()
{
}

