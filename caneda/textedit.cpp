/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "textedit.h"

namespace Caneda
{
    TextEdit::TextEdit(QTextDocument *document)
    {
        setDocument(document);

        connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
        highlightCurrentLine();
    }

    TextEdit::~TextEdit()
    {

    }

    void TextEdit::setPointSize(qreal size)
    {
        QFont fnt = font();
        fnt.setPointSize(static_cast<int>(qRound(size)));
        setFont(fnt);
    }

    void TextEdit::focusInEvent(QFocusEvent *event)
    {
        emit focussed();
        QTextEdit::focusInEvent(event);
    }

    void TextEdit::highlightCurrentLine()
    {
        QList<QTextEdit::ExtraSelection> extraSelections;

        if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = QColor(Qt::lightGray).lighter(126);

            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }

        setExtraSelections(extraSelections);
    }

} // namespace Caneda
