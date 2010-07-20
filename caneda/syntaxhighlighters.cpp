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

    //*************************************************************
    //***************** Abstract highlighter **********************
    //*************************************************************

    Highlighter::Highlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent)
    {
    }

    void Highlighter::highlightBlock(const QString &text)
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

    //*************************************************************
    //******************** Vhdl highlighter ***********************
    //*************************************************************

    VhdlHighlighter::VhdlHighlighter(QTextDocument *parent)
        : Highlighter(parent)
    {
        HighlightingRule rule;

        keywordFormat.setForeground(Qt::black);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns;
        keywordPatterns << "\\bfile\\b" << "\\bpackage\\b" << "\\blibrary\\b" <<
                "\\buse\\b" << "\\baccess\\b" << "\\bafter\\b" <<
                "\\balias\\b" << "\\ball\\b" << "\\bassert\\b" <<
                "\\bbegin\\b" << "\\bblock\\b" << "\\bbody\\b" <<
                "\\bbus\\b" << "\\bcomponent\\b" <<
                "\\bdisconnect\\b" << "\\bdownto\\b" << "\\bend\\b" <<
                "\\bexit\\b" << "\\bfunction\\b" << "\\bgenerate\\b" <<
                "\\bgeneric\\b" << "\\bgroup\\b" << "\\bguarded\\b" <<
                "\\bimpure\\b" << "\\binertial\\b" <<
                "\\blabel\\b" << "\\blinkage\\b" << "\\bliteral\\b" <<
                "\\bmap\\b" << "\\bnew\\b" << "\\bnext\\b" <<
                "\\bnull\\b" << "\\bon\\b" << "\\bopen\\b" <<
                "\\bothers\\b" << "\\bport\\b" << "\\bpostponed\\b" <<
                "\\bprocedure\\b" << "\\bpure\\b" <<
                "\\brange\\b" << "\\brecord\\b" << "\\bregister\\b" <<
                "\\breject\\b" << "\\breport\\b" << "\\breturn\\b" <<
                "\\bselect\\b" << "\\bseverity\\b" << "\\bshared\\b" <<
                "\\bsubtype\\b" << "\\bthen\\b" <<
                "\\bto\\b" << "\\btransport\\b" <<
                "\\bunaffected\\b" << "\\bunits\\b" << "\\buntil\\b" <<
                "\\bwait\\b" << "\\bwhen\\b" <<
                "\\bwith\\b" << "\\bnote\\b" << "\\bwarning\\b" <<
                "\\berror\\b" << "\\bfailure\\b" << "\\bin\\b" <<
                "\\binout\\b" << "\\bout\\b" << "\\bbuffer\\b" <<
                "\\band\\b" << "\\bor\\b" << "\\bxor\\b" <<
                "\\bnot\\b";
        foreach (QString pattern, keywordPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        typeFormat.setForeground(Qt::blue);
        typeFormat.setFontItalic(true);
        QStringList typePatterns;
        typePatterns << "\\bbit\\b" << "\\bbit_vector\\b" << "\\bcharacter\\b" <<
                "\\bboolean\\b" << "\\binteger\\b" << "\\breal\\b" <<
                "\\btime\\b" << "\\bstring\\b" << "\\bseverity_level\\b" <<
                "\\bpositive\\b" << "\\bnatural\\b" << "\\bsigned\\b" <<
                "\\bunsigned\\b" << "\\bline\\b" << "\\btext\\b" <<
                "\\bstd_logic\\b" << "\\bstd_logic_vector\\b" << "\\bstd_ulogic\\b" <<
                "\\bstd_ulogic_vector\\b" << "\\bqsim_state\\b" << "\\bqsim_state_vector\\b" <<
                "\\bqsim_12state\\b" << "\\bqsim_12state_vector\\b" << "\\bqsim_strength\\b" <<
                "\\bmux_bit\\b" << "\\bmux_vector\\b" << "\\breg_bit\\b" <<
                "\\breg_vector\\b" << "\\bwor_bit\\b" << "\\bwor_vector\\b";
        foreach (QString pattern, typePatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = typeFormat;
            highlightingRules.append(rule);
        }

        signalFormat.setForeground(Qt::darkCyan);
        QStringList signalPatterns;
        signalPatterns << "\\bsignal\\b" << "\\bvariable\\b" << "\\bconstant\\b" <<
                "\\btype\\b";
        foreach (QString pattern, signalPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = signalFormat;
            highlightingRules.append(rule);
        }

        blockFormat.setForeground(Qt::darkBlue);
        blockFormat.setFontWeight(QFont::Bold);
        QStringList blockPatterns;
        blockPatterns << "\\bprocess\\b" << "\\bif\\b" << "\\belse\\b" <<
                "\\belsif\\b" << "\\bloop\\b" << "\\bend if\\b";
        foreach (QString pattern, blockPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = blockFormat;
            highlightingRules.append(rule);
        }

        classFormat.setForeground(Qt::darkMagenta);
        classFormat.setFontWeight(QFont::Bold);
        QStringList classPatterns;
        classPatterns << "\\barchitecture+(?= [A-Za-z0-9_]* of [A-Za-z0-9_]* is\\b)" <<
                "\\bentity+(?= [A-Za-z0-9_]* is\\b)" <<
                "\\bcomponent+(?= [A-Za-z0-9_]*\\b)" <<
                "\\bpackage+(?= [A-Za-z0-9_]* is\\b)" <<
                "\\bpackage body+(?= [A-Za-z0-9_]* is\\b)" <<
                "\\bof\\b" << "\\bis\\b";
        foreach (QString pattern, classPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = classFormat;
            highlightingRules.append(rule);
        }

        quotationFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegExp("\"[A-Za-z0-9]*\"|\'[A-Za-z0-9]*\'|\'event");
        rule.format = quotationFormat;
        highlightingRules.append(rule);

        singleLineCommentFormat.setForeground(Qt::red);
        rule.pattern = QRegExp("--[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        multiLineCommentFormat.setForeground(Qt::red);
        commentStartExpression = QRegExp("/\\*");
        commentEndExpression = QRegExp("\\*/");
    }

    //*************************************************************
    //****************** Verilog highlighter **********************
    //*************************************************************

    VerilogHighlighter::VerilogHighlighter(QTextDocument *parent)
        : Highlighter(parent)
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

        quotationFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegExp("\".*\"");
        rule.format = quotationFormat;
        highlightingRules.append(rule);

        singleLineCommentFormat.setForeground(Qt::red);
        rule.pattern = QRegExp("//[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        multiLineCommentFormat.setForeground(Qt::red);
        commentStartExpression = QRegExp("/\\*");
        commentEndExpression = QRegExp("\\*/");
    }

} // namespace Caneda
