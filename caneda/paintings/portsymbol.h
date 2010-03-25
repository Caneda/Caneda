/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef PORTSYMBOL_H
#define PORTSYMBOL_H

#include "painting.h"

#include <QFont>

/*!
 * \brief Represents the port ellipse and port id on schematic.
 *
 * This item is used mostly only in symbol mode. It represents the
 * port location in the subcircuit symbol.
 */
class PortSymbol : public Painting
{
public:
    enum {
        Type = Painting::PortSymbolType
    };

    PortSymbol(const QString& nameStr_= "1",
            const QString& numberStr_= "",
            SchematicScene *scene = 0);

    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

    //! \brief Returns the number part of port id.
    QString numberString() const { return m_numberString; }
    void setNumberString(int num) { setNumberString(QString::number(num)); }
    void setNumberString(QString str);

    //! \brief Returns the name part of port id.
    QString nameString() const { return m_nameString; }
    void setNameString(QString name);

    //! \brief Returns name and number part combined into one string.
    QString text() const { return m_numberString + m_nameString; }

    void updateGeometry();

    //! \brief Returns the font used to draw port id.
    QFont font() const { return m_font; }
    void setFont(const QFont &font);

    void mirrorAlong(Qt::Axis axis);

    int type() const { return PortSymbol::Type; }
    PortSymbol* copy(SchematicScene *scene = 0) const;

    void saveData(Qucs::XmlWriter *writer) const;
    void loadData(Qucs::XmlReader *reader);

private:
    bool m_mirrored;

    QString m_numberString;
    QString m_nameString;
    QFont m_font;
};

#endif //PORTSYMBOL_H
