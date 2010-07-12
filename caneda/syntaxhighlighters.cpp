/***************************************************************************
 * Copyright 2010 Pablo Daniel Pareja Obregon                              *
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

#include "syntaxhighlighters.h"

#include <QtGui>

namespace Caneda
{

    VhdlHighlighter::VhdlHighlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent)
    {
        HighlightingRule rule;

        keywordFormat.setForeground(Qt::darkBlue);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns;
        keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                << "\\bvoid\\b" << "\\bvolatile\\b";
        foreach (QString pattern, keywordPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        classFormat.setFontWeight(QFont::Bold);
        classFormat.setForeground(Qt::darkMagenta);
        rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
        rule.format = classFormat;
        highlightingRules.append(rule);

        singleLineCommentFormat.setForeground(Qt::red);
        rule.pattern = QRegExp("//[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        multiLineCommentFormat.setForeground(Qt::red);

        quotationFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegExp("\".*\"");
        rule.format = quotationFormat;
        highlightingRules.append(rule);

        functionFormat.setFontItalic(true);
        functionFormat.setForeground(Qt::blue);
        rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
        rule.format = functionFormat;
        highlightingRules.append(rule);

        commentStartExpression = QRegExp("/\\*");
        commentEndExpression = QRegExp("\\*/");
    }

    void VhdlHighlighter::highlightBlock(const QString &text)
    {
        foreach (HighlightingRule rule, highlightingRules) {
            QRegExp expression(rule.pattern);
            int index = text.indexOf(expression);
            while (index >= 0) {
                int length = expression.matchedLength();
                setFormat(index, length, rule.format);
                index = text.indexOf(expression, index + length);
            }
        }
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf(commentStartExpression);

        while (startIndex >= 0) {
            int endIndex = text.indexOf(commentEndExpression, startIndex);
            int commentLength;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex
                                + commentEndExpression.matchedLength();
            }
            setFormat(startIndex, commentLength, multiLineCommentFormat);
            startIndex = text.indexOf(commentStartExpression,
                                      startIndex + commentLength);
        }
    }

} // namespace Caneda
