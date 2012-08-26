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
#include "settings.h"
#include "xmlsymbol.h"

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
     *  \brief Constructs library item from reader with file path \a path.
     */
    Library::Library(QString libraryPath)
    {
        m_libraryPath = libraryPath;
        m_libraryName = QFileInfo(libraryPath).baseName();
    }

    //! \brief Returns the shared data of component from given name.
    ComponentDataPtr Library::component(const QString& name) const
    {
        return m_componentHash.contains(name) ?
            m_componentHash[name] : ComponentDataPtr();
    }

    //! \brief Loads the library's components and its translated name.
    bool Library::loadLibrary()
    {
        // Go to base library dir
        QString current = QDir::currentPath();
        if(!QDir::setCurrent(m_libraryPath)) {
            QDir::setCurrent(current);
            return false;
        }

        bool readOk = true;
        // Check if translations file exists and can be opened.
        // This file is necessary to hold the library names in
        // different languages. In this file isn't present, the
        // default library name is chosen (base dir).
        QFile file("translations.xml");
        if(file.open(QIODevice::ReadOnly)) {
            // Read the translations file
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

                        Q_ASSERT(reader.isStartElement() && reader.name() == "library");

                        m_libraryName = reader.attributes().value("name").toString();
                        if(m_libraryName.isEmpty()) {
                            reader.raiseError("Invalid or no 'name' attribute in library tag");
                        }

                        while(!reader.atEnd()) {
                            reader.readNext();
                            if(reader.isEndElement()) {
                                break;
                            }
                            reader.readUnknownElement();
                        }

                    }
                    else {
                        reader.readUnknownElement();
                    }
                }
            }

            if(reader.hasError()) {
                QMessageBox::warning(0, QObject::tr("Error"),
                                      QObject::tr("Invalid library file!"));
                readOk = false;
            }

        }

        QDir libraryPath(m_libraryPath);
        QStringList componentsList = libraryPath.entryList(QStringList("*.xsym"));  // Filter only component files
        foreach (const QString &component, componentsList) {
            // Read all components in the library path
            readOk = readOk & parseComponent(component);
            if(!readOk) {
                QMessageBox::warning(0, QObject::tr("Error"),
                                     QObject::tr("Parsing component data file %1 failed")
                                     .arg(QFileInfo(component).absoluteFilePath()));
            }
        }

        QDir::setCurrent(current);
        return readOk;
    }

    //! \brief Removes the component from library.
    bool Library::removeComponent(QString componentName)
    {
        if(!m_componentHash.contains(componentName)) {
            return false;
        }

        m_componentHash.remove(componentName);
        return true;
    }

    //! \brief Parses the component data from file \a path.
    bool Library::parseComponent(QString componentPath)
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

            ComponentDataPtr dataPtr(new ComponentData);
            dataPtr->library = libraryName();
            dataPtr->filename = componentPath;

            QString parentPath = QFileInfo(componentPath).absolutePath();
            XmlSymbol *format = new XmlSymbol();
            readok = format->readComponentData(&reader, parentPath, dataPtr);

            if(dataPtr.constData() == 0 || reader.hasError() || !readok) {
                qWarning() << "\nWarning: Failed to read data from\n" << QFileInfo(componentPath).absolutePath();
                readok = false;
            }
            else {
                // Register component's data
                if(!m_componentHash.contains(dataPtr->name)) {
                    m_componentHash.insert(dataPtr->name, dataPtr);
                }
            }

        }
        return !reader.hasError() && readok;
    }


    //*************************************************************
    //**********************Library Loader*************************
    //*************************************************************
    //! Constructor
    LibraryManager::LibraryManager(QObject *parent) : QObject(parent)
    {
    }

    //! \brief Returns default instance of library.
    LibraryManager* LibraryManager::instance()
    {
        static LibraryManager *instance = 0;
        if (!instance) {
            instance = new LibraryManager(SingletonOwner::instance());
        }
        return instance;
    }

    /*! Destructor.
     *  \brief Deletes the data belonging to this object.
     */
    LibraryManager::~LibraryManager()
    {
        QHash<QString, QSvgRenderer*>::iterator it = m_dataHash.begin(), end = m_dataHash.end();
        while(it != end) {
            delete it.value();
            it.value() = 0;
            ++it;
        }
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
    Component* LibraryManager::newComponent(QString componentName, CGraphicsScene *scene,
            QString library)
    {
        ComponentDataPtr data;
        if(library.isEmpty()) {
            LibraryHash::const_iterator it = m_libraryHash.constBegin(),
                    end = m_libraryHash.constEnd();
            while(it != end) {
                data = it.value()->component(componentName);
                if(data.constData()) {
                    break;
                }
                ++it;
            }
        }
        else {
            if(m_libraryHash.contains(library)) {
                data = m_libraryHash[library]->component(componentName);
            }
        }

        if(data.constData()) {
            Component* comp = new Component(data, scene);
            comp->setSymbol(comp->symbol());
            return comp;
        }

        return 0;
    }

    //! \brief Create library indicated by path \a libPath.
    bool LibraryManager::newLibrary(const QString& libPath)
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
        return true;
    }

    //! \brief Load library indicated by path \a libPath.
    bool LibraryManager::load(const QString& libPath)
    {
        Library *info = new Library(libPath);
        bool loaded = info->loadLibrary();

        if(!loaded) {
            delete info;
            return false;
        }

        if(library(info->libraryName())) {
            QMessageBox::critical(0, QObject::tr("Error"),
                                  QObject::tr("Library %1 currently opened. Please close"
                                              "library %1 first.").arg(info->libraryName()));
            delete info;
            return false;
        }

        m_libraryHash.insert(info->libraryName(), info);
        return true;
    }

    /*!
     * \brief Unloads given library freeing memory pool.
     * \sa Library::~Library()
     */
    bool LibraryManager::unload(const QString& libName)
    {
        if(m_libraryHash.contains(libName)) {
            m_libraryHash.remove(libName);
            return true;
        }

        return false;
    }

    //! \brief Load the library tree
    bool LibraryManager::loadLibraryTree()
    {
        bool status = true;

        Settings *settings = Settings::instance();
        QStringList libraries;
        libraries << settings->currentValue("libraries/schematic").toStringList();
        foreach (const QString &str, libraries) {
            status = status && load(str);
        }

        if(status) {
            emit basicLibrariesLoaded();
        }

        return status;
    }

    /*!
     * \brief Returns library item corresponding to name.
     *
     * \param str The library's name.
     * \return Library on success and null pointer on failure.
     */
    Library* LibraryManager::library(const QString& str) const
    {
        if(!m_libraryHash.contains(str)) {
            return 0;
        }

        return m_libraryHash[str];
    }

    /*!
     * \brief Registers a component symbol with symbol_id in this instance.
     *
     * Registering is required for rendering any component with the instance of this
     * class. If the symbol_id is already registered does nothing.
     */
    void LibraryManager::registerComponent(const QString& symbol_id, const QByteArray& svg)
    {
        if(m_dataHash.contains(symbol_id)) {
            return;
        }

        m_dataHash[symbol_id] = new QSvgRenderer(svg);
    }

    //! \brief Returns the symbol of a component corresponding to symbol_id.
    QSvgRenderer* LibraryManager::symbolCache(const QString &symbol_id)
    {
        return m_dataHash[symbol_id];
    }

    /*!
     * \brief Returns the cached pixmap of a component.
     *
     * \param symbol_id Symbol id of the component to be rendered.
     */
    const QPixmap LibraryManager::pixmapCache(const QString& symbol_id)
    {
        QPixmap pix;

        if(!QPixmapCache::find(symbol_id, pix)) {
            QSvgRenderer *data = m_dataHash[symbol_id];
            QRect rect =  data->viewBox();
            pix = QPixmap(rect.size());
            pix.fill(Qt::transparent);

            QPainter painter(&pix);

            QPointF offset = -rect.topLeft(); // (0,0)-topLeft()
            painter.translate(offset);

            data->render(&painter, rect);

            QPixmapCache::insert(symbol_id, pix);
        }

        return pix;
    }

} // namespace Caneda
