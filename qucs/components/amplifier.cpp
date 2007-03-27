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

#include "amplifier.h"
#include "shapes.h"

Amplifier::Amplifier(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Amplifier::initConstants()
{
   m_boundingRect = QRectF( -30, -23, 60, 46);

   model = "Amp";
   name = "X";
   description =  QObject::tr("ideal amplifier");

   m_shapes.append(new Line(-16,-20,-16, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16,-20, 16,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16, 20, 16,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-16,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 16,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
}

void Amplifier::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void Amplifier::initProperties()
{
   addProperty("G","10",QObject::tr("voltage gain"),true);
   addProperty("Z1","50 Ohm",QObject::tr("reference impedance of input port"),false);
   addProperty("Z2","50 Ohm",QObject::tr("reference impedance of output port"),false);
}

