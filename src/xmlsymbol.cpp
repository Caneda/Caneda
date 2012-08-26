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

#include "xmlutilities/transformers.h"
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

    bool XmlSymbol::loadSymbol()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ") + fileName());
            return false;
        }

        QTextStream stream(&file);
        bool result = loadFromText(stream.readAll());
        file.close();

        return result;
    }

    //! \brief Parses the component data from file \a path.
    bool XmlSymbol::loadComponent()
    {
        QFile file(QFileInfo(fileName()).absoluteFilePath());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("File open"),
                    QObject::tr("Cannot open file %1").arg(component()->filename));
            return false;
        }

        QTextStream stream(&file);
        Caneda::XmlReader *reader = new Caneda::XmlReader(stream.readAll().toUtf8());
        file.close();

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement() && reader->name() == "component") {
                break;
            }
        }

        bool readok = false;

        if(reader->isStartElement() && reader->name() == "component") {
            readok = readComponentData(reader);

            if(reader->hasError() || !readok) {
                qWarning() << "\nWarning: Failed to read data from\n" << QFileInfo(fileName()).absolutePath();
                readok = false;
            }
        }

        return !reader->hasError() && readok;
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

        writer->writeStartElement("symbol");
        writer->writeAttribute("name", "userdefined");


        // Now we copy all the elements and properties in the schematic
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Painting*> paintings = filterItems<Painting>(items, RemoveItems);
        if(!paintings.isEmpty()) {
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
        }
        // Finally we finish the document
        writer->writeEndElement(); //</symbol>

        // TODO Write properties
        writer->writeStartElement("properties");
        writer->writeEndElement(); //</properties>

        writer->writeEndDocument(); //</component>

        delete writer;
        return retVal;
    }

    bool XmlSymbol::loadFromText(const QString& text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement() && reader->name() == "component" &&
                    Caneda::checkVersion(reader->attributes().value("version").toString())) {

                while(!reader->atEnd()) {
                    reader->readNext();
                    if(reader->isEndElement()) {
                        Q_ASSERT(reader->name() == "component");
                        break;
                    }
                    reader->readNext();

                    if(reader->name() == "displaytext") {
                        // TODO: Implement this.
                        reader->readUnknownElement();
                    }
                    else if(reader->name() == "description") {
                        // TODO: Implement this.
                        reader->readUnknownElement();
                    }
                    else if(reader->name() == "symbol") {

                        while(!reader->atEnd()) {
                            reader->readNext();

                            if(reader->isEndElement()) {
                                Q_ASSERT(reader->name() == "symbol");
                                break;
                            }

                            if(reader->isStartElement()) {
                                if(reader->name() == "painting") {
                                    Painting::loadPainting(reader, cGraphicsScene());
                                }
                                else {
                                    qWarning() << "Error: Found unknown painting type" <<
                                                  reader->name().toString();
                                    reader->readUnknownElement();
                                    reader->raiseError(QObject::tr("Malformatted file: found unknown painting type"));
                                }
                            }
                        }

                    }
                    else if(reader->name() == "properties") {
                        // TODO: Implement this.
                        reader->readUnknownElement();
                    }
                    else {
                        reader->readUnknownElement();
                    }
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

    /*!
     * \brief Reads component data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    bool XmlSymbol::readComponentData(Caneda::XmlReader *reader)
    {
        QXmlStreamAttributes attributes = reader->attributes();

        Q_ASSERT(reader->isStartElement() && reader->name() == "component");

        // Check version compatibility first.
        Q_ASSERT(Caneda::checkVersion(attributes.value("version").toString()));

        // Get name
        component()->name = attributes.value("name").toString();
        Q_ASSERT(!component()->name.isEmpty());

        // Get label
        component()->labelPrefix = attributes.value("label").toString();
        Q_ASSERT(!component()->labelPrefix.isEmpty());

        // Read the component body
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                // Read display text
                if(reader->name() == "displaytext") {
                    component()->displayText = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement());
                }

                // Read description
                else if(reader->name() == "description") {
                    component()->description = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement());
                }

                // Read symbol
                else if(reader->name() == "symbol") {
                    if(readSymbol(reader)==false) {
                        return false;
                    }
                }

                // Read properties
                else if(reader->name() == "properties") {
                    readProperties(reader);
                }

                // Read unknown element
                else {
                    reader->readUnknownElement();
                }
            }
        }

        if(reader->hasError()) {
            return false;
        }
        return true;
    }

    /*!
     * \brief Read symbol section of component description xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    bool XmlSymbol::readSymbol(Caneda::XmlReader *reader)
    {
        // List of symbols and default value
        QStringList parsedSymbols;
        QString defaultSchematic =
            reader->attributes().value("default").toString();

        Q_ASSERT(reader->isStartElement() && reader->name() == "symbol");
        Q_ASSERT(!defaultSchematic.isEmpty());

        // Read all available symbols
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "schematic") {
                    QString schName =
                        reader->attributes().value("name").toString();

                    Q_ASSERT(!schName.isEmpty());

                    parsedSymbols << schName;
                    if(!readSchematic(reader)) {
                        return false;
                    }
                }
                else {
                    Q_ASSERT(!sizeof("Unknown element in schematics element"));
                }
            }
        }

        // Check if default is present
        Q_ASSERT(parsedSymbols.contains(defaultSchematic));

        // Add symbols to property list
        QString symbolDescription =
            QObject::tr("Represents the current symbol of component.");
        QVariant defValue(defaultSchematic);
        Q_ASSERT(defValue.convert(QVariant::String));
        Property symb("symbol", symbolDescription, QVariant::String, false,
                false, defValue, parsedSymbols);
        component()->propertyMap.insert("symbol", symb);

        return true;
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

            else if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    Property prop = PropertyFactory::createProperty(reader);
                    component()->propertyMap.insert(prop.name(), prop);
                }
                else {
                    Q_ASSERT(!sizeof("Found unknown element in properties section"));
                }
            }
        }
    }

    /*!
     * \brief Reads the schematic tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    bool XmlSymbol::readSchematic(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "schematic");

        QString schName = reader->attributes().value("name").toString();
        QString schType = reader->attributes().value("href").toString();
        bool readok;

        // Read svg file
        if(!schType.isEmpty()) {
            QString parentPath = QFileInfo(fileName()).absolutePath();
            QFile svgFile(parentPath + "/" + schType);
            if(!svgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return false;
            }

            QByteArray svgContent(svgFile.readAll());
            if(svgContent.isEmpty()) {
                return false;
            }

            readok = readSchematicSvg(svgContent, schName);
            if(!readok) {
                return false;
            }
        }

        // Read component ports
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "port") {
                readSchematicPort(reader, schName);
            }
            else {
                Q_ASSERT(!sizeof("Unknown element in schematic element"));
            }
        }
        return true;
    }

    /*!
     * \brief Reads the schematic port tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param schName Schematic name
     */
    void XmlSymbol::readSchematicPort(Caneda::XmlReader *reader, const QString & schName)
    {
        QXmlStreamAttributes attribs = reader->attributes();
        qreal x = attribs.value("x").toString().toDouble();
        qreal y = attribs.value("y").toString().toDouble();
        QString portName = attribs.value("name").toString();

        component()->schematicPortMap[schName] <<
            new PortData(QPointF(x, y), portName);

        // Read until end element as all data we require is already obtained.
        reader->readUnknownElement();
    }

    /*!
     * \brief Read an svg schematic
     *
     * \param svgContent svg content as utf8
     * \param schName Schematic name
     */
    bool XmlSymbol::readSchematicSvg(const QByteArray &svgContent,
                                     const QString &schName)
    {
        // Process using xslt
        Caneda::QXmlStreamReaderExt QXmlSvg(svgContent, 0,
                Caneda::transformers::defaultInstance()->componentsvg());

        LibraryManager *libraryManager = LibraryManager::instance();
        QString symbolId = component()->name + "/" + schName;
        libraryManager->registerComponent(symbolId, QXmlSvg.constData());
        if(QXmlSvg.hasError()) {
            qWarning() << "Could not read svg file" << schName << ": " << QXmlSvg.errorString();
            return false;
        }

        return true;;
    }

} // namespace Caneda
