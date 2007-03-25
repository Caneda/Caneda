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

#include "msvia.h"
#include "shapes.h"

MSvia::MSvia(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void MSvia::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -20, -7, 34, 37).adjusted(-pw, -pw, pw, pw);

   model = "MVIA";
   name = "MS";
   description =  QObject::tr("microstrip via");

   m_shapes.append(new Arc(-5,-4, 10,  7,  0, 16*360, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-20,  0, -5,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -5,  0, -5, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  5,  0,  5, 14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-11, 14, 11, 14, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( -7, 20,  7, 20, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( -3, 26,  3, 26, Component::getPen(Qt::darkBlue,3)));
}

void MSvia::initPorts()
{
   addPort(QPointF(-20,0));
}

void MSvia::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("substrate"),true);
   addProperty("D","1 mm",QObject::tr("diameter of round via conductor"),true);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

