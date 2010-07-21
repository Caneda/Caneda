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
                "\\bbus\\b" << "\\bcomponent\\b" << "\\bdisconnect\\b" <<
                "\\bdownto\\b" << "\\bend\\b" << "\\bexit\\b" <<
                "\\bfunction\\b" << "\\bgenerate\\b" << "\\bgeneric\\b" <<
                "\\bgroup\\b" << "\\bguarded\\b" << "\\bimpure\\b" <<
                "\\binertial\\b" << "\\blabel\\b" << "\\blinkage\\b" <<
                "\\bliteral\\b" << "\\bmap\\b" << "\\bnew\\b" <<
                "\\bnext\\b" << "\\bnull\\b" << "\\bon\\b" <<
                "\\bopen\\b" << "\\bothers\\b" << "\\bport\\b" <<
                "\\bpostponed\\b" << "\\bprocedure\\b" << "\\bpure\\b" <<
                "\\brange\\b" << "\\brecord\\b" << "\\bregister\\b" <<
                "\\breject\\b" << "\\breport\\b" << "\\breturn\\b" <<
                "\\bselect\\b" << "\\bseverity\\b" << "\\bshared\\b" <<
                "\\bsubtype\\b" << "\\bthen\\b" << "\\bto\\b" <<
                "\\btransport\\b" << "\\bunaffected\\b" << "\\bunits\\b" <<
                "\\buntil\\b" << "\\bwait\\b" << "\\bwhen\\b" <<
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

        attributeFormat.setForeground(Qt::darkCyan);
        QStringList attributePatterns;
        attributePatterns << "\\bsignal\\b" << "\\bvariable\\b" << "\\bconstant\\b" <<
                "\\btype\\b";
        foreach (QString pattern, attributePatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = attributeFormat;
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

        dataFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegExp("\"[A-Za-z0-9]*\"|\'[A-Za-z0-9]*\'|\'event");
        rule.format = dataFormat;
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

        keywordFormat.setForeground(Qt::black);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns;
        keywordPatterns << "\\bmacromodule\\b" << "\\btask\\b" << "\\bendtask\\b" <<
                "\\bfunction\\b" << "\\bendfunction\\b" << "\\btable\\b" <<
                "\\bendtable\\b" << "\\bspecify\\b" << "\\bspecparam\\b" <<
                "\\bendspecify\\b" << "\\bcase\\b" << "\\bcasex\\b" <<
                "\\bcasez\\b" << "\\bendcase\\b" << "\\bfork\\b" <<
                "\\bjoin\\b" << "\\bdefparam\\b" << "\\bdefault\\b" <<
                "\\bifnone\\b" << "\\bforever\\b" << "\\bwait\\b" <<
                "\\bdisable\\b" << "\\bassign\\b" << "\\bdeassign\\b" <<
                "\\bforce\\b" << "\\brelease\\b" << "\\binitial\\b" <<
                "\\bedge\\b" << "\\bposedge\\b" << "\\bnegedge\\b" <<
                "\\bbegin\\b" << "\\bend\\b";
        foreach (QString pattern, keywordPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        typeFormat.setForeground(Qt::blue);
        typeFormat.setFontItalic(true);
        QStringList typePatterns;
        typePatterns << "\\binput\\b" << "\\boutput\\b" << "\\binout\\b" <<
                "\\bwire\\b" << "\\btri\\b" << "\\btri0\\b" <<
                "\\btri1\\b" << "\\bwand\\b" << "\\bwor\\b" <<
                "\\btriand\\b" << "\\btrior\\b" << "\\bsupply0\\b" <<
                "\\bsupply1\\b" << "\\breg\\b" << "\\binteger\\b" <<
                "\\breal\\b" << "\\brealtime\\b" << "\\btime\\b" <<
                "\\bvectored\\b" << "\\bscalared\\b" << "\\btrireg\\b" <<
                "\\bparameter\\b" << "\\bevent\\b";
        foreach (QString pattern, typePatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = typeFormat;
            highlightingRules.append(rule);
        }

        attributeFormat.setForeground(Qt::darkCyan);
        QStringList attributePatterns;
        attributePatterns << "\\bpullup\\b" << "\\bpulldown\\b" << "\\bcmos\\b" <<
                "\\brcmos\\b" << "\\bnmos\\b" << "\\bpmos\\b" <<
                "\\brnmos\\b" << "\\brpmos\\b" << "\\band\\b" <<
                "\\bnand\\b" << "\\bor\\b" << "\\bnor\\b" <<
                "\\bxor\\b" << "\\bxnor\\b" << "\\bnot\\b" <<
                "\\bbuf\\b" << "\\btran\\b" << "\\brtran\\b" <<
                "\\btranif0\\b" << "\\btranif1\\b" << "\\brtranif0\\b" <<
                "\\brtranif1\\b" << "\\bbufif0\\b" << "\\bbufif1\\b" <<
                "\\bnotif0\\b" << "\\bnotif1\\b";
        foreach (QString pattern, attributePatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = attributeFormat;
            highlightingRules.append(rule);
        }

        blockFormat.setForeground(Qt::darkBlue);
        blockFormat.setFontWeight(QFont::Bold);
        QStringList blockPatterns;
        blockPatterns << "\\bif\\b" << "\\belse\\b" << "\\bwhile\\b" <<
                "\\bfor\\b" << "\\brepeat\\b" << "\\balways\\b";
        foreach (QString pattern, blockPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = blockFormat;
            highlightingRules.append(rule);
        }

        classFormat.setForeground(Qt::darkMagenta);
        classFormat.setFontWeight(QFont::Bold);
        QStringList classPatterns;
        classPatterns << "\\bmodule+(?= [A-Za-z0-9_]*\\b)" <<
                "\\bendmodule\\b";
        foreach (QString pattern, classPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = classFormat;
            highlightingRules.append(rule);
        }

        dataFormat.setForeground(Qt::darkGreen);
        QStringList dataPatterns;
        dataPatterns << "\\bstrong0\\b" << "\\bstrong1\\b" << "\\bpull0\\b" <<
                "\\bpull1\\b" << "\\bweak0\\b" << "\\bweak1\\b" <<
                "\\bhighz0\\b" << "\\bhighz1\\b" << "\\bsmall\\b" <<
                "\\bmedium\\b" << "\\blarge\\b" <<
                "\"[A-Za-z0-9]*\"" << "[\\d_]*'d[\\d_]+" << "[\\d_]*'o[0-7xXzZ_]+" <<
                "[\\d_]*'h[\\da-fA-FxXzZ_]+" << "[\\d_]*'b[01_zZxX]+" << "[\\d]*.[\\d]+";
        foreach (QString pattern, dataPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = dataFormat;
            highlightingRules.append(rule);
        }

        singleLineCommentFormat.setForeground(Qt::red);
        rule.pattern = QRegExp("//[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        multiLineCommentFormat.setForeground(Qt::red);
        commentStartExpression = QRegExp("/\\*");
        commentEndExpression = QRegExp("\\*/");

        systemFormat.setForeground(Qt::darkYellow);
        QStringList systemPatterns;
        systemPatterns << "\\$display" << "\\$write" << "\\$swrite" <<
                "\\$sscanf" << "\\$fopen" << "\\$fdisplay" <<
                "\\$fwrite" << "\\$fscanf" << "\\$fclose" <<
                "\\$readmemh" << "\\$readmemb" << "\\$monitor" <<
                "\\$time" << "\\$dumpfile" << "\\$dumpvars" <<
                "\\$dumpports" << "\\$random";
        foreach (QString pattern, systemPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = systemFormat;
            highlightingRules.append(rule);
        }
    }

} // namespace Caneda
