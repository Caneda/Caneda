/***************************************************************************
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
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

#include "xmlsymbolformat.h"
#include "schematicview.h"
#include "schematicscene.h"
#include "item.h"

#include "component.h"
#include "paintings/painting.h"
#include "wire.h"
#include "wireline.h"
#include "port.h"
#include "xmlutilities/xmlutilities.h"

#include "qucs-tools/global.h"
#include <QtCore/QRectF>
#include <QtCore/QtDebug>

#include <QtGui/QMessageBox>
#include <QtGui/QMatrix>
#include <QtGui/QScrollBar>
#include <QSvgGenerator>
#include <QPicture>

XmlSymbolFormat::XmlSymbolFormat(SchematicView *view) : FileFormatHandler(view)
{
}

QString XmlSymbolFormat::saveText()
{
   if(!m_view)
      return QString();
   SchematicScene *scene = m_view->schematicScene();
   if(!scene)
      return QString();

   QString retVal;
   Qucs::XmlWriter *writer = new Qucs::XmlWriter(&retVal);
   writer->setAutoFormatting(true);
   writer->writeStartDocument();

   //Write all view details
   writer->writeStartElement("component");

   QFileInfo info(scene->fileName());
   writer->writeAttribute("name", info.baseName());
   writer->writeAttribute("version","0.1.0");
   writer->writeAttribute("label","comp");

   writer->writeStartElement("displaytext");
   writer->writeLocaleText("C", "User created component");
//   TODO: When available use this to save user defined displaytext
//   writer->writeLocaleText("C", scene->displayText());
   writer->writeEndElement(); //</displaytext>

   writer->writeStartElement("description");
   writer->writeLocaleText("C", "User created component based on user schematic");
//   TODO: When available use this to save user defined description
//   writer->writeLocaleText("C", scene->description());
   writer->writeEndElement(); //</description>

   writer->writeStartElement("schematics");
   writer->writeAttribute("default","userdefined");
   writer->writeStartElement("schematic");
   writer->writeAttribute("name","userdefined");
   writer->writeAttribute("href",info.baseName()+".svg");

   //Write the ports positions
   QList<QGraphicsItem*> items = scene->items();
   QList<Component*> components = filterItems<Component>(items, RemoveItems);
   if(!components.isEmpty()) {
       foreach(Component *c, components){
           if(c->name() == "Port"){
               writer->writeEmptyElement("port");
               writer->writeAttribute("name", c->label());
               writer->writeAttribute("x", QString::number(c->pos().x()));
               writer->writeAttribute("y", QString::number(c->pos().y()));
           }
       }
   }

   writer->writeEndElement(); //</schematic>
   writer->writeEndElement(); //</schematics>

   //Write ports properties
   writer->writeStartElement("ports");
   if(!components.isEmpty()) {
       foreach(Component *c, components){
           if(c->name() == "Port"){
               writer->writeEmptyElement("port");
               writer->writeAttribute("name", c->label());
               writer->writeAttribute("type", "analog");
//               TODO: To be replaced by the following line once properties are handled
//               writer->writeAttribute("type", c->property("type").toString());
           }
       }
   }
   writer->writeEndElement(); //</ports>

   //TODO Write properties
   writer->writeStartElement("properties");
   writer->writeEndElement(); //</properties>

   //Generate and save svg ************************************
   bool state_useGrid = scene->isGridVisible();
   scene->setGridVisible(false);

   QSvgGenerator svg_engine;
   svg_engine.setSize(scene->imageSize());
   QFile file(info.absolutePath()+"/"+info.baseName()+".svg");
   svg_engine.setOutputDevice(&file);
   QPainter svg_painter(&svg_engine);

   QPicture picture;
   //TODO Correct image symbol size here and place ports in the correct position
   scene->toPaintDevice(picture, scene->imageSize().width()*2, scene->imageSize().height()*2);

   // "plays" the QPicture with a QSvgGenerator
   picture.play(&svg_painter);

   scene->setGridVisible(state_useGrid);
   //*********************************************************

   writer->writeEndDocument(); //</component>

   delete writer;
   return retVal;
}

bool XmlSymbolFormat::loadFromText(const QString& text)
{
   if(!m_view) return false;


   Qucs::XmlReader *reader = new Qucs::XmlReader(text.toUtf8());
   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isStartElement()) {
         if(reader->name() == "qucs" &&
            Qucs::checkVersion(reader->attributes().value("version").toString())) {
            readQucs(reader);
         }
         else {
            reader->raiseError(QObject::tr("Not a qucs file or probably malformatted file"));
         }
      }
   }

   if(reader->hasError()) {
      QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
      delete reader;
      return false;
   }

   delete reader;
   return true;
}

void XmlSymbolFormat::readQucs(Qucs::XmlReader* reader)
{
   if(!reader->isStartElement() || reader->name() != "qucs")
      reader->raiseError(QObject::tr("Not a qucs file or probably malformatted file"));

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         Q_ASSERT(reader->name() == "qucs");
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "components")
            loadComponents(reader);
         else if(reader->name() == "view")
            loadView(reader);
         else if(reader->name() == "wires")
            loadWires(reader);
         else if(reader->name() == "paintings")
            loadPaintings(reader);
         else
            reader->readUnknownElement();
      }
   }
}

void XmlSymbolFormat::loadComponents(Qucs::XmlReader *reader)
{
    SchematicScene *scene = m_view->schematicScene();
    if(!scene) {
       reader->raiseError(QObject::tr("XmlSymbolFormat::loadComponents() : Scene doesn't exist.\n"
                                      "So raising xml error to stop parsing"));
       return;
    }

    if(!reader->isStartElement() || reader->name() != "components")
       reader->raiseError(QObject::tr("Malformatted file"));

    while(!reader->atEnd()) {
       reader->readNext();

       if(reader->isEndElement()) {
          Q_ASSERT(reader->name() == "components");
          break;
       }

       if(reader->isStartElement()) {
          if(reader->name() == "component")
              Component::loadComponentData(reader,scene);
          else {
              qWarning() << "Error: Found unknown component type" << reader->name().toString();
              reader->readUnknownElement();
              reader->raiseError(QObject::tr("Malformatted file"));
          }
       }
    }
}

void XmlSymbolFormat::loadView(Qucs::XmlReader *reader)
{
   SchematicScene *scene = m_view->schematicScene();
   if(!scene) {
      reader->raiseError(QObject::tr("XmlSymbolFormat::loadView() : Scene doesn't exist.\n"
                                     "So raising xml error to stop parsing"));
      return;
   }

   if(!reader->isStartElement() || reader->name() != "view")
      reader->raiseError(QObject::tr("Malformatted file"));

   QRectF sceneRect;
   QTransform viewTransform;
   int horizontalScroll = 0;
   int verticalScroll = 0;
   bool gridVisible = false;
   QSize gridSize;
   QString dataSet;
   QString dataDisplay;
   bool opensDataDisplay = false;
   bool frameVisible = false;
   QStringList frameTexts;

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         Q_ASSERT(reader->name() == "view");
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "scenerect") {
            reader->readFurther();
            sceneRect = reader->readRect();
            if(!sceneRect.isValid()) {
               reader->raiseError(QObject::tr("Invalid QRect attribute"));
               break;
            }
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "scenerect");
         }
         else if(reader->name() == "viewtransform") {
            reader->readFurther();
            viewTransform = reader->readTransform();
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "viewtransform");
         }
         else if(reader->name() == "scrollbarvalues") {
            reader->readFurther();
            horizontalScroll = reader->readInt(/*horizontal*/);
            reader->readFurther();
            verticalScroll = reader->readInt(/*vertical*/);
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "scrollbarvalues");
         }
         else if(reader->name() == "grid") {
            QString att = reader->attributes().value("visible").toString();
            att = att.toLower();
            if(att != "true" && att != "false") {
               reader->raiseError(QObject::tr("Invalid bool attribute"));
               break;
            }
            gridVisible = (att == "true");
            reader->readFurther();
            gridSize = reader->readSize();
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "grid");
         }
         else if(reader->name() == "data") {
            reader->readFurther();
            dataSet = reader->readElementText(/*dataset*/);
            reader->readFurther();
            dataDisplay = reader->readElementText(/*datadisplay*/);
            reader->readFurther();
            opensDataDisplay = (reader->readElementText(/*opensdatadisplay*/) == "true");
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "data");
         }
         else if(reader->name() == "frame") {
            QString att = reader->attributes().value("visible").toString();
            att = att.toLower();
            if(att != "true" && att != "false") {
               reader->raiseError(QObject::tr("Invalid bool attribute"));
               break;
            }
            frameVisible = (att == "true");

            reader->readFurther();
            if(reader->isStartElement() && reader->name() == "frametexts") {
               while(!reader->atEnd()) {
                  reader->readNext();
                  if(reader->isEndElement()) {
                     Q_ASSERT(reader->name() == "frametexts");
                     break;
                  }

                  if(reader->isStartElement()) {
                     if(reader->name() == "text") {
                        QString text = reader->readElementText();
                        frameTexts << text;
                     }
                     else
                        reader->readUnknownElement();
                  }
               }
            }
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "frame");
         }
      }
   }

   if(!reader->hasError()) {
      m_view->setUpdatesEnabled(false);
      m_view->setSceneRect(sceneRect);
      m_view->setTransform(viewTransform);
      m_view->horizontalScrollBar()->setValue(horizontalScroll);
      m_view->verticalScrollBar()->setValue(verticalScroll);
      scene->setGridVisible(gridVisible);
      scene->setGridSize(gridSize.width(), gridSize.height());
      scene->setSceneRect(sceneRect);
      scene->setDataSet(dataSet);
      scene->setDataDisplay(dataDisplay);
      scene->setOpensDataDisplay(opensDataDisplay);
      scene->setFrameVisible(frameVisible);
      scene->setFrameTexts(frameTexts);
      m_view->setUpdatesEnabled(true);
   }
}

