/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>

namespace Caneda
{
    /*!
     * \brief This class implements a very basic text editor, to be used in
     * conjuction with the \ref DocumentViewFramework.
     *
     * In a manner similar to Qt's Graphics View Architecture, the TextView
     * class provides the view widget, which visualizes the contents of the
     * document. The view is included as a pointer to TextEdit, that contains
     * all the view specific methods. This way, this class represents the
     * QObject editor counterpart for the TextView class.
     *
     * Although conceptually the TextEdit class should be part of the TextView
     * class implementation, abstracting the \ref DocumentViewFramework from
     * the QObject implementations increases the resulting code quality and
     * readability a lot (although there is a slight increase in the number of
     * classes used).
     *
     * \sa \ref DocumentViewFramework, TextView
     */
    class TextEdit : public QPlainTextEdit
    {
        Q_OBJECT

    public:
        explicit TextEdit(QTextDocument *document, QWidget *parent = 0);

    Q_SIGNALS:
        void focussed();
        void cursorPositionChanged(const QString& newPos);

    protected:
        void focusInEvent(QFocusEvent *event);

    private slots:
        void updateCursorPosition();
        void highlightCurrentLine();
    };

} // namespace Caneda

#endif //TEXTEDIT_H
