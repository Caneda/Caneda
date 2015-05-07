/***************************************************************************
 * Copyright (C) 2014 by Pablo Daniel Pareja Obregon                       *
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

#include "sidebarwebbrowser.h"

#include "documentviewmanager.h"
#include "global.h"
#include "sidebartextbrowser.h"

#include <QFileSystemModel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace Caneda
{
    //! \brief Constructor.
    SidebarWebBrowser::SidebarWebBrowser(QWidget *parent) : QWidget(parent)
    {
        // Load library database settings
        QString libpath = QString(docDirectory() + "/en/");
        if(QFileInfo(libpath).exists() == false) {
            qDebug() << "Error loading help files";
            return;
        }

        // Fill the treeview and proxy models
        QVBoxLayout *layout = new QVBoxLayout(this);

        m_filterEdit = new QLineEdit();
        m_filterEdit->setClearButtonEnabled(true);
        layout->addWidget(m_filterEdit);

        m_fileModel = new QFileSystemModel;
        QModelIndex rootModelIndex = m_fileModel->setRootPath(libpath);

        m_proxyModel = new FileFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_fileModel);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        QRegExp regExp("*.html", Qt::CaseInsensitive, QRegExp::RegExp);
        regExp.setPatternSyntax(QRegExp::Wildcard);
        m_proxyModel->setFilterRegExp(regExp);

        m_treeView = new QTreeView;
        m_treeView->setModel(m_proxyModel);
        m_treeView->setRootIndex(m_proxyModel->mapFromSource(rootModelIndex));

        m_treeView->setHeaderHidden(true);
        m_treeView->setColumnHidden(1, 1);
        m_treeView->setColumnHidden(2, 1);
        m_treeView->setColumnHidden(3, 1);
        m_treeView->setAnimated(true);
        m_treeView->setAlternatingRowColors(true);

        layout->addWidget(m_treeView);

        connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));

        connect(m_fileModel, SIGNAL(modelReset()), m_treeView, SLOT(expandAll()));
        connect(m_treeView, SIGNAL(activated(const QModelIndex&)), this,
                SLOT(slotOnDoubleClicked(const QModelIndex&)));

        setWindowTitle(tr("Help Files"));
    }

    //! \brief Destructor.
    SidebarWebBrowser::~SidebarWebBrowser()
    {
        m_treeView->setModel(0);
    }

    void SidebarWebBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text() + "*.html";
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        regExp.setPatternSyntax(QRegExp::Wildcard);
        m_proxyModel->setFilterRegExp(regExp);
    }

    void SidebarWebBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(index);

        if(m_fileModel->isDir(sourceIndex)) {
            return;
        }

        // It is a file so we must open it in the help browser
        QString fileName = m_fileModel->fileInfo(sourceIndex).absoluteFilePath();

        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->openFile(fileName);
    }

} // namespace Caneda
