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

#include "bondwire.h"
#include "shapes.h"

BondWire::BondWire(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void BondWire::initConstants()
{
   m_boundingRect = QRectF( -30, -13, 60, 18);

   model = "BOND";
   name = "Line";
   description =  QObject::tr("bond wire");

   m_shapes.append(new Line(-30, 0,-8, 0, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Line( 30, 0, 8, 0, Component::getPen(Qt::darkBlue,3)));
   m_shapes.append(new Arc(-11,-10, 22, 26, 16*30,16*120, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(-19,-13, 10, 13,16*205,16*130, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Arc(  9,-13, 10, 13,16*205,16*130, Component::getPen(Qt::darkBlue,1)));
}

void BondWire::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void BondWire::initProperties()
{
   addProperty("L","3 mm",QObject::tr("length of the wire"),true);
   addProperty("D","50 um",QObject::tr("diameter of the wire"),true);
   addProperty("H","2 mm",QObject::tr("height above ground plane"),true);
   addProperty("rho","0.022e-6",QObject::tr("specific resistance of the metal"),false);
   addProperty("mur","1",QObject::tr("relative permeability of the metal"),false);
   addProperty("Model","FREESPACE",QObject::tr("bond wire model"),false, QString("FREESPACE,MIRROR,DESCHARLES").split(',',QString::SkipEmptyParts));
   addProperty("Subst","Subst1",QObject::tr("substrate"),true);
   addProperty("Temp","26.85",QObject::tr("simulation temperature in degree Celsius"),false);
}

