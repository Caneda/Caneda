/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "resistor.h"
#include "shapes.h"
#include "componentproperty.h"

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

Resistor::Resistor(SchematicScene *s) : MultiSymbolComponent(s)
{
   initConstants();
   initSymbolMap();
   initPorts();
   initProperties();
}

void Resistor::initConstants()
{
   model = name = "R";
   description = QObject::tr("resistor");
}

void Resistor::initPorts()
{
   addPort(QPointF(-30.0,0.0));
   addPort(QPointF(30.0,0.0));
}

void Resistor::initProperties()
{
   addProperty("R","100k","Simple resistor",true);
}

QString Resistor::netlist() const
{
   QString s = model+":"+name;

   // output all node names
//   foreach(ComponentPort *port, m_ports)
   //    s += ' ' + port->node()->name(); // node names

   // output all properties
   foreach(ComponentProperty *prop, m_properties)
   {
      if(prop->name() != "Symbol")
         s += ' ' + prop->name() + "'=\"" + prop->value() + "\"";
   }
   return s;
}

void Resistor::initSymbolMap()
{
   SymbolData *s = new SymbolData();
   s->shapesList.append(new Line(-18, -9, 18, -9,getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( 18, -9, 18,  9,getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( 18,  9,-18,  9,getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line(-18,  9,-18, -9,getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line(-27,  0,-18,  0,getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( 18,  0, 27,  0,getPen(Qt::darkBlue,2)));
   s->boundRect = QRectF(-27,-9,54,18);
   symbolMap["Regular"] = s;

   s = new SymbolData();
   s->shapesList.append(new Line(-27,  0,-18,  0, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line(-18,  0,-15, -7, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line(-15, -7, -9,  7, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( -9,  7, -3, -7, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( -3, -7,  3,  7, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line(  3,  7,  9, -7, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line(  9, -7, 15,  7, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( 15,  7, 18,  0, getPen(Qt::darkBlue,2)));
   s->shapesList.append(new Line( 18,  0, 27,  0, getPen(Qt::darkBlue,2)));
   s->boundRect = QRectF(-27,-9,54,18);
   symbolMap["US"] = s;

   setSymbol("Regular");
}
