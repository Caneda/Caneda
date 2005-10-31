/***************************************************************************
                               symbolwidget.cpp
                              ------------------
    begin                : Sat May 28 2005
    copyright            : (C) 2005 by Michael Margraf
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

#include <math.h>

#include "symbolwidget.h"
#include "qucslib.h"

#include <qpainter.h>
#include <qtextstream.h>


SymbolWidget::SymbolWidget(QWidget *parent) : QWidget(parent)
{
  Arcs.setAutoDelete(true);
  Lines.setAutoDelete(true);
  Rects.setAutoDelete(true);
  Ellips.setAutoDelete(true);
  Texts.setAutoDelete(true);

  Text_x = Text_y = 0;
  PaintText = tr("Symbol:");
  QFont Font(QucsSettings.font);
  QFontMetrics  metrics(Font);
  TextWidth = metrics.width(PaintText) + 4;    // get size of text

  setPaletteBackgroundColor(Qt::white);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

SymbolWidget::~SymbolWidget()
{
}

// ************************************************************
void SymbolWidget::paintEvent(QPaintEvent*)
{
  QPainter Painter(this);
  Painter.drawText(2, 2, 0, 0, Qt::AlignAuto | Qt::DontClip, PaintText);

  // paint all lines
  for(Line *pl = Lines.first(); pl != 0; pl = Lines.next()) {
    Painter.setPen(pl->style);
    Painter.drawLine(cx+pl->x1, cy+pl->y1, cx+pl->x2, cy+pl->y2);
  }

  // paint all arcs
  for(Arc *pc = Arcs.first(); pc != 0; pc = Arcs.next()) {
    Painter.setPen(pc->style);
    Painter.drawArc(cx+pc->x, cy+pc->y, pc->w, pc->h, pc->angle, pc->arclen);
  }

  // paint all rectangles
  Area *pa;
  for(pa = Rects.first(); pa != 0; pa = Rects.next()) {
    Painter.setPen(pa->Pen);
    Painter.setBrush(pa->Brush);
    Painter.drawRect(cx+pa->x, cy+pa->y, pa->w, pa->h);
  }

  // paint all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) {
    Painter.setPen(pa->Pen);
    Painter.setBrush(pa->Brush);
    Painter.drawEllipse(cx+pa->x, cy+pa->y, pa->w, pa->h);
  }

  Painter.setPen(QPen(QPen::black,1));
  QFont Font = Painter.font();   // save current font
  Font.setWeight(QFont::Light);
  // write all text
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    Font.setPointSizeFloat(pt->Size);
    Painter.setFont(Font);
    Painter.setPen(pt->Color);
    Painter.drawText(cx+pt->x, cy+pt->y, pt->s);
  }
}

// ************************************************************
// Creates a symbol from the model name of a component.
int SymbolWidget::createSymbol(const QString& ModelString,
                               const QString& Lib_, const QString& Comp_)
{
  Arcs.clear();
  Lines.clear();
  Rects.clear();
  Ellips.clear();
  Texts.clear();
  LibraryName = Lib_;
  ComponentName = Comp_;

  int PortNo = 0;
  QString Comp = ModelString.section(' ', 0,0);
  Comp.remove(0, 1);  // remove '<'

  QString FirstProp = ModelString.section('"', 1,1);

  if(Comp == "_BJT") {
    Lines.append(new Line(-10,-15,-10, 15,QPen(QPen::darkBlue,3)));
    Lines.append(new Line(-30,  0,-10,  0,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-10, -5,  0,-15,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  0,-15,  0,-30,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-10,  5,  0, 15,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  0, 15,  0, 30,QPen(QPen::darkBlue,2)));

    if(FirstProp == "npn") {
      Lines.append(new Line( -6, 15,  0, 15,QPen(QPen::darkBlue,2)));
      Lines.append(new Line(  0,  9,  0, 15,QPen(QPen::darkBlue,2)));
    }
    else {
      Lines.append(new Line( -5, 10, -5, 16,QPen(QPen::darkBlue,2)));
      Lines.append(new Line( -5, 10,  1, 10,QPen(QPen::darkBlue,2)));
    }

    Arcs.append(new struct Arc(-34, -4, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(-4, -34, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(-4, 26, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));

    PortNo = 3;
    x1 = -34; y1 = -34;
    x2 =   4; y2 =  34;
  }
  else if(Comp == "JFET") {
    Lines.append(new Line(-10,-15,-10, 15,QPen(QPen::darkBlue,3)));
    Lines.append(new Line(-30,  0,-10,  0,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-10,-10,  0,-10,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  0,-10,  0,-30,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-10, 10,  0, 10,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  0, 10,  0, 30,QPen(QPen::darkBlue,2)));

    Lines.append(new Line( -4, 24,  4, 20,QPen(QPen::darkBlue,2)));

    if(FirstProp == "nfet") {
      Lines.append(new Line(-16, -5,-11,  0,QPen(QPen::darkBlue,2)));
      Lines.append(new Line(-16,  5,-11,  0,QPen(QPen::darkBlue,2)));
    }
    else {
      Lines.append(new Line(-18, 0,-13, -5,QPen(QPen::darkBlue,2)));
      Lines.append(new Line(-18, 0,-13,  5,QPen(QPen::darkBlue,2)));
    }

    Arcs.append(new struct Arc(-34, -4, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(-4, -34, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(-4, 26, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));

    PortNo = 3;
    x1 = -30; y1 = -30;
    x2 =   4; y2 =  30;
  }
  else if(Comp == "_MOSFET") {
    Lines.append(new Line(-14,-13,-14, 13,QPen(QPen::darkBlue,3)));
    Lines.append(new Line(-30,  0,-14,  0,QPen(QPen::darkBlue,2)));

    Lines.append(new Line(-10,-11,  0,-11,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  0,-11,  0,-30,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-10, 11,  0, 11,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  0,  0,  0, 30,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-10,  0,  0,  0,QPen(QPen::darkBlue,2)));

    Lines.append(new Line(-10,-16,-10, -7,QPen(QPen::darkBlue,3)));
    Lines.append(new Line(-10,  7,-10, 16,QPen(QPen::darkBlue,3)));

    if(FirstProp == "nfet") {
      Lines.append(new Line( -9,  0, -4, -5,QPen(QPen::darkBlue,2)));
      Lines.append(new Line( -9,  0, -4,  5,QPen(QPen::darkBlue,2)));
    }
    else {
      Lines.append(new Line( -1,  0, -6, -5,QPen(QPen::darkBlue,2)));
      Lines.append(new Line( -1,  0, -6,  5,QPen(QPen::darkBlue,2)));
    }

    // depletion or enhancement MOSFET ?
    if((ModelString.section('"', 3,3).stripWhiteSpace().at(0) == '-') ==
       (FirstProp == "nfet"))
      Lines.append(new Line(-10, -8,-10,  8,QPen(QPen::darkBlue,3)));
    else
      Lines.append(new Line(-10, -4,-10,  4,QPen(QPen::darkBlue,3)));

    Arcs.append(new struct Arc(-34, -4, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(-4, -34, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(-4, 26, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));

    PortNo = 3;
    x1 = -34; y1 = -34;
    x2 =   4; y2 =  34;
  }
  else if(Comp == "Diode") {
    Lines.append(new Line(-30,  0, 30,  0,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( -6, -9, -6,  9,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(  6, -9,  6,  9,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( -6,  0,  6, -9,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( -6,  0,  6,  9,QPen(QPen::darkBlue,2)));

    Arcs.append(new struct Arc(-34, -4, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));
    Arcs.append(new struct Arc(26, -4, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));

    PortNo = 2;
    x1 = -34; y1 = -9;
    x2 =  34; y2 =  9;
  }
  else if(Comp == "SUBST") {
    Lines.append(new Line(-30,-16, 30,-16,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-30,-12, 30,-12,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-30, 16, 30, 16,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-30, 12, 30, 12,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-30,-16,-30, 16,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 30,-16, 30, 16,QPen(QPen::darkBlue,2)));

    Lines.append(new Line(-30,-16, 16,-40,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 30,-16, 80,-40,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 30,-12, 80,-36,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 30, 12, 80,-16,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 30, 16, 80,-12,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 16,-40, 80,-40,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 80,-40, 80,-12,QPen(QPen::darkBlue,2)));
  
    Lines.append(new Line(-30,  0,-18,-12,QPen(QPen::darkBlue,1)));
    Lines.append(new Line(-22, 12,  2,-12,QPen(QPen::darkBlue,1)));
    Lines.append(new Line( -2, 12, 22,-12,QPen(QPen::darkBlue,1)));
    Lines.append(new Line( 18, 12, 30,  0,QPen(QPen::darkBlue,1)));

    Lines.append(new Line( 30,  1, 37,  8,QPen(QPen::darkBlue,1)));
    Lines.append(new Line( 37,-15, 52,  0,QPen(QPen::darkBlue,1)));
    Lines.append(new Line( 52,-22, 66, -8,QPen(QPen::darkBlue,1)));
    Lines.append(new Line( 66,-30, 80,-16,QPen(QPen::darkBlue,1)));

    PortNo = 0;
    x1 = -34; y1 =-44;
    x2 =  84; y2 = 20;
  }
  
  x1 -= 4;   // enlarge component boundings a little
  x2 += 4;
  y1 -= 4;
  y2 += 4;
  cx  = -x1 + TextWidth;
  cy  = -y1;
  int dx = x2-x1 + TextWidth;
  setMinimumSize(dx, y2-y1);
  if(width() > dx)  dx = width();
  resize(dx, y2-y1);
  update();
  return PortNo;
}

// ************************************************************
// Loads the symbol for the component from the symbol field and
// returns the number of painting elements.
int SymbolWidget::setSymbol(const QString& SymbolString,
                            const QString& Lib_, const QString& Comp_)
{
  Arcs.clear();
  Lines.clear();
  Rects.clear();
  Ellips.clear();
  Texts.clear();
  LibraryName = Lib_;
  ComponentName = Comp_;

  QString Line;
  QTextIStream stream(&SymbolString);

  x1 = y1 = INT_MAX;
  x2 = y2 = INT_MIN;

  int z=0, Result;
  while(!stream.atEnd()) {
    Line = stream.readLine();
    Line = Line.stripWhiteSpace();
    if(Line.isEmpty()) continue;

    if(Line.at(0) != '<') return -1;
    if(Line.at(Line.length()-1) != '>') return -1;
    Line = Line.mid(1, Line.length()-2); // cut off start and end character
    Result = analyseLine(Line);
    if(Result < 0) return -6;   // line format error
    z += Result;
  }

  x1 -= 4;   // enlarge component boundings a little
  x2 += 4;
  y1 -= 4;
  y2 += 4;
  cx  = -x1 + TextWidth;
  cy  = -y1;
  int dx = x2-x1 + TextWidth;
  setMinimumSize(dx, y2-y1);
  if(width() > dx)  dx = width();
  resize(dx, y2-y1);
  update();
  return z;      // return number of ports
}

// ---------------------------------------------------------------------
int SymbolWidget::analyseLine(const QString& Row)
{
  QPen Pen;
  QBrush Brush;
  QColor Color;
  QString s;
  int i1, i2, i3, i4, i5, i6;

  s = Row.section(' ',0,0);    // component type
  if(s == ".PortSym") {  // here: ports are open nodes
    if(!getIntegers(Row, &i1, &i2, &i3))  return -1;
    Arcs.append(new struct Arc(i1-4, i2-4, 8, 8, 0, 16*360,
                               QPen(QPen::red,1)));

    if((i1-4) < x1)  x1 = i1-4;  // keep track of component boundings
    if((i1+4) > x2)  x2 = i1+4;
    if((i2-4) < y1)  y1 = i2-4;
    if((i2+4) > y2)  y2 = i2+4;
    return 0;   // do not count Ports
  }
  else if(s == "Line") {
    if(!getIntegers(Row, &i1, &i2, &i3, &i4))  return -1;
    if(!getPen(Row, Pen, 5))  return -1;
    i3 += i1;
    i4 += i2;
    Lines.append(new Line(i1, i2, i3, i4, Pen));

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i1 > x2)  x2 = i1;
    if(i2 < y1)  y1 = i2;
    if(i2 > y2)  y2 = i2;
    if(i3 < x1)  x1 = i3;
    if(i3 > x2)  x2 = i3;
    if(i4 < y1)  y1 = i4;
    if(i4 > y2)  y2 = i4;
    return 1;
  }
  else if(s == "EArc") {
    if(!getIntegers(Row, &i1, &i2, &i3, &i4, &i5, &i6))  return -1;
    if(!getPen(Row, Pen, 7))  return -1;
    Arcs.append(new struct Arc(i1, i2, i3, i4, i5, i6, Pen));

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i1+i3 > x2)  x2 = i1+i3;
    if(i2 < y1)  y1 = i2;
    if(i2+i4 > y2)  y2 = i2+i4;
    return 1;
  }
  else if(s == ".ID") {
    if(!getIntegers(Row, &i1, &i2))  return -1;
    Text_x = i1;
    Text_y = i2;
    Prefix = Row.section(' ',3,3);
    if(Prefix.isEmpty())  Prefix = "X";
    return 0;   // do not count IDs
  }
  else if(s == "Arrow") {
    if(!getIntegers(Row, &i1, &i2, &i3, &i4, &i5, &i6))  return -1;
    if(!getPen(Row, Pen, 7))  return -1;

    double beta   = atan2(double(i6), double(i5));
    double phi    = atan2(double(i4), double(i3));
    double Length = sqrt(double(i6*i6 + i5*i5));

    i3 += i1;
    i4 += i2;
    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i1 > x2)  x2 = i1;
    if(i3 < x1)  x1 = i3;
    if(i3 > x2)  x2 = i3;
    if(i2 < y1)  y1 = i2;
    if(i2 > y2)  y2 = i2;
    if(i4 < y1)  y1 = i4;
    if(i4 > y2)  y2 = i4;

    Lines.append(new Line(i1, i2, i3, i4, Pen));   // base line

    double w = beta+phi;
    i5 = i3-int(Length*cos(w));
    i6 = i4-int(Length*sin(w));
    Lines.append(new Line(i3, i4, i5, i6, Pen)); // arrow head
    if(i5 < x1)  x1 = i5;  // keep track of component boundings
    if(i5 > x2)  x2 = i5;
    if(i6 < y1)  y1 = i6;
    if(i6 > y2)  y2 = i6;

    w = phi-beta;
    i5 = i3-int(Length*cos(w));
    i6 = i4-int(Length*sin(w));
    Lines.append(new Line(i3, i4, i5, i6, Pen));
    if(i5 < x1)  x1 = i5;  // keep track of component boundings
    if(i5 > x2)  x2 = i5;
    if(i6 < y1)  y1 = i6;
    if(i6 > y2)  y2 = i6;

    return 1;
  }
  else if(s == "Ellipse") {
    if(!getIntegers(Row, &i1, &i2, &i3, &i4))  return -1;
    if(!getPen(Row, Pen, 5))  return -1;
    if(!getBrush(Row, Brush, 8))  return -1;
    Ellips.append(new Area(i1, i2, i3, i4, Pen, Brush));

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i1 > x2)  x2 = i1;
    if(i2 < y1)  y1 = i2;
    if(i2 > y2)  y2 = i2;
    if(i1+i3 < x1)  x1 = i1+i3;
    if(i1+i3 > x2)  x2 = i1+i3;
    if(i2+i4 < y1)  y1 = i2+i4;
    if(i2+i4 > y2)  y2 = i2+i4;
    return 1;
  }
  else if(s == "Rectangle") {
    if(!getIntegers(Row, &i1, &i2, &i3, &i4))  return -1;
    if(!getPen(Row, Pen, 5))  return -1;
    if(!getBrush(Row, Brush, 8))  return -1;
    Rects.append(new Area(i1, i2, i3, i4, Pen, Brush));

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i1 > x2)  x2 = i1;
    if(i2 < y1)  y1 = i2;
    if(i2 > y2)  y2 = i2;
    if(i1+i3 < x1)  x1 = i1+i3;
    if(i1+i3 > x2)  x2 = i1+i3;
    if(i2+i4 < y1)  y1 = i2+i4;
    if(i2+i4 > y2)  y2 = i2+i4;
    return 1;
  }
  else if(s == "Text") {  // must be last in order to reuse "s" *********
    if(!getIntegers(Row, &i1, &i2, &i3))  return -1;
    Color.setNamedColor(Row.section(' ',4,4));
    if(!Color.isValid()) return -1;

    s = Row.mid(Row.find('"')+1);    // Text (can contain " !!!)
    s = s.left(s.length()-1);
    if(s.isEmpty()) return -1;
    s.replace("\\n", "\n");
    s.replace("\\\\", "\\");

    Texts.append(new Text(i1, i2, s, Color, float(i3)));

    QFont Font(QucsSettings.font);
    Font.setPointSizeFloat(float(i3));
    QFontMetrics  metrics(Font);
    QSize r = metrics.size(0, s);    // get size of text
    i3 = i1 + r.width();
    i4 = i2 + r.height();

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i3 > x2)  x2 = i3;
    if(i2 < y1)  y1 = i2;
    if(i4 > y2)  y2 = i4;
  }

  return 0;
}

// ---------------------------------------------------------------------
bool SymbolWidget::getIntegers(const QString& s, int *i1, int *i2, int *i3,
			     int *i4, int *i5, int *i6)
{
  bool ok;
  QString n;

  if(!i1) return true;
  n  = s.section(' ',1,1);
  *i1 = n.toInt(&ok);
  if(!ok) return false;

  if(!i2) return true;
  n  = s.section(' ',2,2);
  *i2 = n.toInt(&ok);
  if(!ok) return false;

  if(!i3) return true;
  n  = s.section(' ',3,3);
  *i3 = n.toInt(&ok);
  if(!ok) return false;

  if(!i4) return true;
  n  = s.section(' ',4,4);
  *i4 = n.toInt(&ok);
  if(!ok) return false;

  if(!i5) return true;
  n  = s.section(' ',5,5);
  *i5 = n.toInt(&ok);
  if(!ok) return false;

  if(!i6) return true;
  n  = s.section(' ',6,6);
  *i6 = n.toInt(&ok);
  if(!ok) return false;

  return true;
}

// ---------------------------------------------------------------------
bool SymbolWidget::getPen(const QString& s, QPen& Pen, int i)
{
  bool ok;
  QString n;

  n = s.section(' ',i,i);    // color
  QColor co;
  co.setNamedColor(n);
  Pen.setColor(co);
  if(!Pen.color().isValid()) return false;

  i++;
  n = s.section(' ',i,i);    // thickness
  Pen.setWidth(n.toInt(&ok));
  if(!ok) return false;

  i++;
  n = s.section(' ',i,i);    // line style
  Pen.setStyle((Qt::PenStyle)n.toInt(&ok));
  if(!ok) return false;

  return true;
}

// ---------------------------------------------------------------------
bool SymbolWidget::getBrush(const QString& s, QBrush& Brush, int i)
{
  bool ok;
  QString n;

  n = s.section(' ',i,i);    // fill color
  QColor co;
  co.setNamedColor(n);
  Brush.setColor(co);
  if(!Brush.color().isValid()) return false;

  i++;
  n = s.section(' ',i,i);    // fill style
  Brush.setStyle((Qt::BrushStyle)n.toInt(&ok));
  if(!ok) return false;

  i++;
  n = s.section(' ',i,i);    // filled
  if(n.toInt(&ok) == 0) Brush.setStyle(QBrush::NoBrush);
  if(!ok) return false;

  return true;
}
