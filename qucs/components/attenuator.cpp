/***************************************************************************
                          attenuator.cpp  -  description
                             -------------------
    begin                : Sat Aug 23 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : margraf@mwt.ee.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "attenuator.h"


Attenuator::Attenuator()
{
  Description = QObject::tr("attenuator");

  Lines.append(new Line( -4, -6, -4,  6,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( -4, -6,  4, -6,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  4, -6,  4,  6,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( -4,  6,  4,  6,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  0,-11,  0, -6,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  0,  6,  0, 11,QPen(QPen::darkBlue,2)));

  Lines.append(new Line(-14,-14, 14,-14,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-14, 14, 14, 14,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-14,-14,-14, 14,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( 14,-14, 14, 14,QPen(QPen::darkBlue,2)));

  Lines.append(new Line(-30,  0,-14,  0,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( 14,  0, 30,  0,QPen(QPen::darkBlue,2)));

  Ports.append(new Port(-30,  0));
  Ports.append(new Port( 30,  0));

  x1 = -30; y1 = -17;
  x2 =  30; y2 =  17;

  tx = x1+4;
  ty = y2+4;
  Sign  = "Attenuator";
  Model = "Attenuator";
  Name  = "X";

  Props.append(new Property("L", "10 dB", true, QObject::tr("attenuation")));
  Props.append(new Property("Zref", "50 Ohm", false, QObject::tr("reference impedance")));
}

Attenuator::~Attenuator()
{
}

Attenuator* Attenuator::newOne()
{
  return new Attenuator();
}
