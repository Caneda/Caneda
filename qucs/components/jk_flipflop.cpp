/***************************************************************************
                              jk_flipflop.cpp
                             -----------------
    begin                : Fri Jan 06 2006
    copyright            : (C) 2006 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include "jk_flipflop.h"
#include "node.h"

JK_FlipFlop::JK_FlipFlop()
{
  Type = isDigitalComponent;
  Description = QObject::tr("JK flip flop with asynchron set and reset");

  Props.append(new Property("t", "0", false, QObject::tr("delay time")));

  Lines.append(new Line(-20,-30, 20,-30,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-20, 30, 20, 30,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-20,-30,-20, 30,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( 20,-30, 20, 30,QPen(QPen::darkBlue,2)));

  Lines.append(new Line(-30,-20,-20,-20,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-30, 20,-20, 20,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( 30,-20, 20,-20,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( 30, 20, 20, 20,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-30,  0,-20,  0,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  0,-30,  0,-40,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  0, 30,  0, 40,QPen(QPen::darkBlue,2)));

  Texts.append(new Text( -4,-27, "S", QPen::darkBlue,  9.0));
  Texts.append(new Text( -4, 16, "R", QPen::darkBlue,  9.0));
  Texts.append(new Text(-17,-27, "J", QPen::darkBlue, 12.0));
  Texts.append(new Text(-17, 13, "K", QPen::darkBlue, 12.0));
  Texts.append(new Text(  6,-27, "Q", QPen::darkBlue, 12.0));
  Texts.append(new Text(  6, 13, "Q", QPen::darkBlue, 12.0));
  Lines.append(new Line(  7, 13, 15, 13,QPen(QPen::darkBlue,1)));
  Lines.append(new Line(-20, -4,-12,  0,QPen(QPen::darkBlue,0)));
  Lines.append(new Line(-20,  4,-12,  0,QPen(QPen::darkBlue,0)));

  Ports.append(new Port(-30,-20));  // J
  Ports.append(new Port(-30, 20));  // K
  Ports.append(new Port( 30,-20));  // Q
  Ports.append(new Port( 30, 20));  // nQ
  Ports.append(new Port(-30,  0));  // Clock
  Ports.append(new Port(  0,-40));  // set
  Ports.append(new Port(  0, 40));  // reset

  x1 = -30; y1 = -40;
  x2 =  30; y2 =  40;
  tx = x1+4;
  ty = y2+4;
  Model = "JKFF";
  Name  = "Y";
}

// -------------------------------------------------------
QString JK_FlipFlop::VHDL_Code(int NumPorts)
{
  QString s = ";\n";
  if(NumPorts <= 0)  // no truth table simulation ?
    if(strtod(Props.getFirst()->Value.latin1(), 0) != 0.0)  // delay time
      s = " after " + Props.getFirst()->Value + ";\n";

  s = "  " + Name + " : process (" +
      Ports.at(5)->Connection->Name + ", " +
      Ports.at(6)->Connection->Name + ", " +
      Ports.at(4)->Connection->Name + ")\n  begin\n    if (" +
      Ports.at(6)->Connection->Name + "='1') then  " +
      Ports.at(2)->Connection->Name + " <= '0'" + s +"    elsif (" +
      Ports.at(5)->Connection->Name + "='1') then  " +
      Ports.at(2)->Connection->Name + " <= '1'" + s +"    elsif (" +
      Ports.at(4)->Connection->Name + "='1' and " +
      Ports.at(4)->Connection->Name + "'event) then\n      " +
      Ports.at(2)->Connection->Name + " <= (" +
      Ports.at(0)->Connection->Name + " and not " +
      Ports.at(2)->Connection->Name + ") or (not " +
      Ports.at(1)->Connection->Name + " and " +
      Ports.at(2)->Connection->Name + ")" + s +
      "    end if;\n  end process;\n  " +
      Ports.at(3)->Connection->Name + " <= not " +
      Ports.at(2)->Connection->Name + ";\n";
  return s;
}

// -------------------------------------------------------
Component* JK_FlipFlop::newOne()
{
  return new JK_FlipFlop();
}

// -------------------------------------------------------
Element* JK_FlipFlop::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("JK-FlipFlop");
  BitmapFile = "jkflipflop";

  if(getNewOne)  return new JK_FlipFlop();
  return 0;
}
