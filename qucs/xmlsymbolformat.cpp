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

#include "component.h"
#include "item.h"
#include "schematicscene.h"
#include "schematicview.h"

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QMessageBox>
#include <QSvgGenerator>

//! Constructor
XmlSymbolFormat::XmlSymbolFormat(SchematicScene *scene) : FileFormatHandler(scene)
{
}

bool XmlSymbolFormat::save()
{
    SchematicScene *scene = schematicScene();
    if(!scene) {
        return false;
    }

    //Generate and save the xml description ********************
    QString text = saveText();
    if(text.isEmpty()) {
        qDebug("Looks buggy! Null data to save! Was this expected?");
    }

    QFile file(scene->fileName());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(0, QObject::tr("Error"),
                QObject::tr("Cannot save document!"));
        return false;
    }
    QTextStream stream(&file);
    stream << text;
    file.close();

    //Generate and save svg ************************************
    bool state_useGrid = scene->isGridVisible();
    scene->setGridVisible(false);

    QSvgGenerator svg_engine;
    QFileInfo info(scene->fileName());
    svg_engine.setFileName(info.absolutePath()+"/"+info.baseName()+".svg");
    scene->toPaintDevice(svg_engine, scene->imageBoundingRect().width(), scene->imageBoundingRect().height());

    scene->setGridVisible(state_useGrid);

    return true;
}

bool XmlSymbolFormat::load()
{
}

QString XmlSymbolFormat::saveText()
{
    SchematicScene *scene = schematicScene();

    QString retVal;
    Qucs::XmlWriter *writer = new Qucs::XmlWriter(&retVal);
    writer->setAutoFormatting(true);
    writer->writeStartDocument();

    //Write all view details
    writer->writeStartElement("component");

    QFileInfo info(scene->fileName());
    writer->writeAttribute("name", info.baseName());
    writer->writeAttribute("version", Qucs::version);
    writer->writeAttribute("label", "comp");

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
    writer->writeAttribute("default", "userdefined");
    writer->writeStartElement("schematic");
    writer->writeAttribute("name", "userdefined");
    writer->writeAttribute("href", info.baseName()+".svg");

    //Write the ports positions
    QList<QGraphicsItem*> items = scene->items();
    QList<Component*> components = filterItems<Component>(items, RemoveItems);
    if(!components.isEmpty()) {
        foreach(Component *c, components) {
            if(c->name() == "Port") {
                writer->writeEmptyElement("port");
                writer->writeAttribute("name", c->label());

                // We adjust the port to fit in grid
                QRectF source_area = scene->imageBoundingRect();
                QPointF newOrigin = scene->smartNearingGridPoint(source_area.topLeft());
                source_area.setLeft(newOrigin.x());
                source_area.setTop(newOrigin.y());

                writer->writeAttribute("x", QString::number(c->pos().x() - source_area.x()));
                writer->writeAttribute("y", QString::number(c->pos().y() - source_area.y()));
            }
        }
    }

    writer->writeEndElement(); //</schematic>
    writer->writeEndElement(); //</schematics>

    //Write ports properties
    writer->writeStartElement("ports");
    if(!components.isEmpty()) {
        foreach(Component *c, components) {
            if(c->name() == "Port") {
                writer->writeEmptyElement("port");
                writer->writeAttribute("name", c->label());
                writer->writeAttribute("type", "analog");
                // TODO: To be replaced by the following line once properties are handled
                //       writer->writeAttribute("type", c->property("type").toString());
            }
        }
    }
    writer->writeEndElement(); //</ports>

    //TODO Write properties
    writer->writeStartElement("properties");
    writer->writeEndElement(); //</properties>

    writer->writeEndDocument(); //</component>

    delete writer;
    return retVal;
}
