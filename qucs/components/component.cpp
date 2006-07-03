/***************************************************************************
                               component.cpp
                              ---------------
    begin                : Sat Aug 23 2003
    copyright            : (C) 2003 by Michael Margraf
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

#include "components.h"
#include "node.h"
#include "main.h"
#include "qucs.h"
#include "schematic.h"
#include "viewpainter.h"

#include <qdir.h>
#include <qpen.h>
#include <qpoint.h>
#include <qstring.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qmessagebox.h>



// ***********************************************************************
// **********                                                   **********
// **********                  class "Component"                **********
// **********                                                   **********
// ***********************************************************************
Component::Component()
{
  Type = isAnalogComponent;

  mirroredX = false;
  rotated = 0;
  isSelected = false;
  isActive = COMP_IS_ACTIVE;
  showName = true;

  cx = 0;
  cy = 0;

  Arcs.setAutoDelete(true);
  Lines.setAutoDelete(true);
  Rects.setAutoDelete(true);
  Ellips.setAutoDelete(true);
  Ports.setAutoDelete(true);
  Texts.setAutoDelete(true);
  Props.setAutoDelete(true);
}

// -------------------------------------------------------
Component* Component::newOne()
{
  return new Component();
}

// -------------------------------------------------------
void Component::Bounding(int& _x1, int& _y1, int& _x2, int& _y2)
{
  _x1 = x1+cx;
  _y1 = y1+cy;
  _x2 = x2+cx;
  _y2 = y2+cy;
}

// -------------------------------------------------------
// Size of component text.
int Component::textSize(int& _dx, int& _dy)
{
  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  int tmp, count=0;
  _dx = _dy = 0;
  if(showName) {
    _dx = metrics.width(Name);
    _dy = metrics.height();
    count++;
  }
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      tmp = metrics.width(pp->Name+"="+pp->Value);
      if(tmp > _dx)  _dx = tmp;
      _dy += metrics.height();
      count++;
    }
  return count;
}

// -------------------------------------------------------
// Boundings including the component text.
void Component::entireBounds(int& _x1, int& _y1, int& _x2, int& _y2, float Corr)
{
  _x1 = x1+cx;
  _y1 = y1+cy;
  _x2 = x2+cx;
  _y2 = y2+cy;

  // text boundings
  if(tx < x1) _x1 = tx+cx;
  if(ty < y1) _y1 = ty+cy;

  int dx, dy, ny;
  ny = textSize(dx, dy);
  dy = int(float(ny) / Corr);  // correction for unproportional font scaling

  if((tx+dx) > x2) _x2 = tx+dx+cx;
  if((ty+dy) > y2) _y2 = ty+dy+cy;
}

// -------------------------------------------------------
void Component::setCenter(int x, int y, bool relative)
{
  if(relative) { cx += x;  cy += y; }
  else { cx = x;  cy = y; }
}

// -------------------------------------------------------
void Component::getCenter(int& x, int& y)
{
  x = cx;
  y = cy;
}

// -------------------------------------------------------
int Component::getTextSelected(int x_, int y_, float Corr)
{
  x_ -= cx;
  y_ -= cy;
  if(x_ < tx) return -1;
  if(y_ < ty) return -1;

  x_ -= tx;
  y_ -= ty;
  int w, dy = int(float(y_) * Corr);  // correction for font scaling
  QFontMetrics  metrics(QucsSettings.font);
  if(showName) {
    w  = metrics.width(Name);
    if(dy < 1) {
      if(x_ < w) return 0;
      return -1;
    }
    dy--;
  }

  Property *pp;
  for(pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display)
      if((dy--) < 1) break;
  if(!pp) return -1;

  // get width of text
  w = metrics.width(pp->Name+"="+pp->Value);
  if(x_ > w) return -1;
  return Props.at()+1;  // number the property
}

// -------------------------------------------------------
bool Component::getSelected(int x_, int y_)
{
  x_ -= cx;
  y_ -= cy;
  if(x_ >= x1) if(x_ <= x2) if(y_ >= y1) if(y_ <= y2)
    return true;

  return false;
}

// -------------------------------------------------------
void Component::paint(ViewPainter *p)
{
  Text *pt;
  int x, y, a, b, xb, yb;
  QFont f = p->Painter->font();   // save current font
  QFont newFont = f;

  if(Model.at(0) == '.') {   // is simulation component (dc, ac, ...)
    newFont.setPointSizeFloat(p->Scale * Texts.getFirst()->Size);
    newFont.setWeight(QFont::DemiBold);
    p->Painter->setFont(newFont);
    p->map(cx, cy, x, y);

    p->Painter->setPen(QPen(QPen::darkBlue,2));
    a = b = 0;
    QRect r;
    for(pt = Texts.first(); pt != 0; pt = Texts.next()) {
      p->Painter->drawText(x, y+b, 0, 0, Qt::DontClip, pt->s, -1, &r);
      b += r.height();
      if(a < r.width())  a = r.width();
    }
    xb = a + int(12.0*p->Scale);
    yb = b + int(10.0*p->Scale);
    x2 = x1+25 + int(float(a) / p->Scale);
    y2 = y1+23 + int(float(b) / p->Scale);
    if(ty < y2+1) if(ty > y1-r.height())  ty = y2 + 1;

    p->map(cx-1, cy, x, y);
    p->map(cx-6, cy-5, a, b);
    p->Painter->drawRect(a, b, xb, yb);
    p->Painter->drawLine(x,      y+yb, a,      b+yb);
    p->Painter->drawLine(x+xb-1, y+yb, x,      y+yb);
    p->Painter->drawLine(x+xb-1, y+yb, a+xb,   b+yb);
    p->Painter->drawLine(x+xb-1, y+yb, x+xb-1, y);
    p->Painter->drawLine(x+xb-1, y,    a+xb,   b);
  }
  else {    // normal components go here

    // paint all lines
    for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
      p->Painter->setPen(p1->style);
      p->drawLine(cx+p1->x1, cy+p1->y1, cx+p1->x2, cy+p1->y2);
    }

    // paint all arcs
    for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
      p->Painter->setPen(p3->style);
      p->drawArc(cx+p3->x, cy+p3->y, p3->w, p3->h, p3->angle, p3->arclen);
    }

    // paint all rectangles
    Area *pa;
    for(pa = Rects.first(); pa != 0; pa = Rects.next()) {
      p->Painter->setPen(pa->Pen);
      p->Painter->setBrush(pa->Brush);
      p->drawRect(cx+pa->x, cy+pa->y, pa->w, pa->h);
    }

    // paint all ellipses
    for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) {
      p->Painter->setPen(pa->Pen);
      p->Painter->setBrush(pa->Brush);
      p->drawEllipse(cx+pa->x, cy+pa->y, pa->w, pa->h);
    }
    p->Painter->setBrush(QBrush::NoBrush);

    newFont.setWeight(QFont::Light);
    QWMatrix wm = p->Painter->worldMatrix();
    // write all text
    for(pt = Texts.first(); pt != 0; pt = Texts.next()) {
      p->Painter->setWorldMatrix(
          QWMatrix(pt->mCos, -pt->mSin, pt->mSin, pt->mCos,
                   p->DX + float(cx+pt->x) * p->Scale,
                   p->DY + float(cy+pt->y) * p->Scale));
      newFont.setPointSizeFloat(p->Scale * pt->Size);
      p->Painter->setFont(newFont);
      p->Painter->setPen(pt->Color);
      p->Painter->drawText(0, 0, 0, 0, Qt::DontClip, pt->s);
    }
    p->Painter->setWorldMatrix(wm);
    p->Painter->setWorldXForm(false);
  }
  p->Painter->setFont(f);


  p->Painter->setPen(QPen(QPen::black,1));
  p->map(cx+tx, cy+ty, x, y);
  if(showName) {
    p->Painter->drawText(x, y, 0, 0, Qt::DontClip, Name);
    y += p->LineSpacing;
  }
  // write all properties
  for(Property *p4 = Props.first(); p4 != 0; p4 = Props.next())
    if(p4->display) {
      p->Painter->drawText(x, y, 0, 0, Qt::DontClip, p4->Name+"="+p4->Value);
      y += p->LineSpacing;
    }

  if(isActive == COMP_IS_OPEN)
    p->Painter->setPen(QPen(QPen::red,0));
  else if(isActive & COMP_IS_SHORTEN)
    p->Painter->setPen(QPen(QPen::darkGreen,0));
  if(isActive != COMP_IS_ACTIVE) {
    p->drawRect(cx+x1, cy+y1, x2-x1+1, y2-y1+1);
    p->drawLine(cx+x1, cy+y1, cx+x2, cy+y2);
    p->drawLine(cx+x1, cy+y2, cx+x2, cy+y1);
  }

  if(isSelected) {
    p->Painter->setPen(QPen(QPen::darkGray,3));
    p->drawRoundRect(cx+x1, cy+y1, x2-x1, y2-y1);
  }
}

// -------------------------------------------------------
// Paints the component when moved with the mouse.
void Component::paintScheme(QPainter *p)
{
  if(Model.at(0) == '.') {   // is simulation component (dc, ac, ...)
    Text *pt;
    int a, b, xb, yb;
    QFont newFont = p->font();

    float Scale =
          ((Schematic*)QucsMain->DocumentTab->currentPage())->Scale;
    newFont.setPointSizeFloat(float(Scale) * QucsSettings.largeFontSize);
    newFont.setWeight(QFont::DemiBold);
    QFontMetrics  metrics(newFont);

    a = b = 0;
    QSize r;
    for(pt = Texts.first(); pt != 0; pt = Texts.next()) {
      r = metrics.size(0, pt->s);
      b += r.height();
      if(a < r.width())  a = r.width();
    }
    xb = a + int(12.0*Scale);
    yb = b + int(10.0*Scale);
    x2 = x1+25 + int(float(a) / Scale);
    y2 = y1+23 + int(float(b) / Scale);
    if(ty < y2+1) if(ty > y1-r.height())  ty = y2 + 1;

    p->drawRect(cx-6, cy-5, xb, yb);
    p->drawLine(cx-1, cy+yb, cx-6, cy+yb-5);
    p->drawLine(cx+xb-2, cy+yb, cx-1, cy+yb);
    p->drawLine(cx+xb-2, cy+yb, cx+xb-6, cy+yb-5);
    p->drawLine(cx+xb-2, cy+yb, cx+xb-2, cy);
    p->drawLine(cx+xb-2, cy, cx+xb-6, cy-5);
    return;
  }

  // paint all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next())
    p->drawLine(cx+p1->x1, cy+p1->y1, cx+p1->x2, cy+p1->y2);

  // paint all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next())
    p->drawEllipse(cx+p2->x-4, cy+p2->y-4, 8, 8);

  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next())   // paint all arcs
    p->drawArc(cx+p3->x, cy+p3->y, p3->w, p3->h, p3->angle, p3->arclen);

  Area *pa;
  for(pa = Rects.first(); pa != 0; pa = Rects.next()) // paint all rectangles
    p->drawRect(cx+pa->x, cy+pa->y, pa->w, pa->h);

  for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) // paint all ellipses
    p->drawEllipse(cx+pa->x, cy+pa->y, pa->w, pa->h);
}

// -------------------------------------------------------
// For output on a printer device.
void Component::print(ViewPainter *p, float FontScale)
{
  Text *pt;
  for(pt = Texts.first(); pt != 0; pt = Texts.next())
    pt->Size *= FontScale;

  paint(p);

  for(pt = Texts.first(); pt != 0; pt = Texts.next())
    pt->Size /= FontScale;
}

// -------------------------------------------------------
// Rotates the component 90� counter-clockwise around its center
void Component::rotate()
{
  if(Ports.count() < 1) return;  // do not rotate components without ports
  int tmp, dx, dy;

  // rotate all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
    tmp = -p1->x1;
    p1->x1 = p1->y1;
    p1->y1 = tmp;
    tmp = -p1->x2;
    p1->x2 = p1->y2;
    p1->y2 = tmp;
  }

  // rotate all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next()) {
    tmp = -p2->x;
    p2->x = p2->y;
    p2->y = tmp;
  }

  // rotate all arcs
  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
    tmp = -p3->x;
    p3->x = p3->y;
    p3->y = tmp - p3->w;
    tmp = p3->w;
    p3->w = p3->h;
    p3->h = tmp;
    p3->angle += 16*90;
    if(p3->angle >= 16*360) p3->angle -= 16*360;;
  }

  Area *pa;
  // rotate all rectangles
  for(pa = Rects.first(); pa != 0; pa = Rects.next()) {
    tmp = -pa->x;
    pa->x = pa->y;
    pa->y = tmp - pa->w;
    tmp = pa->w;
    pa->w = pa->h;
    pa->h = tmp;
  }

  // rotate all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) {
    tmp = -pa->x;
    pa->x = pa->y;
    pa->y = tmp - pa->w;
    tmp = pa->w;
    pa->w = pa->h;
    pa->h = tmp;
  }

  // rotate all text
  float ftmp;
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    tmp = -pt->x;
    pt->x = pt->y;
    pt->y = tmp;

    ftmp = -pt->mSin;
    pt->mSin = pt->mCos;
    pt->mCos = ftmp;
  }

  tmp = -x1;   // rotate boundings
  x1  = y1; y1 = -x2;
  x2  = y2; y2 = tmp;

  tmp = -tx;    // rotate text position
  tx  = ty;
  ty  = tmp;
  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  dx = dy = 0;
  if(showName) {
    dx = metrics.width(Name);
    dy = metrics.lineSpacing();
  }
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      tmp = metrics.width(pp->Name+"="+pp->Value);
      if(tmp > dx) dx = tmp;
      dy += metrics.lineSpacing();
    }
  if(tx > x2) ty = y1-ty+y2;    // rotate text position
  else if(ty < y1) ty -= dy;
  else if(tx < x1) { tx += dy-dx;  ty = y1-ty+y2; }
  else ty -= dx;

  rotated++;  // keep track of what's done
  rotated &= 3;
}

// -------------------------------------------------------
// Mirrors the component about the x-axis.
void Component::mirrorX()
{
  if(Ports.count() < 1) return;  // do not rotate components without ports

  // mirror all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
    p1->y1 = -p1->y1;
    p1->y2 = -p1->y2;
  }

  // mirror all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next())
    p2->y = -p2->y;

  // mirror all arcs
  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
    p3->y = -p3->y - p3->h;
    if(p3->angle > 16*180) p3->angle -= 16*360;
    p3->angle  = -p3->angle;    // mirror
    p3->angle -= p3->arclen;    // go back to end of arc
    if(p3->angle < 0) p3->angle += 16*360;  // angle has to be > 0
  }

  Area *pa;
  // mirror all rectangles
  for(pa = Rects.first(); pa != 0; pa = Rects.next())
    pa->y = -pa->y - pa->h;

  // mirror all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next())
    pa->y = -pa->y - pa->h;

  QFont f = QucsSettings.font;
  // mirror all text
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    f.setPointSizeFloat(pt->Size);
    QFontMetrics  smallMetrics(f);
    QSize s = smallMetrics.size(0, pt->s);   // use size for more lines
    pt->y = -pt->y - int(pt->mCos)*s.height() + int(pt->mSin)*s.width();
  }

  int tmp = y1;
  y1  = -y2; y2 = -tmp;   // mirror boundings

  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  int dy = 0;
  if(showName)
    dy = metrics.lineSpacing();   // for "Name"
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display)  dy += metrics.lineSpacing();
  if((tx > x1) && (tx < x2)) ty = -ty-dy;     // mirror text position
  else ty = y1+ty+y2;

  mirroredX = !mirroredX;    // keep track of what's done
  rotated += rotated << 1;
  rotated &= 3;
}

// -------------------------------------------------------
// Mirrors the component about the y-axis.
void Component::mirrorY()
{
  if(Ports.count() < 1) return;  // do not rotate components without ports

  // mirror all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
    p1->x1 = -p1->x1;
    p1->x2 = -p1->x2;
  }

  // mirror all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next())
    p2->x = -p2->x;

  // mirror all arcs
  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
    p3->x = -p3->x - p3->w;
    p3->angle = 16*180 - p3->angle - p3->arclen;  // mirror
    if(p3->angle < 0) p3->angle += 16*360;   // angle has to be > 0
  }

  Area *pa;
  // mirror all rectangles
  for(pa = Rects.first(); pa != 0; pa = Rects.next())
    pa->x = -pa->x - pa->w;

  // mirror all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next())
    pa->x = -pa->x - pa->w;

  int tmp;
  QFont f = QucsSettings.font;
  // mirror all text
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    f.setPointSizeFloat(pt->Size);
    QFontMetrics  smallMetrics(f);
    QSize s = smallMetrics.size(0, pt->s);   // use size for more lines
    pt->x = -pt->x - int(pt->mSin)*s.height() - int(pt->mCos)*s.width();
  }

  tmp = x1;
  x1  = -x2; x2 = -tmp;   // mirror boundings

  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  int dx = 0;
  if(showName)
    dx = metrics.width(Name);
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      tmp = metrics.width(pp->Name+"="+pp->Value);
      if(tmp > dx)  dx = tmp;
    }
  if((ty > y1) && (ty < y2)) tx = -tx-dx;     // mirror text position
  else tx = x1+tx+x2;

  mirroredX = !mirroredX;   // keep track of what's done
  rotated += rotated << 1;
  rotated += 2;
  rotated &= 3;
}

// -------------------------------------------------------
QString Component::NetList()
{
  QString s = Model+":"+Name;

  // output all node names
  for(Port *p1 = Ports.first(); p1 != 0; p1 = Ports.next())
    s += " "+p1->Connection->Name;   // node names
  if(Model.at(0) == '_') {    // add port ? (e.g. BJT without substrate)
    if(Model == "_BJT")
      s += " "+Ports.at(1)->Connection->Name;
    else
      s += " "+Ports.at(2)->Connection->Name;
    s.remove(0,1);  // remove leading '_'
  }
  else if(Model == "MVIA")
    s += " gnd";   // add ground node

  // output all properties
  for(Property *p2 = Props.first(); p2 != 0; p2 = Props.next())
    if(p2->Name != "Symbol")
      s += " "+p2->Name+"=\""+p2->Value+"\"";

  return s;
}

// -------------------------------------------------------
QString Component::getShortenNetlist()
{
  int z=0;
  QString s;
  QString Node1 = Ports.first()->Connection->Name;
  for(Port *pp = Ports.next(); pp != 0; pp = Ports.next())
    s += "R:" + Name + "." + QString::number(z++) + " " +
         Node1 + " " + pp->Connection->Name + " R=\"0\"\n";
  return s;
}

// -------------------------------------------------------
QString Component::VHDL_Code(int)
{
  return QString("");   // no digital model
}

// -------------------------------------------------------
QString Component::getShortenVHDL()
{
  QString s;
  QString Node1 = Ports.first()->Connection->Name;
  for(Port *pp = Ports.next(); pp != 0; pp = Ports.next())
    s += "  " + pp->Connection->Name + " <= " + Node1 + ";\n";
  return s;
}

// -------------------------------------------------------
QString Component::save()
{
  QString s = "<" + Model;

  if(Name.isEmpty()) s += " * ";
  else s += " "+Name+" ";

  int i=0;
  if(!showName)
    i = 4;
  i |= isActive;
  s += QString::number(i);
  s += " "+QString::number(cx)+" "+QString::number(cy);
  s += " "+QString::number(tx)+" "+QString::number(ty);
  if(mirroredX) s += " 1";
  else s += " 0";
  s += " "+QString::number(rotated);

  // write all properties
  for(Property *p1 = Props.first(); p1 != 0; p1 = Props.next()) {
    if(p1->Description.isEmpty())
      s += " \""+p1->Name+"="+p1->Value+"\"";   // e.g. for equations
    else s += " \""+p1->Value+"\"";
    if(p1->display) s += " 1";
    else s += " 0";
  }

  return s+">";
}

// -------------------------------------------------------
bool Component::load(const QString& _s)
{
  bool ok;
  int  ttx, tty, tmp;
  QString s = _s;

  if(s.at(0) != '<') return false;
  if(s.at(s.length()-1) != '>') return false;
  s = s.mid(1, s.length()-2);   // cut off start and end character

  QString n;
  Name = s.section(' ',1,1);    // Name
  if(Name == "*") Name = "";

  n  = s.section(' ',2,2);      // isActive
  tmp = n.toInt(&ok);
  if(!ok) return false;
  isActive = tmp & 3;

  if(tmp & 4)
    showName = false;
  else
    showName = true;

  n  = s.section(' ',3,3);    // cx
  cx = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',4,4);    // cy
  cy = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',5,5);    // tx
  ttx = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',6,6);    // ty
  tty = n.toInt(&ok);
  if(!ok) return false;

  if(Model.at(0) != '.') {  // is simulation component (dc, ac, ...) ?

    n  = s.section(' ',7,7);    // mirroredX
    if(n.toInt(&ok) == 1) mirrorX();
    if(!ok) return false;

    n  = s.section(' ',8,8);    // rotated
    tmp = n.toInt(&ok);
    if(!ok) return false;
    if(rotated > tmp)  // neccessary because of historical flaw in ...
      tmp += 4;        // ... components like "volt_dc"
    for(int z=rotated; z<tmp; z++) rotate();

  }

  tx = ttx; ty = tty; // restore text position (was changed by rotate/mirror)

  unsigned int z=0, counts = s.contains('"');
  // load all properties
  for(Property *p1 = Props.first(); p1 != 0; p1 = Props.next()) {
    z++;
    n = s.section('"',z,z);    // property value
    z++;
    // not all properties have to be mentioned (backward compatible)
    if(z > counts) {
      if(p1->Description.isEmpty())
        Props.remove();    // remove if allocated in vain
      return true;
    }

    if(p1->Description.isEmpty()) {  // unknown number of properties ?
      p1->Name = n.section('=',0,0);
      n = n.section('=',1,1);
      // allocate memory for a new property (e.g. for equations)
      if(Props.count() < (counts>>1)) {
	Props.insert(z >> 1, new Property("y", "1", true));
	Props.prev();
      }
    }
    if(z == 6)  if(counts == 6)     // backward compatible
      if(Model == "R") {
        Props.getLast()->Value = n;
        return true;
      }
    p1->Value = n;

    n  = s.section('"',z,z);    // display
    if(n.toInt(&ok) == 1) p1->display = true;
    else p1->display = false;
    if(!ok) return false;
  }

  return true;
}

// *******************************************************************
// ***  The following functions are used to load the schematic symbol
// ***  from file. (e.g. subcircuit, library component)

int Component::analyseLine(const QString& Row)
{
  QPen Pen;
  QBrush Brush;
  QColor Color;
  QString s;
  int i1, i2, i3, i4, i5, i6;

  s = Row.section(' ',0,0);    // component type
  if((s == "PortSym") || (s == ".PortSym")) {  // backward compatible
    if(!getIntegers(Row, &i1, &i2, &i3))
      return -1;
    for(i6 = Ports.count(); i6<i3; i6++)  // if ports not in numerical order
      Ports.append(new Port(0, 0));

    Ports.at(i3-1)->x  = i1;
    Ports.current()->y = i2;

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i1 > x2)  x2 = i1;
    if(i2 < y1)  y1 = i2;
    if(i2 > y2)  y2 = i2;
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
    if(!getIntegers(Row, &i1, &i2, &i3, &i4, &i5, &i6))
      return -1;
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
    tx = i1;
    ty = i2;
    Name = Row.section(' ',3,3);
    if(Name.isEmpty())  Name = "SUB";
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
    if(!getIntegers(Row, &i1, &i2, &i3, 0, &i4))  return -1;
    Color.setNamedColor(Row.section(' ',4,4));
    if(!Color.isValid()) return -1;

    s = Row.mid(Row.find('"')+1);    // Text (can contain " !!!)
    s = s.left(s.length()-1);
    if(s.isEmpty()) return -1;
    convert2Unicode(s);

    Texts.append(new Text(i1, i2, s, Color, float(i3),
                          float(cos(float(i4)*M_PI/180.0)),
                          float(sin(float(i4)*M_PI/180.0))));

    QFont Font(QucsSettings.font);
    Font.setPointSizeFloat(float(i3));
    QFontMetrics  metrics(Font);
    QSize r = metrics.size(0, s);    // get size of text
    i3 = i1 + int(float(r.width())  * Texts.current()->mCos)
            + int(float(r.height()) * Texts.current()->mSin);
    i4 = i2 + int(float(r.width())  * -Texts.current()->mSin)
            + int(float(r.height()) * Texts.current()->mCos);

    if(i1 < x1)  x1 = i1;  // keep track of component boundings
    if(i2 < y1)  y1 = i2;
    if(i1 > x2)  x2 = i1;
    if(i2 > y2)  y2 = i2;
    
    if(i3 < x1)  x1 = i3;
    if(i4 < y1)  y1 = i4;
    if(i3 > x2)  x2 = i3;
    if(i4 > y2)  y2 = i4;
    return 1;
  }

  return 0;
}

// ---------------------------------------------------------------------
bool Component::getIntegers(const QString& s, int *i1, int *i2, int *i3,
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

  if(i4) {
    n  = s.section(' ',4,4);
    *i4 = n.toInt(&ok);
    if(!ok) return false;
  }

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
bool Component::getPen(const QString& s, QPen& Pen, int i)
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
bool Component::getBrush(const QString& s, QBrush& Brush, int i)
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


// ***********************************************************************
// ********                                                       ********
// ********          Functions of class MultiViewComponent        ********
// ********                                                       ********
// ***********************************************************************
void MultiViewComponent::recreate(Schematic *Doc)
{
  if(Doc) {
    Doc->Components->setAutoDelete(false);
    Doc->deleteComp(this);
  }

  Ellips.clear();
  Texts.clear();
  Ports.clear();
  Lines.clear();
  Rects.clear();
  Arcs.clear();
  createSymbol();
  
  bool mmir = mirroredX;
  int  rrot = rotated;
  if(mmir)  mirrorX();   // mirror
  for(int z=0; z<rrot; z++)  rotate(); // rotate

  rotated = rrot;   // restore properties (were changed by rotate/mirror)
  mirroredX = mmir;

  if(Doc) {
    Doc->insertRawComponent(this);
    Doc->Components->setAutoDelete(true);
  }
}


// ***********************************************************************
// ********                                                       ********
// ********            Functions of class GateComponent           ********
// ********                                                       ********
// ***********************************************************************
GateComponent::GateComponent()
{
  Type = isComponent;   // both analog and digital
  Name  = "Y";

  // the list order must be preserved !!!
  Props.append(new Property("in", "2", false,
		QObject::tr("number of input ports")));
  Props.append(new Property("V", "1 V", false,
		QObject::tr("voltage of high level")));
  Props.append(new Property("t", "0", false,
		QObject::tr("delay time")));

  // this must be the last property in the list !!!
  Props.append(new Property("Symbol", "old", false,
		QObject::tr("schematic symbol")+" [old, DIN40900]"));
}

// -------------------------------------------------------
QString GateComponent::NetList()
{
  QString s = Model+":"+Name;

  // output all node names
  for(Port *pp = Ports.first(); pp != 0; pp = Ports.next())
    s += " "+pp->Connection->Name;   // node names

  // output all properties
  Property *p = Props.at(1);
  s += " " + p->Name + "=\"" + p->Value + "\"";
  p = Props.next();
  s += " " + p->Name + "=\"" + p->Value + "\"";
  return s;
}

// -------------------------------------------------------
QString GateComponent::VHDL_Code(int NumPorts)
{
  Port *pp = Ports.first();
  QString s = "  " + pp->Connection->Name + " <= ";  // output port

  QString Op = ' ' + Model.lower() + ' ';
  if(Model.at(0) == 'N') {
    s += "not (";    // nor, nand is NOT assoziative !!! but xnor is !!!
    Op = Op.remove(1, 1);
  }

  pp = Ports.next();
  s += pp->Connection->Name;   // first input port

  // output all input ports with node names
  for(pp = Ports.next(); pp != 0; pp = Ports.next())
    s += Op + pp->Connection->Name;
  if(Model.at(0) == 'N')
    s += ')';

  if(NumPorts <= 0)  // no truth table simulation ?
    if(strtod(Props.at(2)->Value.latin1(), 0) != 0.0) {  // delay time
      QString t = Props.current()->Value;
      if(!VHDL_Time(t, Name))
        return t;    // time has not VHDL format

      s += " after " + t;
    }

  s += ';';
  return s;
}

// -------------------------------------------------------
void GateComponent::createSymbol()
{
  int Num = Props.getFirst()->Value.toInt();
  if(Num < 2) Num = 2;
  else if(Num > 8) Num = 8;
  Props.getFirst()->Value = QString::number(Num);

  int xl, xr, y = 10*Num, z;
  x1 = -30; y1 = -y-3;
  x2 =  30; y2 =  y+3;

  tx = x1+4;
  ty = y2+4;

  z = 0;
  if(Model.at(0) == 'N')  z = 1;

  if(Props.getLast()->Value.at(0) == 'D') {  // DIN symbol
    xl = -15;
    xr =  15;
    Lines.append(new Line( 15,-y, 15, y,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-15,-y, 15,-y,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-15, y, 15, y,QPen(QPen::darkBlue,2)));
    Lines.append(new Line(-15,-y,-15, y,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 15, 0, 30, 0,QPen(QPen::darkBlue,2)));

    if(Model.at(z) == 'O') {
      Lines.append(new Line(-11, 6-y,-6, 9-y,QPen(QPen::darkBlue,0)));
      Lines.append(new Line(-11,12-y,-6, 9-y,QPen(QPen::darkBlue,0)));
      Lines.append(new Line(-11,14-y,-6,14-y,QPen(QPen::darkBlue,0)));
      Lines.append(new Line(-11,16-y,-6,16-y,QPen(QPen::darkBlue,0)));
      Texts.append(new Text( -4, 3-y, "1", QPen::darkBlue, 15.0));
    }
    else if(Model.at(z) == 'A')
      Texts.append(new Text( -10, 3-y, "&", QPen::darkBlue, 15.0));
    else if(Model.at(0) == 'X') {
      if(Model.at(1) == 'N')
        Texts.append(new Text( -11, 3-y, "=", QPen::darkBlue, 15.0));
      else
        Texts.append(new Text( -12, 3-y, "=1", QPen::darkBlue, 15.0));
    }
  }
  else {   // old symbol

    if(Model.at(z) == 'O')  xl = 10;
    else  xl = -10;
    xr = 10;
    Lines.append(new Line(-10,-y,-10, y,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 10, 0, 30, 0,QPen(QPen::darkBlue,2)));
    Arcs.append(new Arc(-30,-y, 40, 30, 0, 16*90,QPen(QPen::darkBlue,2)));
    Arcs.append(new Arc(-30,y-30, 40, 30, 0,-16*90,QPen(QPen::darkBlue,2)));
    Lines.append(new Line( 10,15-y, 10, y-15,QPen(QPen::darkBlue,2)));

    if(Model.at(0) == 'X') {
      Lines.append(new Line(-5, 0, 5, 0,QPen(QPen::darkBlue,1)));
      if(Model.at(1) == 'N') {
        Lines.append(new Line(-5,-3, 5,-3,QPen(QPen::darkBlue,1)));
        Lines.append(new Line(-5, 3, 5, 3,QPen(QPen::darkBlue,1)));
      }
      else {
        Arcs.append(new Arc(-5,-5, 10, 10, 0, 16*360,QPen(QPen::darkBlue,1)));
        Lines.append(new Line( 0,-5, 0, 5,QPen(QPen::darkBlue,1)));
      }
    }
  }

  if(Model.at(0) == 'N')
    Ellips.append(new Area(xr,-4, 8, 8,
                  QPen(QPen::darkBlue,0), QBrush(QPen::darkBlue)));

  Ports.append(new Port( 30,  0));
  y += 10;
  for(z=0; z<Num; z++) {
    y -= 20;
    Ports.append(new Port(-30, y));
    if(xl == 10) if((z == 0) || (z == Num-1)) {
      Lines.append(new Line(-30, y, 9, y,QPen(QPen::darkBlue,2)));
      continue;
    }
    Lines.append(new Line(-30, y, xl, y,QPen(QPen::darkBlue,2)));
  }
}


// ***********************************************************************
// ********                                                       ********
// ******** The following function does not below to any class.   ********
// ******** It creates a component by getting the identification  ********
// ******** string used in the schematic file and for copy/paste. ********
// ********                                                       ********
// ***********************************************************************

Component* getComponentFromName(QString& Line)
{
  Component *c = 0;

  Line = Line.stripWhiteSpace();
  if(Line.at(0) != '<') {
    QMessageBox::critical(0, QObject::tr("Error"),
			QObject::tr("Format Error:\nWrong line start!"));
    return 0;
  }

  QString cstr = Line.section(' ',0,0); // component type
  char first = Line.at(1).latin1();     // first letter of component name
  cstr.remove(0,2);    // remove leading "<" and first letter

  // to speed up the string comparision, they are ordered by the first
  // letter of their name
  switch(first) {
  case 'R' : if(cstr.isEmpty()) c = new Resistor();
	else if(cstr == "us") c = new Resistor(false);  // backward capatible
	else if(cstr == "SFF") c = new RS_FlipFlop();
	else if(cstr == "elais") c = new Relais();
	break;
  case 'C' : if(cstr.isEmpty()) c = new Capacitor();
	else if(cstr == "CCS") c = new CCCS();
	else if(cstr == "CVS") c = new CCVS();
	else if(cstr == "irculator") c = new Circulator();
	else if(cstr == "oupler") c = new Coupler();
	else if(cstr == "LIN") c = new Coplanar();
	else if(cstr == "OPEN") c = new CPWopen();
	else if(cstr == "SHORT") c = new CPWshort();
	else if(cstr == "GAP") c = new CPWgap();
	else if(cstr == "STEP") c = new CPWstep();
	else if(cstr == "OAX") c = new CoaxialLine();
	break;
  case 'L' : if(cstr.isEmpty()) c = new Inductor();
	else if(cstr == "ib") c = new LibComp();
	break;
  case 'G' : if(cstr == "ND") c = new Ground();
        else if(cstr == "yrator") c = new Gyrator();
        break;
  case 'I' : if(cstr == "Probe") c = new iProbe();
        else if(cstr == "dc") c = new Ampere_dc();
        else if(cstr == "ac") c = new Ampere_ac();
        else if(cstr == "noise") c = new Ampere_noise();
        else if(cstr == "solator") c = new Isolator();
        else if(cstr == "pulse") c = new iPulse();
        else if(cstr == "rect") c = new iRect();
        else if(cstr == "Inoise") c = new Noise_ii();
        else if(cstr == "Vnoise") c = new Noise_iv();
        else if(cstr == "nv") c = new Logical_Inv();
        break;
  case 'J' : if(cstr == "FET") c = new JFET();
	else if(cstr == "KFF") c = new JK_FlipFlop();
        break;
  case 'V' : if(cstr == "dc") c = new Volt_dc();
        else if(cstr == "ac") c = new Volt_ac();
        else if(cstr == "CCS") c = new VCCS();
        else if(cstr == "CVS") c = new VCVS();
        else if(cstr == "Probe") c = new vProbe();
        else if(cstr == "noise") c = new Volt_noise();
        else if(cstr == "pulse") c = new vPulse();
        else if(cstr == "rect") c = new vRect();
        else if(cstr == "Vnoise") c = new Noise_vv();
        else if(cstr == "HDL") c = new VHDL_File();
        break;
  case 'T' : if(cstr == "r") c = new Transformer();
        else if(cstr == "LIN") c = new TLine();
        break;
  case 's' : if(cstr == "Tr") c = new symTrafo();
        break;
  case 'P' : if(cstr == "ac") c = new Source_ac();
        else if(cstr == "ort") c = new SubCirPort();
        else if(cstr == "Shift") c = new Phaseshifter();
        else if(cstr == "M_Mod") c = new PM_Modulator();
        break;
  case 'S' : if(cstr == "Pfile") c = new SParamFile();
        else if(cstr.left(5) == "Pfile") {  // backward compatible
          c = new SParamFile();
          c->Props.getLast()->Value = cstr.mid(5); }
        else if(cstr == "ub")   c = new Subcircuit();
        else if(cstr == "UBST") c = new Substrate();
        else if(cstr == "PICE") c = new SpiceFile();
        else if(cstr == "witch") c = new Switch();
        break;
  case 'D' : if(cstr == "CBlock") c = new dcBlock();
	else if(cstr == "CFeed") c = new dcFeed();
	else if(cstr == "iode") c = new Diode();
	else if(cstr == "igiSource") c = new Digi_Source();
	else if(cstr == "FF") c = new D_FlipFlop();
	break;
  case 'B' : if(cstr == "iasT") c = new BiasT();
        else if(cstr == "JT") c = new BJTsub();
        break;
  case 'A' : if(cstr == "ttenuator") c = new Attenuator();
        else if(cstr == "mp") c = new Amplifier();
        else if(cstr == "ND") c = new Logical_AND();
        else if(cstr == "M_Mod") c = new AM_Modulator();
        break;
  case 'M' : if(cstr == "UT") c = new Mutual();
	else if(cstr == "UT2") c = new Mutual2();
	else if(cstr == "LIN") c = new MSline();
	else if(cstr == "OSFET") c = new MOSFET_sub();
	else if(cstr == "STEP") c = new MSstep();
	else if(cstr == "CORN") c = new MScorner();
	else if(cstr == "TEE") c = new MStee();
	else if(cstr == "CROSS") c = new MScross();
	else if(cstr == "MBEND") c = new MSmbend();
	else if(cstr == "OPEN") c = new MSopen();
	else if(cstr == "GAP") c = new MSgap();
	else if(cstr == "COUPLED") c = new MScoupled();
	else if(cstr == "VIA") c = new MSvia();
	break;
  case 'E' : if(cstr == "qn") c = new Equation();
        break;
  case 'O' : if(cstr == "pAmp") c = new OpAmp();
        else if(cstr == "R") c = new Logical_OR();
        break;
  case 'N' : if(cstr == "OR") c = new Logical_NOR();
        else if(cstr == "AND") c = new Logical_NAND();
        break;
  case '.' : if(cstr == "DC") c = new DC_Sim();
        else if(cstr == "AC") c = new AC_Sim();
        else if(cstr == "TR") c = new TR_Sim();
        else if(cstr == "SP") c = new SP_Sim();
        else if(cstr == "HB") c = new HB_Sim();
        else if(cstr == "SW") c = new Param_Sweep();
        else if(cstr == "Digi") c = new Digi_Sim();
        break;
  case '_' : if(cstr == "BJT") c = new BJT();
	else if(cstr == "MOSFET") c = new MOSFET();
        break;
  case 'X' : if(cstr == "OR") c = new Logical_XOR();
        else if(cstr == "NOR") c = new Logical_XNOR();
        break;
  }
  if(!c) {
    QMessageBox::critical(0, QObject::tr("Error"),
	QObject::tr("Format Error:\nUnknown component!"));
    return 0;
  }

  if(!c->load(Line)) {
    QMessageBox::critical(0, QObject::tr("Error"),
	QObject::tr("Format Error:\nWrong 'component' line format!"));
    delete c;
    return 0;
  }

  cstr = c->Name;   // is perhaps changed in "recreate" (e.g. subcircuit)
  int x = c->tx, y = c->ty;
  c->recreate(0);
  c->Name = cstr;
  c->tx = x;  c->ty = y;
  return c;
}
