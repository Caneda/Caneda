/***************************************************************************
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
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
#include "qucs-tools/global.h"

FolderBrowser::FolderBrowser(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QToolBar *toolbar = new QToolBar;

    QToolButton *buttonUp = new QToolButton();
    buttonUp->setIcon(QIcon(Qucs::bitmapDirectory() + "previous.png"));
    buttonUp->setShortcut(Qt::Key_Backspace);
    buttonUp->setWhatsThis(tr("Go up one folder"));
    QToolButton *buttonBack = new QToolButton();
    buttonBack->setIcon(QIcon(Qucs::bitmapDirectory() + "back.png"));
    buttonBack->setShortcut(Qt::ALT + Qt::Key_Left);
    buttonBack->setWhatsThis(tr("Go previous folder"));
    QToolButton *buttonForward = new QToolButton();
    buttonForward->setIcon(QIcon(Qucs::bitmapDirectory() + "forward.png"));
    buttonForward->setShortcut(Qt::ALT + Qt::Key_Right);
    buttonForward->setWhatsThis(tr("Go next folder"));
    QToolButton *buttonHome = new QToolButton();
    buttonHome->setIcon(QIcon(Qucs::bitmapDirectory() + "home.png"));
    buttonHome->setShortcut(Qt::CTRL + Qt::Key_Home);
    buttonHome->setWhatsThis(tr("Go to the home folder"));
    QToolButton *buttonNewFolder = new QToolButton();
    buttonNewFolder->setIcon(QIcon(Qucs::bitmapDirectory() + "foldernew.png"));
    buttonNewFolder->setWhatsThis(tr("Create new folder"));
    QToolButton *buttonDeleteFile = new QToolButton();
    buttonDeleteFile->setIcon(QIcon(Qucs::bitmapDirectory() + "filedelete.png"));
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

    connect(m_listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotOnDoubleClicked(QModelIndex)));

    layout->addWidget(toolbar);
    layout->addWidget(m_listView);

    setWindowTitle(tr("Folder Browser"));
}

void FolderBrowser::slotOnDoubleClicked(const QModelIndex& index)
{
    if(m_fileModel->isDir(index)){
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(index);
        nextPages.clear();
    }
    else /*if(m_fileModel->fileInfo(index).completeSuffix() == "xsch")*/
        emit itemDoubleClicked(m_fileModel->fileInfo(index).absoluteFilePath());
}

void FolderBrowser::slotUpFolder()
{
    m_listView->setRootIndex(m_listView->rootIndex().parent());
}

void FolderBrowser::slotBackFolder()
{
    if(!previousPages.isEmpty()){
        nextPages << m_listView->rootIndex();
        m_listView->setRootIndex(previousPages.last());
        previousPages.removeLast();
    }
}

void FolderBrowser::slotForwardFolder()
{
    if(!nextPages.isEmpty()){
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(nextPages.last());
        nextPages.removeLast();
    }
}

void FolderBrowser::slotHomeFolder()
{
    m_listView->setRootIndex(m_fileModel->index(QDir::homePath()));
}

void FolderBrowser::slotNewFolder()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New Folder"),
                                         tr("Please enter new folder name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
        m_fileModel->mkdir(m_listView->rootIndex(), text);
}

void FolderBrowser::slotDeleteFile()
{
    int ret = QMessageBox::critical(this, tr("Delete File/Folder"),
                                    tr("You're about to delete one file/folder. This action can't be undone.\n"
                                       "Do you want to continue?"),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    switch (ret) {
        case QMessageBox::Ok:
            if(m_fileModel->isDir(m_listView->currentIndex())){
                bool result = m_fileModel->rmdir(m_listView->currentIndex());
                if(!result)
                    QMessageBox::warning(this, tr("Delete File/Folder"),
                                    tr("Folder not empty. Skipping."),
                                    QMessageBox::Ok);
            }
            else
                m_fileModel->remove(m_listView->currentIndex());
            break;
        case QMessageBox::Cancel:
            break;
        default:
            break;
    }
}

