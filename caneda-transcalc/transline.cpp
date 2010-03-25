/*
 * transline.cpp - base for a transmission line implementation
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

#include "transline.h"
#include "qucstrans.h"
#include "qucs-tools/propertygrid.h"
/* Constructor creates a transmission line instance. */
transline::transline () {
  transWidgets = 0;
}

/* Destructor destroys a transmission line instance. */
transline::~transline () {
}

/* Sets the application instance. */
void transline::setTransWidgets(TransWidgets *tw)
{
  transWidgets = tw;
}

/* Sets a named property to the given value, access through the
   application. */
void transline::setProperty (const QString& prop, double value)
{
  Q_ASSERT(transWidgets->boxWithProperty(prop) != 0l);
  transWidgets->boxWithProperty(prop)->setDoubleValue (prop, value);
}

/* Sets a named property to a given value.  Depending on the source
   and destination unit the value gets previously converted. */
void transline::setProperty (const QString& prop, double value,int unit) {
  

  PropertyBox *box = transWidgets->boxWithProperty(prop);
  Q_ASSERT(box != 0l);
  int dstunit = box->unit(prop);
  Q_ASSERT(dstunit != Units::None);
  box->setDoubleValue(prop,value,unit);
  box->convertTo(prop,dstunit);
}

/* Converts the given value/unit pair into a text representation and
   puts this into the given result line. */
void transline::setResult (const QString& name,double value, const QString& unit)
{
  transWidgets->result->setValue (name, value, unit);
}

/* Puts the text into the given result line. */
void transline::setResult (const QString& name, const QString& text)
{
  transWidgets->result->setValue (name, text);
}

/* Returns a named property value. */
double transline::getProperty (const QString& prop)
{
  PropertyBox *box = transWidgets->boxWithProperty(prop);
  Q_ASSERT(box != 0l);
  return box->doubleValue (prop);
}

/* Returns a named property selection. */
bool transline::isSelected (const QString& prop)
{
  PropertyBox *box = transWidgets->boxWithProperty(prop);
  Q_ASSERT(box != 0l);
  return box->isSelected (prop);
}

/* Returns a named property value.  Depending on the source and
   destination unit the actual value is converted. */
double transline::getProperty (const QString& prop, int unit)
{
  PropertyBox *box = transWidgets->boxWithProperty(prop);
  Q_ASSERT(box != 0l);
  return box->doubleValue(prop,unit);
}

