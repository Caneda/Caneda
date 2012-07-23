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
#include <QSvgRenderer>

namespace Caneda
{
    /*!
     * \brief This represents individual library unit.
     */
    class Library
    {
    public:
        Library(QString libraryPath);
        ~Library() {}

        ComponentDataPtr componentDataPtr(const QString& name) const;

        //! Returns library name.
        QString libraryName() const { return m_libraryName; }

        //! Returns library filename.
        QString libraryFileName() const { return m_libraryFileName; }

        //! Returns symbol id corresponding to library.
        QString defaultSymbolId() const { return m_defaultSymbolId; }

        //! Returns brief text used to display.
        QString displayText() const { return m_displayText; }

        //! Returns the description of library.
        QString description() const { return m_description; }

        //! Returns validity of this instance.
        bool isValid() const { return m_valid; }

        //! Returns the component' s shared hash table.
        const QHash<QString, ComponentDataPtr>& components() const {
            return m_componentHash;
        }

        bool loadLibrary(Caneda::XmlReader *reader);
        bool saveLibrary();
        bool parseExternalComponent(QString componentPath);
        bool removeComponent(QString componentName);

    private:
        bool registerComponentData(Caneda::XmlReader *reader, QString componentPath);
        QString m_libraryName;
        QString m_libraryFileName;
        QString m_defaultSymbolId;
        QString m_displayText;
        QString m_description;

        QHash<QString, ComponentDataPtr> m_componentHash;
        bool m_valid;
    };

    typedef QHash<QString, Library*> LibraryHash;

    /*!
     * \brief This class acts as container and manages libraries.
     *
     * This class acts as container and manages libraries. It also acts as
     * a container for the different symbols loaded during library loading.
     * To render a component, the symbol id must be given and a pointer to
     * the symbol drawing is returned (paths, rectangles, circles, etc).
     * The component to be rendered should be first registered with the
     * instance of this class. A cache of components is created an data
     * needed for painting components is created only once (independently
     * of the number of components used by the user in the final schematic).
     *
     * This class is singleton class and its only static instance returned by
     * \a instance is to be used.
     */
    class LibraryManager : public QObject
    {
        Q_OBJECT
    public:
        static LibraryManager* instance();
        ~LibraryManager();

        Component* newComponent(QString componentName,
                                CGraphicsScene *scene,
                                QString library = QString());

        // Library management related methods
        bool newLibrary(const QString& libPath);
        bool load(const QString& libPath);
        bool loadtree();
        bool unload(const QString& libName);

        Library* library(const QString& libName) const;

        // Symbol caching related methods
        void registerComponent(const QString& symbol_id, const QByteArray& content);
        bool isComponentRegistered(const QString& symbol_id) const;

        QSvgRenderer* symbolCache(const QString &symbol_id);
        const QPixmap pixmapCache(const QString &symbol_id);


    Q_SIGNALS:
        void basicLibrariesLoaded();

    private:
        LibraryManager(QObject *parent = 0);

        //! Hash table to hold libraries
        LibraryHash m_libraryHash;

        //! Hash table to hold Svg renderer (wich has raw svg content cached).
        QHash<QString, QSvgRenderer*> m_dataHash;
    };

} // namespace Caneda

#endif //LIBRARY_H
