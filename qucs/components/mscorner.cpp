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

#include "mscorner.h"
#include "shapes.h"

MScorner::MScorner(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void MScorner::initConstants()
{
   m_boundingRect = QRectF( -30, -11, 41, 41);

   model = "MCORN";
   name = "MS";
   description =  QObject::tr("microstrip corner");

   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, 18,  0, 30, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-18, -8,  8, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-18,  8, -8,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-18, -8,-18,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -8,  8, -8, 18, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  8, -8,  8, 18, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -8, 18,  8, 18, Component::getPen(Qt::darkBlue,2)));
}

void MScorner::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(0,30));
}

void MScorner::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("substrate"),true);
   addProperty("W","1 mm",QObject::tr("width of line"),true);
}

