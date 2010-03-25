/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef XMLFORMAT_H
#define XMLFORMAT_H

#include "fileformathandler.h"
#include "schematicscene.h"

namespace Caneda {
    class XmlReader;
    class XmlWriter;
}

class XmlFormat : public FileFormatHandler
{
public:
    XmlFormat(SchematicScene *scene = 0);
    ~XmlFormat() {}

    bool save();
    bool load();

private:
    QString saveText();
    QString saveSymbolText();
    void saveSchematics(Caneda::XmlWriter *writer);
    void saveView(Caneda::XmlWriter *writer);
    void saveComponents(Caneda::XmlWriter *writer);
    void saveWires(Caneda::XmlWriter *writer);
    void savePaintings(Caneda::XmlWriter *writer);

    void copyCanedaElement(const QString& qualifiedName , Caneda::XmlWriter *writer);

    bool loadFromText(const QString& text);
    bool loadSymbolFromText(const QString& text);
    void loadSchematics(Caneda::XmlReader *reader);
    void loadView(Caneda::XmlReader *reader);
    void loadComponents(Caneda::XmlReader *reader);
    void loadWires(Caneda::XmlReader *reader);
    void loadPaintings(Caneda::XmlReader *reader);
};

#endif //XMLFORMAT_H
