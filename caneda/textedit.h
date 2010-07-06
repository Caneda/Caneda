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

#ifndef CANEDA_TEXTEDIT_H
#define CANEDA_TEXTEDIT_H

#include <QPlainTextEdit>

namespace Caneda
{
    class TextEdit : public QPlainTextEdit
    {
        Q_OBJECT
    public:
        TextEdit(QTextDocument *document);
        ~TextEdit();

        void setPointSize(qreal size);

    Q_SIGNALS:
        void focussed();

    protected:
        void focusInEvent(QFocusEvent *event);

    private slots:
        void highlightCurrentLine();
    };

} // namespace Caneda

#endif // CANEDA_TEXTEDIT_H
