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

#include "mscoupled.h"
#include "shapes.h"

MScoupled::MScoupled(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void MScoupled::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -33, 60, 66).adjusted(-pw, -pw, pw, pw);

   model = "MCOUPLED";
   name = "MS";
   description =  QObject::tr("coupled microstrip line");

   m_shapes.append(new Line(-30,-12,-16,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,-30,-30,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 20,-12, 30,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30,-30, 30,-12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-11,-20, 25,-20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-21, -4, 15, -4, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-11,-20,-21, -4, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 25,-20, 15, -4, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 12,-20, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30, 30,-30, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 16, 12, 30, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 30, 30, 30, 12, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-15,  4, 21,  4, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-25, 20, 11, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-15,  4,-25, 20, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 21,  4, 11, 20, Component::getPen(Qt::darkBlue,2)));
}

void MScoupled::initPorts()
{
   addPort(QPointF(-30,-30));
   addPort(QPointF(30,-30));
   addPort(QPointF(30,30));
   addPort(QPointF(-30,30));
}

void MScoupled::initProperties()
{
   addProperty("Subst","Subst1",QObject::tr("name of substrate definition"),true);
   addProperty("W","1 mm",QObject::tr("width of the line"),true);
   addProperty("L","10 mm",QObject::tr("length of the line"),true);
   addProperty("S","1 mm",QObject::tr("spacing between the lines"),true);
   addProperty("Model","Kirschning",QObject::tr("microstrip model"),false, QString("Kirschning,Hammerstad").split(',',QString::SkipEmptyParts));
   addProperty("DispModel","Kirschning",QObject::tr("microstrip dispersion model"),false, QString("Kirschning,Getsinger").split(',',QString::SkipEmptyParts));
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

