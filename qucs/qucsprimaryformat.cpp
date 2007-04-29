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
#include "qucs-tools/global.h"

#include <QtCore/QTextStream>
#include <QtCore/QFile>

#include <QtGui/QMessageBox>
#include <QtGui/QMatrix>

QucsPrimaryFormat::QucsPrimaryFormat(SchematicView *view) : FileFormatHandler(view)
{
}

int QucsPrimaryFormat::save(const QString& fileName)
{
   QFile file(fileName);
   if(!file.open(QIODevice::WriteOnly)) {
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Cannot save document!"));
      return -1;
   }
   if(!m_view) {
      qDebug("Nothing to save!\n");
      return -1;
   }
   SchematicScene *scene = m_view->schematicScene();
   if(!scene) {
      qDebug("Nothing to save!\n");
      return -1;
   }

   QTextStream stream(&file);

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
   Qucs::convert2ASCII(t = scene->frameText0());
   stream << "  <FrameText0=" << t << ">\n";
   Qucs::convert2ASCII(t = scene->frameText1());
   stream << "  <FrameText1=" << t << ">\n";
   Qucs::convert2ASCII(t = scene->frameText2());
   stream << "  <FrameText2=" << t << ">\n";
   Qucs::convert2ASCII(t = scene->frameText3());
   stream << "  <FrameText3=" << t << ">\n";
   stream << "</Properties>\n";

   //Painting *pp;
/*   stream << "<Symbol>\n";     // save all paintings for symbol
   //for(pp = SymbolPaints.first(); pp != 0; pp = SymbolPaints.next())
   //   stream << "  <" << pp->save() << ">\n";
   stream << "</Symbol>\n";

   stream << "<Components>\n";    // save all components
   for(Component *pc = DocComps.first(); pc != 0; pc = DocComps.next())
      stream << "  " << pc->save() << "\n";
   stream << "</Components>\n";

   stream << "<Wires>\n";    // save all wires
   for(Wire *pw = DocWires.first(); pw != 0; pw = DocWires.next())
      stream << "  " << pw->save() << "\n";

   // save all labeled nodes as wires
   for(Node *pn = DocNodes.first(); pn != 0; pn = DocNodes.next())
      if(pn->Label) stream << "  " << pn->Label->save() << "\n";
   stream << "</Wires>\n";

   stream << "<Diagrams>\n";    // save all diagrams
   for(Diagram *pd = DocDiags.first(); pd != 0; pd = DocDiags.next())
      stream << "  " << pd->save() << "\n";
   stream << "</Diagrams>\n";

   stream << "<Paintings>\n";     // save all paintings
   for(pp = DocPaints.first(); pp != 0; pp = DocPaints.next())
      stream << "  <" << pp->save() << ">\n";
   stream << "</Paintings>\n";
*/
   file.close();
   return 0;
}

int QucsPrimaryFormat::load(const QString& fileName)
{
   QFile f(fileName);
   f.open(QIODevice::ReadOnly);
   return 0;
}
