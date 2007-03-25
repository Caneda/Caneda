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

#include "msopen.h"
#include "shapes.h"

MSopen::MSopen(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void MSopen::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -11, 46, 22).adjusted(-pw, -pw, pw, pw);

   model = "MOPEN";
   name = "MS";
   description =  QObject::tr("microstrip open");

   m_shapes.append(new Line(-30,  0,-18,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8, 13, -8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,  8,  3,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-13, -8,-23,  8, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 13, -8,  3,  8, Component::getPen(Qt::darkBlue,2)));
}

void MSopen::initPorts()
{
   addPort(QPointF(-30,0));
}

void MSopen::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("name of substrate definition"),true);
   addProperty("W","1 mm",QObject::tr("width of the line"),true);
   addProperty("MSModel","Hammerstad",QObject::tr("quasi-static microstrip model"),false, QString("Hammerstad,Wheeler,Schneider").split(',',QString::SkipEmptyParts));
   addProperty("MSDispModel","Kirschning",QObject::tr("microstrip dispersion model"),false, QString("Kirschning,Kobayashi,Yamashita,Hammerstad,Getsinger,Schneider,Pramanick").split(',',QString::SkipEmptyParts));
   addProperty("Model","Kirschning",QObject::tr("microstrip open end model"),false, QString("Kirschning,Hammerstad,Alexopoulos").split(',',QString::SkipEmptyParts));
}

