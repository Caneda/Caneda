/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2012 by Pablo Daniel Pareja Obregon                  *
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

#include "formatxmlschematic.h"

#include "cgraphicsscene.h"
#include "component.h"
#include "global.h"
#include "port.h"
#include "schematicdocument.h"
#include "wire.h"
#include "xmlutilities.h"

#include "paintings/painting.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    //! Constructor
    FormatXmlSchematic::FormatXmlSchematic(SchematicDocument *doc):
        m_schematicDocument(doc)
    {
    }

    bool FormatXmlSchematic::save()
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

    bool FormatXmlSchematic::load()
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

    QString FormatXmlSchematic::saveText()
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

    void FormatXmlSchematic::saveSchematics(Caneda::XmlWriter *writer)
    {
        saveComponents(writer);
        saveWires(writer);
        savePaintings(writer);
    }

    void FormatXmlSchematic::saveComponents(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        QList<Component*> components = filterItems<Component>(items);
        if(!components.isEmpty()) {
            writer->writeStartElement("components");
            foreach(Component *c, components) {
                c->saveData(writer);
            }
            writer->writeEndElement(); //</components>
        }
    }

    void FormatXmlSchematic::saveWires(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Wire*> wires = filterItems<Wire>(items);

        if(wires.isEmpty()) {
            return;
        }

        int wireId = 0;
        int equiId = 0;
        QList<Wire*> parsedWires;

        writer->writeStartElement("wires");

        foreach(Wire *w, wires) {
            if(parsedWires.contains(w)) {
                continue;
            }

            QList<Wire*> equi;
            w->getConnectedWires(equi);

            writer->writeStartElement("equipotential");
            writer->writeAttribute("id", QString::number(equiId++));
            foreach(Wire *wire, equi) {
                wire->saveData(writer, wireId++);
            }
            writer->writeEndElement();

            parsedWires += equi;
        }

        writer->writeEndElement(); //</wires>

    }

    void FormatXmlSchematic::savePaintings(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        QList<Painting*> paintings = filterItems<Painting>(items);
        if(!paintings.isEmpty()) {
            writer->writeStartElement("paintings");
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</paintings>
        }
    }

    bool FormatXmlSchematic::loadFromText(const QString& text)
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

    void FormatXmlSchematic::loadSchematics(Caneda::XmlReader* reader)
    {
        if(reader->isStartElement()) {
            if(reader->name() == "components") {
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

    void FormatXmlSchematic::loadComponents(Caneda::XmlReader *reader)
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
                    Component::loadComponent(reader,scene);
                }
                else {
                    qWarning() << "Error: Found unknown component type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    void FormatXmlSchematic::loadWires(Caneda::XmlReader* reader)
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
                        Wire *w = Wire::loadWire(reader,scene);
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

    void FormatXmlSchematic::loadPaintings(Caneda::XmlReader *reader)
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

    SchematicDocument* FormatXmlSchematic::schematicDocument() const
    {
        return m_schematicDocument;
    }

    CGraphicsScene* FormatXmlSchematic::cGraphicsScene() const
    {
        return m_schematicDocument ? m_schematicDocument->cGraphicsScene() : 0;
    }

    QString FormatXmlSchematic::fileName() const
    {
        return m_schematicDocument ? m_schematicDocument->fileName() : QString();
    }

} // namespace Caneda
