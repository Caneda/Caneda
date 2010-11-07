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

#include "textcontext.h"

#include "documentviewmanager.h"
#include "settings.h"
#include "singletonowner.h"
#include "textdocument.h"
#include "textview.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QSettings>
#include <QStringList>
#include <QTextCodec>
#include <QTreeView>

namespace Caneda
{
    TextContext::TextContext(QObject *parent) : IContext(parent)
    {
        QSettings qSettings;

        Settings *settings = Settings::instance();
        settings->load(qSettings);

        /* Load library database settings */
        QString libpath = settings->currentValue("sidebarLibrary").toString() +
                          "/components/hdl";
        if(QFileInfo(libpath).exists() == false) {
            qDebug() << "Error loading text libraries";
            return;
        }

        m_fileModel = new QFileSystemModel;
        m_fileModel->setRootPath(libpath);

        m_treeView = new QTreeView;
        m_treeView->setModel(m_fileModel);
        m_treeView->setRootIndex(m_fileModel->index(libpath));

        m_treeView->setHeaderHidden(true);
        m_treeView->setColumnHidden(1, 1);
        m_treeView->setColumnHidden(2, 1);
        m_treeView->setColumnHidden(3, 1);
        m_treeView->setAnimated(true);

        connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)), this,
                SLOT(slotPasteTemplate(QModelIndex)));
    }

    TextContext* TextContext::instance()
    {
        static TextContext *instance = 0;
        if (!instance) {
            instance = new TextContext(SingletonOwner::instance());
        }

        return instance;
    }

    TextContext::~TextContext()
    {
    }

    void TextContext::init()
    {
    }

    QWidget* TextContext::sideBarWidget()
    {
        return m_treeView;
    }

    bool TextContext::canOpen(const QFileInfo& info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "";
        supportedSuffixes << "txt";
        supportedSuffixes << "log";
        supportedSuffixes << "net";
        supportedSuffixes << "cir";
        supportedSuffixes << "spc";
        supportedSuffixes << "sp";
        supportedSuffixes << "vhd";
        supportedSuffixes << "vhdl";
        supportedSuffixes << "v";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList TextContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Spice netlist (*.spc *.sp *.net *.cir)")+" (*.spc *.sp *.net *.cir);;";
        nameFilters << QObject::tr("HDL source (*.vhdl *.vhd *.v)")+" (*.vhdl *.vhd *.v);;";
        nameFilters << QObject::tr("Text file (*.txt)")+" (*.txt);;";

        return nameFilters;
    }

    IDocument* TextContext::newDocument()
    {
        return new TextDocument;
    }

    IDocument* TextContext::open(const QString& fileName, QString *errorMessage)
    {
        TextDocument *document = new TextDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

    void TextContext::slotPasteTemplate(const QModelIndex& index)
    {
        if(m_fileModel->isDir(index)) {
            return;
        }

        // It is a file so we paste the template
        QFile file(m_fileModel->fileInfo(index).absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << tr("Could not open file for reading");
            return;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        QString content = stream.readAll();
        file.close();

        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        TextDocument *textDoc = qobject_cast<TextDocument*>(doc);

        if (textDoc) {
            textDoc->pasteTemplate(content);
        }

    }

} // namespace Caneda
