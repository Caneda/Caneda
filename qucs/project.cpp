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

#include <QMessageBox>

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
}

void Project::slotNewProject()
{
//    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
//            "", tr("Qucs Projects (*.xpro)"));
//    if(!fileName.isEmpty()) {
//        if(QString(QFileInfo(fileName).suffix()).isEmpty()) {
//            fileName = fileName + ".xpro";
//        }
//
//        LibraryLoader *library = LibraryLoader::defaultInstance();
//
//        if(library->newLibrary(fileName)) {
//            slotCloseProject();
//            projectLibrary = library->library(fileName);
//            projectLibrary->saveLibrary();
//            qDebug() << "Succesfully created library!";
//            m_projectsSidebar->plugLibrary(fileName, "root");
//        }
//        else {
//            QMessageBox::critical(this, tr("Error"),
//                    tr("Invalid project file!"));
//            return;
//        }
//    }
}

void Project::slotOpenProject()
{
//    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
//            "", tr("Qucs Projects (*.xpro)"));
//    if(!fileName.isEmpty()) {
//        slotFileOpen(fileName);
//    }
}

void Project::slotAddToProject()
{
//    if(projectLibrary) {
//        AddToProjectDialog *p = new AddToProjectDialog(this);
//        if(p->accepted()) {
//
//            if(p->userChoice() == Qucs::ExistingComponent) {
//
//                QString fileName = QFileDialog::getOpenFileName(this, tr("Add File to Project"),
//                                                                "", tr("Component-xml (*.xsch *.xsym)"));
//                if(!fileName.isEmpty()) {
//                    //If we selected a schematic, we must generate the corresponding symbol
//                    if(QString(QFileInfo(fileName).suffix()) == "xsch") {
//                        QucsView *view = new SchematicView(0, this);
//                        view->toSchematicView()->schematicScene()->setMode(Qucs::SymbolMode);
//
//                        if(!view->load(fileName)) {
//                            QMessageBox::critical(this, tr("Error"),
//                                                  tr("Could not open file!"));
//                            delete view;
//                            return;
//                        }
//
//                        fileName.replace(".xsch",".xsym");
//                        view->setFileName(fileName);
//                        XmlSymbolFormat *symbol = new XmlSymbolFormat(view->toSchematicView()->schematicScene());
//                        symbol->save();
//
//                        delete view;
//                    }
//                    projectLibrary->parseExternalComponent(fileName);
//                    projectLibrary->saveLibrary();
//                    m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
//                    m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
//                }
//            }
//            else if(p->userChoice() == Qucs::NewComponent) {
//                QString fileName = QFileInfo(projectLibrary->libraryFileName()).absolutePath() + "/" + p->fileName()+".xsch";
//
//                QucsView *view = new SchematicView(0, this);
//                view->setFileName(fileName);
//
//                //When the component is already created, we return.
//                if(QFileInfo(fileName).exists()) {
//                    QMessageBox::critical(this, tr("Error"),
//                                          tr("Component already created!"));
//                    return;
//                }
//
//                if(!view->save()) {
//                    QMessageBox::critical(this, tr("Error"),
//                                          tr("Could not save file!"));
//                    delete view;
//                    return;
//                }
//
//                slotFileOpen(fileName);
//
//                view->toSchematicView()->schematicScene()->setMode(Qucs::SymbolMode);
//
//                fileName.replace(".xsch",".xsym");
//                view->setFileName(fileName);
//                XmlSymbolFormat *symbol = new XmlSymbolFormat(view->toSchematicView()->schematicScene());
//                symbol->save();
//
//                delete view;
//
//                projectLibrary->parseExternalComponent(fileName);
//                projectLibrary->saveLibrary();
//                m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
//                m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
//            }
//            else {
//                //TODO in case of adding a component from another project, we
//                //should copy the component as well as all its dependencies.
//            }
//        }
//    }
//    else {
//        QMessageBox::critical(this, tr("Error"),
//                              tr("Invalid project!"));
//        return;
//    }
}

void Project::slotRemoveFromProject()
{
//    if(projectLibrary) {
//        if(!m_projectsSidebar->currentComponent().isEmpty()) {
//            projectLibrary->removeComponent(m_projectsSidebar->currentComponent());
//            projectLibrary->saveLibrary();
//            m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
//            m_projectsSidebar->plugLibrary(projectLibrary->libraryFileName(), "root");
//        }
//    }
//    else {
//        QMessageBox::critical(this, tr("Error"),
//                tr("Invalid project!"));
//        return;
//    }
}

void Project::slotCloseProject()
{
//    if(projectLibrary) {
//        m_projectsSidebar->unPlugLibrary(projectLibrary->libraryFileName(), "root");
//        LibraryLoader *library = LibraryLoader::defaultInstance();
//        library->unload(projectLibrary->libraryFileName());
//        projectLibrary = 0;
//    }
}

void Project::slotOnDoubleClicked()
{
}
