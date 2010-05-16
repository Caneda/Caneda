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

#include <QProcess>

namespace Caneda
{

    /*! Constructor
     * \brief This class implements a git object
     * This handles the repository initialization, commits, etc.
     *
     * \param parent Parent of the widget.
     */
    GitManager::GitManager(const QString& dir,
            QWidget *parent) : QDialog(parent)
    {
        // Check if directory exists
        m_path = ( dir.isEmpty() ? QString(".") : dir );

        m_output = "";
    }

    GitManager::~GitManager()
    {
    }

    void GitManager::initCreate()
    {
        // Run 'git init'
        QProcess gitp;
        gitp.setWorkingDirectory(m_path);
        gitp.start(QString("git init"));

//        connect(gitp, SIGNAL(stateChanged()), this, SLOT(updateOutput(gitp.readAllStandardOutput)));
        updateOutput(QByteArray("Data out test"));
    }

    void GitManager::status()
    {
    }

    void GitManager::commit()
    {
    }

    void GitManager::history()
    {
    }

    void GitManager::revert()
    {
    }

    void GitManager::updateOutput(const QByteArray& data)
    {
        m_output.append(data);
        emit outputDataReady(m_output);
    }

} // namespace Caneda
