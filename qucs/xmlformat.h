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

namespace Qucs {
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
    void saveSchematics(Qucs::XmlWriter *writer);
    void saveView(Qucs::XmlWriter *writer);
    void saveComponents(Qucs::XmlWriter *writer);
    void saveWires(Qucs::XmlWriter *writer);
    void savePaintings(Qucs::XmlWriter *writer);

    void copyQucsElement(const QString& qualifiedName , Qucs::XmlWriter *writer);

    bool loadFromText(const QString& text);
    bool loadSymbolFromText(const QString& text);
    void loadSchematics(Qucs::XmlReader *reader);
    void loadView(Qucs::XmlReader *reader);
    void loadComponents(Qucs::XmlReader *reader);
    void loadWires(Qucs::XmlReader *reader);
    void loadPaintings(Qucs::XmlReader *reader);
};

#endif //XMLFORMAT_H
