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

#ifndef QUCSPRIMARYFORMAT_H
#define QUCSPRIMARYFORMAT_H

#include "fileformathandler.h"

class QTextStream;

class QucsPrimaryFormat : public FileFormatHandler
{
public:
    QucsPrimaryFormat(SchematicView *view = 0);
    ~QucsPrimaryFormat() {}


    QString saveText();
    bool loadFromText(const QString& text);

    bool loadProperties(QTextStream &stream);
    bool loadComponents(QTextStream &stream);
    bool loadWires(QTextStream &stream);
    bool loadDiagrams(QTextStream &stream);
    bool loadPaintings(QTextStream &stream);
    bool loadWireFromLine(QString s);
};

#endif //QUCSPRIMARYFORMAT_H
