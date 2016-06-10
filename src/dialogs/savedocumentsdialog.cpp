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

#include "icontext.h"
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
                                             IDocument *document,
                                             QWidget *parent) :
        QWidget(parent),
        m_item(item),
        m_document(document)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);

        m_lineEdit = new QLineEdit(this);
        m_lineEdit->setReadOnly(true);

        m_browseButton = new QToolButton(this);
        m_browseButton->setIcon(Caneda::icon("document-open"));

        layout->addWidget(m_lineEdit);
        layout->addWidget(m_browseButton);

        connect(m_browseButton, SIGNAL(clicked()), SLOT(browseButtonClicked()));

        updateTexts(m_document->fileName());
    }

    //! \brief Returns current filename to be saved.
    QString FileBrowserLineEdit::fileName() const
    {
        return m_lineEdit->text();
    }

    //! \brief Create a custom dialog with the default suffix.
    void FileBrowserLineEdit::browseButtonClicked()
    {
        QFileDialog dialog(this, tr("Save File"));
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);

        QFileInfo fileInfo(fileName());
        QString suffix = fileInfo.suffix().isEmpty() ?
                    m_document->context()->defaultSuffix() : fileInfo.suffix();

        dialog.setNameFilters(m_document->context()->fileNameFilters());
        dialog.selectFile(fileName());
        dialog.setDefaultSuffix(suffix);

        QString fileName;
        if (dialog.exec()) {
            fileName = dialog.selectedFiles().first();
            updateTexts(fileName);
        }
    }

    //! \brief Update current item text acording to the given filename.
    void FileBrowserLineEdit::updateTexts(const QString &fileName)
    {
        QFileInfo fileInfo(fileName);
        QString doc = fileInfo.fileName();
        QString path = fileInfo.filePath();

        if(doc.isEmpty()) {
            doc = tr("untitled") + "." + m_document->context()->defaultSuffix();
        }

        if(path.isEmpty()) {
            path = QDir::homePath() + "/" + doc;
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

            FileBrowserLineEdit *widget = new FileBrowserLineEdit(item, m_modifiedDocuments[i], this);
            ui.treeWidget->setItemWidget(item, 1, widget);
        }

        connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)),
                this, SLOT(buttonClicked(QAbstractButton*)));
    }

    //! \brief Check what button was pressed and accept/reject the dialog.
    void SaveDocumentsDialog::buttonClicked(QAbstractButton *button)
    {
        if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
            // Save documents button was pressed, accept the dialog saving the selected files

            // Save all selected files
            bool failedInBetween = false;

            for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = ui.treeWidget->topLevelItem(i);
                if (item->checkState(0) == Qt::Checked) {

                    FileBrowserLineEdit *widget =
                        qobject_cast<FileBrowserLineEdit*>(ui.treeWidget->itemWidget(item, 1));

                    IDocument *document = m_modifiedDocuments[i];
                    const QString newFileName = widget->fileName();
                    QString oldFileName = document->fileName();

                    document->setFileName(newFileName);
                    if (!document->save()) {
                        failedInBetween = true;  // Using this flag allows to continue saving the rest of the documents upon any error.
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
