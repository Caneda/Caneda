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

#include "cpwshort.h"
#include "shapes.h"

CPWshort::CPWshort(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void CPWshort::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -24, 44, 48).adjusted(-pw, -pw, pw, pw);

   model = "CSHORT";
   name = "CL";
   description =  QObject::tr("coplanar short");

   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,  3, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,  8, -7,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,-23,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25,-13,  6,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25, 13,-10, 13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  6,-13,  3, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -7,  8,-10, 13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-24,-21,-16,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-16,-21, -8,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -8,-21,  0,-13, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,-21, 12, -9, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  8,-21, 12,-17, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  4, -9, 12, -1, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  1, -4, 12,  7, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -2,  1, 12, 15, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25, 18,-22, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-22, 13,-14, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-14, 13, -6, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -8, 11,  2, 21, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( -5,  6, 10, 21, Component::getPen(Qt::darkBlue,2)));
}

void CPWshort::initPorts()
{
   addPort(QPointF(-30,0));
}

void CPWshort::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("name of substrate definition"),true);
   addProperty("W","1 mm",QObject::tr("width of the line"),true);
   addProperty("S","1 mm",QObject::tr("width of a gap"),true);
   addProperty("Backside","Air",QObject::tr("material at the backside of the substrate"),false, QString("Metal,Air").split(',',QString::SkipEmptyParts));
}

