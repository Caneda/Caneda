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

#ifndef PROJECT_FILE_NEW_DIALOG_H
#define PROJECT_FILE_NEW_DIALOG_H

#include "ui_projectfilenewdialog.h"

#include <QDialog>

namespace Caneda
{
    enum ProjectFileNewChoice {
        NewComponent,
        ExistingComponent,
        ImportFromProject
    };

    /*!
     * \brief This class represents the configuration dialog to add a component
     * to a project.
     */
    class ProjectFileNewDialog : public QDialog
    {
        Q_OBJECT

    public:
        ProjectFileNewDialog(QWidget *parent = 0);

        QString fileName() const { return m_filename; }
        Caneda::ProjectFileNewChoice userChoice() const { return m_userchoice; }

    public Q_SLOTS:
        virtual void done(int r);

    private:
        QString m_filename;
        Caneda::ProjectFileNewChoice m_userchoice;

        Ui::ProjectFileNewDialog ui;
    };

} // namespace Caneda

#endif //PROJECT_FILE_NEW_DIALOG_H
