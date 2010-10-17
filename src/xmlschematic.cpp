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

#include "xmlschematic.h"

#include "cgraphicsitem.h"
#include "cgraphicsscene.h"
#include "component.h"
#include "global.h"
#include "port.h"
#include "schematicdocument.h"
#include "wire.h"

#include "paintings/painting.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    static void getConnectedWires(Wire *wire, QList<Wire*> &out)
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

    void writeEquiWires(Caneda::XmlWriter *writer, int id, int wireStartId,
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
    XmlSchematic::XmlSchematic(SchematicDocument *doc):
        m_schematicDocument(doc)
    {
    }

    bool XmlSchematic::save()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug("Looks buggy! Null data to save! Was this expected?");
        }

        QFile file(fileName());
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

    bool XmlSchematic::load()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ")+fileName());
            return false;
        }

        QTextStream stream(&file);

        bool result = loadFromText(stream.readAll());

        file.close();
        return result;
    }

    QString XmlSchematic::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        //Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        //Now we copy all the elements and properties in the schematic
        saveSchematics(writer);

        //Finally we finish the document
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    void XmlSchematic::saveSchematics(Caneda::XmlWriter *writer)
    {
        saveView(writer);
        saveComponents(writer);
        saveWires(writer);
        savePaintings(writer);
    }

    void XmlSchematic::saveView(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
        writer->writeStartElement("view");

        writer->writeStartElement("frame");
        writer->writeAttribute("visible", Caneda::boolToString(scene->isFrameVisible()));
        writer->writeSize(QSize(scene->frameWidth(), scene->frameHeight()), "size");
        writer->writeSize(QSize(scene->frameColumns(), scene->frameRows()), "geometry");
        writer->writeStartElement("frametexts");
        foreach(QString text, scene->frameTexts()) {
            writer->writeElement("text",text);
        }
        writer->writeEndElement(); //</frametexts>
        writer->writeEndElement(); //</frame>
        writer->writeEndElement(); //</view>
    }

    void XmlSchematic::saveComponents(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
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

    void XmlSchematic::saveWires(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
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

    void XmlSchematic::savePaintings(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
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

    bool XmlSchematic::loadFromText(const QString& text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement()) {
                if(reader->name() == "caneda" &&
                        Caneda::checkVersion(reader->attributes().value("version").toString())) {

                    while(!reader->atEnd()) {
                        reader->readNext();
                        if(reader->isEndElement()) {
                            Q_ASSERT(reader->name() == "caneda");
                            break;
                        }
                        loadSchematics(reader);
                    }
                }
                else {
                    reader->raiseError(QObject::tr("Not a caneda file or probably malformatted file"));
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

    void XmlSchematic::loadSchematics(Caneda::XmlReader* reader)
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

    void XmlSchematic::loadView(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!reader->isStartElement() || reader->name() != "view") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        bool frameVisible = false;
        QSize frameGeometry;
        QSize frameSize;
        QStringList frameTexts;

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "view");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "frame") {
                    QString att = reader->attributes().value("visible").toString();
                    att = att.toLower();
                    if(att != "true" && att != "false") {
                        reader->raiseError(QObject::tr("Invalid bool attribute"));
                        break;
                    }
                    frameVisible = (att == "true");

                    reader->readFurther();
                    if(reader->isStartElement() && reader->name() == "size") {
                        frameSize = reader->readSize();
                    }

                    reader->readFurther();
                    if(reader->isStartElement() && reader->name() == "geometry") {
                        frameGeometry = reader->readSize();
                    }

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
            scene->setFrameVisible(frameVisible);
            scene->setFrameSize(frameSize.width(), frameSize.height());
            scene->setFrameGeometry(frameGeometry.height(), frameGeometry.width());
            scene->setFrameTexts(frameTexts);
        }
    }

    void XmlSchematic::loadComponents(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
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

    void XmlSchematic::loadWires(Caneda::XmlReader* reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
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
                        w->checkAndConnect(Caneda::DontPushUndoCmd);
                    }
                    else {
                        reader->readUnknownElement();
                        reader->raiseError(QObject::tr("Malformatted file"));
                    }
                }
            }
        }
    }

    void XmlSchematic::loadPaintings(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
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

    SchematicDocument* XmlSchematic::schematicDocument() const
    {
        return m_schematicDocument;
    }

    CGraphicsScene* XmlSchematic::cGraphicsScene() const
    {
        return m_schematicDocument ? m_schematicDocument->cGraphicsScene() : 0;
    }

    QString XmlSchematic::fileName() const
    {
        return m_schematicDocument ? m_schematicDocument->fileName() : QString();
    }

} // namespace Caneda
