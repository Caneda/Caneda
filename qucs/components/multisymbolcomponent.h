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

#ifndef __MULTISYMBOLCOMPONENT_H
#define __MULTISYMBOLCOMPONENT_H

#include "component.h"
#include "shapes.h"

class MultiSymbolComponent : public Component
{
   public:
      // enum {
// 	 Type = QucsItem::MultiSymbolComponentType
//       };

      explicit MultiSymbolComponent(SchematicScene *scene = 0) : Component(scene){}
      //int type() const { return QucsItem::MultiSymbolComponentType; }

      inline void setSymbol(const QString& symbol);

   protected:
      struct SymbolData
      {
	    ShapesList shapesList;
	    QRectF boundRect;

	    SymbolData() {}
	    SymbolData(const SymbolData& s)
	    {
	       shapesList = s.shapesList;
	       boundRect = s.boundRect;
	    }

	    void operator=(const SymbolData& s)
	    {
	       shapesList = s.shapesList;
	       boundRect = s.boundRect;
	    }
      };

      QMap<QString,SymbolData*> symbolMap;

};

inline void MultiSymbolComponent::setSymbol(const QString& symbol)
{
   if(!symbolMap.contains(symbol))
      return;
   prepareGeometryChange();
   m_shapes = symbolMap[symbol]->shapesList;
   m_boundingRect = symbolMap[symbol]->boundRect;
}

#endif //__MULTISYMBOLCOMPONENT_H
