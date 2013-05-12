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

#ifndef FORMAT_XML_SYMBOL_H
#define FORMAT_XML_SYMBOL_H

#include "component.h"

// Forward declarations
class QString;

namespace Caneda
{
    // Forward declarations
    class CGraphicsScene;
    class SymbolDocument;
    class XmlReader;

    /*!
     * \brief This class handles all the access to the symbol documents file
     * format.
     *
     * This class is in charge of saving and loading all symbol related
     * documents. This is the only class that knows about symbol document
     * formats, and has the access functions to return a SymbolDocument,
     * with all of its components.
     *
     * \sa \ref DocumentFormats
     */
    class FormatXmlSymbol
    {
    public:
        FormatXmlSymbol(SymbolDocument *doc = 0);
        FormatXmlSymbol(ComponentData *component);

        bool save();
        bool load();

        SymbolDocument* symbolDocument() const;
        CGraphicsScene* cGraphicsScene() const;
        ComponentData* component() const;

        QString fileName() const;

    private:
        QString saveText();
        bool loadFromText(const QString& text);

        void readSymbol(Caneda::XmlReader *reader);
        void readPorts(Caneda::XmlReader *reader);
        void readProperties(Caneda::XmlReader *reader);

        SymbolDocument *m_symbolDocument;
        ComponentData *m_component;
        QString m_fileName;
    };

} // namespace Caneda

#endif //FORMAT_XML_SYMBOL_H
