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

#include "component.h"
#include "item.h"
#include "port.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "wire.h"
#include "wireline.h"

#include "paintings/painting.h"

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QMatrix>
#include <QMessageBox>
#include <QRectF>
#include <QScrollBar>
#include <QtDebug>

void getConnectedWires(Wire *wire, QList<Wire*> &out)
{
    if(out.contains(wire)) {
        return;
    }

    out << wire;

    QList<Port*> *port1_connections = wire->port1()->connections();
    QList<Port*> *port2_connections = wire->port2()->connections();

    if(port1_connections) {
        foreach(Port *port, *port1_connections) {
            if(port->owner()->isWire()) {
                getConnectedWires(port->owner()->wire(), out);
            }
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

void writeEquiWires(Qucs::XmlWriter *writer, int id, int wireStartId,
        const QList<Wire*> &wires)
{
    writer->writeStartElement("equipotential");
    writer->writeAttribute("id", QString::number(id));

    foreach(Wire *wire, wires) {
        wire->saveData(writer, wireStartId++);
    }

    writer->writeEndElement();
}

//! Constructor
XmlFormat::XmlFormat(SchematicScene *scene) : FileFormatHandler(scene)
{
}

bool XmlFormat::save()
{
    SchematicScene *scene = schematicScene();
    if(!scene) {
        return false;
    }

    QString text;
    if(scene->currentMode() == Qucs::SchematicMode) {
        text = saveText();
    }
    else if(scene->currentMode() == Qucs::SymbolMode) {
        text = saveSymbolText();
    }

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

    return true;
}

bool XmlFormat::load()
{
    SchematicScene *scene = schematicScene();
    if(!scene) {
        return false;
    }

    QFile file(scene->fileName());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(0, QObject::tr("Error"),
                QObject::tr("Cannot load document ")+scene->fileName());
        return false;
    }

    QTextStream stream(&file);

    bool result = false;
    if(scene->currentMode() == Qucs::SchematicMode) {
        result = loadFromText(stream.readAll());
    }
    else if(scene->currentMode() == Qucs::SymbolMode) {
        result = loadSymbolFromText(stream.readAll());
    }

    file.close();
    return result;
}

QString XmlFormat::saveText()
{
    QString retVal;
    Qucs::XmlWriter *writer = new Qucs::XmlWriter(&retVal);
    writer->setAutoFormatting(true);

    //Fist we start the document and write current version
    writer->writeStartDocument();
    writer->writeDTD(QString("<!DOCTYPE qucs>"));
    writer->writeStartElement("qucs");
    writer->writeAttribute("version", Qucs::version);

    //Now we copy all the elements and properties in the schematic
    saveSchematics(writer);

    //Now we copy the previously defined symbol if created
    copyQucsElement("symbol", writer);

    //Finally we finish the document
    writer->writeEndDocument(); //</qucs>

    delete writer;
    return retVal;
}

QString XmlFormat::saveSymbolText()
{
    SchematicScene *scene = schematicScene();
    QString retVal;
    Qucs::XmlWriter *writer = new Qucs::XmlWriter(&retVal);
    writer->setAutoFormatting(true);

    //Fist we start the document and write current version
    writer->writeStartDocument();
    writer->writeDTD(QString("<!DOCTYPE qucs>"));
    writer->writeStartElement("qucs");
    writer->writeAttribute("version", Qucs::version);

    //Now we copy all the elements and properties previously defined in the schematic
    QFile file(scene->fileName());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }

    QTextStream stream(&file);
    QString text = stream.readAll();
    file.close();
    QXmlStreamReader *reader = new QXmlStreamReader(text.toUtf8());
    while(!reader->atEnd()) {
        reader->readNext();
        if(reader->isStartElement() && reader->name() != "qucs"
                && reader->name() != "symbol") {

            QString qualifiedName = reader->name().toString();
            writer->writeStartElement(qualifiedName);
            reader->readNext();
            while(!reader->isEndElement() || reader->name() != qualifiedName){
                writer->writeCurrentToken(*reader);
                reader->readNext();
            }
            writer->writeEndElement();
        }
        else if(reader->isStartElement() && reader->name() == "symbol"){
            while(!reader->isEndElement() || reader->name() != "symbol") {
                reader->readNext();
            }
        }
    }

    //Now we save the symbol
    writer->writeStartElement("symbol");
    //saveSchematics(writer);
    saveComponents(writer);
    saveWires(writer);
    savePaintings(writer);
    writer->writeEndElement(); //</symbol>

    //Finally we finish the document
    writer->writeEndDocument(); //</qucs>

    delete reader;
    delete writer;
    return retVal;
}

void XmlFormat::saveSchematics(Qucs::XmlWriter *writer)
{
    saveView(writer);
    saveComponents(writer);
    saveWires(writer);
    savePaintings(writer);
}

void XmlFormat::saveView(Qucs::XmlWriter *writer)
{
    SchematicScene *scene = schematicScene();
    writer->writeStartElement("view");

    writer->writeStartElement("scenerect");
    writer->writeRect(scene->sceneRect());
    writer->writeEndElement(); //</scenerect>

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
    writer->writeSize(QSize(scene->frameColumns(), scene->frameRows()));
    writer->writeStartElement("frametexts");
    foreach(QString text, scene->frameTexts()) {
        writer->writeElement("text",text);
    }
    writer->writeEndElement(); //</frametexts>
    writer->writeEndElement(); //</frame>
    writer->writeEndElement(); //</view>
}

void XmlFormat::saveComponents(Qucs::XmlWriter *writer)
{
    SchematicScene *scene = schematicScene();
    QList<QGraphicsItem*> items = scene->items();
    QList<Component*> components = filterItems<Component>(items, RemoveItems);
    if(!components.isEmpty()) {
        writer->writeStartElement("components");
        foreach(Component *c, components) {
            c->saveData(writer);
        }
        writer->writeEndElement(); //</components>
    }
}

void XmlFormat::saveWires(Qucs::XmlWriter *writer)
{
    SchematicScene *scene = schematicScene();
    QList<QGraphicsItem*> items = scene->items();
    QList<Wire*> wires = filterItems<Wire>(items, RemoveItems);
    if(!wires.isEmpty()) {
        int wireId = 0;
        int equiId = 0;
        writer->writeStartElement("wires");

        QList<Wire*> parsedWires;
        foreach(Wire *w, wires) {
            if(parsedWires.contains(w)) {
                continue;
            }

            QList<Wire*> equi;
            getConnectedWires(w, equi);
            writeEquiWires(writer, equiId++, wireId, equi);

            parsedWires += equi;
            wireId += equi.size();
        }

        writer->writeEndElement(); //</wires>
    }
}

void XmlFormat::savePaintings(Qucs::XmlWriter *writer)
{
    SchematicScene *scene = schematicScene();
    QList<QGraphicsItem*> items = scene->items();
    QList<Painting*> paintings = filterItems<Painting>(items, RemoveItems);
    if(!paintings.isEmpty()) {
        writer->writeStartElement("paintings");
        foreach(Painting *p, paintings) {
            p->saveData(writer);
        }
        writer->writeEndElement(); //</paintings>
    }
}

//Copies a previously defined element if created. Empty otherwise
void XmlFormat::copyQucsElement(const QString& qualifiedName , Qucs::XmlWriter *writer)
{
    SchematicScene *scene = schematicScene();
    writer->writeStartElement(qualifiedName);

    QFile file(scene->fileName());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    QString text = stream.readAll();
    file.close();
    QXmlStreamReader *reader = new QXmlStreamReader(text.toUtf8());
    while(!reader->atEnd()) {
        reader->readNext();
        if(reader->isStartElement() && reader->name() == qualifiedName) {
            reader->readNext();
            while(!reader->isEndElement() || reader->name() != qualifiedName) {
                writer->writeCurrentToken(*reader);
                reader->readNext();
            }
        }
    }

    writer->writeEndElement();
    delete reader;
}

bool XmlFormat::loadFromText(const QString& text)
{
    Qucs::XmlReader *reader = new Qucs::XmlReader(text.toUtf8());
    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isStartElement()) {
            if(reader->name() == "qucs" &&
                    Qucs::checkVersion(reader->attributes().value("version").toString())) {

                while(!reader->atEnd()) {
                    reader->readNext();
                    if(reader->isEndElement()) {
                        Q_ASSERT(reader->name() == "qucs");
                        break;
                    }
                    loadSchematics(reader);
                }
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

bool XmlFormat::loadSymbolFromText(const QString& text)
{
    Qucs::XmlReader *reader = new Qucs::XmlReader(text.toUtf8());
    while(!reader->atEnd()) {
        reader->readNext();
        if(reader->isStartElement() && reader->name() == "symbol") {
            reader->readNext();
            while(!reader->isEndElement() || reader->name() != "symbol"){
                loadSchematics(reader);
                reader->readNext();
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

void XmlFormat::loadSchematics(Qucs::XmlReader* reader)
{
    if(reader->isStartElement()) {
        if(reader->name() == "view") {
            loadView(reader);
        }
        else if(reader->name() == "components") {
            loadComponents(reader);
        }
        else if(reader->name() == "wires") {
            loadWires(reader);
        }
        else if(reader->name() == "paintings") {
            loadPaintings(reader);
        }
        else {
            reader->readUnknownElement();
        }
    }
}

void XmlFormat::loadView(Qucs::XmlReader *reader)
{
    SchematicScene *scene = schematicScene();
    if(!reader->isStartElement() || reader->name() != "view") {
        reader->raiseError(QObject::tr("Malformatted file"));
    }

    QRectF sceneRect;
    bool gridVisible = false;
    QSize gridSize;
    QString dataSet;
    QString dataDisplay;
    bool opensDataDisplay = false;
    bool frameVisible = false;
    QSize frameSize;
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
                frameSize = reader->readSize();

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
                            else {
                                reader->readUnknownElement();
                            }
                        }
                    }
                }
                reader->readFurther();
                Q_ASSERT(reader->isEndElement() && reader->name() == "frame");
            }
        }
    }

    if(!reader->hasError()) {
        scene->setSceneRect(sceneRect);
        scene->setGridVisible(gridVisible);
        scene->setGridSize(gridSize.width(), gridSize.height());
        scene->setSceneRect(sceneRect);
        scene->setDataSet(dataSet);
        scene->setDataDisplay(dataDisplay);
        scene->setOpensDataDisplay(opensDataDisplay);
        scene->setFrameVisible(frameVisible);
        scene->setFrameSize(frameSize.height(), frameSize.width());
        scene->setFrameTexts(frameTexts);
    }
}

void XmlFormat::loadComponents(Qucs::XmlReader *reader)
{
    SchematicScene *scene = schematicScene();
    if(!reader->isStartElement() || reader->name() != "components") {
        reader->raiseError(QObject::tr("Malformatted file"));
    }

    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isEndElement()) {
            Q_ASSERT(reader->name() == "components");
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "component") {
                Component::loadComponentData(reader,scene);
            }
            else {
                qWarning() << "Error: Found unknown component type" << reader->name().toString();
                reader->readUnknownElement();
                reader->raiseError(QObject::tr("Malformatted file"));
            }
        }
    }
}

