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

#include "noise_vv.h"
#include "shapes.h"

Noise_vv::Noise_vv(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void Noise_vv::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -44, -30, 88, 60).adjusted(-pw, -pw, pw, pw);

   model = "VVnoise";
   name = "SRC";
   description =  QObject::tr("correlated current sources");

   m_shapes.append(new Arc(-42,-12, 24, 24,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 30,-30, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-30,-30,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-29, 12,-42, -1, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-24, 10,-40, -6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,  7,-37,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-18,  2,-32,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( 18,-12, 24, 24,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 30, 30, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-30, 30,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 31, 12, 18, -1, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 36, 10, 20, -6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 40,  7, 23,-10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 42,  2, 28,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-18,  0, 18,  0, Component::getPen(Qt::darkBlue,3)));
}

void Noise_vv::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void Noise_vv::initProperties()
{
   addProperty("v1","1e-6",QObject::tr("voltage power spectral density of source 1"),true);
   addProperty("v2","1e-6",QObject::tr("voltage power spectral density of source 2"),true);
   addProperty("C","0.5",QObject::tr("normalized correlation coefficient"),true);
   addProperty("e","0",QObject::tr("frequency exponent"),false);
   addProperty("c","1",QObject::tr("frequency coefficient"),false);
   addProperty("a","0",QObject::tr("additive frequency term"),false);
}

