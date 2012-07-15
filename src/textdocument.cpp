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

#include "textdocument.h"

#include "documentviewmanager.h"
#include "syntaxhighlighters.h"
#include "textcontext.h"
#include "textedit.h"
#include "textview.h"

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextStream>
#include <QUndoStack>

namespace Caneda
{
    TextDocument::TextDocument()
    {
        m_textDocument = new QTextDocument;
        m_textDocument->setModified(false);

        connect(m_textDocument, SIGNAL(undoAvailable(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_textDocument, SIGNAL(redoAvailable(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_textDocument, SIGNAL(modificationChanged(bool)),
                this, SLOT(emitDocumentChanged()));
        connect(m_textDocument, SIGNAL(contentsChanged()),
                this, SLOT(onContentsChanged()));
    }

    TextDocument::~TextDocument()
    {
        delete m_textDocument;
    }

    IContext* TextDocument::context()
    {
        return TextContext::instance();
    }

    bool TextDocument::isModified() const
    {
        return m_textDocument->isModified();
    }

    bool TextDocument::canUndo() const
    {
        return m_textDocument->isUndoAvailable();
    }

    bool TextDocument::canRedo() const
    {
        return m_textDocument->isRedoAvailable();
    }

    void TextDocument::undo()
    {
        m_textDocument->undo();
    }

    void TextDocument::redo()
    {
        m_textDocument->redo();
    }

    QUndoStack* TextDocument::undoStack()
    {
        QUndoStack *stack = new QUndoStack(this);
        return stack;
    }

    bool TextDocument::canCut() const
    {
        return true;
    }

    bool TextDocument::canCopy() const
    {
        return true;
    }

    bool TextDocument::canPaste() const
    {
        return true;
    }

    void TextDocument::cut()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->cut();
    }

    void TextDocument::copy()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->copy();
    }

    void TextDocument::paste()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->paste();
    }

    void TextDocument::selectAll()
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->selectAll();
    }

    bool TextDocument::printSupportsFitInPage() const
    {
        return false;
    }

    void TextDocument::print(QPrinter *printer, bool fitInView)
    {
        Q_UNUSED(fitInView);

        m_textDocument->print(printer);
    }

    bool TextDocument::load(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty filename");
            }
            return false;
        }

        QFile file(fileName());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage) {
                *errorMessage = tr("Could not open file for reading");
            }
            return false;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        QString content = stream.readAll();
        m_textDocument->setPlainText(content);

        file.close();

        QFileInfo fileInfo(fileName());
        if ( fileInfo.suffix() == "vhdl" || fileInfo.suffix() == "vhd" ) {
            VhdlHighlighter *highlighter = new VhdlHighlighter(m_textDocument);
        }
        else if ( fileInfo.suffix() == "v" ) {
            VerilogHighlighter *highlighter = new VerilogHighlighter(m_textDocument);
        }
        else if ( fileInfo.suffix() == "net" ||
                  fileInfo.suffix() == "cir" ||
                  fileInfo.suffix() == "spc" ) {
            SpiceHighlighter *highlighter = new SpiceHighlighter(m_textDocument);
        }

        m_textDocument->setModified(false);
        return true;
    }

    bool TextDocument::save(QString *errorMessage)
    {
        if (fileName().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Empty filename");
            }
            return false;
        }

        QFile file(fileName());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            if (errorMessage) {
                *errorMessage = tr("Could not open file for writing");
            }
            return false;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        stream << m_textDocument->toPlainText();

        file.close();

        m_textDocument->setModified(false);
        return true;
    }

    IView* TextDocument::createView()
    {
        return new TextView(this);
    }

    void TextDocument::updateSettingsChanges()
    {
    }

    QTextDocument* TextDocument::textDocument() const
    {
        return m_textDocument;
    }

    void TextDocument::pasteTemplate(const QString& text)
    {
        TextEdit *te = activeTextEdit();
        if (!te) {
            return;
        }
        te->insertPlainText(text);
    }

    TextEdit* TextDocument::activeTextEdit()
    {
        IView *view = DocumentViewManager::instance()->currentView();
        TextView *active = qobject_cast<TextView*>(view);

        if (active) {
            return active->textEdit();
        }

        return 0;
    }

    void TextDocument::onContentsChanged()
    {
        if (!m_textDocument->isModified()) {
            m_textDocument->setModified(true);
        }
    }

} // namespace Caneda
