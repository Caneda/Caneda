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

#include "dcfeed.h"
#include "shapes.h"

dcFeed::dcFeed(SchematicScene *s) : Component(s)
{
   initConstants();
   initPorts();
   initProperties();
}

void dcFeed::initConstants()
{
   qreal pw = 0.5;
   m_boundingRect = QRectF( -30, -15, 60, 31).adjusted(-pw, -pw, pw, pw);

   model = "DCFeed";
   name = "L";
   description =  QObject::tr("dc feed");

   m_shapes.append(new Arc(-17, -6, 12, 12,  0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc( -6, -6, 12, 12,  0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Arc(  5, -6, 12, 12,  0, 16*180, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-30,  0,-17,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line( 17,  0, 30,  0, Component::getPen(Qt::darkBlue,2)));
   m_shapes.append(new Line(-23,-13, 23,-13, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-23, 13, 23, 13, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line(-23,-13,-23, 13, Component::getPen(Qt::darkBlue,1)));
   m_shapes.append(new Line( 23,-13, 23, 13, Component::getPen(Qt::darkBlue,1)));
}

void dcFeed::initPorts()
{
   addPort(QPointF(-30,0));
   addPort(QPointF(30,0));
}

void dcFeed::initProperties()
{
   addProperty("L","1 uH",QObject::tr("for transient simulation: inductance in Henry"),false);
}

