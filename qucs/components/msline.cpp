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

#include "msline.h"
#include "shapes.h"

MSline::MSline(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void MSline::initConstants()
{
   m_boundingRect = QRectF( -30, -11, 60, 22);

   model = "MLIN";
   name = "MS";
   description =  QObject::tr("microstrip line");

   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 18,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8, 23, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,  8, 13,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,-23,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 23, -8, 13,  8, Component::getPen(Qt::darkBlue,2)));
}

void MSline::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void MSline::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("name of substrate definition"),true);
   addProperty("W","1 mm",QObject::tr("width of the line"),true);
   addProperty("L","10 mm",QObject::tr("length of the line"),true);
   addProperty("Model","Hammerstad",QObject::tr("quasi-static microstrip model"),false, QString("Hammerstad,Wheeler,Schneider").split(',',QString::SkipEmptyParts));
   addProperty("DispModel","Kirschning",QObject::tr("microstrip dispersion model"),false, QString("Kirschning,Kobayashi,Yamashita,Hammerstad,Getsinger,Schneider,Pramanick").split(',',QString::SkipEmptyParts));
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

