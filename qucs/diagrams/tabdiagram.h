/***************************************************************************
                          tabdiagram.h  -  description
                             -------------------
    begin                : Fri Oct 24 2003
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

#ifndef TABDIAGRAM_H
#define TABDIAGRAM_H

#include "diagram.h"

/**
  *@author Michael Margraf
  */

class TabDiagram : public Diagram  {
public: 
  TabDiagram(int _cx=0, int _cy=0);
  ~TabDiagram();

  Diagram* newOne();
  int  calcDiagram();
  bool scroll(int);

private:
  int  checkColumnWidth(const QString&, const QFontMetrics&, int, int, int);
};

#endif
