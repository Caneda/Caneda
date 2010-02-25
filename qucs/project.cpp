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

#include "project.h"

#include "qucs-tools/global.h"

#include "componentssidebar.h"
#include "library.h"
#include "qucsview.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "xmlsymbolformat.h"

#include "dialogs/addtoprojectdialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

/*! Constructor
 * \brief This class implements the project management
 *
 * This also handles the mouse and keyboad events, and sends
 * when appropiate, the file names to be opened by the parent.
 *
 * \param parent Parent of the widget.
 */
Project::Project(QWidget *parent) : QWidget(parent)
{
    projectLibrary = 0;

    QVBoxLayout *layout = new QVBoxLayout(this);

    QToolBar *toolbar = new QToolBar;

    QToolButton *projNew = new QToolButton();
    projNew->setIcon(QIcon(Qucs::bitmapDirectory() + "project-new.png"));
    projNew->setStatusTip(tr("Creates a new project"));
    projNew->setToolTip(tr("Creates a new project"));
    projNew->setWhatsThis(tr("New Project\n\nCreates a new project"));

    QToolButton *projOpen = new QToolButton();
    projOpen->setIcon(QIcon(Qucs::bitmapDirectory() + "fileopen.png"));
    projOpen->setStatusTip(tr("Opens an existing project"));
    projOpen->setToolTip(tr("Opens an existing project"));
    projOpen->setWhatsThis(tr("Open Project\n\nOpens an existing project"));

    QToolButton *addToProj = new QToolButton();
    addToProj->setIcon(QIcon(Qucs::bitmapDirectory() + "filenew.png"));
    addToProj->setStatusTip(tr("Adds a file to current project"));
    addToProj->setToolTip(tr("Adds a file to current project"));
    addToProj->setWhatsThis(tr("Add File to Project\n\nAdds a file to current project"));

    QToolButton *projDel = new QToolButton();
    projDel->setIcon(QIcon(Qucs::bitmapDirectory() + "fileclose.png"));
    projDel->setStatusTip(tr("Removes a file from current project"));
    projDel->setToolTip(tr("Removes a file from current project"));
    projDel->setWhatsThis(tr("Remove from Project\n\nRemoves a file from current project"));

    QToolButton *projClose = new QToolButton();
    projClose->setIcon(QIcon(Qucs::bitmapDirectory() + "project-close.png"));
    projClose->setStatusTip(tr("Closes the current project"));
    projClose->setToolTip(tr("Closes the current project"));
    projClose->setWhatsThis(tr("Close Project\n\nCloses the current project"));

    connect(projNew, SIGNAL(clicked()), this, SLOT(slotNewProject()));
    connect(projOpen, SIGNAL(clicked()), this, SLOT(slotOpenProject()));
    connect(addToProj, SIGNAL(clicked()), this, SLOT(slotAddToProject()));
    connect(projDel, SIGNAL(clicked()), this, SLOT(slotRemoveFromProject()));
    connect(projClose, SIGNAL(clicked()), this, SLOT(slotCloseProject()));

    toolbar->addWidget(projNew);
    toolbar->addWidget(projOpen);
    toolbar->addWidget(addToProj);
    toolbar->addWidget(projDel);
    toolbar->addWidget(projClose);

    m_projectsSidebar = new ComponentsSidebar(this);
    connect(m_projectsSidebar, SIGNAL(itemClicked(const QString&, const QString&)), this,
            SLOT(slotOnClicked(const QString&, const QString&)));

    layout->addWidget(toolbar);
    layout->addWidget(m_projectsSidebar);

    setWindowTitle(tr("Project View"));
}

void Project::slotNewProject()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                    "", tr("Qucs Projects (*.xpro)"));
    if(!fileName.isEmpty()) {
        if(QString(QFileInfo(fileName).suffix()).isEmpty()) {
            fileName = fileName + ".xpro";
        }

        LibraryLoader *library = LibraryLoader::defaultInstance();

        if(library->newLibrary(fileName)) {
            slotCloseProject();
            projectLibrary = library->library(fileName);
            projectLibrary->saveLibrary();
            qDebug() << "Succesfully created library!";
            m_projectsSidebar->plugLibrary(fileName, "root");
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Invalid project file!"));
            return;
        }
    }
}

