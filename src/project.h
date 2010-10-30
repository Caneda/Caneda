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

#ifndef PROJECT_H
#define PROJECT_H

#include <QWidget>

namespace Caneda
{
    // Forward declarations
    class SidebarBrowser;
    class Library;

    class Project : public QWidget
    {
        Q_OBJECT;

    public:
        Project(QWidget *parent = 0);
        ~Project() {}

        //! Returns project name.
        QString libraryName() const { return m_libraryName; }
        //! Returns project filename.
        QString libraryFileName() const { return m_libraryFileName; }

        bool isValid();

    public Q_SLOTS:
        void slotNewProject();
        void slotOpenProject(QString fileName = 0);
        void slotAddToProject();
        void slotRemoveFromProject();
        void slotCloseProject();
        void slotBackupAndHistory();

    signals:
        void itemClicked(const QString&, const QString&);
        void itemDoubleClicked(const QString&);

    private Q_SLOTS:
        void slotOnDoubleClicked(const QString&, const QString&);

    private:
        void setCurrentLibrary(const QString&);

        void addExistingComponent();
        void addNewComponent(const QString&);
        void importFromProject();
        void generateSymbol(const QString&);

        SidebarBrowser *m_projectsSidebar;
        Library *projectLibrary;
        QString m_libraryName;
        QString m_libraryFileName;
    };

} // namespace Caneda

#endif //PROJECT_H
