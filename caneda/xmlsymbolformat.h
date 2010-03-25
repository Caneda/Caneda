/***************************************************************************
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
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

#ifndef XMLSYMBOLFORMAT_H
#define XMLSYMBOLFORMAT_H

#include "fileformathandler.h"

namespace Qucs {
    class XmlReader;
}

class XmlSymbolFormat : public FileFormatHandler
{
public:
    XmlSymbolFormat(SchematicScene *scene = 0);
    ~XmlSymbolFormat() {}

    bool save();
    bool load();

private:
    QString saveText();
};

#endif //XMLSYMBOLFORMAT_H
