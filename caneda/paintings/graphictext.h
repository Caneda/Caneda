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

#ifndef GRAPHICTEXT_H
#define GRAPHICTEXT_H

#include "painting.h"

//! \brief Represent's text item on schematic.
class GraphicText : public Painting
{
public:
    enum {
        Type = Painting::GraphicTextType
    };

    GraphicText(const QString &text = QString(), SchematicScene *scene = 0);
    ~GraphicText();

    QString plainText() const;
    void setPlainText(const QString &text);

    QString richText() const;
    void setRichText(const QString &text);

    void setText(const QString &text);

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    int type() const { return GraphicText::Type; }
    GraphicText* copy(SchematicScene *scene = 0) const;

    void saveData(Qucs::XmlWriter *writer) const;
    void loadData(Qucs::XmlReader *reader);

    int launchPropertyDialog(Qucs::UndoOption opt);

private:
    QGraphicsTextItem *m_textItem;
};

#endif //GRAPHICTEXT_H
