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

#ifndef LIBRARY_H
#define LIBRARY_H

#include "component.h"

#include <QHash>
#include <QObject>

namespace Caneda
{
    /*!
     * \brief This class represents an individual library unit.
     *
     * Caneda's libraries contain pointers to the different components
     * available to the user. Each pair (component name, library name) define
     * a unique component thoughout all Caneda's usage (file saving or loading,
     * component referencing, etc.). This class also handles the loading of all
     * components in a library at once.
     *
     * \sa LibraryManager, Component
     */
    class Library
    {
    public:
        Library(QString libraryPath);
        ~Library() {}

        //! Returns library name.
        QString libraryName() const { return m_libraryName; }
        //! Returns library filename.
        QString libraryPath() const { return m_libraryPath; }

        ComponentDataPtr component(const QString& name) const;
        //! Returns the components list.
        const QList<QString> componentsList() const { return m_componentHash.uniqueKeys(); }

        bool loadLibrary();
        bool removeComponent(QString componentName);

    private:
        //! Library name. If not specified in "translations.xml", it is the base dir name.
        QString m_libraryName;
        //! Library full path.
        QString m_libraryPath;

        QHash<QString, ComponentDataPtr> m_componentHash;
    };

    /*!
     * \brief This class is a container and manager for all Caneda's libraries.
     *
     * This class handles loading and saving of libraries in a generic way,
     * calling library specific methods, and saving a pointer for all loaded
     * libraries.
     *
     * It also acts as a container for the different symbols loaded during
     * library loading. To render a component, the symbol id must be given and
     * a pointer to the symbol drawing is returned (paths, rectangles, circles,
     * etc). The component to be rendered should be first registered with the
     * instance of this class. A cache of components is created an data needed
     * for painting components is created only once (independently of the
     * number of components used by the user in the final schematic).
     *
     * This class is singleton class and its only static instance (returned by
     * instance()) is to be used.
     *
     * \sa Library
     */
    class LibraryManager : public QObject
    {
        Q_OBJECT
    public:
        static LibraryManager* instance();
        ~LibraryManager();

        Component* newComponent(QString componentName,
                                CGraphicsScene *scene,
                                QString library);

        // Library management related methods
        bool newLibrary(const QString& libPath);
        bool load(const QString& libPath);
        bool unload(const QString& libName);
        bool loadLibraryTree();

        Library* library(const QString& libName) const;
        //! Returns the libraries list.
        const QList<QString> librariesList() const { return m_libraryHash.uniqueKeys(); }

        // Symbol caching related methods
        void registerComponent(const QString& symbol_id, const QPainterPath& content);

        QPainterPath symbolCache(const QString &symbol_id);
        const QPixmap pixmapCache(const QString &symbol_id);

    Q_SIGNALS:
        void basicLibrariesLoaded();

    private:
        LibraryManager(QObject *parent = 0);

        //! Hash table to hold libraries.
        QHash<QString, Library*> m_libraryHash;

        //! Symbol cache (hash table) to hold QPainterPaths contents (symbols' cache).
        QHash<QString, QPainterPath> m_dataHash;
    };

} // namespace Caneda

#endif //LIBRARY_H
