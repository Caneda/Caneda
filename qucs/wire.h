/***************************************************************************
                          wire.h  -  description
                             -------------------
    begin                : Wed Sep 3 2003
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

#ifndef WIRE_H
#define WIRE_H

#include "element.h"
#include "components/component.h"    // because of struct Port

#include <qpainter.h>
#include <qstring.h>
#include <qptrlist.h>

/**
  *@author Michael Margraf
  */


class Wire : public Element {
public: 
  Wire(int _x1=0, int _y1=0, int _x2=0, int _y2=0, Node *n1=0, Node *n2=0, const QString& _Name=0);
  virtual ~Wire();

  virtual void paintScheme(QPainter *p);
  virtual void setCenter(int x, int y, bool relative=false);
  void    setName(const QString& Name_);

  Node    *Port1, *Port2;
  QString Name;
  int     NameDX, NameDY;  // size of Name string on screen
  int     nx, ny, delta;   // position of the nodename label

  void    paint(QPainter *p);
  void    rotate();
  QString save();
  bool    load(const QString& s);
  bool    isHorizontal();
};

#endif
