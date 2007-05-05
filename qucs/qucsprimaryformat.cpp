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

#include "qucsprimaryformat.h"
#include "schematicview.h"
#include "schematicscene.h"
#include "components/component.h"
#include "paintings/painting.h"
#include "wire.h"
#include "diagrams/diagram.h"
#include "qucs-tools/global.h"

#include <QtCore/QTextStream>
#include <QtCore/QFile>

#include <QtGui/QMessageBox>
#include <QtGui/QMatrix>


QucsPrimaryFormat::QucsPrimaryFormat(SchematicView *view) : FileFormatHandler(view)
{
}

QString QucsPrimaryFormat::saveText()
{
   QString retVal;
   if(!m_view)
      return retVal;
   SchematicScene *scene = m_view->schematicScene();
   if(!scene)
      return retVal;
   QTextStream stream(&retVal);

   stream << "<Qucs Schematic " << Qucs::version << ">\n";
   stream << "<Properties>\n";

   switch(scene->currentMode()) {
      case Qucs::SchematicMode:
         // Using integer coordinates to support old version.
         int x1,y1,x2,y2;
         int dummyContentsX,dummyContentsY;
         dummyContentsX = dummyContentsY = 0;
         m_view->sceneRect().toRect().getCoords(&x1,&y1,&x2,&y2);
         stream <<  "  <View=" << x1<<","<<y1<<"," << x2 << "," << y2 << ",";
         stream << m_view->matrix().m11() << "," << dummyContentsX << "," << dummyContentsY << ">\n";
         break;

      case Qucs::SymbolMode:
         //TODO: Handle for symbolmode
         ;
      default:;
   };


   stream << "  <Grid=" << scene->xGridSize() << ","<< scene->yGridSize() <<","
          << scene->isGridShown() << ">\n";
   stream << "  <DataSet=" << scene->dataSet() << ">\n";
   stream << "  <DataDisplay=" << scene->dataDisplay() << ">\n";
   stream << "  <OpenDisplay=" << scene->simOpenDpl() << ">\n";
   stream << "  <showFrame=" << scene->isFrameShown() << ">\n";

   QString t;
   t = scene->frameText0();
   Qucs::convert2ASCII(t);
   stream << "  <FrameText0=" << t << ">\n";
   t = scene->frameText1();
   Qucs::convert2ASCII(t);
   stream << "  <FrameText1=" << t << ">\n";
   t = scene->frameText2();
   Qucs::convert2ASCII(t);
   stream << "  <FrameText2=" << t << ">\n";
   t = scene->frameText3();
   Qucs::convert2ASCII(t);
   stream << "  <FrameText3=" << t << ">\n";
   stream << "</Properties>\n";

   stream << "<Symbol>\n";     // save all paintings for symbol
   foreach(Painting *p, scene->symbolPaintings())
      stream << "  <" << p->saveString() << ">\n";
   stream << "</Symbol>\n";

   stream << "<Components>\n";    // save all components
   foreach(Component *c, scene->components())
      stream << "  " << c->saveString() << "\n";
   stream << "</Components>\n";

   stream << "<Wires>\n";    // save all wires
   foreach(Wire *pw,scene->wires())
      stream << "  " << pw->saveString() << "\n";

   // save all labeled nodes as wires
   // for(Node *pn = DocNodes.first(); pn != 0; pn = DocNodes.next())
//       if(pn->Label) stream << "  " << pn->Label->save() << "\n";
   stream << "</Wires>\n";

   stream << "<Diagrams>\n";    // save all diagrams
   foreach(Diagram *pd,scene->diagrams())
      stream << "  " << pd->saveString() << "\n";
   stream << "</Diagrams>\n";

   stream << "<Paintings>\n";     // save all paintings
   foreach(Painting *p, scene->paintings())
      stream << "  <" << p->saveString() << ">\n";
   stream << "</Paintings>\n";

   return retVal;
}

void QucsPrimaryFormat::loadFromText(const QString& text)
{
   if(text.isNull());
}
