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

#include "msgap.h"
#include "shapes.h"

MSgap::MSgap(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void MSgap::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -11, 60, 22).adjusted(-pw, -pw, pw, pw);

   model = "MGAP";
   name = "MS";
   description =  QObject::tr("microstrip gap");

   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 18,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,  0, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, -8, 23, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,  8,-10,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0,  8, 13,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,-23,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 23, -8, 13,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(  0, -8,-10,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 10, -8,  0,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-22, -4,-26,  4, Component::getPen(Qt::darkBlue,2)));
}

void MSgap::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void MSgap::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("name of substrate definition"),true);
   addProperty("W1","1 mm",QObject::tr("width of the line 1"),true);
   addProperty("W2","1 mm",QObject::tr("width of the line 2"),true);
   addProperty("S","1 mm",QObject::tr("spacing between the microstrip ends"),true);
   addProperty("MSModel","Hammerstad",QObject::tr("quasi-static microstrip model"),false, QString("Hammerstad,Wheeler,Schneider").split(',',QString::SkipEmptyParts));
   addProperty("MSDispModel","Kirschning",QObject::tr("microstrip dispersion model"),false, QString("Kirschning,Kobayashi,Yamashita,Hammerstad,Getsinger,Schneider,Pramanick").split(',',QString::SkipEmptyParts));
}

