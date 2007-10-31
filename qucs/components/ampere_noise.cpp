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

#include "ampere_noise.h"
#include "shapes.h"

Ampere_noise::Ampere_noise(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
   rotate90();
}

void Ampere_noise::initConstants()
{
   m_boundingRect = QRectF( -30, -15, 60, 30);

   model = "Inoise";
   name = "I";
   description =  QObject::tr("noise current source");

   m_shapes.append(new Arc(-12,-12, 24, 24,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,  0, 12,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7,  0,  7,  0, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  6,  0,  0, -4, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(  6,  0,  0,  4, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line(-12,  1,  1,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,  6, -7,  3, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  3, -7,  6,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7, 10, -2,  5, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  7, -4, 10, -7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -1, 12, 12, -1, Component::getPen(Qt::darkBlue,2)));
}

void Ampere_noise::initPorts()
{
   addPort(QPointF(30,0));
   addPort(QPointF(-30,0));
}

void Ampere_noise::initProperties()
{
   addProperty("i","1e-6",QObject::tr("current power spectral density in A^2/Hz"),true);
   addProperty("e","0",QObject::tr("frequency exponent"),false);
   addProperty("c","1",QObject::tr("frequency coefficient"),false);
   addProperty("a","0",QObject::tr("additive frequency term"),false);
}

