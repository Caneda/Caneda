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

#include "caneda-tools/global.h"

#include "actionmanager.h"
#include "componentssidebar.h"
#include "library.h"
#include "canedaview.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "xmlsymbolformat.h"

#include "dialogs/addtoprojectdialog.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Caneda
{

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
        m_libraryFileName = "";
        m_libraryName = "";

        QVBoxLayout *layout = new QVBoxLayout(this);

        QToolBar *toolbar = new QToolBar;
        toolbar->setIconSize(QSize(15, 15));

        ActionManager* am = ActionManager::instance();
        toolbar->addAction(am->actionForName("projNew"));
        toolbar->addAction(am->actionForName("projOpen"));
        toolbar->addAction(am->actionForName("addToProj"));
        toolbar->addAction(am->actionForName("projDel"));
        toolbar->addAction(am->actionForName("projClose"));

        m_projectsSidebar = new ComponentsSidebar(this);
        connect(m_projectsSidebar, SIGNAL(itemClicked(const QString&, const QString&)), this,
                SIGNAL(itemClicked(const QString&, const QString&)));
        connect(m_projectsSidebar, SIGNAL(itemDoubleClicked(const QString&, const QString&)), this,
                SLOT(slotOnDoubleClicked(const QString&, const QString&)));


        layout->addWidget(toolbar);
        layout->addWidget(m_projectsSidebar);

        setWindowTitle(tr("Project View"));
    }

    bool Project::isValid()
    {
        if(projectLibrary == 0) {
            return false;
        }
        else {
            return true;
        }
    }

    void Project::slotNewProject()
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                        "", tr("Caneda Projects (*.xpro)"));
        if(!fileName.isEmpty()) {
            if(QFileInfo(fileName).suffix().isEmpty()) {
                fileName = fileName + ".xpro";
            }

            //First we create the folder structure where files are to be placed
            QFileInfo fileInfo = QFileInfo(fileName);
            QDir filePath = QDir(fileInfo.absolutePath() + "/" + fileInfo.baseName());
            if(!filePath.exists()) {
                filePath.mkpath(filePath.absolutePath());
            }
            fileName = filePath.absolutePath() + "/" + fileInfo.fileName();

            //Then we create the library/project
            LibraryLoader *library = LibraryLoader::instance();

            if(library->newLibrary(fileName)) {
                slotCloseProject();
                setCurrentLibrary(fileName);
                projectLibrary = library->library(m_libraryName);
                projectLibrary->saveLibrary();
                qDebug() << "Succesfully created library!";
                m_projectsSidebar->plugLibrary(m_libraryName, "root");
            }
        }
    }

    void Project::slotOpenProject(QString fileName)
    {
        //If no name is provided, we open a dialog asking the user for a project to be opened
        if(fileName == 0) {
            fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    "", tr("Caneda Projects (*.xpro)"));
        }

        if(!fileName.isEmpty()) {

            LibraryLoader *library = LibraryLoader::instance();

            if(library->load(fileName)) {
                slotCloseProject();
                setCurrentLibrary(fileName);
                projectLibrary = library->library(m_libraryName);
                qDebug() << "Succesfully loaded library!";
                m_projectsSidebar->plugLibrary(m_libraryName, "root");
            }
        }
    }

    void Project::slotAddToProject()
    {
        if(projectLibrary) {
            AddToProjectDialog *p = new AddToProjectDialog(this);

            if(p->accepted()) {
                QString fileName;
                if(p->userChoice() == Caneda::ExistingComponent) {

                    QString sourceFileName = QFileDialog::getOpenFileName(this, tr("Add File to Project"),
                                                                    "", tr("Component-xml (*.xsch)"));

                    if(!sourceFileName.isEmpty()) {
                        CanedaView *viewFile = new SchematicView(0, this);
                        if(!viewFile->load(sourceFileName)) {
                            QMessageBox::critical(this, tr("Error"),
                                                  tr("Could not open file!"));

                            return;
                        }

                        //We copy the selected file to current project folder
                        fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + QFileInfo(sourceFileName).fileName();
                        if(!QFile::copy(sourceFileName, fileName)) {
                            QMessageBox::critical(this, tr("Error"),
                                    tr("Component %1 already exists in project!").arg(QFileInfo(fileName).baseName()));
                            return;
                        }
                    }
                }
                else if(p->userChoice() == Caneda::NewComponent) {
                    fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + p->fileName()+".xsch";

                    //When the component is already created, we return.
                    if(QFileInfo(fileName).exists()) {
                        QMessageBox::critical(this, tr("Error"),
                                tr("Component already created!"));
                        return;
                    }

                    CanedaView *viewFile = new SchematicView(0, this);
                    viewFile->setFileName(fileName);
                    if(!viewFile->save()) {
                        QMessageBox::critical(this, tr("Error"),
                                tr("Could not save file!"));
                        return;
                    }
                }
                else {
                    //TODO in case of adding a component from another project, we
                    //should copy the component as well as all its dependencies.
                }

                //We generate the corresponding symbol
                CanedaView *view = new SchematicView(0, this);
                view->setFileName(fileName);
                view->toSchematicView()->schematicScene()->setMode(Caneda::SymbolMode);

                fileName.replace(".xsch",".xsym");
                view->setFileName(fileName);
                XmlSymbolFormat *symbol = new XmlSymbolFormat(view->toSchematicView()->schematicScene());
                symbol->save();

                //Now we load the new component in the library
                projectLibrary->parseExternalComponent(fileName);
                projectLibrary->saveLibrary();
                m_projectsSidebar->unPlugLibrary(m_libraryName, "root");
                m_projectsSidebar->plugLibrary(m_libraryName, "root");

                //Finally we open the component
                fileName.replace(".xsym",".xsch");
                emit itemDoubleClicked(fileName);
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

                int ret = QMessageBox::warning(this, tr("Delete component"),
                                  tr("You're about to delete one component. This action can't be undone.\n"
                                     "Do you want to continue?"),
                                  QMessageBox::Ok | QMessageBox::Cancel);

                switch (ret) {
                    case QMessageBox::Ok: {

                        QString fileName = projectLibrary->componentDataPtr(m_projectsSidebar->currentComponent())->filename;
                        fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + QFileInfo(fileName).baseName();

                        QFile::remove(fileName + ".xsch");
                        QFile::remove(fileName + ".xsym");
                        QFile::remove(fileName + ".svg");
                        
                        projectLibrary->removeComponent(m_projectsSidebar->currentComponent());
                        projectLibrary->saveLibrary();
                        m_projectsSidebar->unPlugLibrary(m_libraryName, "root");
                        m_projectsSidebar->plugLibrary(m_libraryName, "root");
                        break;
                    }
                    case QMessageBox::Cancel:
                        break;

                    default:
                        break;
                }
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
            m_projectsSidebar->unPlugLibrary(m_libraryName, "root");
            LibraryLoader *library = LibraryLoader::instance();
            library->unload(m_libraryName);

            projectLibrary = 0;
            m_libraryFileName = "";
            m_libraryName = "";
        }
    }

    void Project::slotOnDoubleClicked(const QString& item, const QString& category)
    {
        emit itemDoubleClicked(QFileInfo(m_libraryFileName).absolutePath() + "/" + item + ".xsch");
    }

    void Project::setCurrentLibrary(const QString& libFileName)
    {
        m_libraryFileName = libFileName;

        //The library name is created from the basename with the first letter in uppercase
        m_libraryName = QFileInfo(libFileName).baseName();
        m_libraryName.replace(0, 1, m_libraryName.left(1).toUpper()); // First letter in uppercase
    }

} // namespace Caneda
