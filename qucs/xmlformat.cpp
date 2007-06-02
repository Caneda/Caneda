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

#include "xmlformat.h"
#include "schematicview.h"
#include "schematicscene.h"
#include "components/component.h"
#include "components/componentproperty.h"
#include "paintings/painting.h"
#include "wire.h"
#include "wireline.h"
#include "diagrams/diagram.h"

#include "qucs-tools/global.h"
#include <QtXml/QDomDocument>
#include <QtCore/QRectF>
#include <QtCore/QtDebug>

#include <QtGui/QMessageBox>
#include <QtGui/QMatrix>
#include <QtGui/QScrollBar>

#define CreateElement(ele,par,docu) QDomElement _##ele = docu.createElement(#ele); \
   _##par.appendChild(_##ele)

inline QDomText createIntNode(int val, QDomDocument& doc)
{
   return doc.createTextNode(QString::number(val));
}

inline QDomText createRealNode(qreal val, QDomDocument& doc)
{
   return doc.createTextNode(Qucs::realToString(val));
}


XmlFormat::XmlFormat(SchematicView *view) : FileFormatHandler(view)
{
}

QString XmlFormat::saveText()
{
   if(!m_view)
      return QString();
   SchematicScene *scene = m_view->schematicScene();
   if(!scene)
      return QString();

   QDomDocument doc("qucs");
   QDomElement _root = doc.createElement("qucs");
   _root.setAttribute("version","0.1.0");
   doc.appendChild(_root);

   {
      CreateElement(view,root,doc);
      CreateElement(scenerect,view,doc);
      QRectF r = m_view->sceneRect();

      CreateElement(x,scenerect,doc);
      _x.appendChild(createRealNode(r.x(),doc));

      CreateElement(y,scenerect,doc);
      _y.appendChild(createRealNode(r.y(),doc));

      CreateElement(width,scenerect,doc);
      _width.appendChild(createRealNode(r.width(),doc));

      CreateElement(height,scenerect,doc);
      _height.appendChild(createRealNode(r.height(),doc));

      CreateElement(scale,view,doc);
      _scale.appendChild(createRealNode(m_view->matrix().m11(),doc));

      CreateElement(scrollbar,view,doc);
      int horScroll,verScroll;
      horScroll = m_view->horizontalScrollBar() ? m_view->horizontalScrollBar()->value() : 0;
      verScroll = m_view->verticalScrollBar() ? m_view->verticalScrollBar()->value() : 0;
      CreateElement(horizontal,scrollbar,doc);
      _horizontal.appendChild(createIntNode(horScroll,doc));
      CreateElement(vertical,scrollbar,doc);
      _vertical.appendChild(createIntNode(verScroll,doc));


      CreateElement(grid,view,doc);
      _grid.setAttribute("visible",QString::number(scene->isGridShown()));
      CreateElement(xsize,grid,doc);
      _xsize.appendChild(createIntNode(scene->xGridSize(),doc));
      CreateElement(ysize,grid,doc);
      _ysize.appendChild(createIntNode(scene->yGridSize(),doc));

      CreateElement(data,view,doc);
      CreateElement(dataset,data,doc);
      _dataset.appendChild(doc.createTextNode(scene->dataSet()));
      CreateElement(datadisplay,data,doc);
      _datadisplay.appendChild(doc.createTextNode(scene->dataDisplay()));
      CreateElement(opendisplay,data,doc);
      _opendisplay.appendChild(createIntNode(scene->simOpenDpl(),doc));

      CreateElement(frame,view,doc);
      _frame.setAttribute("visible",QString::number(scene->isFrameShown()));
      QString t;
      CreateElement(text0,frame,doc);
      t = scene->frameText0();
      Qucs::convert2ASCII(t);
      _text0.appendChild(doc.createTextNode(t));
      CreateElement(text1,frame,doc);
      t = scene->frameText1();
      Qucs::convert2ASCII(t);
      _text1.appendChild(doc.createTextNode(t));
      CreateElement(text2,frame,doc);
      t = scene->frameText2();
      Qucs::convert2ASCII(t);
      _text2.appendChild(doc.createTextNode(t));
      CreateElement(text3,frame,doc);
      t = scene->frameText3();
      Qucs::convert2ASCII(t);
      _text3.appendChild(doc.createTextNode(t));
   }

   CreateElement(components,root,doc);
   foreach(Component *c, scene->components()) {
      CreateElement(component,components,doc);
      _component.setAttribute("model",c->model);
      _component.setAttribute("activestatus",c->activeStatus);

      {
         CreateElement(name,component,doc);
         _name.setAttribute("visible",QString::number(c->showName));
         QString name = c->name.isEmpty() ? "*" : c->name;
         _name.appendChild(doc.createTextNode(name));
      }
      CreateElement(pos,component,doc);
      {
         CreateElement(x,pos,doc);
         _x.appendChild(createRealNode(c->pos().x(),doc));
         CreateElement(y,pos,doc);
         _y.appendChild(createRealNode(c->pos().y(),doc));
      }

      CreateElement(textpos,component,doc);
      {
         QPointF textPos = c->propertyGroup() ? c->propertyGroup()->pos() : QPointF(0.,0.);
         CreateElement(x,textpos,doc);
         _x.appendChild(createRealNode(textPos.x(),doc));
         CreateElement(y,textpos,doc);
         _y.appendChild(createRealNode(textPos.y(),doc));
      }

      CreateElement(matrix,component,doc);
      QMatrix m = c->matrix();
      CreateElement(m11,matrix,doc);
      _m11.appendChild(createRealNode(m.m11(),doc));
      CreateElement(m12,matrix,doc);
      _m12.appendChild(createRealNode(m.m12(),doc));
      CreateElement(m21,matrix,doc);
      _m21.appendChild(createRealNode(m.m21(),doc));
      CreateElement(m22,matrix,doc);
      _m22.appendChild(createRealNode(m.m22(),doc));
      CreateElement(dx,matrix,doc);
      _dx.appendChild(createRealNode(m.dx(),doc));
      CreateElement(dy,matrix,doc);
      _dy.appendChild(createRealNode(m.dy(),doc));

      CreateElement(properties,component,doc);

      foreach(ComponentProperty *p1, c->properties()) {
         CreateElement(property,properties,doc);
         _property.setAttribute("visible",QString::number(p1->isVisible()));
         CreateElement(name,property,doc);
         _name.appendChild(doc.createTextNode(p1->name()));
         CreateElement(value,property,doc);
         _value.appendChild(doc.createTextNode(p1->value()));
      }
   }

   CreateElement(wires,root,doc);
   foreach(Wire *w, scene->wires()) {
      CreateElement(wire,wires,doc);
      foreach(WireLine l, w->wireLines()) {
         CreateElement(wireline,wire,doc);
         CreateElement(x1,wireline,doc);
         _x1.appendChild(createRealNode(l.x1(),doc));
         CreateElement(y1,wireline,doc);
         _y1.appendChild(createRealNode(l.y1(),doc));
         CreateElement(x2,wireline,doc);
         _x2.appendChild(createRealNode(l.x2(),doc));
         CreateElement(y2,wireline,doc);
         _y2.appendChild(createRealNode(l.y2(),doc));
      }
   }
   return doc.toString(4);
}

