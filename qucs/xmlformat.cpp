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
#include "item.h"

#include "component.h"
#include "paintings/painting.h"
#include "wire.h"
#include "wireline.h"
#include "port.h"
#include "xmlutilities/xmlutilities.h"
#include "diagrams/diagram.h"

#include "qucs-tools/global.h"
#include <QtXml/QDomDocument>
#include <QtCore/QRectF>
#include <QtCore/QtDebug>

#include <QtGui/QMessageBox>
#include <QtGui/QMatrix>
#include <QtGui/QScrollBar>

#include <QtXml/QXmlStreamWriter>

XmlFormat::XmlFormat(SchematicView *view) : FileFormatHandler(view)
{
}

void getConnectedWires(Wire *wire, QList<Wire*> &out)
{
   if(out.contains(wire)) return;

   out << wire;

   QList<Port*> *port1_connections = wire->port1()->connections();
   QList<Port*> *port2_connections = wire->port2()->connections();

   if(port1_connections) {
      foreach(Port *port, *port1_connections) {
         if(port->owner()->isWire())
            getConnectedWires(port->owner()->wire(), out);
      }
   }

   if(port2_connections) {
      foreach(Port *port, *port2_connections) {
         if(port->owner()->isWire()) {
            getConnectedWires(port->owner()->wire(), out);
         }
      }
   }
}

void writeEquiWires(Qucs::XmlWriter *writer, int id, int wireStartId, const QList<Wire*> &wires)
{
   writer->writeStartElement("equipotential");
   writer->writeAttribute("id", QString::number(id));

   foreach(Wire *wire, wires) {
      wire->saveData(writer, wireStartId++);
   }

   writer->writeEndElement();
}

QString XmlFormat::saveText()
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
   writer->writeDTD(QString("<!DOCTYPE qucs>"));
   writer->writeStartElement("qucs");
   writer->writeAttribute("version", Qucs::version);

   //TODO:mainwindow geometry
   //write all view details
   writer->writeStartElement("view");

   writer->writeStartElement("scenerect");
   writer->writeRect(m_view->sceneRect());
   writer->writeEndElement(); //</scenerect>

   writer->writeStartElement("viewtransform");
   writer->writeTransform(m_view->transform());
   writer->writeEndElement(); //</viewtransform>

   writer->writeStartElement("scrollbarvalues");
   writer->writeElement("horizontal", m_view->horizontalScrollBar()->value());
   writer->writeElement("vertical", m_view->verticalScrollBar()->value());
   writer->writeEndElement(); //</scrollbarvalues>

   writer->writeStartElement("grid");
   writer->writeAttribute("visible", Qucs::boolToString(scene->isGridVisible()));
   writer->writeSize(QSize(scene->gridWidth(), scene->gridHeight()));
   writer->writeEndElement(); //</grid>

   writer->writeStartElement("data");
   writer->writeElement("dataset", scene->dataSet());
   writer->writeElement("datadisplay", scene->dataDisplay());
   writer->writeElement("opensdatadisplay", scene->opensDataDisplay());
   writer->writeEndElement(); //</data>

   writer->writeStartElement("frame");
   writer->writeAttribute("visible", Qucs::boolToString(scene->isFrameVisible()));

   writer->writeStartElement("frametexts");
   foreach(QString text, scene->frameTexts()) {
      //Qucs::convert2ASCII(text);
      writer->writeElement("text",text);
   }
   writer->writeEndElement(); //</frametexts>
   writer->writeEndElement(); //</frame>
   writer->writeEndElement(); //</view>

   QList<QGraphicsItem*> items = scene->items();
   //Write all the components now

   QList<Component*> components = filterItems<Component>(items, RemoveItems);
   if(!components.isEmpty()) {
       writer->writeStartElement("components");
       foreach(Component *c, components)
           c->saveData(writer);
       writer->writeEndElement(); //</components>
   }

   QList<Wire*> wires = filterItems<Wire>(items, RemoveItems);
   if(!wires.isEmpty()) {
      int wireId = 0;
      int equiId = 0;
      writer->writeStartElement("wires");

      QList<Wire*> parsedWires;
      foreach(Wire *w, wires) {
         if(parsedWires.contains(w))
            continue;

         QList<Wire*> equi;
         getConnectedWires(w, equi);
         writeEquiWires(writer, equiId++, wireId, equi);

         parsedWires += equi;
         wireId += equi.size();
      }

      writer->writeEndElement(); //</wires>
   }

   QList<QucsItem*> qItems = filterItems<QucsItem>(items, RemoveItems);
   if(!qItems.isEmpty()) {
      qDebug() << "Some items not saved. Should implement them still.";
   }

   writer->writeEndDocument(); //</qucs>

   delete writer;
   return retVal;
}

bool XmlFormat::loadFromText(const QString& text)
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

void XmlFormat::readQucs(Qucs::XmlReader* reader)
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
         else
            reader->readUnknownElement();
      }
   }
}

void XmlFormat::loadComponents(Qucs::XmlReader *reader)
{
    SchematicScene *scene = m_view->schematicScene();
    if(!scene) {
       reader->raiseError(QObject::tr("XmlFormat::loadComponents() : Scene doesn't exist.\n"
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

void XmlFormat::loadView(Qucs::XmlReader *reader)
{
   SchematicScene *scene = m_view->schematicScene();
   if(!scene) {
      reader->raiseError(QObject::tr("XmlFormat::loadView() : Scene doesn't exist.\n"
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
      scene->setDataSet(dataSet);
      scene->setDataDisplay(dataDisplay);
      scene->setOpensDataDisplay(opensDataDisplay);
      scene->setFrameVisible(frameVisible);
      scene->setFrameTexts(frameTexts);
      m_view->setUpdatesEnabled(true);
   }
}

void XmlFormat::loadWires(Qucs::XmlReader* reader)
{
    SchematicScene *scene = m_view->schematicScene();
    if(!scene) {
       reader->raiseError(QObject::tr("XmlFormat::loadWires() : Scene doesn't exist.\n"
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
               if(reader->name() == "wire")
                   Wire::loadWireData(reader,scene);
               else {
                   reader->readUnknownElement();
                   reader->raiseError(QObject::tr("Malformatted file"));
               }
           }
       }
   }
}
