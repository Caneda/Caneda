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

#include "graphictextdialog.h"

#include "global.h"
#include "graphicsscene.h"
#include "undocommands.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
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

namespace Caneda
{
    //! \brief Constructor.
    GraphicTextDialog::GraphicTextDialog(GraphicText *text,
                                         bool enableUndoCommand,
                                         QWidget *parent) :
        QDialog(parent),
        textItem(text),
        enableUndoCommand(enableUndoCommand)
    {
        mainLayout = new QVBoxLayout(this);
        toolBarLayout = new QHBoxLayout;

        toolBar = new QToolBar(this);

        toolBarLayout->addWidget(toolBar);
        mainLayout->addItem(toolBarLayout);

        setupEditActions();
        setupTextActions();

        textEdit = new QTextEdit;
        if(textItem) {
            QString latex = Caneda::unicodeToLatex(textItem->richText());
            textEdit->setHtml(latex);
        }

        mainLayout->addWidget(textEdit);

        connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &GraphicTextDialog::currentCharFormatChanged);
        connect(textEdit, &QTextEdit::cursorPositionChanged,    this, &GraphicTextDialog::cursorPositionChanged);

        textEdit->setFocus();

        fontChanged(textEdit->font());
        colorChanged(textEdit->textColor());
        alignmentChanged(textEdit->alignment());
        subSuperAlignmentChanged(textEdit->currentCharFormat().verticalAlignment());

        connect(textEdit->document(), &QTextDocument::undoAvailable, actionUndo, &QAction::setEnabled);
        connect(textEdit->document(), &QTextDocument::redoAvailable, actionRedo, &QAction::setEnabled);

        actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
        actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

        connect(actionUndo, &QAction::triggered, textEdit, &QTextEdit::undo);
        connect(actionRedo, &QAction::triggered, textEdit, &QTextEdit::redo);

        actionCut->setEnabled(false);
        actionCopy->setEnabled(false);

        connect(actionCut,   &QAction::triggered, textEdit, &QTextEdit::cut);
        connect(actionCopy,  &QAction::triggered, textEdit, &QTextEdit::copy);
        connect(actionPaste, &QAction::triggered, textEdit, &QTextEdit::paste);

        connect(textEdit, &QTextEdit::copyAvailable, actionCut,  &QAction::setEnabled);
        connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

        connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &GraphicTextDialog::clipboardDataChanged);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                           Qt::Horizontal, this);

        buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));

        connect(buttonBox, &QDialogButtonBox::accepted, this, &GraphicTextDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &GraphicTextDialog::reject);

        mainLayout->addWidget(buttonBox);
        adjustSize();
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
                GraphicsScene *scene = qobject_cast<GraphicsScene*>(textItem->scene());

                QString oldText = textItem->richText();
                QString newText = richText();
                if(oldText != newText) {
                    if(enableUndoCommand == true) {
                        QUndoCommand *cmd = new ChangeGraphicTextCmd(textItem, oldText, newText);
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

        a = actionUndo = new QAction(Caneda::icon("edit-undo"), tr("&Undo"), this);
        a->setShortcut(QKeySequence::Undo);
        tb->addAction(a);

        a = actionRedo = new QAction(Caneda::icon("edit-redo"), tr("&Redo"), this);
        a->setShortcut(QKeySequence::Redo);
        tb->addAction(a);

        a = actionCut = new QAction(Caneda::icon("edit-cut"), tr("Cu&t"), this);
        a->setShortcut(QKeySequence::Cut);
        tb->addAction(a);

        a = actionCopy = new QAction(Caneda::icon("edit-copy"), tr("&Copy"), this);
        a->setShortcut(QKeySequence::Copy);
        tb->addAction(a);

        a = actionPaste = new QAction(Caneda::icon("edit-paste"), tr("&Paste"), this);
        a->setShortcut(QKeySequence::Paste);
        tb->addAction(a);

        actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
        tb->addSeparator();
    }

    void GraphicTextDialog::setupTextActions()
    {
        QToolBar *tb = toolBar;

        actionTextBold = new QAction(Caneda::icon("format-text-bold"), tr("&Bold"), this);
        actionTextBold->setShortcut(Qt::CTRL | Qt::Key_B);
        QFont bold;
        bold.setBold(true);
        actionTextBold->setFont(bold);
        connect(actionTextBold, &QAction::triggered, this, &GraphicTextDialog::textBold);
        tb->addAction(actionTextBold);

        actionTextBold->setCheckable(true);

        actionTextItalic = new QAction(Caneda::icon("format-text-italic"), tr("&Italic"), this);
        actionTextItalic->setShortcut(Qt::CTRL | Qt::Key_I);
        QFont italic;
        italic.setItalic(true);
        actionTextItalic->setFont(italic);
        connect(actionTextItalic, &QAction::triggered, this, &GraphicTextDialog::textItalic);
        tb->addAction(actionTextItalic);

        actionTextItalic->setCheckable(true);

        actionTextUnderline = new QAction(Caneda::icon("format-text-underline"), tr("&Underline"), this);
        actionTextUnderline->setShortcut(Qt::CTRL | Qt::Key_U);
        QFont underline;
        underline.setUnderline(true);
        actionTextUnderline->setFont(underline);
        connect(actionTextUnderline, &QAction::triggered, this, &GraphicTextDialog::textUnderline);
        tb->addAction(actionTextUnderline);

        actionTextUnderline->setCheckable(true);

        QActionGroup *grp = new QActionGroup(this);
        connect(grp, &QActionGroup::triggered, this, &GraphicTextDialog::textAlign);

        actionAlignLeft = new QAction(Caneda::icon("format-justify-left"), tr("&Left"), this);
        actionAlignLeft->setShortcut(Qt::CTRL | Qt::Key_L);
        actionAlignLeft->setCheckable(true);
        grp->addAction(actionAlignLeft);
        actionAlignCenter = new QAction(Caneda::icon("format-justify-center"), tr("C&enter"), this);
        actionAlignCenter->setShortcut(Qt::CTRL | Qt::Key_E);
        actionAlignCenter->setCheckable(true);
        grp->addAction(actionAlignCenter);
        actionAlignRight = new QAction(Caneda::icon("format-justify-right"), tr("&Right"), this);
        actionAlignRight->setShortcut(Qt::CTRL | Qt::Key_R);
        actionAlignRight->setCheckable(true);
        grp->addAction(actionAlignRight);
        actionAlignJustify = new QAction(Caneda::icon("format-justify-fill"), tr("&Justify"), this);
        actionAlignJustify->setShortcut(Qt::CTRL | Qt::Key_J);
        actionAlignJustify->setCheckable(true);
        grp->addAction(actionAlignJustify);

        tb->addActions(grp->actions());
        tb->addSeparator();

        QPixmap pix(16, 16);
        pix.fill(Qt::black);
        actionTextColor = new QAction(pix, tr("&Color..."), this);
        connect(actionTextColor, &QAction::triggered, this, &GraphicTextDialog::textColor);
        tb->addAction(actionTextColor);

        tb = new QToolBar(this);
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
        connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &GraphicTextDialog::textStyle);

        comboFont = new QFontComboBox(tb);
        tb->addWidget(comboFont);
        connect(comboFont, &QFontComboBox::textActivated, this, &GraphicTextDialog::textFamily);
        comboFont->setCurrentFont(font());

        comboSize = new QComboBox(tb);
        comboSize->setObjectName("comboSize");
        tb->addWidget(comboSize);
        comboSize->setEditable(true);

        QFontDatabase db;
        foreach(int size, db.standardSizes()) {
            comboSize->addItem(QString::number(size));
        }

        connect(comboSize, &QComboBox::textActivated, this, &GraphicTextDialog::textSize);

        comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                        .pointSize())));
        tb->addSeparator();
        grp = new QActionGroup(this);
        connect(grp, &QActionGroup::triggered, this, &GraphicTextDialog::textAlignSubSuperScript);

        actionAlignSubscript = new QAction(Caneda::icon("format-text-subscript"), tr("Subscript"), this);
        actionAlignSubscript->setCheckable(true);
        grp->addAction(actionAlignSubscript);

        actionAlignSupersript = new QAction(Caneda::icon("format-text-superscript"), tr("Superscript"), this);
        actionAlignSupersript->setCheckable(true);
        grp->addAction(actionAlignSupersript);

        actionAlignNormalscript = new QAction(Caneda::icon("format-text-bold"), tr("Normal"), this);
        actionAlignNormalscript->setCheckable(true);
        grp->addAction(actionAlignNormalscript);

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
        fmt.setFontFamilies({f});
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

} // namespace Caneda