bool XmlFormat::loadFromText(const QString& text)
{
   QString errorStr;
   int errorLine;
   int errorColumn;

   QDomDocument doc;
   if (!doc.setContent(text, &errorStr, &errorLine,
                       &errorColumn)) {
      QMessageBox::information(0,QObject::tr("Qucs Schematic"),
                               QObject::tr("Parse error at line %1, column %2:\n%3")
                               .arg(errorLine)
                               .arg(errorColumn)
                               .arg(errorStr));
      return false;
   }

   QDomElement root = doc.documentElement();
   if (root.tagName() != "qucs") {
      QMessageBox::information(0,QObject::tr("Qucs Schematic"),
                               QObject::tr("The file is not an qucs xml schematic file."));
      return false;
   } else if (root.hasAttribute("version")
              && !Qucs::checkVersion(root.attribute("version"))) {
      QMessageBox::information(0, QObject::tr("Error"),
                               QObject::tr("Wrong document version"));
      return false;
   }


   QDomElement ele = root.firstChildElement();
   while(!ele.isNull()) {
      if(ele.tagName() == "view") {
         if(!loadView(ele))
            return false;
      }
      else if(ele.tagName() == "components") {
         if(!loadComponents(ele))
            return false;
      }
      else if(ele.tagName() == "wires") {
         if(!loadWires(ele))
            return false;
      }
      else {
         QMessageBox::critical(0, QObject::tr("Error"),
                               QObject::tr("Unknown tag %1").arg(ele.tagName()));
         return false;
      }

      ele = ele.nextSiblingElement();
   }

   return true;
}

