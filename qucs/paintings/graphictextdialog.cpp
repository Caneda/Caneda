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

/* Note: Part of the code is taken from qt4.3/demos/textedit.cpp under GPL V2.0 */

#include "graphictextdialog.h"

#include "mnemo.h"
#include "schematicscene.h"
#include "undocommands.h"

#include "qucs-tools/global.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextList>
#include <QToolBar>

GraphicTextDialog::GraphicTextDialog(GraphicText *text, Qucs::UndoOption opt, QWidget *parent)
: QDialog(parent), textItem(text), undoOption(opt)
{
    mainLayout = new QVBoxLayout(this);

    mainLayout->setSpacing(-1);
    toolBarLayout = new QHBoxLayout;
    mainLayout->addItem(toolBarLayout);
    toolBar = new QToolBar(this);

    toolBar->setIconSize(QSize(16, 16));
    toolBarLayout->addWidget(toolBar);
    toolBarLayout->setSpacing(-1);
    toolBarLayout->setContentsMargins(0,0,0,0);

    setupEditActions();
    setupTextActions();

    textEdit = new QTextEdit;
    if(textItem) {
        QString latex = Qucs::unicodeToLatex(textItem->richText());
        textEdit->setHtml(latex);
    }

    mainLayout->addWidget(textEdit);

    connect(textEdit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
            SLOT(currentCharFormatChanged(const QTextCharFormat &)));
    connect(textEdit, SIGNAL(cursorPositionChanged()), SLOT(cursorPositionChanged()));

    textEdit->setFocus();

    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());
    subSuperAlignmentChanged(textEdit->currentCharFormat().verticalAlignment());

    connect(textEdit->document(), SIGNAL(undoAvailable(bool)),
            actionUndo, SLOT(setEnabled(bool)));
    connect(textEdit->document(), SIGNAL(redoAvailable(bool)),
            actionRedo, SLOT(setEnabled(bool)));

    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

    connect(actionUndo, SIGNAL(triggered()), textEdit, SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), textEdit, SLOT(redo()));

    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    connect(actionCut, SIGNAL(triggered()), textEdit, SLOT(cut()));
    connect(actionCopy, SIGNAL(triggered()), textEdit, SLOT(copy()));
    connect(actionPaste, SIGNAL(triggered()), textEdit, SLOT(paste()));

    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                Qt::Horizontal, this);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
    adjustSize();
}

GraphicTextDialog::~GraphicTextDialog()
{
}

QString GraphicTextDialog::plainText() const
{
    return textEdit->toPlainText();
}

QString GraphicTextDialog::richText() const
{
    return textEdit->toHtml();
}

void GraphicTextDialog::accept()
{
    if(plainText().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("The text must not be empty!"));
    }
    else {
        if(textItem) {
            SchematicScene *scene = textItem->schematicScene();

            QString oldText = textItem->richText();
            QString newText = richText();
            if(oldText != newText) {
                if(undoOption == Qucs::PushUndoCmd) {
                    QUndoCommand *cmd = new GraphicTextChangeCmd(textItem, oldText, newText);
                    scene->undoStack()->push(cmd);
                }
                else {
                    textItem->setText(newText);
                }
            }
        }
        QDialog::accept();
    }
}

void GraphicTextDialog::setupEditActions()
{
    QToolBar *tb = toolBar;

    QAction *a;

    QString rsrcPath = Qucs::bitmapDirectory();
    a = actionUndo = new QAction(QIcon(rsrcPath + "undo.png"), tr("&Undo"), this);
    a->setShortcut(QKeySequence::Undo);
    tb->addAction(a);

    a = actionRedo = new QAction(QIcon(rsrcPath + "redo.png"), tr("&Redo"), this);
    a->setShortcut(QKeySequence::Redo);
    tb->addAction(a);


    a = actionCut = new QAction(QIcon(rsrcPath + "editcut.png"), tr("Cu&t"), this);
    a->setShortcut(QKeySequence::Cut);
    tb->addAction(a);

    a = actionCopy = new QAction(QIcon(rsrcPath + "editcopy.png"), tr("&Copy"), this);
    a->setShortcut(QKeySequence::Copy);
    tb->addAction(a);

    a = actionPaste = new QAction(QIcon(rsrcPath + "editpaste.png"), tr("&Paste"), this);
    a->setShortcut(QKeySequence::Paste);
    tb->addAction(a);

    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    tb->addSeparator();
}

