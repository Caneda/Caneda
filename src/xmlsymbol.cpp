/***************************************************************************
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

#include "xmlsymbol.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "library.h"
#include "symboldocument.h"

#include "paintings/painting.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    //! Constructor
    XmlSymbol::XmlSymbol(SymbolDocument *doc) :
        m_symbolDocument(doc)
    {
        if(m_symbolDocument) {
            m_fileName = m_symbolDocument->fileName();
        }
        else {
            m_fileName = QString();
        }

        m_component = 0;
    }

    XmlSymbol::XmlSymbol(ComponentData *component) :
        m_component(component)
    {
        if(m_component) {
            m_fileName = m_component->filename;
        }
        else {
            m_fileName = QString();
        }

        m_symbolDocument = 0;
    }

    bool XmlSymbol::save()
    {
        if(!cGraphicsScene()) {
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

    //! \brief Parses the component data from file \a path.
    bool XmlSymbol::load()
    {
        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot open file %1").arg(fileName()));
            return false;
        }

        QTextStream stream(&file);
        bool result = loadFromText(stream.readAll());
        file.close();

        return result;
    }

    SymbolDocument* XmlSymbol::symbolDocument() const
    {
        return m_symbolDocument;
    }

    CGraphicsScene* XmlSymbol::cGraphicsScene() const
    {
        return m_symbolDocument ? m_symbolDocument->cGraphicsScene() : 0;
    }

    ComponentData* XmlSymbol::component() const
    {
        return m_component;
    }

    QString XmlSymbol::fileName() const
    {
        return m_fileName;
    }

    QString XmlSymbol::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        // Fist we start the document
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));

        // Write all view details
        writer->writeStartElement("component");
        QFileInfo info(fileName());
        writer->writeAttribute("name", info.baseName());
        writer->writeAttribute("version", Caneda::version());
        writer->writeAttribute("label", "comp");

        writer->writeStartElement("displaytext");
        writer->writeLocaleText("C", "User created component");
        // TODO: When available use this to save user defined displaytext
        // writer->writeLocaleText("C", scene->displayText());
        writer->writeEndElement(); //</displaytext>

        writer->writeStartElement("description");
        writer->writeLocaleText("C", "User created component based on user symbol");
        // TODO: When available use this to save user defined description
        // writer->writeLocaleText("C", scene->description());
        writer->writeEndElement(); //</description>

        // Write symbol geometry (drawing)
        writer->writeStartElement("symbol");

        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Painting*> paintings = filterItems<Painting>(items, RemoveItems);
        if(!paintings.isEmpty()) {
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
        }

        writer->writeEndElement(); //</symbol>

        // TODO Write ports
        writer->writeStartElement("ports");
        writer->writeEndElement(); //</ports>

        // TODO Write properties
        writer->writeStartElement("properties");
        writer->writeEndElement(); //</properties>

        // Finally we finish the document
        writer->writeEndDocument(); //</component>

        delete writer;
        return retVal;
    }

    /*!
     * \brief Reads symbol data from xml file text.
     *
     * \param text String containing xml data from component/symbol file.
     */
    bool XmlSymbol::loadFromText(const QString &text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());

        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement() && reader->name() == "component") {
                break;
            }
        }

        if(reader->isStartElement() && reader->name() == "component") {

            // Check version compatibility, get name and label
            QXmlStreamAttributes attributes = reader->attributes();
            Q_ASSERT(Caneda::checkVersion(attributes.value("version").toString()));

            // Check if we are opening the file for edition or to include it in a library
            if(cGraphicsScene()) {
                // We are opening the file for symbol edition
                // TODO: Implement this.
            }
            else if(component()) {
                // We are opening the file as a component to include it in a library
                component()->name = attributes.value("name").toString();
                component()->labelPrefix = attributes.value("label").toString();
            }


            // Read the component body
            while(!reader->atEnd()) {
                reader->readNext();

                if(reader->isEndElement()) {
                    break;
                }

                if(reader->isStartElement()) {

                    // Read display text
                    if(reader->name() == "displaytext") {
                        // Check if we are opening the file for edition or to include it in a library
                        if(cGraphicsScene()) {
                            // We are opening the file for symbol edition
                            // TODO: Implement this.
                            reader->readUnknownElement();
                        }
                        else if(component()) {
                            // We are opening the file as a component to include it in a library
                            component()->displayText = reader->readLocaleText(Caneda::localePrefix());
                        }
                    }


                    // Read description
                    else if(reader->name() == "description") {
                        // Check if we are opening the file for edition or to include it in a library
                        if(cGraphicsScene()) {
                            // We are opening the file for symbol edition
                            // TODO: Implement this.
                            reader->readUnknownElement();
                        }
                        else if(component()) {
                            // We are opening the file as a component to include it in a library
                            component()->description = reader->readLocaleText(Caneda::localePrefix());
                        }
                    }

                    // Read symbol
                    else if(reader->name() == "symbol") {
                        readSymbol(reader);
                    }

                    // Read ports
                    else if(reader->name() == "ports") {
                        readPorts(reader);
                    }

                    // Read properties
                    else if(reader->name() == "properties") {
                        readProperties(reader);
                    }
                }

            }
        }

        if(reader->hasError()) {
            qWarning() << "\nWarning: Failed to read data from\n" << fileName();
            QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
            delete reader;
            return false;
        }

        delete reader;
        return true;
    }

    /*!
     * \brief Read symbol section of component description xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readSymbol(Caneda::XmlReader *reader)
    {
        QPainterPath data;

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "painting") {

                // Check if we are opening the file for edition or to include it in a library
                if(cGraphicsScene()) {
                    // We are opening the file for symbol edition
                    Painting::loadPainting(reader, cGraphicsScene());
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    Painting *newSymbol = Painting::loadPainting(reader);
                    data.addPath(newSymbol->shapeForRect(newSymbol->paintingRect()));
                }

            }
        }


        // If we are opening the file as a component, register the recreated QPainterPath
        if(component()) {
            LibraryManager *libraryManager = LibraryManager::instance();
            libraryManager->registerComponent(component()->name, data);
        }
    }

    /*!
     * \brief Reads the ports data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readPorts(Caneda::XmlReader *reader)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "port") {
                QXmlStreamAttributes attribs = reader->attributes();
                qreal x = attribs.value("x").toString().toDouble();
                qreal y = attribs.value("y").toString().toDouble();
                QString portName = attribs.value("name").toString();

                // Check if we are opening the file for edition or to include it in a library
                if(cGraphicsScene()) {
                    // We are opening the file for symbol edition
                    // TODO: Implement this.
                    reader->readUnknownElement();
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    component()->ports << new PortData(QPointF(x, y), portName);
                }

                // Read until end of element
                reader->readUnknownElement();
            }
        }
    }

    /*!
     * \brief Reads component properties data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readProperties(Caneda::XmlReader *reader)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "property") {
                Property prop = PropertyFactory::createProperty(reader);

                // Check if we are opening the file for edition or to include it in a library
                if(cGraphicsScene()) {
                    // We are opening the file for symbol edition
                    // TODO: Implement this.
                    reader->readUnknownElement();
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    component()->propertyMap.insert(prop.name(), prop);
                }

            }
        }
    }

} // namespace Caneda
