/*
 * microstrip.h - coaxial class definition
 *
 * Copyright (C) 2001 Gopal Narayanan <gopal@astro.umass.edu>
 * Copyright (C) 2005 Stefan Jahn <stefan@lkcc.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 */

#ifndef __COAX_H
#define __COAX_H

class coax : public transline {
 public:
  coax();
  ~coax();

 private:
  double er;               /* dielectric constant */
  double mur;              /* rel. permeability */
  double tand;             /* Dielectric Loss Tangent */
  double sigma;            /* Conductivity of the metal */
  double f;                /* Frequency of operation */
  double din;              /* Inner diameter of cable */
  double dout;             /* Outer diameter of cable */
  double l;                /* Length of cable */
  double Z0;               /* characteristic impedance */
  double ang_l;            /* Electrical length in angle */
  double er_eff;           /* Effective dielectric constant */
  double atten_dielectric; /* Loss in dielectric (dB) */
  double atten_cond;       /* Loss in conductors (dB) */
  double fc;               /* Cutoff frequency for higher order modes */

 public:
  void analyze ();
  void synthesize ();

 private:
  void get_coax_sub();
  void get_coax_comp();
  void get_coax_phys();
  void get_coax_elec();
  void fixdin();
  void fixdout();
  double alphad_coax();
  double alphac_coax();
  void show_results();
};

#endif /* __COAX_H */
