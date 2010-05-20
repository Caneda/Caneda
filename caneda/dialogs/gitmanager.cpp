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

#include <QXmlStreamReader>

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
        ui.setupUi(this);

        // Check if directory exists
        m_path = ( dir.isEmpty() ? QString(".") : dir );

        // Set up git processes
        gitProcess = new QProcess(this);
        gitProcess->setWorkingDirectory(m_path);
        connect(gitProcess, SIGNAL(finished(int)), SLOT(slotUpdateOutput()));

        gitProcessHistory = new QProcess(this);
        gitProcessHistory->setWorkingDirectory(m_path);
        connect(gitProcessHistory, SIGNAL(finished(int)), SLOT(slotUpdateHistory()));

        // Conections signal/slots
        connect(ui.btnSaveBackup, SIGNAL(clicked()), SLOT(slotSaveBackup()));
        connect(ui.btnRestoreBackup, SIGNAL(clicked()), SLOT(slotRestore()));
        connect(ui.btnRevertStep, SIGNAL(clicked()), SLOT(slotRevert()));

        // Show actual history
        slotHistory();
    }

    GitManager::~GitManager()
    {
    }

    void GitManager::slotSaveBackup()
    {
        // Saves new backup
        // Run 'git init'
        gitProcess->start(QString("git init"));
        gitProcess->waitForFinished();
        // Run 'git commit'
        gitProcess->start(QString("git add *"));
        gitProcess->waitForFinished();
        QString description = (ui.editDescription->text().isEmpty() ? QString("Backup saved by user") : ui.editDescription->text());
        ui.editDescription->clear();
        gitProcess->start(QString("git commit -a -m \"") + description + QString("\""));
        gitProcess->waitForFinished();
        slotHistory();
    }

    void GitManager::slotRevert()
    {
        // Run 'git revert'
        gitProcess->start(QString("git checkout"));
        gitProcess->waitForFinished();
        QString hash = ui.listHistory->currentItem()->data(Qt::AccessibleDescriptionRole).toString();
        gitProcess->start(QString("git revert --no-edit ") + hash);
        gitProcess->waitForFinished();
        slotHistory();
    }

    void GitManager::slotRestore()
    {
        // Restore previous backup
        QString hash = ui.listHistory->currentItem()->data(Qt::AccessibleDescriptionRole).toString();
        gitProcess->start(QString("git checkout -b temporal ") + hash);
        gitProcess->waitForFinished();
        gitProcess->start(QString("git merge master -s ours"));
        gitProcess->waitForFinished();
        gitProcess->start(QString("git checkout master"));
        gitProcess->waitForFinished();
        gitProcess->start(QString("git merge temporal"));
        gitProcess->waitForFinished();
        gitProcess->start(QString("git branch -D temporal"));
        gitProcess->waitForFinished();
        slotHistory();
    }

    void GitManager::slotHistory()
    {
        // Run 'git log' with custom xml format to parse it with streamReader
        gitProcessHistory->start(QString("git log --reverse --relative-date --format=format:\"<commit>%n<hash>%H</hash>%n<msg>%ar - %s</msg>%n</commit>\""));
    }

    void GitManager::slotUpdateOutput()
    {
        QString data = QString(gitProcess->readAllStandardOutput());
        if(!data.isEmpty()) {
            ui.editOutput->appendPlainText(data);
        }
    }

    void GitManager::slotUpdateHistory()
    {
        QString data = "<gitHistory>\n";
        data.append(gitProcessHistory->readAllStandardOutput());
        data.append("\n</gitHistory>");

        ui.listHistory->clear();
        QXmlStreamReader *reader = new QXmlStreamReader(data.toUtf8());

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement() && reader->name() == "commit") {
                QListWidgetItem *item = new QListWidgetItem(ui.listHistory);
                while(!reader->isEndElement() || reader->name() != "commit") {

                    reader->readNext();
                    if(reader->isStartElement() && reader->name() == "msg") {
                        item->setText(reader->readElementText());
                    }
                    else if(reader->isStartElement() && reader->name() == "hash") {
                        item->setData(Qt::AccessibleDescriptionRole, reader->readElementText());
                    }
                }
            }
        }

    }

} // namespace Caneda
