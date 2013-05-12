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

#ifndef GITMANAGER_H
#define GITMANAGER_H

#include "ui_gitmanager.h"

#include <QDialog>

// Forward declarations.
class QProcess;

namespace Caneda
{
    /*!
     * \brief This class implements a git manager object
     *
     * This handles the repository initialization, commits, etc. The idea is
     * to provide the user a very simple backup tool, and not clutter the
     * interface with too many options. Some basic git options should be
     * transparent to the user (for example "git init"), as the user does not
     * need to know about git.
     *
     * \sa Project
     */
    class GitManager : public QDialog
    {
        Q_OBJECT

    public:
        GitManager(const QString& dir, QWidget *parent = 0);

    public:
        const QString& path() const { return m_path; }

    private Q_SLOTS:
        void slotSaveBackup();
        void slotRestore();
        void slotHistory();

        void slotUpdateOutput();
        void slotUpdateHistory();

    private:
        QString m_path; // Path to the repository
        QProcess *gitProcess; // Git process
        QProcess *gitProcessHistory; // Git process

        Ui::GitManager ui;
    };

} // namespace Caneda

#endif //GITMANAGER_H
