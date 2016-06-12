/***************************************************************************
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

#ifndef SYNTAXHIGHLIGHTERS_H
#define SYNTAXHIGHLIGHTERS_H

#include <QSyntaxHighlighter>

// Forward declarations
class QTextDocument;
class QTextCharFormat;

namespace Caneda
{
    /*!
     * \brief This class implements generic highlighting methods to be used in
     * common by all the different highlighting classes (spice, vhdl, verilog,
     * etc).
     *
     * To implement a new highlighting class, inherit this class and implement
     * the specific highlighting rules, corresponding to the new document type.
     *
     * \sa VhdlHighlighter, VerilogHighlighter, SpiceHighlighter
     */
    class Highlighter : public QSyntaxHighlighter
    {
        Q_OBJECT

    public:
        explicit Highlighter(QTextDocument *parent = 0);

    protected:
        void highlightBlock(const QString &text);

        struct HighlightingRule
        {
            QRegExp pattern;
            QTextCharFormat format;
        };
        QVector<HighlightingRule> highlightingRules;

        QRegExp commentStartExpression;
        QRegExp commentEndExpression;

        QTextCharFormat keywordFormat;
        QTextCharFormat typeFormat;
        QTextCharFormat attributeFormat;
        QTextCharFormat blockFormat;
        QTextCharFormat classFormat;
        QTextCharFormat dataFormat;
        QTextCharFormat singleLineCommentFormat;
        QTextCharFormat multiLineCommentFormat;
        QTextCharFormat systemFormat;
    };

    /*!
     * \brief This class inherits the Highlighter class, and implements the
     * vhdl highlighting rules.
     *
     * \sa Highlighter
     */
    class VhdlHighlighter : public Highlighter
    {
        Q_OBJECT

    public:
        explicit VhdlHighlighter(QTextDocument *parent = 0);
    };

    /*!
     * \brief This class inherits the Highlighter class, and implements the
     * verilog highlighting rules.
     *
     * \sa Highlighter
     */
    class VerilogHighlighter : public Highlighter
    {
        Q_OBJECT

    public:
        explicit VerilogHighlighter(QTextDocument *parent = 0);
    };

    /*!
     * \brief This class inherits the Highlighter class, and implements the
     * spice highlighting rules.
     *
     * \sa Highlighter
     */
    class SpiceHighlighter : public Highlighter
    {
        Q_OBJECT

    public:
        explicit SpiceHighlighter(QTextDocument *parent = 0);
    };

} // namespace Caneda

#endif // SYNTAXHIGHLIGHTERS_H
