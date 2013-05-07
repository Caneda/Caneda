/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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
#include "cgraphicsview.h"
#include "formatxmlsymbol.h"
#include "global.h"
#include "singletonowner.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QByteArray>
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

    /*!
     * \brief Constructor
     *
     * Constructs library item from reader with file path \a path.
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
        foreach (const QString &componentPath, componentsList) {
            // Read all components in the library path
            ComponentData *component = new ComponentData();
            component->library = libraryName();
            component->filename = componentPath;

            FormatXmlSymbol *format = new FormatXmlSymbol(component);
            readOk = readOk & format->load();

            if(!readOk) {
                QMessageBox::warning(0, QObject::tr("Error"),
                                     QObject::tr("Parsing component data file %1 failed")
                                     .arg(QFileInfo(componentPath).absoluteFilePath()));
            }
            else {
                // Register component's data
                if(!m_componentHash.contains(component->name)) {
                    ComponentDataPtr componentDataPtr(component);
                    m_componentHash.insert(component->name, componentDataPtr);
                }
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

    //*************************************************************
    //**********************Library Loader*************************
    //*************************************************************
    //! Constructor
    LibraryManager::LibraryManager(QObject *parent) : QObject(parent)
    {
    }

    //! \copydoc MainWindow::instance()
    LibraryManager* LibraryManager::instance()
    {
        static LibraryManager *instance = 0;
        if (!instance) {
            instance = new LibraryManager(SingletonOwner::instance());
        }
        return instance;
    }

    //! Destructor
    LibraryManager::~LibraryManager()
    {
    }

    /*!
     * \brief Constructs a new component given its name and library.
     *
     * \param componentName The component's name.
     * \param scene The scene on which component is to be rendered.
     * \param library The library to which the \a componentName belongs.
     * \return Component on success and null pointer on failure.
     */
    Component* LibraryManager::newComponent(QString componentName, CGraphicsScene *scene,
            QString library)
    {
        ComponentDataPtr data;

        if(m_libraryHash.contains(library)) {
            data = m_libraryHash[library]->component(componentName);
        }

        if(data.constData()) {
            Component* comp = new Component(data, scene);
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
                                  QObject::tr("Only one library %1 can be opened at the same time. Please remove one of the "
                                              "libraries named %1 from the library tree first.").arg(info->libraryName()));
            delete info;
            return false;
        }

        m_libraryHash.insert(info->libraryName(), info);
        return true;
    }

    /*!
     * \brief Unloads given library freeing memory pool.
     *
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
     * \brief Registers a component symbol with an associated key in this
     * instance.
     *
     * Registering is required for rendering any component with the instance of
     * this class. If the symbol key is already registered, this method does
     * nothing.
     *
     * Each component's key is saved in the form "componentName:libraryName" to
     * allow for different libraries to have components with the same name.
     * This is specially useful to let the user choose whatever name he wants,
     * without having to check for existing names.
     *
     * \param compName Component name, used as part of the key
     * \param libName Library name, used as part of the key
     * \param content QPainterPath containing the symbol to register
     *
     * \sa symbolCache(), pixmapCache()
     */
    void LibraryManager::registerComponent(const QString &compName, const QString &libName, const QPainterPath& content)
    {
        QString symbol_id = compName + ":" + libName;

        if(m_dataHash.contains(symbol_id)) {
            return;
        }

        m_dataHash[symbol_id] = content;
    }

    /*!
     * \brief Returns the symbol (QPainterPath) of a component corresponding to
     * a key.
     *
     * Each component's key is saved in the form "componentName:libraryName" to
     * allow for different libraries to have components with the same name.
     *
     * \param compName Component name, used as part of the key
     * \param libName Library name, used as part of the key
     * \return QPainterPath corresponding to the symbol
     *
     * \sa registerComponent(), pixmapCache()
     */
    QPainterPath LibraryManager::symbolCache(const QString &compName, const QString &libName)
    {
        QString symbol_id = compName + ":" + libName;
        return m_dataHash[symbol_id];
    }

    /*!
     * \brief Returns the cached pixmap of a component corresponding to
     * a key.
     *
     * Each component's key is saved in the form "componentName:libraryName" to
     * allow for different libraries to have components with the same name.
     *
     * \param compName Component name, used as part of the key
     * \param libName Library name, used as part of the key
     * \return QPixmap corresponding to the symbol
     *
     * \sa registerComponent(), symbolCache()
     */
    const QPixmap LibraryManager::pixmapCache(const QString &compName, const QString &libName)
    {
        QString symbol_id = compName + ":" + libName;
        QPixmap pix;

        if(!QPixmapCache::find(symbol_id, pix)) {

            QPainterPath data = m_dataHash[symbol_id];
            QRect rect =  data.boundingRect().toRect();
            rect.adjust(-1.0, -1.0, 1.0, 1.0); // Adjust rect to avoid clipping due to rounding (rectF -> rect)
            pix = QPixmap(rect.size());
            pix.fill(Qt::transparent);

            QPainter painter(&pix);
            painter.setRenderHints(Caneda::DefaulRenderHints);

            Settings *settings = Settings::instance();
            painter.setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                settings->currentValue("gui/lineWidth").toInt()));

            QPointF offset = -rect.topLeft(); // (0,0)-topLeft()
            painter.translate(offset);
            painter.drawPath(data);

            QPixmapCache::insert(symbol_id, pix);
        }

        return pix;
    }

} // namespace Caneda
