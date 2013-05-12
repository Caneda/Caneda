/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef FORMAT_XML_SCHEMATIC_H
#define FORMAT_XML_SCHEMATIC_H

// Forward declarations
class QString;

namespace Caneda
{
    // Forward declarations
    class CGraphicsScene;
    class SchematicDocument;

    class XmlReader;
    class XmlWriter;

    /*!
     * \brief This class handles all the access to the schematic documents file
     * format.
     *
     * This class is in charge of saving and loading all schematic related
     * documents. This is the only class that knows about schematic document
     * formats, and has the access functions to return a SchematicDocument,
     * with all of its components.
     *
     * \sa \ref DocumentFormats
     */
    class FormatXmlSchematic
    {
    public:
        FormatXmlSchematic(SchematicDocument *doc = 0);

        bool save();
        bool load();

        SchematicDocument* schematicDocument() const;
        CGraphicsScene* cGraphicsScene() const;
        QString fileName() const;

    private:
        QString saveText();
        void saveSchematics(Caneda::XmlWriter *writer);
        void saveComponents(Caneda::XmlWriter *writer);
        void saveWires(Caneda::XmlWriter *writer);
        void savePaintings(Caneda::XmlWriter *writer);

        bool loadFromText(const QString& text);
        void loadSchematics(Caneda::XmlReader *reader);
        void loadComponents(Caneda::XmlReader *reader);
        void loadWires(Caneda::XmlReader *reader);
        void loadPaintings(Caneda::XmlReader *reader);

        SchematicDocument *m_schematicDocument;
    };

} // namespace Caneda

#endif //FORMAT_XML_SCHEMATIC_H