void Project::slotOpenProject(QString fileName)
{
    if(fileName == 0) {
        fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                "", tr("Qucs Projects (*.xpro)"));
    }

    if(!fileName.isEmpty()) {
        LibraryLoader *library = LibraryLoader::defaultInstance();

        if(!library->library(fileName)) {
            if(library->load(fileName)) {
                slotCloseProject();
                projectLibrary = library->library(fileName);
                qDebug() << "Succesfully loaded library!";
                m_projectsSidebar->plugLibrary(fileName, "root");
            }
            else {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Invalid project file!"));
                return;
            }
        }
    }
}

void Project::slotAddToProject()
{
    if(projectLibrary) {
        AddToProjectDialog *p = new AddToProjectDialog(this);

        if(p->accepted()) {
            if(p->userChoice() == Qucs::ExistingComponent) {

                QString fileName = QFileDialog::getOpenFileName(this, tr("Add File to Project"),
                                                                "", tr("Component-xml (*.xsch *.xsym)"));
                if(!fileName.isEmpty()) {
                    //If we selected a schematic, we must generate the corresponding symbol
                    if(QString(QFileInfo(fileName).suffix()) == "xsch") {
                        QucsView *view = new SchematicView(0, this);
                        view->toSchematicView()->schematicScene()->setMode(Qucs::SymbolMode);

                        if(!view->load(fileName)) {
                            QMessageBox::critical(this, tr("Error"),
                                                  tr("Could not open file!"));
                            delete view;
                            return;
                        }

                        fileName.replace(".xsch",".xsym");
                        view->setFileName(fileName);
                        XmlSymbolFormat *symbol = new XmlSymbolFormat(view->toSchematicView()->schematicScene());
                        symbol->save();

                        delete view;
                    }
                    projectLibrary->parseExternalComponent(fileName);
                    projectLibrary->saveLibrary();
                    m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
                    m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
                }
            }
            else if(p->userChoice() == Qucs::NewComponent) {
                QString fileName = QFileInfo(projectLibrary->libraryFileName()).absolutePath() + "/" + p->fileName()+".xsch";

                QucsView *view = new SchematicView(0, this);
                view->setFileName(fileName);

                //When the component is already created, we return.
                if(QFileInfo(fileName).exists()) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Component already created!"));
                    return;
                }

                if(!view->save()) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Could not save file!"));
                    delete view;
                    return;
                }

// **************TODO Open schematic here calling to slotOnDoubleClick when implemented
//                slotFileOpen(fileName);

                view->toSchematicView()->schematicScene()->setMode(Qucs::SymbolMode);

                fileName.replace(".xsch",".xsym");
                view->setFileName(fileName);
                XmlSymbolFormat *symbol = new XmlSymbolFormat(view->toSchematicView()->schematicScene());
                symbol->save();

                delete view;

                projectLibrary->parseExternalComponent(fileName);
                projectLibrary->saveLibrary();
                m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
                m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
            }
            else {
                //TODO in case of adding a component from another project, we
                //should copy the component as well as all its dependencies.
            }
        }
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Invalid project!"));
        return;
    }
}

void Project::slotRemoveFromProject()
{
    if(projectLibrary) {
        if(!m_projectsSidebar->currentComponent().isEmpty()) {
            projectLibrary->removeComponent(m_projectsSidebar->currentComponent());
            projectLibrary->saveLibrary();
            m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
            m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
        }
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Invalid project!"));
        return;
    }
}

void Project::slotCloseProject()
{
    if(projectLibrary) {
        m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
        LibraryLoader *library = LibraryLoader::defaultInstance();
        library->unload(projectLibrary->libraryFileName());
        projectLibrary = 0;
    }
}

void Project::slotOnClicked(const QString& item, const QString& category)
{
    emit itemClicked(item, category);
}

void Project::slotOnDoubleClicked()
{
}
