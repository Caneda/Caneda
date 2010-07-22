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

#ifndef PROJECT_FILE_NEW_DIALOG_H
#define PROJECT_FILE_NEW_DIALOG_H

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QRadioButton;

namespace Caneda
{
    enum ProjectFileNewChoice {
        NewComponent,
        ExistingComponent,
        ImportFromProject
    };

    /*!
     * This class represents the configuration dialog to add a component to
     * a project.
     */
    class ProjectFileNewDialog : public QWidget
    {
        Q_OBJECT;

    public:
        ProjectFileNewDialog(QWidget * = 0);
        ~ProjectFileNewDialog();

        QString fileName() const;
        void setFileName(const QString &);

        Caneda::ProjectFileNewChoice userChoice() const;
        void setUserChoice(const Caneda::ProjectFileNewChoice);

        bool accepted();

    private:
        void buildComponentTypeDialog();


    private Q_SLOTS:
        void updateComponentTypeDialog();
        void acceptComponentDialog();

    private:
        QString filename;
        Caneda::ProjectFileNewChoice userchoice;

        QDialog *dialog;
        QRadioButton *newComponent;
        QRadioButton *existingComponent;
        QRadioButton *importFromProject;
        QLineEdit *editFilepath;

        bool stateAccepted;
    };

} // namespace Caneda

#endif //PROJECT_FILE_NEW_DIALOG_H
