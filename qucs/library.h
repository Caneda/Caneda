/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __LIBRARY_H
#define __LIBRARY_H

#include "component.h"
#include <QtCore/QHash>

typedef QSharedDataPointer<Qucs::ComponentData> ComponentDataPtr;

/*!
 * \brief This represents individual library unit.
 */
class LibraryItem
{
   public:
      LibraryItem(Qucs::XmlReader *reader, QString path, SvgPainter *painter);
      ComponentDataPtr componentDataPtr(const QString& name) const;

      //! Returns svg painter in use.
      SvgPainter* svgPainter() const { return m_svgPainter; }
      //! Returns library name.
      QString libraryName() const { return m_libraryName; }
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

      void render(QPainter *painter, QString component, QString symbol = QString()) const;
      QPixmap renderedPixmap(QString component, QString symbol = QString()) const;

   private:
      bool parseLibrary(Qucs::XmlReader *reader, QString path);
      bool parseExternalComponent(QString path, SchematicScene *scene);

      void registerComponentData(Qucs::XmlReader *reader, QString path,
                                 SchematicScene *scene);
      QString m_libraryName;
      QString m_defaultSymbolId;
      QString m_displayText;
      QString m_description;

      SvgPainter *m_svgPainter;
      QHash<QString, ComponentDataPtr> m_componentHash;
      bool m_valid;
};

typedef QHash<QString, LibraryItem*> LibraryHash;

/*!
 * \brief This class acts as container for \a LibraryItem s
 *
 * This class is singleton class and its only static instance returned by
 * \a defaultInstance is to be used.
 */
class Library
{
   public:
      ~Library();

      static Library* defaultInstance();
      Qucs::Component* newComponent(QString componentName,
                                    SchematicScene *scene,
                                    QString library = QString());
      bool loadLibrary(const QString& libPath);
      //! Returns library hash table
      const LibraryHash& libraries() const { return m_libraryHash; }

      const LibraryItem* libraryItem(const QString& libName) const;

      //! Returns svg painter used.
      SvgPainter* svgPainter() const { return m_svgPainter; }

   private:
      Library();
      LibraryHash m_libraryHash;
      SvgPainter *m_svgPainter;
};

#endif //__LIBRARY_H
