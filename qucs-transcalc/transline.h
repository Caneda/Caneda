/*
 * transline.h - base for a transmission line class definition
 *
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
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.  
 *
 */

#ifndef __TRANSLINE_H
#define __TRANSLINE_H
#include <QtCore/QString>
class TransWidgets;
class QString;

class transline {
 public:
  transline ();
  virtual ~transline ();

  void   setTransWidgets (TransWidgets *tw);
  void   setProperty (const QString& name, double val);
  void   setProperty (const QString& name, double val, int unit);
  double getProperty (const QString& name);
  double getProperty (const QString&, int unit);
  double convertProperty (const QString&, double, int, int);
  void   setResult (const QString&, double, const QString&);
  void   setResult (const QString&,const QString&);
  bool   isSelected (const QString&);

  virtual void synthesize () { };
  virtual void analyze () { };

  QString description;
  
 protected:
  TransWidgets * transWidgets;
};

#endif /* __TRANSLINE_H */
