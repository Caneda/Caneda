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

#ifndef FILEFORMATHANDLER_H
#define FILEFORMATHANDLER_H

class QString;
class SchematicScene;

/*!
 * This class is used to save and load files.
 * Using this base class we can support any fileformat
 */
class FileFormatHandler
{
public:
    FileFormatHandler(SchematicScene *scene=0);
    virtual ~FileFormatHandler() {}

    virtual bool save() = 0;

    /*!
     * Loads the document. If non-negative is returned
     * the operation is successful. Negative return
     * value indicated failure
     */
    virtual bool load() = 0;

    SchematicScene* schematicScene() const { return m_schematicScene; }

    static FileFormatHandler* handlerFromSuffix(const QString& extension,
            SchematicScene *scene = 0);

protected:
    SchematicScene *m_schematicScene;
};

#endif //FILEFORMATHANDLER_H

