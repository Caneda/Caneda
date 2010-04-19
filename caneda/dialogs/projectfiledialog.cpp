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

#include "projectfiledialog.h"

#include "caneda-tools/global.h"

#include "componentssidebar.h"
#include "library.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

/*!
 * Constructor
 * @param parent  parent Widget of the dialog
 */
ProjectFileDialog::ProjectFileDialog(QString libraryFileName, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(tr("Open component"));
    this->setMinimumWidth(400);

    m_fileName = "";

    //Add components browser
    m_projectsSidebar = new ComponentsSidebar(this);
    LibraryLoader *library = LibraryLoader::instance();
    if(!libraryFileName.isEmpty()) {
        m_libraryFileName = libraryFileName;
        m_libraryName = QFileInfo(libraryFileName).baseName();
        m_libraryName.replace(0, 1, m_libraryName.left(1).toUpper()); // First letter in uppercase

        m_projectsSidebar->plugLibrary(m_libraryName, "root");
    }

    connect(m_projectsSidebar, SIGNAL(itemDoubleClicked(const QString&, const QString&)), this,
            SLOT(slotOnDoubleClick(const QString&, const QString&)));

    //Add Ok/Cancel buttons
    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttons, SIGNAL(accepted()), this, SLOT(slotAccept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    //Organize layout
    QVBoxLayout *vlayout = new QVBoxLayout();

    vlayout->addWidget(m_projectsSidebar);
    vlayout->addWidget(buttons);

    this->setLayout(vlayout);

    this->exec();
}

//! Destructor
ProjectFileDialog::~ProjectFileDialog()
{
}

void ProjectFileDialog::slotAccept()
{
    if(!m_projectsSidebar->currentComponent().isEmpty()) {
        m_fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + m_projectsSidebar->currentComponent() + ".xsch";
        accept();
    }
}

void ProjectFileDialog::slotOnDoubleClick(const QString& item, const QString& category)
{
    m_fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + item + ".xsch";
    accept();
}
