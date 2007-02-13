/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "schematicview.h"
#include "schematicscene.h"
#include "components/resistor.h"

#include "wire.h"
#include "node.h"

#include <QtGui/QWheelEvent>
#include <QtCore/QDebug>
#include <cstdlib>

SchematicView::SchematicView(QGraphicsScene *sc,QWidget *parent) : QGraphicsView(sc,parent)
{
   Q_ASSERT(sc == 0);
   setScene(new SchematicScene(0,0,800,600));
   //setCacheMode(CacheBackground);
   setDragMode(RubberBandDrag);
   setAcceptDrops(true);
   setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
   setWindowTitle("Untitled");
   init();
}

void SchematicView::init()
{
   SchematicScene *s = qobject_cast<SchematicScene *>(scene());
   Q_ASSERT(s != 0);
   for(int i=0; i <20;i++)
   {
      Resistor *r = new Resistor(s);
      r->setPos(rand() %800,rand() % 600);
   }
   //scale(10,10);
}

SchematicScene* SchematicView::schematicScene() const
{
   SchematicScene* s = qobject_cast<SchematicScene*>(scene());
   Q_ASSERT(s);
   return s;
}
