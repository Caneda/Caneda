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

#include "fileformathandler.h"

#include "schematicdocument.h"
#include "xmlformat.h"
#include "xmlsymbolformat.h"

#include <QString>

namespace Caneda
{
    //! Constructor
    FileFormatHandler::FileFormatHandler(SchematicDocument *document) :
        m_schematicDocument(document)
    {
    }

    /*!
     * \brief Factory method to return appropritate file handler based on file
     * format.
     *
     * Returns NULL if there doesn't exist a handler for given extension.
     */
    FileFormatHandler* FileFormatHandler::handlerFromSuffix(const QString& ext,
            SchematicDocument *document)
    {
        if(ext == "xsch") {
            return new XmlFormat(document);
        }
        else if(ext == "xsym") {
            return new XmlSymbolFormat(document);
        }
        return 0;
    }

    SchematicDocument* FileFormatHandler::schematicDocument() const
    {
        return m_schematicDocument;
    }

    SchematicScene* FileFormatHandler::schematicScene() const
    {
        return m_schematicDocument ? m_schematicDocument->schematicScene() : 0;
    }

    QString FileFormatHandler::fileName() const
    {
        return m_schematicDocument ? m_schematicDocument->fileName() : QString();
    }

} // namespace Caneda
