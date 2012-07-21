/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "library.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "singletonowner.h"

#include "xmlutilities/xmlutilities.h"
#include "xmlutilities/validators.h"

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <QPixmapCache>
#include <QString>
#include <QTextStream>

namespace Caneda
{
    //*************************************************************
    //*************************Library*****************************
    //*************************************************************

    /*! Constructor
     *  \brief Constructs library item from reader with file path \a path and svgpainter \a painter.
     */
    Library::Library(QString libraryPath)
    {
        m_libraryName = QFileInfo(libraryPath).baseName();
        m_libraryName.replace(0, 1, m_libraryName.left(1).toUpper()); // First letter in uppercase

        m_libraryFileName = libraryPath;

        if(m_libraryName.isEmpty()) {
            qWarning() << "\nWarning: Invalid or no 'name' attribute in library tag";
            return;
        }

        m_displayText = "User library";
        m_description = "User created library";

        m_valid = true;
    }

    //! \brief Returns the shared data of component from given name.
    ComponentDataPtr Library::componentDataPtr(const QString& name) const
    {
        return m_componentHash.contains(name) ?
            m_componentHash[name] : ComponentDataPtr();
    }

    /*!
     * \brief Parses the library xml file.
     *
     * \param reader XmlReader corresponding to file.
     */
    bool Library::loadLibrary(Caneda::XmlReader *reader)
    {
        bool readok = true;
        Q_ASSERT(reader->isStartElement() && reader->name() == "library");

        m_libraryName = reader->attributes().value("name").toString();
        if(m_libraryName.isEmpty()) {
            reader->raiseError("Invalid or no 'name' attribute in library tag");
            m_valid = false;
            return m_valid;
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "displaytext") {
                    m_displayText = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement() && reader->name() == "displaytext");
                }
                else if(reader->name() == "description") {
                    m_description = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement() && reader->name() == "description");
                }
                else if(reader->name() == "component") {
                    QString externalPath = reader->attributes().value("href").toString();
                    if(!externalPath.isEmpty()) {
                        bool status = parseExternalComponent(externalPath);
                        if(!status) {
                            QString errorString("Parsing external component data file %1 "
                                    "failed");
                            errorString = errorString.arg(QFileInfo(externalPath).absoluteFilePath());
                            reader->raiseError(errorString);
                        } else {
                            //ignore rest of component' tag as the main data is only external
                            reader->readUnknownElement();
                        }
                    }
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }
        m_valid = !reader->hasError() && readok;
        return m_valid;
    }

    //! \brief Saves the library to a file.
    bool Library::saveLibrary()
    {
        QString saveText;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&saveText);
        writer->setAutoFormatting(true);

        writer->writeStartDocument();
        writer->writeStartElement("library");
        writer->writeAttribute("name", libraryName());
        writer->writeAttribute("version", Caneda::version());

        writer->writeStartElement("displaytext");
        writer->writeLocaleText("en", displayText());
        writer->writeEndElement(); //</displaytext>

        writer->writeStartElement("description");
        writer->writeLocaleText("en", description());
        writer->writeEndElement(); //</description>

        //Save all components in library
        QList<ComponentDataPtr> componentsList = components().values();
        foreach(const ComponentDataPtr data, componentsList) {
            writer->writeEmptyElement("component");
            writer->writeAttribute("href", QFileInfo(data->filename).fileName());
        }

        writer->writeEndDocument(); //</library>
        delete writer;


        if(saveText.isEmpty()) {
            qDebug("Looks buggy! Null data to save! Was this expected ??");
        }

        QFile file(libraryFileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save library!"));
            return false;
        }
        QTextStream stream(&file);
        stream << saveText;
        file.close();

        return true;
    }

    //! \brief Parses the component data from file \a path.
    bool Library::parseExternalComponent(QString componentPath)
    {
        bool readok = true;
        QFile file(QFileInfo(componentPath).absoluteFilePath());

        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(0, QObject::tr("File open"),
                    QObject::tr("Cannot open file %1").arg(componentPath));
            return false;
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QByteArray data = in.readAll().toUtf8();

        Caneda::XmlReader reader(data,
                Caneda::validators::defaultInstance()->components());

        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isStartElement() && reader.name() == "component") {
                break;
            }
        }
        if(reader.isStartElement() && reader.name() == "component") {
            readok = registerComponentData(&reader, componentPath);
        }
        return !reader.hasError() && readok;
    }

    //! \brief Removes the component from library.
    bool Library::removeComponent(QString componentName)
    {
        if(!m_componentHash.contains(componentName)) {
            return false;
        }
        else {
            m_componentHash.remove(componentName);
        }

        return true;
    }

    //! \brief Registers svg as well as the component's shared data.
    bool Library::registerComponentData(Caneda::XmlReader *reader, QString componentPath)
    {
        bool readok;

        //Automatically registers svgs on success
        ComponentDataPtr dataPtr(new ComponentData);
        dataPtr->library = libraryName();
        dataPtr->filename = componentPath;

        QString parentPath = QFileInfo(componentPath).absolutePath();
        readok = Caneda::readComponentData(reader, parentPath, dataPtr);

        if(dataPtr.constData() == 0 || reader->hasError() || !readok) {
            qWarning() << "\nWarning: Failed to read data from\n" << QFileInfo(componentPath).absolutePath();
            return false;
        }

        if(!m_componentHash.contains(dataPtr->name)) {
            m_componentHash.insert(dataPtr->name, dataPtr);
        }
        return true;
    }

    //*************************************************************
    //**********************Library Loader*************************
    //*************************************************************

    //! Constructor
    LibraryLoader::LibraryLoader(QObject *parent) : QObject(parent)
    {
    }

    //! \brief Returns default instance of library.
    LibraryLoader* LibraryLoader::instance()
    {
        static LibraryLoader *instance = 0;
        if (!instance) {
            instance = new LibraryLoader(SingletonOwner::instance());
        }
        return instance;
    }

    /*!
     * \brief Returns library item corresponding to name.
     *
     * \param str The library's name.
     * \return Library on success and null pointer on failure.
     */
    Library* LibraryLoader::library(const QString& str) const
    {
        if(!m_libraryHash.contains(str)) {
            return 0;
        }

        return m_libraryHash[str];
    }

    /*!
     * \brief Constructs a new component given its name and library.
     *
     * \param componentName The component's name.
     * \param scene The scene on which component is to be rendered.
     * \param library The library to which the \a componentName belongs. If this is empty,
     *  it searches for all libraries for component and returns first match.
     * \return Component on success and null pointer on failure.
     */
    Component* LibraryLoader::newComponent(QString componentName, CGraphicsScene *scene,
            QString library)
    {
        ComponentDataPtr data;
        if(library.isEmpty()) {
            LibraryHash::const_iterator it = m_libraryHash.constBegin(),
                end = m_libraryHash.constEnd();
            while(it != end) {
                data = it.value()->componentDataPtr(componentName);
                if(data.constData()) {
                    break;
                }
                ++it;
            }
        }
        else {
            if(m_libraryHash.contains(library)) {
                data = m_libraryHash[library]->componentDataPtr(componentName);
            }
        }

        SvgPainter *svgPainter = SvgPainter::instance();

        if(data.constData()) {
            Q_ASSERT(svgPainter);
            Component* comp = new Component(data, scene);
            comp->setSymbol(comp->symbol());
            return comp;
        }

        return 0;
    }

    //! \brief Create library indicated by path \a libPath.
    bool LibraryLoader::newLibrary(const QString& libPath)
    {
        // Go to base dir
        QString libParentPath = QFileInfo(libPath).dir().absolutePath();
        QString current = QDir::currentPath();
        if(!QDir::setCurrent(libParentPath)) {
            (void) QDir::setCurrent(current);
            return false;
        }

        Library *info = new Library(libPath);

        m_libraryHash.insert(info->libraryName(), info);

        return info->isValid();
    }

    //! \brief Load library indicated by path \a libPath.
    bool LibraryLoader::load(const QString& libPath)
    {
        // Open file
        QFile file(libPath);
        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(0, QObject::tr("File open"),
                    QObject::tr("Cannot open file %1\n").arg(libPath));
            return false;
        }

        // Go to base dir
        QString libParentPath = QFileInfo(libPath).dir().absolutePath();
        QString current = QDir::currentPath();
        if(!QDir::setCurrent(libParentPath)) {
            (void) QDir::setCurrent(current);
            return false;
        }

        QTextStream in(&file);
        in.setCodec("UTF-8");
        QByteArray data = in.readAll().toUtf8();

        Caneda::XmlReader reader(data);
        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isEndElement()) {
                break;
            }

            if(reader.isStartElement()) {
                if(reader.name() == "library") {
                    Library *info = new Library(libPath);
                    info->loadLibrary(&reader);

                    if(reader.hasError()) {
                        QMessageBox::critical(0, QObject::tr("Load library"),
                                QObject::tr("Parsing library failed with following"
                                            " error: \"%1\"").arg(reader.errorString()));
                        delete info;
                        return false;
                    }

                    if(!library(info->libraryName())) {
                        m_libraryHash.insert(info->libraryName(), info);
                    }
                    else {
                        QMessageBox::critical(0, QObject::tr("Error"),
                                QObject::tr("Library %1 currently opened. Please close"
                                            "library %1 first.").arg(info->libraryName()));
                        delete info;
                        return false;
                    }

                }
                else {
                    reader.readUnknownElement();
                }
            }
        }

        if(reader.hasError()) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Invalid library file!"));
        }

        (void) QDir::setCurrent(current);
        return !reader.hasError();
    }

    /*!
     * \brief Load a library tree
     * \todo Implement a true loader
     */
    bool LibraryLoader::loadtree(const QString& libpathtree)
    {
        bool status = true;

        status = status && load(libpathtree + "/components/basic/passive.xpro");
        status = status && load(libpathtree + "/components/basic/active.xpro");
        status = status && load(libpathtree + "/components/basic/semiconductor.xpro");

        if(status) {
            emit basicLibrariesLoaded();
        }

        return status;
    }

    /*!
     * \brief Unloads given library freeing memory pool.
     * \sa Library::~Library()
     */
    bool LibraryLoader::unload(const QString& libName)
    {
        if(m_libraryHash.contains(libName)) {
            m_libraryHash.remove(libName);
            return true;
        }

        return false;
    }

} // namespace Caneda
