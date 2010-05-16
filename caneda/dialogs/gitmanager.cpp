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

#include "gitmanager.h"

#include <QPushButton>
#include <QVBoxLayout>

namespace Caneda
{

    /*! Constructor
     * \brief This class implements a git object
     * This handles the repository initialization, commits, etc. The idea is
     * to provide the user a very simple backup tool, and not clutter the
     * interface with too many options. Some basic git options should be
     * transparent to the user (for example "git init"), as the user does not
     * need to know about git.
     *
     * \param parent Parent of the widget.
     */
    GitManager::GitManager(const QString& dir,
            QWidget *parent) : QDialog(parent)
    {
        // Check if directory exists
        m_path = ( dir.isEmpty() ? QString(".") : dir );

        gitProcess = new QProcess(this);
        gitProcess->setWorkingDirectory(m_path);

        // The minimum size of the dialogue is fixed
        setMinimumSize(800, 390);
        resize(minimumSize());
        setWindowTitle(tr("Backup and History Browser", "window title"));

        // The dialog has buttons to provide the different git options
        QPushButton *btnGitInit = new QPushButton(tr("Git Init"), this);
        QPushButton *btnGitStatus = new QPushButton(tr("Git Status"), this);
        QPushButton *btnGitCommit = new QPushButton(tr("Git Commit"), this);

        // The dialog has a QTextEdit to provide the git log
        m_textEdit = new QTextEdit(tr("History manager output"), this);
        m_textEdit->setReadOnly(true);
        m_textEdit->setFocusPolicy(Qt::NoFocus);

        // Layout of elements
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(btnGitInit);
        layout->addWidget(btnGitStatus);
        layout->addWidget(btnGitCommit);
        layout->addWidget(m_textEdit);

        // Conections signal/slots
        connect(btnGitInit, SIGNAL(clicked()), SLOT(slotInitCreate()));
        connect(btnGitStatus, SIGNAL(clicked()), SLOT(slotStatus()));
        connect(btnGitCommit, SIGNAL(clicked()), SLOT(slotCommit()));
        connect(gitProcess, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(slotUpdateOutput()));
    }

    GitManager::~GitManager()
    {
    }

    void GitManager::slotInitCreate()
    {
        // Run 'git init'
        gitProcess->start(QString("git init"));
    }

    void GitManager::slotStatus()
    {
        // Run 'git status'
        gitProcess->start(QString("git status"));
    }

    void GitManager::slotCommit()
    {
        // Run 'git status'
        gitProcess->start(QString("git add *"));
        gitProcess->waitForFinished();
        gitProcess->start(QString("git commit -a"));
    }

    void GitManager::slotHistory()
    {
    }

    void GitManager::slotRevert()
    {
    }

    void GitManager::slotUpdateOutput()
    {
        QString data = QString(gitProcess->readAllStandardOutput());
        if(!data.isEmpty()) {
            m_textEdit->append(data);
        }
    }

} // namespace Caneda
