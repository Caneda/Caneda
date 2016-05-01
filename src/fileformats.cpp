/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "fileformats.h"

#include "component.h"
#include "chartitem.h"
#include "chartscene.h"
#include "global.h"
#include "graphicsscene.h"
#include "idocument.h"
#include "library.h"
#include "painting.h"
#include "port.h"
#include "portsymbol.h"
#include "wire.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QString>

namespace Caneda
{
    /*************************************************************************
     *                         FormatXmlSchematic                            *
     *************************************************************************/
    //! \brief Constructor.
    FormatXmlSchematic::FormatXmlSchematic(SchematicDocument *doc):
        m_schematicDocument(doc)
    {
    }

    /*!
     * \brief Saves current scene data to an xml file.
     *
     * This method checks the file to be written is accessible and that the
     * user has the correct permissions to write it, and then calls the
     * saveText() method to generate the xml data to save.
     *
     * \sa saveText(), load()
     */
    bool FormatXmlSchematic::save()
    {
        if(!graphicsScene()) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
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

    /*!
     * \brief Loads current scene data to an xml file.
     *
     * This method checks the file to be read is accessible and that the
     * user has the correct permissions to read it, and then calls the
     * loadFromText() method to read the xml data into the scene.
     *
     * \sa loadFromText(), save()
     */
    bool FormatXmlSchematic::load()
    {
        GraphicsScene *scene = graphicsScene();
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

    /*!
     * \brief Saves an xml file description into a QString, obtaining the data
     * from a scene and associated objects (componts, paintings, etc).
     *
     * This method is used to generate an xml file into a QString to be saved
     * by the save() method. Not only scene sections are created (components,
     * paintings, etc) but also file header information, for example document
     * version and name. Each section is created, in its turn, by calling an
     * appropiated method an thus improving source code readability by
     * splitting the different actions.
     *
     * \return QString containing xml data to be saved.
     *
     * \sa save()
     */
    QString FormatXmlSchematic::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        // Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        // Now we copy all the elements and properties in the schematic
        saveComponents(writer);
        savePorts(writer);
        saveWires(writer);
        savePaintings(writer);

        // Finally we finish the document
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    /*!
     * \brief Saves the scene components to an XmlWriter.
     *
     * This method saves all scene components to an XmlWriter. To do so, it
     * takes each Component from the scene, and saves the data using the
     * Component::saveData() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa Component::saveData()
     */
    void FormatXmlSchematic::saveComponents(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<Component*> components = filterItems<Component>(items);

        if(!components.isEmpty()) {
            writer->writeStartElement("components");
            foreach(Component *c, components) {
                c->saveData(writer);
            }
            writer->writeEndElement(); //</components>
        }
    }

    /*!
     * \brief Saves the scene ports to an XmlWriter.
     *
     * This method saves all scene ports to an XmlWriter. To do so, it takes
     * each PortSymbol from the scene, and saves the data using the
     * PortSymbol::saveData() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa PortSymbol::saveData()
     */
    void FormatXmlSchematic::savePorts(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);

        if(!portSymbols.isEmpty()) {
            writer->writeStartElement("ports");
            foreach(PortSymbol *p, portSymbols) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</ports>
        }
    }

    /*!
     * \brief Saves the scene wires to an XmlWriter.
     *
     * This method saves all scene wires to an XmlWriter. To do so, it takes
     * each Wire from the scene, and saves the data using the Wire::saveData()
     * method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa Wire::saveData()
     */
    void FormatXmlSchematic::saveWires(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<Wire*> wires = filterItems<Wire>(items);

        if(!wires.isEmpty()) {
            writer->writeStartElement("wires");
            foreach(Wire *w, wires) {
                w->saveData(writer);
            }
            writer->writeEndElement(); //</wires>
        }
    }

    /*!
     * \brief Saves the scene paintings to an XmlWriter.
     *
     * This method saves all scene paintings to an XmlWriter. To do so, it
     * takes each Painting from the scene, and saves the data using the
     * GraphicsItem::saveData() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa GraphicsItem::saveData()
     */
    void FormatXmlSchematic::savePaintings(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
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
     * \brief Reads an xml file and constructs a scene and associated
     * objects (componts, paintings, etc) from the data read.
     *
     * \param text String containing xml data to be read.
     */
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

                        if(reader->isStartElement()) {
                            if(reader->name() == "components") {
                                loadComponents(reader);
                            }
                            else if(reader->name() == "ports") {
                                loadPorts(reader);
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

    /*!
     * \brief Reads the components section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSchematic::loadComponents(Caneda::XmlReader *reader)
    {
        GraphicsScene *scene = graphicsScene();
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
                    Component *component = new Component(scene);
                    component->loadData(reader);
                    scene->connectItems(component);
                }
                else {
                    qWarning() << "Error: Found unknown component type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    /*!
     * \brief Reads the ports section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSchematic::loadPorts(Caneda::XmlReader *reader)
    {
        GraphicsScene *scene = graphicsScene();
        if(!reader->isStartElement() || reader->name() != "ports") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "ports");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "port") {
                    PortSymbol *portSymbol = new PortSymbol(scene);
                    portSymbol->loadData(reader);
                    scene->connectItems(portSymbol);
                }
                else {
                    qWarning() << "Error: Found unknown port type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    /*!
     * \brief Reads the wires section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSchematic::loadWires(Caneda::XmlReader* reader)
    {
        GraphicsScene *scene = graphicsScene();
        if(!reader->isStartElement() || reader->name() != "wires") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "wires");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "wire") {
                    Wire *wire = new Wire(QPointF(10,10), QPointF(50,50), scene);
                    wire->loadData(reader);
                    scene->connectItems(wire);
                }
                else {
                    qWarning() << "Error: Found unknown wire type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    /*!
     * \brief Reads the paintings section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSchematic::loadPaintings(Caneda::XmlReader *reader)
    {
        GraphicsScene *scene = graphicsScene();
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
                    QString name = reader->attributes().value("name").toString();
                    Painting *painting = Painting::fromName(name);
                    painting->loadData(reader);
                    scene->addItem(painting);
                }
                else {
                    qWarning() << "Error: Found unknown painting type" << reader->name().toString();
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

    GraphicsScene* FormatXmlSchematic::graphicsScene() const
    {
        return m_schematicDocument ? m_schematicDocument->graphicsScene() : 0;
    }

    QString FormatXmlSchematic::fileName() const
    {
        return m_schematicDocument ? m_schematicDocument->fileName() : QString();
    }


    /*************************************************************************
     *                           FormatXmlSymbol                             *
     *************************************************************************/
    //! \brief Constructor.
    FormatXmlSymbol::FormatXmlSymbol(SymbolDocument *doc) :
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

    //! \brief Constructor.
    FormatXmlSymbol::FormatXmlSymbol(ComponentData *component) :
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

    /*!
     * \brief Saves current scene data to an xml file.
     *
     * This method checks the file to be written is accessible and that the
     * user has the correct permissions to write it, and then calls the
     * saveText() method to generate the xml data to save.
     *
     * \sa saveText(), load()
     */
    bool FormatXmlSymbol::save()
    {
        if(!graphicsScene()) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
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

    /*!
     * \brief Loads current scene data to an xml file.
     *
     * This method checks the file to be read is accessible and that the
     * user has the correct permissions to read it, and then calls the
     * loadFromText() method to read the xml data into the scene.
     *
     * \sa loadFromText(), save()
     */
    bool FormatXmlSymbol::load()
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

    SymbolDocument* FormatXmlSymbol::symbolDocument() const
    {
        return m_symbolDocument;
    }

    GraphicsScene* FormatXmlSymbol::graphicsScene() const
    {
        return m_symbolDocument ? m_symbolDocument->graphicsScene() : 0;
    }

    ComponentData* FormatXmlSymbol::component() const
    {
        return m_component;
    }

    QString FormatXmlSymbol::fileName() const
    {
        return m_fileName;
    }

    /*!
     * \brief Saves an xml file description into a QString, obtaining the data
     * from a scene and associated objects (componts, paintings, etc).
     *
     * This method is used to generate an xml file into a QString to be saved
     * by the save() method. Not only scene sections are created (components,
     * paintings, etc) but also file header information, for example document
     * version and name. Each section is created, in its turn, by calling an
     * appropiated method an thus improving source code readability by
     * splitting the different actions.
     *
     * \return QString containing xml data to be saved.
     *
     * \sa save()
     */
    QString FormatXmlSymbol::saveText()
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
        writer->writeAttribute("label", "X");

        writer->writeStartElement("displaytext");
        writer->writeLocaleText("C", "User created component");
        /*!
         * \todo When available use this to save user defined displaytext
         * writer->writeLocaleText("C", scene->displayText());
         */
        writer->writeEndElement(); //</displaytext>

        writer->writeStartElement("description");
        writer->writeLocaleText("C", "User created component based on user symbol");
        /*!
         * \todo When available use this to save user defined description
         * writer->writeLocaleText("C", scene->description());
         */
        writer->writeEndElement(); //</description>

        // Write symbol geometry (drawing), ports, properties and models
        saveSymbol(writer);
        savePorts(writer);
        saveProperties(writer);
        saveModels(writer);

        // Finally we finish the document
        writer->writeEndDocument(); //</component>

        delete writer;
        return retVal;
    }

    /*!
     * \brief Saves the scene paintings to an XmlWriter.
     *
     * This method saves all scene paintings to an XmlWriter. To do so, it
     * takes each Painting from the scene, and saves the data using the
     * GraphicsItem::saveData() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa GraphicsItem::saveData()
     */
    void FormatXmlSymbol::saveSymbol(XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<Painting*> paintings = filterItems<Painting>(items);

        if(!paintings.isEmpty()) {
            writer->writeStartElement("symbol");
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</symbol>
        }
    }

    /*!
     * \brief Saves the scene ports to an XmlWriter.
     *
     * This method saves all scene ports to an XmlWriter. To do so, it takes
     * each PortSymbol from the scene, and saves the data using the
     * PortSymbol::saveData() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa PortSymbol::saveData()
     */
    void FormatXmlSymbol::savePorts(XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);

        if(!portSymbols.isEmpty()) {
            writer->writeStartElement("ports");
            foreach(PortSymbol *p, portSymbols) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</ports>
        }
    }

    /*!
     * \brief Saves the scene properties to an XmlWriter.
     *
     * This method saves all scene related propeties to an XmlWriter. To do so,
     * it takes each property from the PropertyGroup of the scene, and saves
     * the data using the Property::saveProperty() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa Property::saveProperty()
     */
    void FormatXmlSymbol::saveProperties(XmlWriter *writer)
    {
        PropertyGroup *properties = graphicsScene()->properties();

        if(!properties->propertyMap().isEmpty()) {
            writer->writeStartElement("properties");
            writer->writePointAttribute(properties->pos(), "pos");
            foreach(Property property, properties->propertyMap()) {
                property.saveProperty(writer);
            }
            writer->writeEndElement(); //</properties>
        }
    }

    /*!
     * \brief Saves spice model data to xml file text.
     *
     * When editing a schematic's symbol, the spice model should be
     * automatically generated from the symbol properties. Generally
     * speaking, the model should have the following syntax:
     *
     * \code
     * X%label %port{1} %port{2} ... %port{n} modelname property_1=%property{property_1} property_2=%property{property_2} ... property_m=%property{property_m}
     * %subcircuit{modelname  %port{1} %port{2} ... %port{n} property_1=0 property_2=0 ... property_m=0
     * %n.include %librarypath/modelname.net}
     * %generateNetlist
     * \endcode
     *
     * XmlWriter doesn't allow including new lines, so the resulting
     * syntax will be appended to the writer in only one line. This,
     * however, doesn't affect the results.
     *
     * \param reader XmlWriter responsible for writing xml data.
     */
    void FormatXmlSymbol::saveModels(XmlWriter *writer)
    {
        GraphicsScene *scene = graphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        PropertyGroup *properties = scene->properties();
        QFileInfo info(fileName());

        // Generate the spice model syntax
        QString syntax = "X%label";

        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);
        if(!portSymbols.isEmpty()) {
            foreach(PortSymbol *p, portSymbols) {
                syntax.append(" %port{" + p->label() + "}");
            }
        }

        syntax.append(" " + info.baseName());

        foreach(Property property, properties->propertyMap()) {
            syntax.append(" " + property.name() + "=%property{" + property.name() + "}");
        }

        syntax.append(" %subcircuit{" + info.baseName());
        if(!portSymbols.isEmpty()) {
            foreach(PortSymbol *p, portSymbols) {
                syntax.append(" " + p->label());
            }
        }

        foreach(Property property, properties->propertyMap()) {
            syntax.append(" " + property.name() + "=0");
        }

        syntax.append(" %n.include %librarypath/" + info.baseName() + ".net}");
        syntax.append(" %generateNetlist");

        // Write the result to the XmlWriter
        writer->writeStartElement("models");

        writer->writeEmptyElement("model");
        writer->writeAttribute("type", "spice");
        writer->writeAttribute("syntax", syntax);

        writer->writeEndElement(); // </models>
    }

    /*!
     * \brief Reads an xml file and constructs a scene and associated
     * objects (componts, paintings, etc) from the data read.
     *
     * \param text String containing xml data to be read.
     */
    bool FormatXmlSymbol::loadFromText(const QString &text)
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
            if(graphicsScene()) {
                // We are opening the file for symbol edition
                //! \todo Implement this.
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
                        if(graphicsScene()) {
                            // We are opening the file for symbol edition
                            //! \todo Implement this.
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
                        if(graphicsScene()) {
                            // We are opening the file for symbol edition
                            //! \todo Implement this.
                            reader->readUnknownElement();
                        }
                        else if(component()) {
                            // We are opening the file as a component to include it in a library
                            component()->description = reader->readLocaleText(Caneda::localePrefix());
                        }
                    }

                    // Read symbol
                    else if(reader->name() == "symbol") {
                        loadSymbol(reader);
                    }

                    // Read ports
                    else if(reader->name() == "ports") {
                        loadPorts(reader);
                    }

                    // Read properties
                    else if(reader->name() == "properties") {
                        loadProperties(reader);
                    }

                    // Read models
                    else if(reader->name() == "models") {
                        loadModels(reader);
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
     * \brief Reads the symbol section of an xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::loadSymbol(Caneda::XmlReader *reader)
    {
        QPainterPath data;

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "painting") {

                // Check if we are opening the file for edition or to include it in a library
                if(graphicsScene()) {
                    // We are opening the file for symbol edition
                    QString name = reader->attributes().value("name").toString();
                    Painting *painting = Painting::fromName(name);
                    painting->loadData(reader);
                    graphicsScene()->addItem(painting);
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    QString name = reader->attributes().value("name").toString();
                    Painting *newSymbol = Painting::fromName(name);
                    newSymbol->loadData(reader);

                    QRectF rect = newSymbol->paintingRect();
                    rect.moveTo(newSymbol->pos());
                    data.addPath(newSymbol->shapeForRect(rect));
                }

            }
        }

        // If we are opening the file as a component, register the recreated QPainterPath
        if(component()) {
            LibraryManager *libraryManager = LibraryManager::instance();
            libraryManager->registerComponent(component()->name, component()->library, data);
        }
    }

    /*!
     * \brief Reads the ports section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::loadPorts(Caneda::XmlReader *reader)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "port") {
                // Check if we are opening the file for edition or to include it in a library
                if(graphicsScene()) {
                    // We are opening the file for symbol edition
                    PortSymbol *portSymbol = new PortSymbol(graphicsScene());
                    portSymbol->loadData(reader);
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    QPointF pos = reader->readPointAttribute("pos");
                    QString portName = reader->attributes().value("name").toString();
                    component()->ports << new PortData(pos, portName);

                    // Read until end of element
                    reader->readUnknownElement();
                }

            }
        }
    }

    /*!
     * \brief Reads the properties section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::loadProperties(Caneda::XmlReader *reader)
    {
        GraphicsScene *scene = graphicsScene();

        // If we are opening the file for symbol edition, read and set the
        // properties position in the scene
        if(scene) {
            PropertyGroup *properties = scene->properties();
            properties->setPos(reader->readPointAttribute("pos"));
        }

        // Read every individual property
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "properties");
                break;
            }

            if(reader->isStartElement() && reader->name() == "property") {
                Property prop = Property::loadProperty(reader);

                // Check if we are opening the file for edition or to include it in a library
                if(scene) {
                    // We are opening the file for symbol edition
                    scene->addProperty(prop);
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    component()->properties->addProperty(prop.name(), prop);
                }

            }
        }

    }

    /*!
     * \brief Reads the models section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::loadModels(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "models");

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "model") {
                    QXmlStreamAttributes attribs(reader->attributes());
                    QString modelType = attribs.value("type").toString();
                    QString modelSyntax = attribs.value("syntax").toString();

                    // Check if we are opening the file for edition or to include it in a library
                    GraphicsScene *scene = graphicsScene();
                    if(scene) {
                        // We are opening the file for symbol edition
                        //! \todo We must add the model as a special property, or allow some form of editing the model
                        // scene->addProperty(prop);
                    }
                    else if(component()) {
                        // We are opening the file as a component to include it in a library
                        component()->models.insert(modelType, modelSyntax);
                    }

                    // Read till end element
                    reader->readUnknownElement();
            }
        }

    }


    /*************************************************************************
     *                           FormatXmlLayout                             *
     *************************************************************************/
    //! \brief Constructor.
    FormatXmlLayout::FormatXmlLayout(LayoutDocument *doc):
        m_layoutDocument(doc)
    {
    }

    /*!
     * \brief Saves current scene data to an xml file.
     *
     * This method checks the file to be written is accessible and that the
     * user has the correct permissions to write it, and then calls the
     * saveText() method to generate the xml data to save.
     *
     * \sa saveText(), load()
     */
    bool FormatXmlLayout::save()
    {
        if(!graphicsScene()) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
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

    /*!
     * \brief Loads current scene data to an xml file.
     *
     * This method checks the file to be read is accessible and that the
     * user has the correct permissions to read it, and then calls the
     * loadFromText() method to read the xml data into the scene.
     *
     * \sa loadFromText(), save()
     */
    bool FormatXmlLayout::load()
    {
        GraphicsScene *scene = graphicsScene();
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

    /*!
     * \brief Saves an xml file description into a QString, obtaining the data
     * from a scene and associated objects (componts, paintings, etc).
     *
     * This method is used to generate an xml file into a QString to be saved
     * by the save() method. Not only scene sections are created (components,
     * paintings, etc) but also file header information, for example document
     * version and name. Each section is created, in its turn, by calling an
     * appropiated method an thus improving source code readability by
     * splitting the different actions.
     *
     * \return QString containing xml data to be saved.
     *
     * \sa save()
     */
    QString FormatXmlLayout::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        // Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        // Now we copy all the elements and properties in the schematic
        savePaintings(writer);

        // Finally we finish the document
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    /*!
     * \brief Saves the scene paintings to an XmlWriter.
     *
     * This method saves all scene paintings to an XmlWriter. To do so, it
     * takes each Painting from the scene, and saves the data using the
     * GraphicsItem::saveData() method.
     *
     * \param writer XmlWriter responsible for writing the xml data.
     *
     * \sa GraphicsItem::saveData()
     */
    void FormatXmlLayout::savePaintings(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
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
     * \brief Reads an xml file and constructs a scene and associated
     * objects (componts, paintings, etc) from the data read.
     *
     * \param text String containing xml data to be read.
     */
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

    /*!
     * \brief Reads the paintings section of an xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlLayout::loadPaintings(Caneda::XmlReader *reader)
    {
        GraphicsScene *scene = graphicsScene();
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
                    QString name = reader->attributes().value("name").toString();
                    Painting *painting = Painting::fromName(name);
                    painting->loadData(reader);
                    scene->addItem(painting);
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

    LayoutDocument* FormatXmlLayout::layoutDocument() const
    {
        return m_layoutDocument;
    }

    GraphicsScene* FormatXmlLayout::graphicsScene() const
    {
        return m_layoutDocument ? m_layoutDocument->graphicsScene() : 0;
    }

    QString FormatXmlLayout::fileName() const
    {
        return m_layoutDocument ? m_layoutDocument->fileName() : QString();
    }


    /*************************************************************************
     *                             FormatSpice                               *
     *************************************************************************/
    //! \brief Constructor.
    FormatSpice::FormatSpice(SchematicDocument *doc) :
        m_schematicDocument(doc)
    {
    }

    bool FormatSpice::save()
    {
        GraphicsScene *scene = graphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }

        QString text = generateNetlist();
        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
        }

        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    SchematicDocument *FormatSpice::schematicDocument() const
    {
        return m_schematicDocument;
    }

    GraphicsScene *FormatSpice::graphicsScene() const
    {
        return m_schematicDocument ? m_schematicDocument->graphicsScene() : 0;
    }

    QString FormatSpice::fileName() const
    {
        if(m_schematicDocument) {
            QFileInfo info(m_schematicDocument->fileName());
            QString baseName = info.completeBaseName();
            QString path = info.path();

            return path + "/" + baseName + ".net";
        }

        return QString();
    }

    /*!
     *  \brief Generate netlist
     *
     *  Iterate over all components, saving to a string the schematic netlist
     *  according to the model provided as a set of rules. In order to do so,
     *  the netlist topology must also be created, that is the connections
     *  between the multiple components must be determined and numbered to be
     *  used for the spice netlist. The set of rules used for generating the
     *  netlist from the model is specified in \ref ModelsFormat.
     *
     *  \sa generateNetlistTopology(), \ref ModelsFormat
     */
    QString FormatSpice::generateNetlist()
    {
        LibraryManager *libraryManager = LibraryManager::instance();
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<Component*> components = filterItems<Component>(items);
        PortsNetlist netlist = generateNetlistTopology();

        QStringList modelsList;
        QStringList subcircuitsList;
        QStringList directivesList;
        QStringList schematicsList;

        // Start the document and write the header
        QString retVal;
        retVal.append("* Spice automatic export. Generated by Caneda.\n");
        retVal.append("\n* Spice netlist.\n");

        // Copy all the elements and properties in the schematic by
        // iterating over all schematic components.
        // *Note*: the parsing order is important to allow, for example
        // cascadable commands and if control statements correct extraction.
        foreach(Component *c, components) {

            // Get the spice model (multiple models may be available)
            QString model = c->model("spice");

            // ************************************************************
            // Parse and replace the simple commands (e.g. label)
            // ************************************************************
            model.replace("%label", c->label());
            model.replace("%n", "\n");

            QString path = libraryManager->library(c->library())->libraryPath();
            model.replace("%librarypath", path);

            path = QFileInfo(m_schematicDocument->fileName()).absolutePath();
            model.replace("%filepath", path);

            // ************************************************************
            // Parse and replace the commands with parameters
            // ************************************************************
            QStringList commands;
            QRegularExpression re;
            QRegularExpressionMatchIterator it;

            // ************************************************************
            // First parse the cascadable commands (e.g. properties)
            // ************************************************************
            commands.clear();
            re.setPattern("(%\\w+\{([\\w+-]+)})");
            it = re.globalMatch(model);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                commands << match.captured(0);
            }

            // For each command replace the parameter with the correct value
            for(int i=0; i<commands.size(); i++){

                // Extract the parameters, removing the comand (including the
                // "{" and the last character "}")
                QString parameter = commands.at(i);
                parameter.remove(QRegularExpression("(%\\w+\{)")).chop(1);

                if(commands.at(i).startsWith("%port")){
                    foreach(Port *_port, c->ports()) {
                        if(_port->name() == parameter) {
                            // Found the port, now look for its netlist name
                            for(int j = 0; j < netlist.size(); ++j) {
                                if(netlist.at(j).first == _port) {
                                    model.replace(commands.at(i), netlist.at(j).second);
                                }
                            }
                        }
                    }
                }
                else if(commands.at(i).startsWith("%property")){
                    model.replace(commands.at(i), c->properties()->propertyValue(parameter));
                }
            }

            // ************************************************************
            // Parse if control statements
            // ************************************************************
            commands.clear();
            re.setPattern("(%\\w+\{([\\w =+-,]*)})");
            it = re.globalMatch(model);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                commands << match.captured(0);
            }

            // For each command replace the parameter with the correct value
            for(int i=0; i<commands.size(); i++){

                // Extract the parameters, removing the comand (including the
                // "{" and the last character "}")
                QString parameter = commands.at(i);
                parameter.remove(QRegularExpression("(%\\w+\{)")).chop(1);
                QStringList controlStrings = parameter.split(",");

                if(commands.at(i).startsWith("%if")){
                    if(!controlStrings.at(0).isEmpty() && controlStrings.size() > 1) {
                        model.replace(commands.at(i), controlStrings.at(1));
                    }
                    else {
                        model.remove(commands.at(i));
                    }
                }
            }

            // ************************************************************
            // Now parse the non-cascadable commands (e.g. models), which may
            // have a cascadable command as an argument (e.g. properties).
            // ************************************************************
            commands.clear();
            re.setPattern("(%\\w+\{([\\w =+-\\\\(\\\\)\\n\\*\{}]+)})");
            it = re.globalMatch(model);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                commands << match.captured(0);
            }

            // For each command replace the parameter with the correct value
            for(int i=0; i<commands.size(); i++){

                // Extract the parameters, removing the comand (including the
                // "{" and the last character "}")
                QString parameter = commands.at(i);
                parameter.remove(QRegularExpression("(%\\w+\{)")).chop(1);

                if(commands.at(i).startsWith("%model")){

                    // Models should be added to a temporal list to be included
                    // only once at the end of the spice file.
                    if(!modelsList.contains(parameter)) {
                        modelsList << parameter;
                    }

                    model.remove(commands.at(i));

                }
                else if(commands.at(i).startsWith("%subcircuit")){

                    // Subcircuits should be added to a temporal list to be included
                    // only once at the end of the spice file.
                    if(!subcircuitsList.contains(parameter)) {
                        subcircuitsList << parameter;
                    }

                    model.remove(commands.at(i));

                }
                else if(commands.at(i).startsWith("%directive")){

                    // Directives should be added to a temporal list to be included
                    // only once at the end of the spice file.
                    if(!directivesList.contains(parameter)) {
                        directivesList << parameter;
                    }

                    model.remove(commands.at(i));

                }
            }

            // ************************************************************
            // Now parse the generateNetlist command, which creates a
            // temporal list of schematics needed for recursive netlists
            // generation (for recursive simulations).
            // ************************************************************
            if(model.contains("%generateNetlist")){

                QFileInfo info(c->filename());
                QString baseName = info.completeBaseName();
                QString path = libraryManager->library(c->library())->libraryPath();
                QString schematic = path + "/" + baseName + ".xsch";

                if(!schematicsList.contains(schematic)) {
                    schematicsList << schematic;
                }

                model.remove("%generateNetlist");
            }

            // Add the model and a newline to the file
            retVal.append(model + "\n");
        }

        // ************************************************************
        // Write the QStringLists that should be in the end of the
        // file (e.g. device models).
        // ************************************************************
        // Append the spice models in modelsList
        if(!modelsList.isEmpty()) {
            retVal.append("\n* Device models.\n");
            for(int i=0; i<modelsList.size(); i++){
                retVal.append(".model " + modelsList.at(i) + "\n");
            }
        }

        // Append the spice subcircuits in subcircuitsList
        if(!subcircuitsList.isEmpty()) {
            retVal.append("\n* Subcircuits models.\n");
            for(int i=0; i<subcircuitsList.size(); i++){
                retVal.append(".subckt " + subcircuitsList.at(i) + "\n"
                              + ".ends" + "\n");
            }
        }

        // Append the spice directives in directivesList
        if(!directivesList.isEmpty()) {
            retVal.append("\n* Spice directives.\n");
            for(int i=0; i<directivesList.size(); i++){
                retVal.append(directivesList.at(i) + "\n");
            }
        }

        // ************************************************************
        // Create the needed recursive netlist documents
        // ************************************************************
        if(!schematicsList.isEmpty()) {
            for(int i=0; i<schematicsList.size(); i++){

                SchematicDocument *document = new SchematicDocument();
                document->setFileName(schematicsList.at(i));

                if(document->load()) {
                    // Export the schematic to a spice netlist
                    FormatSpice *format = new FormatSpice(document);
                    format->save();
                }

                delete document;
            }
        }

        // Remove multiple white spaces to clean up the file
        QRegularExpression re(" {2,}");
        retVal.replace(re, " ");

        return retVal;
    }

    /*!
     *  \brief Generate netlist net numbers
     *
     *  Iterate over all ports, to group all connected ports under
     *  the same name (name = equiId). This name or net number must
     *  be used afterwads by all component ports during netlist
     *  generation.
     *
     *  We use all connected ports (including those connected by wires),
     *  instead of connected wires during netlist generation. This allows
     *  to create a netlist node even on those places not connected by
     *  wires (for example when connecting two components together).
     *
     *  \sa saveComponents(), Port::getEquipotentialPorts()
     */
    PortsNetlist FormatSpice::generateNetlistTopology()
    {
        /*! \todo Investigate: If we use QList<GraphicsItem*> canedaItems = filterItems<Ports>(items);
         *  some phantom ports appear, and seem to be uninitialized, generating an ugly crash. Hence
         *  we filter generic items and use an iteration over their ports as a workaround.
         */
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<GraphicsItem*> canedaItems = filterItems<GraphicsItem>(items);
        QList<Port*> ports;
        foreach(GraphicsItem *i, canedaItems) {
            ports << i->ports();
        }

        int equiId = 1;
        PortsNetlist netlist;
        QList<Port*> parsedPorts;

        foreach(Port *p, ports) {
            if(parsedPorts.contains(p)) {
                continue;
            }

            QList<Port*> equi;
            p->getEquipotentialPorts(equi);
            foreach(Port *_port, equi) {
                netlist.append(qMakePair(_port, QString::number(equiId)));
            }

            equiId++;
            parsedPorts += equi;
        }

        replacePortNames(&netlist);

        return netlist;
    }

    /*!
     * \brief Replace net names in the netlist by those specified by
     * portSymbols.
     *
     * Iterate over all nets in the netlist, replacing those names that
     * correspond to the ones selected by the user using PortSymbols.
     * Take special care of the ground nets, that must be named "0" to
     * be complatible with the spice netlist format.
     *
     * \param netlist Netlist which is to be used in PortSymbol names
     * replacement.
     *
     * \sa PortSymbol, generateNetlistTopology()
     */
    void FormatSpice::replacePortNames(PortsNetlist *netlist)
    {
        QList<QGraphicsItem*> items = graphicsScene()->items();
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);

        // Iterate over all PortSymbols
        foreach(PortSymbol *p, portSymbols) {

            // Given the port, look for its netlist name
            QString netName;
            for(int i = 0; i < netlist->size(); ++i) {
                if(netlist->at(i).first == p->port()) {
                    netName = netlist->at(i).second;
                }
            }

            // Given the netlist name, rename all occurencies with the new name
            for(int i = 0; i < netlist->size(); ++i) {
                if(netlist->at(i).second == netName) {
                    if(p->label().toLower() == "ground" || p->label().toLower() == "gnd") {
                        netlist->replace(i, qMakePair(netlist->at(i).first, QString::number(0)));
                    }
                    else {
                        netlist->replace(i, qMakePair(netlist->at(i).first, p->label()));
                    }
                }
            }
        }
    }


