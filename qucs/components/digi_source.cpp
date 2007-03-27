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

#include "digi_source.h"
#include "shapes.h"

Digi_Source::Digi_Source(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Digi_Source::initConstants()
{
   m_boundingRect = QRectF( -39, -14, 39, 28);

   model = "DigiSource";
   name = "S";
   description =  QObject::tr("digital source");

   m_shapes.append(new Line(-10,  0,  0,  0, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-20,-10,-10,  0, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-20, 10,-10,  0, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-35,-10,-20,-10, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-35, 10,-20, 10, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-35,-10,-35, 10, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-32, 5,-28, 5, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-28,-5,-24,-5, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-24, 5,-20, 5, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-28,-5,-28, 5, Component::getPen(Qt::darkGreen,2)));
   m_shapes.append(new Line(-24,-5,-24, 5, Component::getPen(Qt::darkGreen,2)));
}

void Digi_Source::initPorts()
{
   addPort(QPointF(0,0));
}

void Digi_Source::initProperties()
{
   addProperty("Num","1",QObject::tr("number of the port"),true);
   addProperty("init","low",QObject::tr("initial output value"),false, QString("low,high").split(',',QString::SkipEmptyParts));
   addProperty("times","1ns; 1ns",QObject::tr("list of times for changing output value"),false);
   addProperty("V","1 V",QObject::tr("voltage of high level"),false);
}

