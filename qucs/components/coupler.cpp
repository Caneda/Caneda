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

#include "coupler.h"
#include "shapes.h"

Coupler::Coupler(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Coupler::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -25, 60, 50).adjusted(-pw, -pw, pw, pw);

   model = "Coupler";
   name = "X";
   description =  QObject::tr("ideal coupler");

   m_shapes.append(new Line(-23,-24, 23,-24, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 23,-24, 23, 24, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line( 23, 24,-23, 24, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line(-23, 24,-23,-24, Component::getPen(Qt::darkGray,1)));
   m_shapes.append(new Line(-30,-20,-20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-20, 20,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,-20, 20,-20, Component::getPen(Qt::darkBlue,4)));
   m_shapes.append(new Line(-30, 20,-20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 20, 20, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20, 20, 20, 20, Component::getPen(Qt::darkBlue,4)));
   m_shapes.append(new Line( 14, 14,-14,-14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-14,-14, -9,-14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-14,-14,-14, -9, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(  9, 14, 14, 14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 14,  9, 14, 14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 14,-14,-14, 14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 14,-14,  9,-14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 14,-14, 14, -9, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-14, 14, -9, 14, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-14, 14,-14,  9, Component::getPen(Qt::darkBlue,1)));
}

void Coupler::initPorts()
{
   addPort(QPointF(-30,-20));
   addPort(QPointF(30,-20));
   addPort(QPointF(30,20));
   addPort(QPointF(-30,20));
}

void Coupler::initProperties()
{
   addProperty("k","0.7071",QObject::tr("coupling factor"),true);
   addProperty("phi","180",QObject::tr("phase shift of coupling path in degree"),true);
   addProperty("Z","50 Ohm",QObject::tr("reference impedance"),false);
}

