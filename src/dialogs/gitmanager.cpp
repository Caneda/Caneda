/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include <QProcess>
#include <QXmlStreamReader>

namespace Caneda
{

    /*!
     * \brief Constructs a git manager dialog.
     *
     * \param parent Parent of the widget.
     */
    GitManager::GitManager(const QString& dir,
            QWidget *parent) : QDialog(parent)
    {
        ui.setupUi(this);

        // Set lineedit properties
        ui.editDescription->setClearButtonEnabled(true);

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

        // Show actual history
        slotHistory();
    }

    /*!
     * \brief Saves a new backup.
     *
     * First we initialize git repository in case it was not created.
     * Then we add all files modified, and finally we commit the
     * result, with an optional message from the user.
     */
    void GitManager::slotSaveBackup()
    {
        // Run 'git init'
        gitProcess->start(QString("git init"));
        gitProcess->waitForFinished();
        // Run 'git commit'
        gitProcess->start(QString("git add *"));
        gitProcess->waitForFinished();
        QString description = (ui.editDescription->text().isEmpty() ? QString(tr("Backup saved by user")) : ui.editDescription->text());
        ui.editDescription->clear();
        gitProcess->start(QString("git commit -a -m \"") + description + QString("\""));
        gitProcess->waitForFinished();
        slotHistory();
    }

    /*!
     * \brief Restores a previous backup.
     *
     * First we discard all current changes. After that we create
     * a new temporal branch and reset it to the user selected point.
     * Then we merge the temporal with master, discarding all differences
     * from master. Finally we merge the result with master and delete
     * temporal branch.
     * In this way, we can go back to a previous backup, without losing
     * intermediate history (as would happen with a direct git-reset
     * directly over master).
     */
    void GitManager::slotRestore()
    {
        gitProcess->start(QString("git checkout ."));
        gitProcess->waitForFinished();
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

    /*!
     * \brief Runs git-log to get the history
     *
     * We run git-log with a custom format. In the format
     * we use a temporal xml string, allowing for an
     * easy parse once the result is written to standard
     * output.
     */
    void GitManager::slotHistory()
    {
        // Run 'git log' with custom xml format to parse it with streamReader
        gitProcessHistory->start(QString("git log --reverse --relative-date --format=format:\"<commit>%n<hash>%H</hash>%n<msg>%ar - %s</msg>%n</commit>\""));
    }

    /*!
     * \brief Shows the standard output from git in a widget
     */
    void GitManager::slotUpdateOutput()
    {
        QString data = QString(gitProcess->readAllStandardOutput());
        if(!data.isEmpty()) {
            ui.editOutput->appendPlainText(data);
        }
    }

    /*!
     * \brief Parse and show the output from git-log
     *
     * We parse the output from previous git-log, and
     * then show the result in a QListWidget. The hash from
     * git log is not shown, but is used later to allow the
     * user to select an item, an recover that point in
     * history.
     */
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
