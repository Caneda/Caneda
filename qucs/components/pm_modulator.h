/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#ifndef __PM_MODULATOR_H
#define __PM_MODULATOR_H

#include "component.h"

class PM_Modulator : public Component
{
public:
   PM_Modulator(SchematicScene *scene = 0);

   inline QRectF boundingRect() const;

private:
   void initConstants();
   void initPorts();
   void initProperties();

   QRectF m_boundingRect;
};

inline QRectF PM_Modulator::boundingRect() const
{
   return m_boundingRect;
}

#endif
