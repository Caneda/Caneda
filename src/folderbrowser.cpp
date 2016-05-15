/***************************************************************************
 * Copyright (C) 2009 by Pablo Daniel Pareja Obregon                       *
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

#include "folderbrowser.h"

#include "global.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Caneda
{
    /*!
     * \brief Constructs a folder browser widget.
     *
     * \param parent Parent of the widget.
     */
    FolderBrowser::FolderBrowser(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        QToolBar *toolbar = new QToolBar;

        QToolButton *buttonUp = new QToolButton();
        buttonUp->setIcon(Caneda::icon("go-up"));
        buttonUp->setShortcut(Qt::Key_Backspace);
        buttonUp->setStatusTip(tr("Go up one folder"));
        buttonUp->setToolTip(tr("Go up one folder"));
        buttonUp->setWhatsThis(tr("Go up one folder"));

        buttonBack = new QToolButton();
        buttonBack->setIcon(Caneda::icon("go-previous"));
        buttonBack->setShortcut(Qt::ALT + Qt::Key_Left);
        buttonBack->setStatusTip(tr("Go previous folder"));
        buttonBack->setToolTip(tr("Go previous folder"));
        buttonBack->setWhatsThis(tr("Go previous folder"));
        buttonBack->setEnabled(false);

        buttonForward = new QToolButton();
        buttonForward->setIcon(Caneda::icon("go-next"));
        buttonForward->setShortcut(Qt::ALT + Qt::Key_Right);
        buttonForward->setStatusTip(tr("Go next folder"));
        buttonForward->setToolTip(tr("Go next folder"));
        buttonForward->setWhatsThis(tr("Go next folder"));
        buttonForward->setEnabled(false);

        QToolButton *buttonHome = new QToolButton();
        buttonHome->setIcon(Caneda::icon("go-home"));
        buttonHome->setShortcut(Qt::CTRL + Qt::Key_Home);
        buttonHome->setStatusTip(tr("Go to the home folder"));
        buttonHome->setToolTip(tr("Go to the home folder"));
        buttonHome->setWhatsThis(tr("Go to the home folder"));

        QToolButton *buttonNewFolder = new QToolButton();
        buttonNewFolder->setIcon(Caneda::icon("folder-new"));
        buttonNewFolder->setStatusTip(tr("Create new folder"));
        buttonNewFolder->setToolTip(tr("Create new folder"));
        buttonNewFolder->setWhatsThis(tr("Create new folder"));

        QToolButton *buttonDeleteFile = new QToolButton();
        buttonDeleteFile->setIcon(Caneda::icon("archive-remove"));
        buttonDeleteFile->setStatusTip(tr("Delete file/folder"));
        buttonDeleteFile->setToolTip(tr("Delete file/folder"));
        buttonDeleteFile->setWhatsThis(tr("Delete file/folder"));

        connect(buttonUp, SIGNAL(clicked()), this, SLOT(slotUpFolder()));
        connect(buttonBack, SIGNAL(clicked()), this, SLOT(slotBackFolder()));
        connect(buttonForward, SIGNAL(clicked()), this, SLOT(slotForwardFolder()));
        connect(buttonHome, SIGNAL(clicked()), this, SLOT(slotHomeFolder()));
        connect(buttonNewFolder, SIGNAL(clicked()), this, SLOT(slotNewFolder()));
        connect(buttonDeleteFile, SIGNAL(clicked()), this, SLOT(slotDeleteFile()));

        toolbar->addWidget(buttonUp);
        toolbar->addWidget(buttonBack);
        toolbar->addWidget(buttonForward);
        toolbar->addWidget(buttonHome);
        toolbar->addWidget(buttonNewFolder);
        toolbar->addWidget(buttonDeleteFile);

        m_fileModel = new QFileSystemModel;
        m_fileModel->setRootPath(QDir::homePath());

        m_listView = new QListView;
        m_listView->setModel(m_fileModel);
        m_listView->setRootIndex(m_fileModel->index(QDir::homePath()));

        connect(m_listView, SIGNAL(activated(QModelIndex)),
                this, SLOT(slotOnDoubleClicked(QModelIndex)));

        layout->addWidget(toolbar);
        layout->addWidget(m_listView);

        setWindowTitle(tr("Folder Browser"));
    }

    void FolderBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        if(m_fileModel->isDir(index)) {
            previousPages << m_listView->rootIndex();
            m_listView->setRootIndex(index);
            nextPages.clear();

            buttonBack->setEnabled(true);
            buttonForward->setEnabled(false);
        }
        // it is a file so we let the main window handle the action
        else {
            emit itemDoubleClicked(m_fileModel->fileInfo(index).absoluteFilePath());
        }
    }

    void FolderBrowser::slotUpFolder()
    {
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(m_listView->rootIndex().parent());
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    void FolderBrowser::slotBackFolder()
    {
        if(!previousPages.isEmpty()) {
            nextPages << m_listView->rootIndex();
            m_listView->setRootIndex(previousPages.last());
            previousPages.removeLast();

            buttonForward->setEnabled(true);
            if(previousPages.isEmpty()) {
                buttonBack->setEnabled(false);
            }
        }
    }

    void FolderBrowser::slotForwardFolder()
    {
        if(!nextPages.isEmpty()) {
            previousPages << m_listView->rootIndex();
            m_listView->setRootIndex(nextPages.last());
            nextPages.removeLast();

            buttonBack->setEnabled(true);
            if(nextPages.isEmpty()) {
                buttonForward->setEnabled(false);
            }
        }
    }

    void FolderBrowser::slotHomeFolder()
    {
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(m_fileModel->index(QDir::homePath()));
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    void FolderBrowser::slotNewFolder()
    {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             tr("New Folder"),
                                             tr("Please enter new folder name:"),
                                             QLineEdit::Normal,
                                             QString(),
                                             &ok);

        if(ok && !text.isEmpty()) {
            m_fileModel->mkdir(m_listView->rootIndex(), text);
        }
    }

    void FolderBrowser::slotDeleteFile()
    {
        int ret = QMessageBox::critical(this, tr("Delete File/Folder"),
                tr("You're about to delete one file/folder. This action can't be undone.\n"
                    "Do you want to continue?"),
                QMessageBox::Ok | QMessageBox::Cancel);
        switch (ret) {
            case QMessageBox::Ok:
                if(m_fileModel->isDir(m_listView->currentIndex())) {
                    bool result = m_fileModel->rmdir(m_listView->currentIndex());
                    if(!result) {
                        QMessageBox::warning(this, tr("Delete File/Folder"),
                                tr("Folder not empty. Skipping."),
                                QMessageBox::Ok);
                    }
                }
                else {
                    m_fileModel->remove(m_listView->currentIndex());
                }
                break;

            case QMessageBox::Cancel:
                break;

            default:
                break;
        }
    }

} // namespace Caneda
