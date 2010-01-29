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

/* Note: Part of the code is taken from qt4.3/demos/textedit.h under GPL V2.0 */

#ifndef GRAPHICTEXTDLG_H
#define GRAPHICTEXTDLG_H

#include "graphictext.h"

#include <QDialog>
#include <QTextFormat>

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QToolBar;
class QHBoxLayout;
class QVBoxLayout;

class GraphicTextDialog : public QDialog
{
    Q_OBJECT;

public:
    GraphicTextDialog(GraphicText *text, Qucs::UndoOption opt,
            QWidget *parent = 0);
    ~GraphicTextDialog();

    QString plainText() const;
    QString richText() const;

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void textAlignSubSuperScript(QAction *);
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();

private:
    void setupEditActions();
    void setupTextActions();

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);
    void subSuperAlignmentChanged(QTextCharFormat::VerticalAlignment a);

    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionAlignSubscript;
    QAction *actionAlignSupersript;
    QAction *actionAlignNormalscript;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar *toolBar;
    QString fileName;
    QTextEdit *textEdit;

    QHBoxLayout *toolBarLayout;
    QVBoxLayout *mainLayout;

    GraphicText *textItem;
    Qucs::UndoOption undoOption;
};

#endif