    /*************************************************************************
     *                         FormatRawSimulation                           *
     *************************************************************************/
    //! \brief Constructor.
    FormatRawSimulation::FormatRawSimulation(SimulationDocument *doc) :
        m_simulationDocument(doc)
    {
    }

    //! \brief Load the waveform file indicated by \a filename.
    bool FormatRawSimulation::load()
    {
        ChartScene *scene = chartScene();
        if(!scene) {
            return false;
        }

        QString filename = m_simulationDocument->fileName();
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ") + filename);
            return false;
        }

        QTextStream in(&file);
        parseFile(&in);  // Parse the raw file
        file.close();

        return true;
    }

    /*!
     * \brief Parse the raw file
     *
     * Parse the raw file. First check the header of the file and then call
     * the parseAsciiData() or parseBinaryData() method depending on the
     * type of file.
     *
     * \sa parseAsciiData(), parseBinaryData()
     *
     * \todo There can be more than one plot set. This should be considered.
     */
    void FormatRawSimulation::parseFile(QTextStream *file)
    {
        int nvars = 0;     // Number of variables
        int npoints = 0;   // Number of points in the simulation
        bool real = true;  // Transient/AC simulation: real = transient / false = ac (complex numbers)

        QString line = file->readLine();

        while(!line.isNull()) {

            line = line.toLower();  // Don't care the case of the entry
            QStringList tok = line.split(":");
            QString keyword = tok.at(0);

            // Ignore the following keywords: title, date, plotname
            if( keyword == "flags" ) {
                if(tok.at(1) == " real") {
                    real = true;
                }
                else if(tok.at(1) == " complex") {
                    real = false;
                }
                else {
                    qDebug() << "Warning: unknown flag: " + tok.at(1);
                }
            }
            else if( keyword == "no. variables") {
                nvars = tok.at(1).toInt();
            }
            else if( keyword == "no. points") {
                npoints = tok.at(1).toInt();
            }
            else if( keyword == "variables") {

                for(int i = 0; i < nvars; i++) {
                    line = file->readLine();

                    tok = line.split("\t", QString::SkipEmptyParts);
                    if(tok.size() >= 3){
                        // Number property not used: number = tok.at(0)

                        // Create a new curve, and add it to the list
                        if(real) {
                            // If dealing with real numbers, create an array only for the magnitude and use the provided curve types
                            ChartSeries *curve = new ChartSeries(tok.at(1));  // tok.at(1) = name
                            curve->setType(tok.at(2));  // tok.at(2) = type of curve (voltage, current, etc)
                            plotCurves.append(curve);   // Append new curve to the list
                        }
                        else {
                            // If dealing with complex numbers, create an array for the magnitude and another one for the phase
                            ChartSeries *curve = new ChartSeries("Mag(" + tok.at(1) + ")");       // tok.at(1) = name
                            ChartSeries *curvePhase = new ChartSeries("Phase(" + tok.at(1) + ")");  // tok.at(1) = name
                            curve->setType("magnitude");         // type of curve (magnitude, phase, etc)
                            curvePhase->setType("phase");        // type of curve (magnitude, phase, etc)
                            plotCurves.append(curve);            // Append new curve to the list
                            plotCurvesPhase.append(curvePhase);  // Append new curve to the list
                        }

                    }
                    else {
                        qDebug() << "List of variables too short.";
                    }
                }
            }
            else if( keyword == "values" ) {
                parseAsciiData(file, nvars, npoints, real);  // Read the data itself
            }
            else if( keyword == "binary") {
                parseBinaryData(file, nvars, npoints, real);  // Read the data itself
            }

            // Read the next line
            line = file->readLine();
        }
    }

    /*!
     * \brief Read the data in Ascii format implementation
     *
     * Read the data in Ascii format implementation. Here we use a QTextStream
     * object to serially read text data from the file. QTextStream assumes
     * text data and "translates it" to text using the default codec (utf-8,
     * iso-8859-1, etc), and interpreting special characters as for example
     * newlines.
     *
     * \sa parseBinaryData(), parseFile()
     */
    void FormatRawSimulation::parseAsciiData(QTextStream *file, const int nvars, const int npoints, const bool real)
    {
        // Create the arrays to deal with the data
        QList<double*> dataSamples;               // List of curve's magnitude data. Once filled, used to set data in plotCurves.
        QList<double*> dataSamplesPhase;          // List of curve's phase data. Used for complex numbers. Once filled, used to set data in plotCurves.

        for(int i = 0; i < nvars; i++) {
            if(real) {
                // If dealing with real numbers, create an array only for the magnitude and use the provided curve types
                double *data = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
            }
            else {
                // If dealing with complex numbers, create an array for the magnitude and another one for the phase
                double *data = new double[npoints];  // Create new dataset
                double *dataPhase = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
                dataSamplesPhase.append(dataPhase);  // Append new data set to the list
            }
        }

        // Read the data
        if(real) {
            // The data is of type real
            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    QString line = file->readLine();
                    dataSamples[j][i] = line.split("\t").last().toDouble();
                }
            }

            // Avoid the first var, as it is the time base for the rest
            // of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                // Add the curve to the scene
                chartScene()->addItem(plotCurves[i]);
            }
        }
        else {
            // The data is of type complex
            double real = 0;
            double imaginary = 0;
            double magnitude = 0;
            double phase = 0;

            // Read the data values, converting the complex data into
            // magnitude and phase data.
            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    QString line = file->readLine();
                    QStringList tok = line.split("\t");
                    line = tok.last();  // Get the complex numeric data
                    tok = line.split(",");  // Split real and imaginary part

                    real = tok.first().toDouble();  // Get the real part
                    imaginary = tok.last().toDouble();  // Get the imaginary part

                    magnitude = qSqrt(real*real + imaginary*imaginary);  // Calculate the magnitude part
                    phase = qAtan(imaginary/real) * 180/M_PI;  // Calculate the phase part

                    dataSamples[j][i] = magnitude;
                    dataSamplesPhase[j][i] = phase;
                }
            }

            // Convert the magnitude values into dB ( dB = 20*log10(V) ).
            // Avoid the first var (var=0), as it is the frequency base
            // for the rest of the curves.
            for(int i = 0; i < npoints; i++){
                for(int j = 1; j < nvars; j++){
                    dataSamples[j][i] = 20*log10(dataSamples[j][i]);
                }
            }

            // Avoid the first var, as it is the frequency base for the
            // rest of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                plotCurvesPhase[i]->setSamples(dataSamples[0], dataSamplesPhase[i], npoints);
                // Add the curve to the scene
                chartScene()->addItem(plotCurves[i]);
                chartScene()->addItem(plotCurvesPhase[i]);
            }
        }
    }

    /*!
     * \brief Read the data in Binary format implementation
     *
     * Read the data in Binary format implementation. Here we must use a
     * QDataStream object to serially read raw data from the file. We cannot
     * use a QTextStream object as it assumes text data and "translates it"
     * to text using the default codec (utf-8, iso-8859-1, etc). The data to
     * be read from the file is composed by float numbers of 64 bit precision,
     * little endian format.
     *
     * \sa parseAsciiData(), parseFile()
     */
    void FormatRawSimulation::parseBinaryData(QTextStream *file, const int nvars, const int npoints, const bool real)
    {
        // Create the arrays to deal with the data
        QList<double*> dataSamples;               // List of curve's magnitude data. Once filled, used to set data in plotCurves.
        QList<double*> dataSamplesPhase;          // List of curve's phase data. Used for complex numbers. Once filled, used to set data in plotCurves.

        for(int i = 0; i < nvars; i++) {
            if(real) {
                // If dealing with real numbers, create an array only for the magnitude and use the provided curve types
                double *data = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
            }
            else {
                // If dealing with complex numbers, create an array for the magnitude and another one for the phase
                double *data = new double[npoints];  // Create new dataset
                double *dataPhase = new double[npoints];  // Create new dataset
                dataSamples.append(data);  // Append new data set to the list
                dataSamplesPhase.append(dataPhase);  // Append new data set to the list
            }
        }

        // Construct a QDataStream object to serially read raw data.
        // We cannot use QTextStream as it is locale aware, and will
        // automatically decode the input using a codec.
        QIODevice *device = file->device();
        device->seek(file->pos());  // Seek the previous file position (where the QTextStream left off).

        QDataStream out(device);  // Construct a QDataStream object to serially read raw data
        out.setByteOrder(QDataStream::LittleEndian);  // Use little endian format.
        out.setFloatingPointPrecision(QDataStream::DoublePrecision);  // Use 64 bit precision (this shouldn't be neccessary as it is the default).

        // Read the data
        if(real) {
            // The data is of type real
            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    out >> dataSamples[j][i];
                }
            }

            // Avoid the first var, as it is the time/frequency base
            // for the rest of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                // Add the curve to the scene
                chartScene()->addItem(plotCurves[i]);
            }
        }
        else {
            // The data is of type complex
            double real = 0;
            double imaginary = 0;
            double magnitude = 0;
            double phase = 0;

            // Read the data values, converting the complex data into
            // magnitude and phase data.
            for(int i = 0; i < npoints; i++){
                for(int j = 0; j < nvars; j++){
                    out >> real;  // Get the real part
                    out >> imaginary;  // Get the imaginary part

                    magnitude = qSqrt(real*real + imaginary*imaginary);  // Calculate the magnitude part
                    phase = qAtan(imaginary/real) * 180/M_PI;  // Calculate the phase part

                    dataSamples[j][i] = magnitude;
                    dataSamplesPhase[j][i] = phase;
                }
            }

            // Convert the magnitude values into dB ( dB = 20*log10(V) ).
            // Avoid the first var (var=0), as it is the frequency base
            // for the rest of the curves.
            for(int i = 0; i < npoints; i++){
                for(int j = 1; j < nvars; j++){
                    dataSamples[j][i] = 20*log10(dataSamples[j][i]);
                }
            }

            // Avoid the first var, as it is the time/frequency base
            // for the rest of the curves.
            for(int i = 1; i < nvars; i++){
                // Copy the data into the curves
                plotCurves[i]->setSamples(dataSamples[0], dataSamples[i], npoints);
                plotCurvesPhase[i]->setSamples(dataSamples[0], dataSamplesPhase[i], npoints);
                // Add the curve to the scene
                chartScene()->addItem(plotCurves[i]);
                chartScene()->addItem(plotCurvesPhase[i]);
            }
        }
    }

    ChartScene* FormatRawSimulation::chartScene() const
    {
        return m_simulationDocument ? m_simulationDocument->chartScene() : 0;
    }

} // namespace Caneda
