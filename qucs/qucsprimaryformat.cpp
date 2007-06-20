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
#include <QtGui/QScrollBar>

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
         int horScroll,verScroll;
         horScroll = m_view->horizontalScrollBar() ? m_view->horizontalScrollBar()->value() : 0;
         verScroll = m_view->verticalScrollBar() ? m_view->verticalScrollBar()->value() : 0;
         m_view->sceneRect().toRect().getCoords(&x1,&y1,&x2,&y2);
         stream <<  "  <View=" << x1<<","<<y1<<"," << x2 << "," << y2 << ",";
         stream << m_view->matrix().m11() << "," << horScroll << "," << verScroll << ">\n";
         break;

      case Qucs::SymbolMode:
         //TODO: Handle for symbolmode
         ;
      default:;
   };


   stream << "  <Grid=" << scene->gridWidth() << ","<< scene->gridHeight() <<","
          << scene->isGridVisible() << ">\n";
   stream << "  <DataSet=" << scene->dataSet() << ">\n";
   stream << "  <DataDisplay=" << scene->dataDisplay() << ">\n";
   stream << "  <OpenDisplay=" << scene->opensDataDisplay() << ">\n";
   stream << "  <showFrame=" << scene->isFrameVisible() << ">\n";

   QString t;
   QStringList frameTexts = scene->frameTexts();
   t = frameTexts[0];
   Qucs::convert2ASCII(t);
   stream << "  <FrameText0=" << t << ">\n";
   t = frameTexts[1];
   Qucs::convert2ASCII(t);
   stream << "  <FrameText1=" << t << ">\n";
   t = frameTexts[2];
   Qucs::convert2ASCII(t);
   stream << "  <FrameText2=" << t << ">\n";
   t = frameTexts[3];
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

bool QucsPrimaryFormat::loadFromText(const QString& t)
{
   QString text(t);
   QTextStream stream(&text);
   QString Line;

   do {
      if(stream.atEnd())
         return true;
      Line = stream.readLine();
   } while(Line.isEmpty());

   if(Line.left(16) != "<Qucs Schematic ") {  // wrong file type ?

      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Wrong document type: ")+m_view->fileName());
      return false;
   }

   Line = Line.mid(16, Line.length()-17);
   if(!Qucs::checkVersion(Line)) { // wrong version number ?

      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Wrong document version: ")+Line);
      return false;
   }

   // read content *************************
   while(!stream.atEnd()) {
      Line = stream.readLine();
      Line = Line.trimmed();
      if(Line.isEmpty()) continue;

      if(Line == "<Symbol>") {
         if(!loadPaintings(stream)) {
            return false;
         }
      }
      else
         if(Line == "<Properties>") {
            if(!loadProperties(stream)) {  return false; } }
         else
            if(Line == "<Components>") {
               if(!loadComponents(stream)) {  return false; } }
            else
               if(Line == "<Wires>") {
                  if(!loadWires(stream)) {  return false; } }
               else
                  if(Line == "<Diagrams>") {
                     if(!loadDiagrams(stream)) {  return false; } }
                  else
                     if(Line == "<Paintings>") {
                        if(!loadPaintings(stream)) {  return false; } }
                     else {
                        QMessageBox::critical(0, QObject::tr("Error"),
                                              QObject::tr("File Format Error:\nUnknown field!"));

                        return false;
                     }
   }


   return true;
}

bool QucsPrimaryFormat::loadProperties(QTextStream &stream)
{
   QString Line;
   while(!stream.atEnd()) {
      Line = stream.readLine();
      if(Line.at(0) == '<') if(Line.at(1) == '/') return true;  // field end ?
   }
   return false;
}

bool QucsPrimaryFormat::loadComponents(QTextStream &stream)
{
   QString Line, cstr;
   Component *c;
   while(!stream.atEnd()) {
      Line = stream.readLine();
      if(Line.at(0) == '<') if(Line.at(1) == '/') return true;
      Line = Line.trimmed();
      if(Line.isEmpty()) continue;
      c = Component::componentFromLine(Line,m_view->schematicScene());
      if(!c) return false;

      m_view->schematicScene()->insertComponent(c);
   }

   QMessageBox::critical(0, QObject::tr("Error"),
                         QObject::tr("Format Error:\n'Component' field is not closed!"));
   return false;
}

bool QucsPrimaryFormat::loadWires(QTextStream &stream)
{
   QString Line;
   while(!stream.atEnd()) {
      Line = stream.readLine();
      if(Line.at(0) == '<') if(Line.at(1) == '/') return true;  // field end ?

      Line = Line.trimmed();
      if(Line.isEmpty()) continue;

      if(!loadWireFromLine(Line))
	 break;
   }
   return false;
}

bool QucsPrimaryFormat::loadWireFromLine(QString s)
{
   bool ok;
   QPointF p;
   QString n;

   if(s.at(0) != '<' || s.at(s.length()-1) != '>')
      return false;
   s = s.mid(1,s.length()-2); // cut off start and end character

   n = s.section(' ',0,0);
   p.setX(n.toDouble(&ok));
   if(!ok) return false;

   n = s.section(' ',1,1);
   p.setY(n.toDouble(&ok));
   if(!ok) return false;

   Node *n1 = m_view->schematicScene()->nodeAt(p);
   if(!n1)
      n1 = m_view->schematicScene()->createNode(p);

   n = s.section(' ',2,2);
   p.setX(n.toDouble(&ok));
   if(!ok) return false;


   n = s.section(' ',3,3);
   p.setY(n.toDouble(&ok));
   if(!ok) return false;


   Node *n2 = m_view->schematicScene()->nodeAt(p);
   if(!n2)
      n2 = m_view->schematicScene()->createNode(p);

   m_view->schematicScene()->insertWire(new Wire(m_view->schematicScene(),n1,n2));
   return true;
}

bool QucsPrimaryFormat::loadDiagrams(QTextStream &stream)
{
   QString Line;
   while(!stream.atEnd()) {
      Line = stream.readLine();
      if(Line.at(0) == '<') if(Line.at(1) == '/') return true;  // field end ?
   }
   return false;
}

bool QucsPrimaryFormat::loadPaintings(QTextStream &stream)
{
   QString Line;
   while(!stream.atEnd()) {
      Line = stream.readLine();
      if(Line.at(0) == '<') if(Line.at(1) == '/') return true;  // field end ?
   }
   return false;
}