void XmlSymbolFormat::loadWires(Qucs::XmlReader* reader)
{
    SchematicScene *scene = m_view->schematicScene();
    if(!scene) {
       reader->raiseError(QObject::tr("XmlSymbolFormat::loadWires() : Scene doesn't exist.\n"
                                      "So raising xml error to stop parsing"));
       return;
    }

    if(!reader->isStartElement() || reader->name() != "wires")
       reader->raiseError(QObject::tr("Malformatted file"));

    while(!reader->atEnd()) {
       reader->readFurther();

       if(reader->isEndElement()) {
          Q_ASSERT(reader->name() == "wires");
          break;
       }

       if(!reader->isStartElement() || reader->name() != "equipotential")
           reader->raiseError(QObject::tr("Malformatted file")+reader->name().toString());

       while(!reader->atEnd()){
           reader->readNext();

           if(reader->isEndElement()) {
               Q_ASSERT(reader->name() == "equipotential");
               break;
           }

           if(reader->isStartElement()) {
               if(reader->name() == "wire") {
                   Wire *w = Wire::loadWireData(reader,scene);
                   w->checkAndConnect(Qucs::DontPushUndoCmd);
               }
               else {
                   reader->readUnknownElement();
                   reader->raiseError(QObject::tr("Malformatted file"));
               }
           }
       }
   }
}

void XmlSymbolFormat::loadPaintings(Qucs::XmlReader *reader)
{
    SchematicScene *scene = m_view->schematicScene();
    if(!scene) {
       reader->raiseError(QObject::tr("XmlSymbolFormat::loadPaintings() : Scene doesn't exist.\n"
                                      "So raising xml error to stop parsing"));
       return;
    }

    if(!reader->isStartElement() || reader->name() != "paintings")
       reader->raiseError(QObject::tr("Malformatted file"));

    while(!reader->atEnd()) {
       reader->readNext();

       if(reader->isEndElement()) {
          Q_ASSERT(reader->name() == "paintings");
          break;
       }

       if(reader->isStartElement()) {
          if(reader->name() == "painting")
              Painting::loadPainting(reader,scene);
          else {
              qWarning() << "Error: Found unknown painting type" << reader->name().toString();
              reader->readUnknownElement();
              reader->raiseError(QObject::tr("Malformatted file"));
          }
       }
    }
}

