/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#include "savedocumentsdialog.h"

#include "idocument.h"
#include "global.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

namespace Caneda
{
    /*************************************************************************
     *                          FileBrowserLineEdit                          *
     *************************************************************************/
    //! \brief Constructor.
    FileBrowserLineEdit::FileBrowserLineEdit(QTreeWidgetItem *item,
                                             const QFileInfo& fileInfo,
                                             QWidget *parent) :
        QWidget(parent),
        m_item(item),
        m_fileInfo(fileInfo)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);

        m_lineEdit = new QLineEdit(this);
        m_lineEdit->setReadOnly(true);
        m_lineEdit->setText(m_fileInfo.absolutePath());

        m_browseButton = new QToolButton(this);
        m_browseButton->setIcon(Caneda::icon("document-open"));

        layout->addWidget(m_lineEdit);
        layout->addWidget(m_browseButton);

        connect(m_browseButton, SIGNAL(clicked()), SLOT(browseButtonClicked()));

        updateTexts();
    }

    QFileInfo FileBrowserLineEdit::fileInfo() const
    {
        return m_fileInfo;
    }

    void FileBrowserLineEdit::browseButtonClicked()
    {
        QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save File"), m_fileInfo.absoluteFilePath());

        QFileInfo fi(fileName);
        if (!fi.fileName().isEmpty() && QFileInfo(fi.absolutePath()).exists()) {
            m_fileInfo = fi;
            updateTexts();
        }
    }

    void FileBrowserLineEdit::updateTexts()
    {
        QString doc = m_fileInfo.fileName();
        QString path = m_fileInfo.absolutePath();

        if (doc.isEmpty()) {
            doc = tr("Untitled");
        }

        m_lineEdit->setText(path);
        m_item->setText(0, doc);
    }


    /*************************************************************************
     *                          SaveDocumentsDialog                          *
     *************************************************************************/
    //! \brief Constructor.
    SaveDocumentsDialog::SaveDocumentsDialog(const QList<IDocument*> &modifiedDocuments,
                                             QWidget *parent) :
        QDialog(parent),
        m_modifiedDocuments(modifiedDocuments)
    {
        ui.setupUi(this);

        ui.buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save Selected"));
        ui.buttonBox->button(QDialogButtonBox::Discard)->setText(tr("Do not Save"));
        ui.buttonBox->button(QDialogButtonBox::Save)->setFocus();

        // Populate items in the tree
        for (int i = 0; i < m_modifiedDocuments.count(); ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget);
            item->setCheckState(0, Qt::Checked);

            QFileInfo fileInfo(m_modifiedDocuments[i]->fileName());
            FileBrowserLineEdit *widget = new FileBrowserLineEdit(item, fileInfo);
            ui.treeWidget->setItemWidget(item, 1, widget);
        }

        connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)),
                this, SLOT(buttonClicked(QAbstractButton*)));
    }

    //! \brief Check what button was pressed and accept/reject the dialog.
    void SaveDocumentsDialog::buttonClicked(QAbstractButton *button)
    {
        // Check what button was pressed.
        if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
            // Discard button was pressed, accept the dialog saving the selected files

            // Check that all files selected for saving have a filename selected.
            for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = ui.treeWidget->topLevelItem(i);
                if (item->checkState(0) == Qt::Checked) {

                    FileBrowserLineEdit *widget =
                        qobject_cast<FileBrowserLineEdit*>(ui.treeWidget->itemWidget(item, 1));

                    if (widget->fileInfo().fileName().isEmpty()) {
                        QMessageBox::warning(0, tr("Filename not set"),
                                tr("Please set file names for untitled documents"));
                        return;
                    }
                }
            }

            // Save all selected files
            bool failedInBetween = false;

            for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = ui.treeWidget->topLevelItem(i);
                if (item->checkState(0) == Qt::Checked) {

                    FileBrowserLineEdit *widget =
                        qobject_cast<FileBrowserLineEdit*>(ui.treeWidget->itemWidget(item, 1));

                    IDocument *document = m_modifiedDocuments[i];
                    const QString newFileName = widget->fileInfo().absoluteFilePath();
                    QString oldFileName = document->fileName();

                    document->setFileName(newFileName);
                    if (!document->save()) {
                        failedInBetween = true;
                        document->setFileName(oldFileName);
                    }
                }
            }

            // If there was an error, discard the dialog
            if (failedInBetween) {
                QMessageBox::critical(0, tr("File save error"),
                        tr("Could not save some files"));
                reject();
                return;
            }

            accept();
        }
        else if(ui.buttonBox->buttonRole(button) == QDialogButtonBox::DestructiveRole) {
            // Discard button was pressed, accept the dialog without saving
            accept();
        }
        else {
            // Cancel button was pressed, discard the dialog
            reject();
        }

    }

} // namespace Caneda
