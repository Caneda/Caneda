/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#ifndef FORMAT_XML_LAYOUT_H
#define FORMAT_XML_LAYOUT_H

// Forward declarations
class QString;

namespace Caneda
{
    // Forward declarations
    class CGraphicsScene;
    class LayoutDocument;

    class XmlReader;
    class XmlWriter;

    /*!
     * \brief This class handles all the access to the layout documents file
     * format.
     *
     * This class is in charge of saving and loading all layout related
     * documents. This is the only class that knows about layout document
     * formats, and has the access functions to return a LayoutDocument,
     * with all of its components.
     *
     * \sa \ref DocumentFormats
     */
    class FormatXmlLayout
    {
    public:
        FormatXmlLayout(LayoutDocument *doc = 0);

        bool save();
        bool load();

        LayoutDocument* layoutDocument() const;
        CGraphicsScene* cGraphicsScene() const;
        QString fileName() const;

    private:
        QString saveText();
        void savePaintings(Caneda::XmlWriter *writer);

        bool loadFromText(const QString& text);
        void loadPaintings(Caneda::XmlReader *reader);

        LayoutDocument *m_layoutDocument;
    };

} // namespace Caneda

#endif //FORMAT_XML_LAYOUT_H