bool XmlFormat::loadView(const QDomElement& ele)
{
   SchematicScene *scene = m_view->schematicScene();
   if(!scene) return false;
   QDomElement child = ele.firstChildElement();
   while( !child.isNull()) {
      QString tag = child.tagName();

      if(tag == "scenerect") {
         QDomElement grandChild = child.firstChildElement("x");
         if(grandChild.isNull()) return false;
         QRectF r;
         bool ok,k=true;
         r.setX(grandChild.text().toDouble(&ok));
         k &= ok;

         grandChild = child.firstChildElement("y");
         if(grandChild.isNull()) return false;
         r.setY(grandChild.text().toDouble(&ok));
         k &= ok;

         grandChild = child.firstChildElement("width");
         if(grandChild.isNull()) return false;
         r.setWidth(grandChild.text().toDouble(&ok));
         k &= ok;

         grandChild = child.firstChildElement("height");
         if(grandChild.isNull()) return false;
         r.setHeight(grandChild.text().toDouble(&ok));
         k &= ok;
         if(!k) return false;
         m_view->setSceneRect(r);
      }

      else if(tag == "scale") {
         bool ok;
         qreal scale = child.text().toDouble(&ok);
         if(!ok) return false;
         m_view->scale(scale,scale);
      }

      else if(tag == "scrollbar") {
         int val;
         bool ok;

         QDomElement hor = child.firstChildElement("horizontal");
         if(hor.isNull()) return false;
         val = hor.text().toInt(&ok);
         if(!ok) return false;
         m_view->horizontalScrollBar()->setValue(val);

         QDomElement ver = child.firstChildElement("vertical");
         if(ver.isNull()) return false;
         val = ver.text().toInt(&ok);
         if(!ok) return false;
         m_view->verticalScrollBar()->setValue(val);
      }

      else if(tag == "grid") {
         bool ok;
         int val;
         if(child.hasAttribute("visible")) {
            val = child.attribute("visible").toInt(&ok);
            if(!ok) return false;
            ok = (val != 0); //reusing ok
            scene->setGridVisible(ok);
         }

         QDomElement xsize = child.firstChildElement("xsize");
         if(xsize.isNull()) return false;
         val = xsize.text().toInt(&ok);
         if(!ok) return false;
         scene->setXGridSize(val);

         QDomElement ysize = child.firstChildElement("ysize");
         if(ysize.isNull()) return false;
         val = ysize.text().toInt(&ok);
         if(!ok) return false;
         scene->setYGridSize(val);
      }

      else if(tag == "data") {
         int val;
         bool ok;

         QDomElement dataset = child.firstChildElement("dataset");
         if(dataset.isNull()) return false;
         scene->setDataSet(dataset.text());

         QDomElement datadisplay = child.firstChildElement("datadisplay");
         if(datadisplay.isNull()) return false;
         scene->setDataDisplay(datadisplay.text());

         QDomElement opendisplay = child.firstChildElement("opendisplay");
         if(opendisplay.isNull()) return false;
         val = opendisplay.text().toInt(&ok);
         if(!ok) return false;
         ok = (val != 0);
         scene->setOpenDisplay(ok);
      }

      else if(tag == "frame") {
         bool ok;
         int val;
         if(child.hasAttribute("visible")) {
            val = child.attribute("visible").toInt(&ok);
            if(!ok) return false;
            ok = (val != 0); //reusing ok
            scene->setFrameVisible(ok);
         }

         QDomElement text0 = child.firstChildElement("text0");
         if(text0.isNull()) return false;
         scene->setFrameText0(text0.text());

         QDomElement text1 = child.firstChildElement("text1");
         if(text1.isNull()) return false;
         scene->setFrameText1(text1.text());

         QDomElement text2 = child.firstChildElement("text2");
         if(text2.isNull()) return false;
         scene->setFrameText2(text2.text());

         QDomElement text3 = child.firstChildElement("text3");
         if(text3.isNull()) return false;
         scene->setFrameText3(text3.text());

      }
      child = child.nextSiblingElement();
   }
   return true;
}

