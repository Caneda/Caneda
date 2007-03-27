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

#include "cpwopen.h"
#include "shapes.h"

CPWopen::CPWopen(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void CPWopen::initConstants()
{
   m_boundingRect = QRectF( -30, -24, 47, 48);

   model = "COPEN";
   name = "CL";
   description =  QObject::tr("coplanar open");

   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,  0, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,  8,-10,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,-23,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, -8,-10,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25,-13, 11,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25, 13, -5, 13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 11,-13, -5, 13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-24,-21,-16,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16,-21, -8,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -8,-21,  0,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,-21,  8,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  8,-21, 15,-14, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10,-11, 15, -6, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  7, -6, 15,  2, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  4, -1, 15, 10, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  1,  4, 15, 18, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25, 18,-22, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-22, 13,-14, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14, 13, -6, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -6, 13,  2, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -2,  9, 10, 21, Component::getPen(Qt::darkBlue,2)));
}

void CPWopen::initPorts()
{
   addPort(QPointF(-30,0));
}

void CPWopen::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("name of substrate definition"),true);
   addProperty("W","1 mm",QObject::tr("width of the line"),true);
   addProperty("S","1 mm",QObject::tr("width of a gap"),true);
   addProperty("G","5 mm",QObject::tr("width of gap at end of line"),true);
   addProperty("Backside","Air",QObject::tr("material at the backside of the substrate"),false, QString("Metal,Air").split(',',QString::SkipEmptyParts));
}

