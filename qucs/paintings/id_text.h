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

#ifndef ID_TEXT_H
#define ID_TEXT_H

#include "painting.h"

#include <QFont>

/*!
 * \brief Represents parameter of sub ckt.
 *
 * This class simply abstracts the properties of sub ckt component.
 * This objects of this class is used mostly in symbol mode.
 */
struct SubParameter
{
    SubParameter(bool display_, const QString& Name_,
            QString Descr_ = QString(), QString defVal = QString()):
        display(display_),
        name(Name_),
        description(Descr_),
        defaultValue(defVal)
    {};

    QString text() const;

    bool display;
    QString name;
    QString description;
    QString defaultValue;
};

/*!
 * \brief Represents subckt component's id and properties as text item.
 *
 * This class provides convinient approach set subckt properties. Parameters
 * can be defined by user as well which can be added to this using \a addParameter.
 * This also holds the subckt prefix which is later used in netlist.
 */
class IdText : public Painting
{
public:
    enum {
        Type = Painting::IdTextType
    };

    IdText(SchematicScene *scene = 0);
    ~IdText();

    //! \brief Returns the prefix of subckt component.
    QString prefix() const { return m_prefix; }
    void setPrefix(const QString &prefix);

    //! \brief Returns the font used to draw subckt text.
    QFont font() const { return m_font; }
    void setFont(const QFont &font);

    void addParameter(bool display, QString name, QString description = QString(),
            QString defVal = QString());
    void removeParameter(QString name);

    SubParameter* parameter(const QString &name) const;

    //! \brief Disable rotate.
    void rotate90(Qucs::AngleDirection) {};
    //! \brief Disable mirroring.
    void mirrorAlong(Qt::Axis) {};

    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

    void updateGeometry();

    int type() const { return IdText::Type; }
    IdText* copy(SchematicScene *scene) const;

    void saveData(Qucs::XmlWriter *writer) const;
    void loadData(Qucs::XmlReader *reader);

private:
    QString m_prefix;
    QFont m_font;
    QList<SubParameter*> m_parameters;
};

#endif //ID_TEXT_H