void GraphicTextDialog::setupTextActions()
{
    QToolBar *tb = toolBar;

    QString rsrcPath = Qucs::bitmapDirectory();
    actionTextBold = new QAction(QIcon(rsrcPath + "textbold.png"), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    tb->addAction(actionTextBold);

    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon(rsrcPath + "textitalic.png"), tr("&Italic"), this);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    tb->addAction(actionTextItalic);

    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon(rsrcPath + "textunder.png"), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    tb->addAction(actionTextUnderline);

    actionTextUnderline->setCheckable(true);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction *)), this, SLOT(textAlign(QAction *)));

    actionAlignLeft = new QAction(QIcon(rsrcPath + "textleft.png"), tr("&Left"), grp);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignCenter = new QAction(QIcon(rsrcPath + "textcenter.png"), tr("C&enter"), grp);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignRight = new QAction(QIcon(rsrcPath + "textright.png"), tr("&Right"), grp);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignJustify = new QAction(QIcon(rsrcPath + "textjustify.png"), tr("&Justify"), grp);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);

    tb->addActions(grp->actions());
    tb->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    tb->addAction(actionTextColor);

    tb = new QToolBar(this);
    tb->setIconSize(QSize(16, 16));
    mainLayout->insertWidget(2, tb);
    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    connect(comboStyle, SIGNAL(activated(int)),
            this, SLOT(textStyle(int)));

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, SIGNAL(activated(const QString &)),
            this, SLOT(textFamily(const QString &)));
    comboFont->setCurrentFont(font());

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    QFontDatabase db;
    foreach(int size, db.standardSizes()) {
        comboSize->addItem(QString::number(size));
    }

    connect(comboSize, SIGNAL(activated(const QString &)),
            this, SLOT(textSize(const QString &)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                    .pointSize())));
    tb->addSeparator();
    grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction *)), this, SLOT(textAlignSubSuperScript(QAction *)));

    actionAlignSubscript = new QAction(QIcon(rsrcPath + "sub.png"), tr("Subscript"), grp);
    actionAlignSubscript->setCheckable(true);

    actionAlignSupersript = new QAction(QIcon(rsrcPath + "super.png"), tr("Superscript"), grp);
    actionAlignSupersript->setCheckable(true);

    actionAlignNormalscript = new QAction(QIcon(rsrcPath + "text.png"), tr("Normal"), grp);
    actionAlignNormalscript->setCheckable(true);

    tb->addActions(grp->actions());
}

void GraphicTextDialog::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void GraphicTextDialog::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void GraphicTextDialog::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void GraphicTextDialog::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void GraphicTextDialog::textSize(const QString &p)
{
    QTextCharFormat fmt;
    fmt.setFontPointSize(p.toFloat());
    mergeFormatOnWordOrSelection(fmt);
}

void GraphicTextDialog::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();

    if(styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;

            case 2:
                style = QTextListFormat::ListCircle;
                break;

            case 3:
                style = QTextListFormat::ListSquare;
                break;

            case 4:
                style = QTextListFormat::ListDecimal;
                break;

            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;

            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if(cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void GraphicTextDialog::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if(!col.isValid()) {
        return;
    }
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void GraphicTextDialog::textAlign(QAction *a)
{
    if(a == actionAlignLeft) {
        textEdit->setAlignment(Qt::AlignLeft);
    }
    else if(a == actionAlignCenter) {
        textEdit->setAlignment(Qt::AlignHCenter);
    }
    else if(a == actionAlignRight) {
        textEdit->setAlignment(Qt::AlignRight);
    }
    else if(a == actionAlignJustify) {
        textEdit->setAlignment(Qt::AlignJustify);
    }
}

void GraphicTextDialog::textAlignSubSuperScript(QAction *a)
{
    QTextCharFormat fmt;
    if(a == actionAlignSubscript) {
        fmt.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    }
    else if(a == actionAlignSupersript) {
        fmt.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    }
    else {
        fmt.setVerticalAlignment(QTextCharFormat::AlignNormal);
    }
    mergeFormatOnWordOrSelection(fmt);
}

void GraphicTextDialog::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
    subSuperAlignmentChanged(format.verticalAlignment());
}

void GraphicTextDialog::cursorPositionChanged()
{
    alignmentChanged(textEdit->alignment());
}

void GraphicTextDialog::clipboardDataChanged()
{
    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void GraphicTextDialog::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void GraphicTextDialog::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void GraphicTextDialog::subSuperAlignmentChanged(QTextCharFormat::VerticalAlignment align)
{
    if(align == QTextCharFormat::AlignNormal) {
        actionAlignNormalscript->setChecked(true);
    }
    else if(align == QTextCharFormat::AlignSubScript) {
        actionAlignSubscript->setChecked(true);
    }
    else if(align == QTextCharFormat::AlignSuperScript) {
        actionAlignSupersript->setChecked(true);
    }
}

void GraphicTextDialog::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void GraphicTextDialog::alignmentChanged(Qt::Alignment a)
{
    if(a & Qt::AlignLeft) {
        actionAlignLeft->setChecked(true);
    } else if(a & Qt::AlignHCenter) {
        actionAlignCenter->setChecked(true);
    } else if(a & Qt::AlignRight) {
        actionAlignRight->setChecked(true);
    } else if(a & Qt::AlignJustify) {
        actionAlignJustify->setChecked(true);
    }
}
