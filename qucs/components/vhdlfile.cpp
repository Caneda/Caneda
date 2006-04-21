/***************************************************************************
                               vhdlfile.cpp
                              --------------
    begin                : Sat Apr 15 2006
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

#include "vhdlfile.h"
#include "qucs.h"
#include "main.h"
#include "schematic.h"

#include <qregexp.h>


VHDL_File::VHDL_File()
{
  Type = isDigitalComponent;
  Description = QObject::tr("VHDL file");

  Props.append(new Property("File", "sub.vhdl", false,
		QObject::tr("Name of VHDL file")));

  createSymbol();
  tx = x1+4;
  ty = y2+4;
  Model = "VHDL";
  Name  = "X";
}

VHDL_File::~VHDL_File()
{
}

Component* VHDL_File::newOne()
{
  VHDL_File *p = new VHDL_File();
  p->Props.getFirst()->Value = Props.getFirst()->Value;
  p->recreate(0);
  return p;
}

// -------------------------------------------------------
Element* VHDL_File::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("VHDL file");
  BitmapFile = "vhdlfile";

  if(getNewOne)  return new VHDL_File();
  return 0;
}

// -------------------------------------------------------
QString VHDL_File::VHDL_Code(int)
{
  QString s;
  Port *pp = Ports.first();
  if(pp) {
    s = "  " + Name + ": entity " + EntityName + " port map (";

    // output all node names
    if(pp)  s += pp->Connection->Name;
    for(pp = Ports.next(); pp != 0; pp = Ports.next())
      s += ", "+pp->Connection->Name;   // node names

    s += ");";
  }
  return s;
}

// -------------------------------------------------------
// Returns a comma separated list of the port names of the last
// entity in this file.
QString VHDL_File::loadFile()
{
  QString s, File(Props.getFirst()->Value);
  QFileInfo Info(File);
  if(Info.isRelative())
    File = QucsWorkDir.filePath(File);

  QFile f(File);
  if(!f.open(IO_ReadOnly))
    return QString("");

  QTextStream stream(&f);
  File = stream.read();   // QString is better for "find" function
  f.close();

  int i=0, j, k=0;
  while((i=File.find("--", i)) >= 0) { // remove all VHDL comments
    j = File.find('\n', i+2);          // (This also finds "--" within a ...
    if(j < 0)                          //  string, but as no strings are ...
      File = File.left(i);             //  allowed in entity headers, it ...
    else                               //  does not matter.)
      File.remove(i, j-i);
    i--;
  }

  
  QRegExp Expr;
  Expr.setCaseSensitive(false);
  for(;;) {
    k--;
    Expr.setPattern("\\bentity\\b");  // start of last entity
    k = File.findRev(Expr, k);
    if(k < 0)
      return QString("");

    Expr.setPattern("\\bend\\b");    // end of last entity
    i = File.find(Expr, k+7);
    if(i < 0)
      return QString("");
    s = File.mid(k+7, i-k-7);  // cut out entity declaration

    Expr.setPattern("\\b");
    i = s.find(Expr);
    if(i < 0)
      return QString("");
    j = s.find(Expr, i+1);
    if(j < 0)
      return QString("");
    EntityName = s.mid(i, j-i);  // save entity name

    i = s.find(Expr, j+1);
    if(i < 0)
      return QString("");
    j = s.find(Expr, i+1);
    if(j < 0)
      return QString("");
    if(s.mid(i, j-i).lower() == "is")   // really found start of entity ?
      break;

    if(k < 1)    // already searched the whole text
      return QString("");
  }

  Expr.setPattern("\\bport\\b");  // start of interface definition
  i = s.find(Expr, j+1);
  if(i < 0)
    return QString("");
  i = s.find('(', i+4) + 1;
  if(i <= 0)
    return QString("");
  j = s.find(')', i);
  if(j < 0)
    return QString("");
  s = s.mid(i, j-i);
  s.remove(' ');

  i = 0;    // remove all VHDL identifiers (e.g. ": out bit;")
  while((i=s.find(':', i)) >= 0) {
    j = s.find(';', i+2);
    if(j < 0)
      s = s.left(i);
    else
      s.remove(i, j-i);
    i--;
  }

  s.remove('\n');
  s.remove('\t');
  s.replace(';', ',');
  return s;
}

// -------------------------------------------------------
void VHDL_File::createSymbol()
{
  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  int fHeight = metrics.lineSpacing();

  int No = 0;
  QString tmp, PortNames = loadFile();
  if(!PortNames.isEmpty())
    No = PortNames.contains(',') + 1;


  int h = 30*((No-1)/2) + 15;
  Lines.append(new Line(-16, -h, 16, -h,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( 16, -h, 16,  h,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-16,  h, 16,  h,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-16, -h,-16,  h,QPen(QPen::darkBlue,2)));

  tmp = QObject::tr("vhdl");
  int w = metrics.width(tmp);
  Texts.append(new Text(w/-2, fHeight/-2, tmp));


  int y = 15-h, i = 0;
  while(i<No) {
    Lines.append(new Line(-30,  y,-16,  y,QPen(QPen::darkBlue,2)));
    Ports.append(new Port(-30,  y));
    tmp = PortNames.section(',', i, i);
    w = metrics.width(tmp);
    Texts.append(new Text(-19-w, y-fHeight-2, tmp));
    i++;

    if(i == No) break;
    Lines.append(new Line( 16,  y, 30,  y,QPen(QPen::darkBlue,2)));
    Ports.append(new Port( 30,  y));
    tmp = PortNames.section(',', i, i);
    Texts.append(new Text( 20, y-fHeight-2, tmp));
    y += 60;
    i++;
  }

  x1 = -30; y1 = -h-2;
  x2 =  30; y2 =  h+2;
}

// -------------------------------------------------------
void VHDL_File::recreate(Schematic *Doc)
{
  if(Doc) {
    Doc->Components->setAutoDelete(false);
    Doc->deleteComp(this);
  }

  Ports.clear();
  Lines.clear();
  Texts.clear();
  createSymbol();
  performModification();  // rotate and mirror

  if(Doc) {
    Doc->insertRawComponent(this);
    Doc->Components->setAutoDelete(true);
  }
}
