/***************************************************************************
 * Copyright (C) 2010-2012 by Pablo Daniel Pareja Obregon                  *
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

#include "formatxmllayout.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "idocument.h"
#include "xmlutilities.h"

#include "paintings/painting.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    //! \brief Constructor.
    FormatXmlLayout::FormatXmlLayout(LayoutDocument *doc):
        m_layoutDocument(doc)
    {
    }

    bool FormatXmlLayout::save()
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

    bool FormatXmlLayout::load()
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

    QString FormatXmlLayout::saveText()
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
        savePaintings(writer);
        saveProperties(writer);

        //Finally we finish the document
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    void FormatXmlLayout::savePaintings(Caneda::XmlWriter *writer)
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

    /*!
     * \brief Save scene properties to an XmlWriter.
     *
     * This method saves all scene related propeties to an XmlWriter. To do so,
     * it takes each property from the PropertyGroup of the scene, and saves
     * the data using the Property::saveProperty() method.
     *
     * \param writer XmlWriter to save properties into.
     * \sa Property::saveProperty()
     */
    void FormatXmlLayout::saveProperties(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();

        writer->writeStartElement("properties");

        PropertyGroup *properties = scene->properties();
        foreach(Property property, properties->propertyMap()) {
            property.saveProperty(writer);
        }

        writer->writeEndElement(); //</properties>
    }

    bool FormatXmlLayout::loadFromText(const QString& text)
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

                        if(reader->isStartElement()) {
                            if(reader->name() == "paintings") {
                                loadPaintings(reader);
                            }
                            else if(reader->name() == "properties") {
                                loadProperties(reader);
                            }
                            else {
                                reader->readUnknownElement();
                            }
                        }

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

    void FormatXmlLayout::loadPaintings(Caneda::XmlReader *reader)
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

    void FormatXmlLayout::loadProperties(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "properties");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    Property prop = Property::loadProperty(reader);
                    scene->addProperty(prop);
                }
                else {
                    qWarning() << "Error: Found unknown property type" <<
                                  reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    LayoutDocument* FormatXmlLayout::layoutDocument() const
    {
        return m_layoutDocument;
    }

    CGraphicsScene* FormatXmlLayout::cGraphicsScene() const
    {
        return m_layoutDocument ? m_layoutDocument->cGraphicsScene() : 0;
    }

    QString FormatXmlLayout::fileName() const
    {
        return m_layoutDocument ? m_layoutDocument->fileName() : QString();
    }

} // namespace Caneda
