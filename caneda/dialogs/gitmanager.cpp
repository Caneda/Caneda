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
        QPushButton *btnGitHistory = new QPushButton(tr("Git History"), this);
        QPushButton *btnGitRevert = new QPushButton(tr("Git Revert"), this);

        // Input text to specify the backup to revert to
        editRevert = new QLineEdit(this);

        // The dialog has a QTextEdit to provide the git log
        editOutput = new QTextEdit(tr("History manager output"), this);
        editOutput->setReadOnly(true);
        editOutput->setFocusPolicy(Qt::NoFocus);

        // Layout of elements
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(btnGitInit);
        layout->addWidget(btnGitStatus);
        layout->addWidget(btnGitCommit);
        layout->addWidget(btnGitHistory);
        layout->addWidget(btnGitRevert);
        layout->addWidget(editRevert);

        layout->addWidget(editOutput);

        // Conections signal/slots
        connect(btnGitInit, SIGNAL(clicked()), SLOT(slotInitCreate()));
        connect(btnGitStatus, SIGNAL(clicked()), SLOT(slotStatus()));
        connect(btnGitCommit, SIGNAL(clicked()), SLOT(slotCommit()));
        connect(btnGitHistory, SIGNAL(clicked()), SLOT(slotLog()));
        connect(btnGitRevert, SIGNAL(clicked()), SLOT(slotRevert()));
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
        // Run 'git commit'
        gitProcess->start(QString("git add *"));
        gitProcess->waitForFinished();
        gitProcess->start(QString("git commit -a -m \"Backup saved by user\""));
    }

    void GitManager::slotLog()
    {
        // Run 'git log'
        gitProcess->start(QString("git log --reverse --relative-date"));
    }

    void GitManager::slotRevert()
    {
        // Run 'git revert'
        gitProcess->start(QString("git checkout"));
        gitProcess->waitForFinished();
        gitProcess->start(QString("git revert --no-edit ") + editRevert->text());
    }

    void GitManager::slotUpdateOutput()
    {
        QString data = QString(gitProcess->readAllStandardOutput());
        if(!data.isEmpty()) {
            editOutput->append(data);
        }
    }

} // namespace Caneda
