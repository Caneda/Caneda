/***************************************************************************
                       Caneda Attenuator Synthesis
                             attenuatorfunc.h
                               ------------
    begin                : Jun 14 2006

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CANEDA_ATT_H
#define CANEDA_ATT_H

#define PI_TYPE 0
#define TEE_TYPE 1
#define BRIDGE_TYPE 2

#include <math.h>

struct tagATT
{
  int Topology;
  double Zin;
  double Zout;
  double Attenuation;
  double MinimumATT;
  double R1;
  double R2;
  double R3;
};

class QString;

class CANEDA_Att
{
 public:
  CANEDA_Att();
  ~CANEDA_Att();

  int Calc(tagATT*);
  static QString* createSchematic(tagATT*);
};

#endif