bool XmlFormat::loadComponents(const QDomElement& ele)
{
   SchematicScene *scene = m_view->schematicScene();
   if(!scene) return false;
   QDomElement child = ele.firstChildElement();
   while( !child.isNull()) {
      if(child.tagName() != "component" ||
         !child.hasAttribute("model"))
         return false;

      Component *c = Component::componentFromModel(child.attribute("model"), scene);
      if(!c) return false;

      if(child.hasAttribute("activestatus")) {
         int status;
         bool ok;
         status = child.attribute("activestatus").toInt(&ok);
         if(!ok) return false;
         c->activeStatus = (Component::ActiveStatus)status;
      }

      QDomElement grandChild = child.firstChildElement();

      while( !grandChild.isNull()) {
         QString tag = grandChild.tagName();
         if(tag == "name") {
            if(!grandChild.hasAttribute("visible"))
               return false;
            int val;
            bool ok;
            val = grandChild.attribute("visible").toInt(&ok);
            if(!ok) return false;
            c->name = grandChild.text();
            //TODO: Make name property
         }

         else if(tag == "pos") {
            QPointF p;
            bool ok;
            QDomElement _x = grandChild.firstChildElement("x");
            if(_x.isNull()) return false;
            p.setX(_x.text().toDouble(&ok));
            if(!ok) return false;

            QDomElement _y = grandChild.firstChildElement("y");
            if(_y.isNull()) return false;
            p.setY(_y.text().toDouble(&ok));
            if(!ok) return false;

            c->setPos(p);
         }

         else if(tag == "textpos") {
            bool ok;
            QPointF p;
            QDomElement _x = grandChild.firstChildElement("x");
            if(_x.isNull()) return false;
            p.setX(_x.text().toDouble(&ok));
            if(!ok) return false;

            QDomElement _y = grandChild.firstChildElement("y");
            if(_y.isNull()) return false;
            p.setY(_y.text().toDouble(&ok));
            if(!ok) return false;

            c->propertyGroup()->setPos(p);
         }

         else if(tag == "matrix") {
            bool ok;
            QDomElement _m11 = grandChild.firstChildElement("m11");
            if(_m11.isNull()) return false;
            qreal m11 = _m11.text().toDouble(&ok);
            if(!ok) return false;

            QDomElement _m12 = grandChild.firstChildElement("m12");
            if(_m12.isNull()) return false;
            qreal m12 = _m12.text().toDouble(&ok);
            if(!ok) return false;

            QDomElement _m21 = grandChild.firstChildElement("m21");
            if(_m21.isNull()) return false;
            qreal m21 = _m21.text().toDouble(&ok);
            if(!ok) return false;

            QDomElement _m22 = grandChild.firstChildElement("m22");
            if(_m22.isNull()) return false;
            qreal m22 = _m22.text().toDouble(&ok);
            if(!ok) return false;

            QDomElement _dx = grandChild.firstChildElement("dx");
            if(_dx.isNull()) return false;
            qreal dx = _dx.text().toDouble(&ok);
            if(!ok) return false;

            QDomElement _dy = grandChild.firstChildElement("dy");
            if(_dy.isNull()) return false;
            qreal dy = _dy.text().toDouble(&ok);
            if(!ok) return false;

            c->setMatrix(QMatrix(m11,m12,m21,m22,dx,dy));
         }

         else if(tag == "properties") {
            QDomElement property = grandChild.firstChildElement();
            while(!property.isNull()) {
               if(property.tagName() != "property")
                  return false;

               QString name;
               QString value;

               QDomElement _name = property.firstChildElement("name");
               if(_name.isNull()) return false;
               name = _name.text();

               QDomElement _value = property.firstChildElement("value");
               if(_value.isNull()) return false;
               value = _value.text();

               ComponentProperty *p = c->property(name);
               if(!p) return false;
               *p = value;

               if(property.hasAttribute("visible")) {
                  int val;
                  bool ok;
                  val = property.attribute("visible").toInt(&ok);
                  if(!ok) return false;
                  ok = (val != 0);
                  if(ok)
                     p->show();
                  else
                     p->hide();
               }

               property = property.nextSiblingElement();
            }
         }

         grandChild = grandChild.nextSiblingElement();
      }

      child = child.nextSiblingElement();
   }
   return true;
}

bool XmlFormat::loadWires(const QDomElement& ele)
{
   SchematicScene *scene = m_view->schematicScene();
   if(!scene) return false;
   QDomElement child = ele.firstChildElement();
   while(!child.isNull()) {
      QList<WireLine> wireLines;
      if(child.tagName() != "wire")
         return false;
      QDomElement _line = child.firstChildElement("wireline");
      while(!_line.isNull()) {
         qreal x1,y1,x2,y2;
         bool ok;

         QDomElement _x1 = _line.firstChildElement("x1");
         if(_x1.isNull()) return false;
         x1 = _x1.text().toDouble(&ok);
         if(!ok) return false;

         QDomElement _y1 = _line.firstChildElement("y1");
         if(_y1.isNull()) return false;
         y1 = _y1.text().toDouble(&ok);
         if(!ok) return false;

         QDomElement _x2 = _line.firstChildElement("x2");
         if(_x2.isNull()) return false;
         x2 = _x2.text().toDouble(&ok);
         if(!ok) return false;

         QDomElement _y2 = _line.firstChildElement("y2");
         if(_y2.isNull()) return false;
         y2 = _y2.text().toDouble(&ok);
         if(!ok) return false;

         wireLines << WireLine(x1,y1,x2,y2);
         _line = _line.nextSiblingElement();
      }

      child = child.nextSiblingElement();

      if(wireLines.isEmpty()) continue;
      QPointF p1 = wireLines[0].p1();
      QPointF p2 = wireLines.last().p2();
      Node *n1 = scene->nodeAt(p1);
      if(!n1)
         n1 = scene->createNode(p1);
      Node *n2 = scene->nodeAt(p2);
      if(!n2)
         n2 = scene->createNode(p2);

      Wire *w = new Wire(scene,n1,n2);
      w->setWireLines(wireLines);
   }
   return true;
}
