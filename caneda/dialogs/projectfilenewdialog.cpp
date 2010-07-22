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

#include "projectfilenewdialog.h"

#include "caneda-tools/global.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>

namespace Caneda
{
    /*!
     * Constructor
     * @param parent  parent Widget of the dialog
     */
    ProjectFileNewDialog::ProjectFileNewDialog(QWidget *parent) :
            QWidget(parent),
            dialog(0)
    {
        userchoice = Caneda::NewComponent;
        stateAccepted = false;

        //We display a dialog asking the user the kind of component to add
        buildComponentTypeDialog();
        dialog->exec();
    }

    //! Destructor
    ProjectFileNewDialog::~ProjectFileNewDialog()
    {
        delete dialog;
    }

    //! Defines the file name if the user selects new component
    void ProjectFileNewDialog::setFileName(const QString &name)
    {
        filename = name;
    }

    //! @return Component name
    QString ProjectFileNewDialog::fileName() const
    {
        return(filename);
    }

    //! Defines the user choice
    void ProjectFileNewDialog::setUserChoice(Caneda::ProjectFileNewChoice choice)
    {
        userchoice = choice;
    }

    //! @return User choice
    Caneda::ProjectFileNewChoice ProjectFileNewDialog::userChoice() const
    {
        return(userchoice);
    }

    /*!
     * Returns whether the dialog was accepted or not.
     */
    bool ProjectFileNewDialog::accepted()
    {
        return(stateAccepted);
    }

    /*!
     * Construct a non-standard dialog asking the user what type of
     * component to add: new component, existing component or
     * import from a project.
     */
    void ProjectFileNewDialog::buildComponentTypeDialog()
    {
        dialog = new QDialog(parentWidget());
        dialog->setWindowTitle(tr("Component type choice"));
        dialog->setMinimumWidth(400);

        QLabel *labelComponentType  = new QLabel(tr("What do you want to do?"));
        QLabel *iconNewComponent = new QLabel();
        QLabel *iconExistingComponent = new QLabel();
        QLabel *iconImportComponentFromProject = new QLabel();

        iconNewComponent->setPixmap(QPixmap(Caneda::bitmapDirectory() + "filenew.png"));
        iconExistingComponent->setPixmap(QPixmap(Caneda::bitmapDirectory() + "fileopen.png"));
        iconImportComponentFromProject->setPixmap(QPixmap(Caneda::bitmapDirectory() + "project-new.png"));

        QButtonGroup *componentTypeChoice = new QButtonGroup();
        newComponent = new QRadioButton(tr("Create new component"));
        existingComponent = new QRadioButton(tr("Add existing component"));
        importFromProject = new QRadioButton(tr("Import component from existing project"));

        componentTypeChoice->addButton(newComponent);
        componentTypeChoice->addButton(existingComponent);
        componentTypeChoice->addButton(importFromProject);
        newComponent->setChecked(true);

        editFilepath = new QLineEdit();

        QDialogButtonBox *buttons =
                new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        if(!filename.isEmpty()) {
            editFilepath->setText(filename);
        }

        connect(newComponent, SIGNAL(toggled(bool)), this, SLOT(updateComponentTypeDialog()));
        connect(existingComponent, SIGNAL(toggled(bool)), this, SLOT(updateComponentTypeDialog()));
        connect(importFromProject, SIGNAL(toggled(bool)), this, SLOT(updateComponentTypeDialog()));
        connect(buttons, SIGNAL(accepted()), this, SLOT(acceptComponentDialog()));
        connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

        //Organize layout
        QGridLayout *glayout = new QGridLayout();
        QVBoxLayout *vlayout = new QVBoxLayout();

        glayout->addWidget(editFilepath, 0, 1);
        glayout->addWidget(iconNewComponent, 1, 0);
        glayout->addWidget(newComponent, 1, 1);
        glayout->addWidget(iconExistingComponent, 2, 0);
        glayout->addWidget(existingComponent, 2, 1);
        glayout->addWidget(iconImportComponentFromProject, 3, 0);
        glayout->addWidget(importFromProject, 3, 1);

        vlayout->addWidget(labelComponentType);
        vlayout->addLayout(glayout);
        vlayout->addWidget(buttons);

        dialog->setLayout(vlayout);

        updateComponentTypeDialog();
    }

    //! Ensures coherence of the type of component dialogue
    void ProjectFileNewDialog::updateComponentTypeDialog()
    {
        bool newfile = newComponent->isChecked();
        editFilepath->setEnabled(newfile);
    }

    //! Checks the status of the print type dialogue.
    void ProjectFileNewDialog::acceptComponentDialog()
    {
        if(newComponent->isChecked()) {
            setUserChoice(Caneda::NewComponent);
        }
        else if(existingComponent->isChecked()) {
            setUserChoice(Caneda::ExistingComponent);
        }
        else {
            setUserChoice(Caneda::ImportFromProject);
        }

        if(newComponent->isChecked()) {
            if(editFilepath->text().isEmpty()) {
                QMessageBox::information(parentWidget(),
                                         tr("Component name missing"),
                                         tr("You must enter the name of the new component."));
            }
            else {
                setFileName(editFilepath->text());
                stateAccepted = true;
                dialog->accept();
            }
        }
        else {
            stateAccepted = true;
            dialog->accept();
        }
    }

} // namespace Caneda
