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

        // Check if we are reading a symbol or loading a component
        bool result;
        if(cGraphicsScene()) {
            result = loadSymbol(stream.readAll());
        }
        else if(component()) {
            result = loadComponent(stream.readAll());
        }
        else {
            result = false;
        }

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
        writer->writeAttribute("name", "userdefined");

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

    bool XmlSymbol::loadSymbol(const QString& text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());

        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement() && reader->name() == "component") {
                break;
            }
        }

        if(reader->isStartElement() && reader->name() == "component") {

            // Check version compatibility
            QXmlStreamAttributes attributes = reader->attributes();
            Q_ASSERT(Caneda::checkVersion(attributes.value("version").toString()));

            // Read the component body
            while(!reader->atEnd()) {
                reader->readNext();

                if(reader->isEndElement()) {
                    break;
                }

                if(reader->isStartElement()) {
                    // Read display text
                    if(reader->name() == "displaytext") {
                        // TODO: Implement this.
                        reader->readUnknownElement();
                    }

                    // Read description
                    else if(reader->name() == "description") {
                        // TODO: Implement this.
                        reader->readUnknownElement();
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
     * \brief Reads component data from xml file text.
     *
     * \param text String containing xml data from component/symbol file.
     */
    bool XmlSymbol::loadComponent(const QString &text)
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
            component()->name = attributes.value("name").toString();
            component()->labelPrefix = attributes.value("label").toString();

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
                    }

                    // Read description
                    else if(reader->name() == "description") {
                        component()->description = reader->readLocaleText(Caneda::localePrefix());
                    }

                    // Read symbol
                    else if(reader->name() == "symbol") {
                        readComponentSymbol(reader);
                    }

                    // Read ports
                    else if(reader->name() == "ports") {
                        readComponentPorts(reader);
                    }

                    // Read properties
                    else if(reader->name() == "properties") {
                        readComponentProperties(reader);
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
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "painting") {
                    Painting::loadPainting(reader, cGraphicsScene());
                }
            }
        }
    }

    /*!
     * \brief Reads the ports data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readPorts(Caneda::XmlReader *reader)
    {
        // TODO: Implement this.
        reader->readUnknownElement();
    }

    /*!
     * \brief Reads component properties data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readProperties(Caneda::XmlReader *reader)
    {
        // TODO: Implement this.
        reader->readUnknownElement();
    }

    /*!
     * \brief Read symbol section of component description xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readComponentSymbol(Caneda::XmlReader *reader)
    {
        // List of symbols and default value
        QStringList parsedSymbols;
        QString defaultSchematic =
            reader->attributes().value("default").toString();

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
                    if(!readSvg(reader)) {
                        reader->raiseError(QObject::tr("Malformatted file: found problems during geometry loading"));
                    }
                }
            }
        }

        // Check if default is present
        if(!defaultSchematic.isEmpty() && parsedSymbols.contains(defaultSchematic)) {
            // Add symbols to property list
            QString symbolDescription =
                QObject::tr("Represents the current symbol of component.");
            QVariant defValue(defaultSchematic);
            Property symb("symbol", symbolDescription, QVariant::String, false,
                    false, defValue, parsedSymbols);
            component()->propertyMap.insert("symbol", symb);
        }

    }

    /*!
     * \brief Reads the ports data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void XmlSymbol::readComponentPorts(Caneda::XmlReader *reader)
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

                component()->ports << new PortData(QPointF(x, y), portName);

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
    void XmlSymbol::readComponentProperties(Caneda::XmlReader *reader)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "property") {
                Property prop = PropertyFactory::createProperty(reader);
                component()->propertyMap.insert(prop.name(), prop);
            }
        }
    }

    /*!
     * \brief Reads the schematic tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    bool XmlSymbol::readSvg(Caneda::XmlReader *reader)
    {
        QString schName = reader->attributes().value("name").toString();
        QString schType = reader->attributes().value("href").toString();

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

            // Read until end of element
            reader->readUnknownElement();
        }

        return true;
    }

} // namespace Caneda
