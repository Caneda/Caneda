/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#include "quickopen.h"

#include "global.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of the widget.
     */
    QuickOpen::QuickOpen(QWidget *parent) : QMenu(parent)
    {
        // Set window geometry
        setMinimumSize(300,300);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // Create the toolbar
        QToolBar *toolbar = new QToolBar(this);

        QToolButton *buttonUp = new QToolButton(this);
        buttonUp->setIcon(Caneda::icon("go-up"));
        buttonUp->setShortcut(Qt::Key_Backspace);
        buttonUp->setStatusTip(tr("Go up one folder"));
        buttonUp->setToolTip(tr("Go up one folder"));
        buttonUp->setWhatsThis(tr("Go up one folder"));

        buttonBack = new QToolButton(this);
        buttonBack->setIcon(Caneda::icon("go-previous"));
        buttonBack->setShortcut(Qt::ALT + Qt::Key_Left);
        buttonBack->setStatusTip(tr("Go previous folder"));
        buttonBack->setToolTip(tr("Go previous folder"));
        buttonBack->setWhatsThis(tr("Go previous folder"));
        buttonBack->setEnabled(false);

        buttonForward = new QToolButton(this);
        buttonForward->setIcon(Caneda::icon("go-next"));
        buttonForward->setShortcut(Qt::ALT + Qt::Key_Right);
        buttonForward->setStatusTip(tr("Go next folder"));
        buttonForward->setToolTip(tr("Go next folder"));
        buttonForward->setWhatsThis(tr("Go next folder"));
        buttonForward->setEnabled(false);

        QToolButton *buttonHome = new QToolButton(this);
        buttonHome->setIcon(Caneda::icon("go-home"));
        buttonHome->setShortcut(Qt::CTRL + Qt::Key_Home);
        buttonHome->setStatusTip(tr("Go to the home folder"));
        buttonHome->setToolTip(tr("Go to the home folder"));
        buttonHome->setWhatsThis(tr("Go to the home folder"));

        QToolButton *buttonNewFolder = new QToolButton(this);
        buttonNewFolder->setIcon(Caneda::icon("folder-new"));
        buttonNewFolder->setStatusTip(tr("Create new folder"));
        buttonNewFolder->setToolTip(tr("Create new folder"));
        buttonNewFolder->setWhatsThis(tr("Create new folder"));

        QToolButton *buttonDeleteFile = new QToolButton(this);
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
        layout->addWidget(toolbar);

        // Set lineEdit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        m_filterEdit->installEventFilter(this);
        layout->addWidget(m_filterEdit);

        // Create a new filesystem model
        m_model = new QFileSystemModel(this);
        m_model->setRootPath(QDir::homePath());

        // Create proxy model and set its properties.
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setSourceModel(m_model);

        // Create a list view, set properties and proxy model
        m_listView = new QListView(this);
        m_listView->setModel(m_proxyModel);
        m_listView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(QDir::homePath())));
        layout->addWidget(m_listView);

        // Signals and slots connections
        connect(m_filterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged()));
        connect(m_filterEdit, SIGNAL(returnPressed()), this, SLOT(slotOnDoubleClicked()));
        connect(m_listView, SIGNAL(activated(QModelIndex)), this, SLOT(slotOnDoubleClicked()));

        // Start with the focus on the filter
        m_filterEdit->setFocus();
    }

    //! \brief Filter event to select the listView on down arrow key event
    bool QuickOpen::eventFilter(QObject *object, QEvent *event)
    {
        if(object == m_filterEdit) {
            if(event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if(keyEvent->key() == Qt::Key_Down) {
                    // Set the row next to the currently selected one
                    int index = m_listView->currentIndex().row() + 1;
                    m_listView->setCurrentIndex(m_proxyModel->index(index,0));
                    m_listView->setFocus();

                    return true;
                }
            }

            return false;
        }

        return QMenu::eventFilter(object, event);
    }

    //! \brief Filters actions according to user input on a QLineEdit.
    void QuickOpen::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
        m_listView->setCurrentIndex(m_proxyModel->index(0,0));
    }

    //! \brief Accept the dialog and run the selected action.
    void QuickOpen::slotOnDoubleClicked()
    {
        if(m_listView->currentIndex().isValid()) {

            if(m_model->isDir(m_proxyModel->mapToSource(m_listView->currentIndex()))) {
                previousPages << m_listView->rootIndex();
                m_listView->setRootIndex(m_listView->currentIndex());
                nextPages.clear();

                buttonBack->setEnabled(true);
                buttonForward->setEnabled(false);
            }
            // it is a file so we let the main window handle the action
            else {
                emit itemDoubleClicked(m_model->fileInfo(m_proxyModel->mapToSource(m_listView->currentIndex())).absoluteFilePath());
                hide();
            }

        }
    }

    //! \brief Go up one folder in the filesystem.
    void QuickOpen::slotUpFolder()
    {
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(m_listView->rootIndex().parent());
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    //! \brief Go the the previous folder.
    void QuickOpen::slotBackFolder()
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

    //! \brief Go the the next folder.
    void QuickOpen::slotForwardFolder()
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

    //! \brief Go the the home folder.
    void QuickOpen::slotHomeFolder()
    {
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(QDir::homePath())));
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    //! \brief Create a new folder.
    void QuickOpen::slotNewFolder()
    {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             tr("New Folder"),
                                             tr("Please enter new folder name:"),
                                             QLineEdit::Normal,
                                             QString(),
                                             &ok);

        if(ok && !text.isEmpty()) {
            m_model->mkdir(m_listView->rootIndex(), text);
        }
    }

    //! \brief Delete currently selected file.
    void QuickOpen::slotDeleteFile()
    {
        int ret = QMessageBox::critical(this, tr("Delete File/Folder"),
                tr("You're about to delete one file/folder. This action can't be undone.\n"
                    "Do you want to continue?"),
                QMessageBox::Ok | QMessageBox::Cancel);
        switch (ret) {
            case QMessageBox::Ok:
                if(m_model->isDir(m_listView->currentIndex())) {
                    bool result = m_model->rmdir(m_listView->currentIndex());
                    if(!result) {
                        QMessageBox::warning(this, tr("Delete File/Folder"),
                                tr("Folder not empty. Skipping."),
                                QMessageBox::Ok);
                    }
                }
                else {
                    m_model->remove(m_listView->currentIndex());
                }
                break;

            case QMessageBox::Cancel:
                break;

            default:
                break;
        }
    }

} // namespace Caneda
