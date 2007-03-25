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

#include "transformer.h"
#include "shapes.h"

Transformer::Transformer(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Transformer::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -33, -34, 66, 68).adjusted(-pw, -pw, pw, pw);

   model = "Tr";
   name = "Tr";
   description =  QObject::tr("ideal transformer");

   m_shapes.append(new Arc(-16,-18,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16, -6,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(-16,  6,12,12, 16*270,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4,-18,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4, -6,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  4,  6,12,12,  16*90,16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-18,-10,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10,-30,-30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-18, 10,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-30, 30,-30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 18,-10, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-10, 30,-30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, 18, 10, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, 30, 30, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -1,-20, -1, 20, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(  1,-20,  1, 20, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Text( -21, -18, "T"));
   m_shapes.append(new Arc(-21,-24,  5,  5,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 15,-24,  5,  5,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
}

void Transformer::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void Transformer::initProperties()
{
   addProperty("T","1",QObject::tr("voltage transformation ratio"),true);
}

