/***************************************************************************
                          arrow.h  -  description
                             -------------------
    begin                : Sun Nov 23 2003
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

#ifndef ARROW_H
#define ARROW_H

#include "painting.h"

#include <qpen.h>


/**
  *@author Michael Margraf
  */

class Arrow : public Painting  {
public: 
  Arrow();
  ~Arrow();

  void paintScheme(QPainter *p);
  void getCenter(int& x, int &y);
  void setCenter(int x, int y, bool relative=false);

  Arrow* newOne();
  bool load(const QString& s);
  QString save();
  void paint(QPainter *p);
  void MouseMoving(int x, int y, int gx, int gy, QPainter *p, bool drawn);
  bool MousePressing();
  bool getSelected(int x, int y);
  void Bounding(int&, int&, int&, int&);

  void rotate();
  void mirrorX();
  void mirrorY();
  bool Dialog();

  void calcArrowHead();
  
  QPen   Pen;
  double Height, Width;  // size of the arrow head
  double Length, beta;
  int    xp1, yp1, xp2, yp2;   // coordinates to paint the arrow head
};

#endif