void XmlFormat::loadWires(Qucs::XmlReader* reader)
{
    SchematicScene *scene = schematicScene();
    if(!reader->isStartElement() || reader->name() != "wires") {
        reader->raiseError(QObject::tr("Malformatted file"));
    }

    while(!reader->atEnd()) {
        reader->readFurther();

        if(reader->isEndElement()) {
            Q_ASSERT(reader->name() == "wires");
            break;
        }

        if(!reader->isStartElement() || reader->name() != "equipotential") {
            reader->raiseError(QObject::tr("Malformatted file")+reader->name().toString());
        }

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

void XmlFormat::loadPaintings(Qucs::XmlReader *reader)
{
    SchematicScene *scene = schematicScene();
    if(!reader->isStartElement() || reader->name() != "paintings") {
        reader->raiseError(QObject::tr("Malformatted file"));
    }

    while(!reader->atEnd()) {
        reader->readNext();

        if(reader->isEndElement()) {
            Q_ASSERT(reader->name() == "paintings");
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "painting") {
                Painting::loadPainting(reader,scene);
            }
            else {
                qWarning() << "Error: Found unknown painting type" <<
                    reader->name().toString();
                reader->readUnknownElement();
                reader->raiseError(QObject::tr("Malformatted file"));
            }
        }
    }
}
