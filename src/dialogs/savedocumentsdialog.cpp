/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "documentviewmanager.h"
#include "idocument.h"
#include "iview.h"
#include "mainwindow.h"
#include "settings.h"
#include "tabs.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>

namespace Caneda
{
    /*************************************************************************
     *                      FileBrowserLineEditPrivate                       *
     *************************************************************************/
    struct FileBrowserLineEditPrivate
    {
        QTreeWidgetItem *item;
        QFileInfo fileInfo;

        QLineEdit *lineEdit;
        QToolButton *browseButton;
    };


    /*************************************************************************
     *                          FileBrowserLineEdit                          *
     *************************************************************************/
    //! \brief Constructor.
    FileBrowserLineEdit::FileBrowserLineEdit(QTreeWidgetItem *item,
            const QFileInfo& fileInfo,
            QWidget *parent) :
        QWidget(parent)
    {
        d = new FileBrowserLineEditPrivate;
        d->item = item;
        d->fileInfo = fileInfo;

        QHBoxLayout *layout = new QHBoxLayout(this);

        d->lineEdit = new QLineEdit(this);
        d->lineEdit->setReadOnly(true);
        d->lineEdit->setText(d->fileInfo.absolutePath());
        layout->addWidget(d->lineEdit);

        d->browseButton = new QToolButton(this);
        d->browseButton->setText("...");
        layout->addWidget(d->browseButton);

        connect(d->browseButton, SIGNAL(clicked()), this,
                SLOT(browseButtonClicked()));
    }

    //! \brief Destructor.
    FileBrowserLineEdit::~FileBrowserLineEdit()
    {
        delete d;
    }

    QFileInfo FileBrowserLineEdit::fileInfo() const
    {
        return d->fileInfo;
    }

    void FileBrowserLineEdit::browseButtonClicked()
    {
        QString fileName =
            QFileDialog::getSaveFileName(0, tr("Save File"), d->fileInfo.absoluteFilePath(),
                    Settings::instance()->currentValue("nosave/canedaFilter").toString());
        QFileInfo fi(fileName);
        if (fi.fileName().isEmpty() == false &&
                QFileInfo(fi.absolutePath()).exists()) {
            d->fileInfo = fi;
            updateTexts();
        }
    }

    void FileBrowserLineEdit::updateTexts()
    {
        QString doc = d->fileInfo.fileName();
        QString path = d->fileInfo.absolutePath();
        if (doc.isEmpty()) {
            doc = tr("Untitled");
        }
        d->lineEdit->setText(path);
        d->item->setText(0, doc);
    }

    struct SaveDocumentsDialogPrivate
    {
        QList<IDocument*> modifiedDocuments;
        QList<QPair<IDocument*, QString> > newFilePaths;
    };


    /*************************************************************************
     *                          SaveDocumentsDialog                          *
     *************************************************************************/
    //! \brief Constructor.
    SaveDocumentsDialog::SaveDocumentsDialog(const QList<IDocument*> &modifiedDocuments,
            QWidget *parent) : QDialog(parent)
    {
        d = new SaveDocumentsDialogPrivate;
        d->modifiedDocuments = modifiedDocuments;

        ui.setupUi(this);

        ui.buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save Selected"));
        ui.buttonBox->button(QDialogButtonBox::Discard)->setText(tr("Do not Save"));
        ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

        ui.buttonBox->button(QDialogButtonBox::Save)->setFocus();

        // Populate items in tree.
        for (int i = 0; i < modifiedDocuments.count(); ++i) {
            IDocument *document = modifiedDocuments[i];
            QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget);
            item->setCheckState(0, Qt::Checked);

            QFileInfo fileInfo(document->fileName());
            FileBrowserLineEdit *widget = new FileBrowserLineEdit(item, fileInfo);
            ui.treeWidget->setItemWidget(item, 1, widget);
            widget->updateTexts();
        }

        connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)),
                this, SLOT(slotButtonClicked(QAbstractButton*)));
        connect(ui.treeWidget, SIGNAL(clicked(const QModelIndex&)),
                this, SLOT(slotHandleClick(const QModelIndex&)));
    }

    //! \brief Destructor.
    SaveDocumentsDialog::~SaveDocumentsDialog()
    {
        delete d;
    }

    QList<QPair<IDocument*, QString> > SaveDocumentsDialog::newFilePaths() const
    {
        return d->newFilePaths;
    }

    void SaveDocumentsDialog::slotButtonClicked(QAbstractButton *button)
    {
        int buttonRole = ui.buttonBox->buttonRole(button);
        if (buttonRole == SaveSelected) {
            int selectedCount = 0;
            for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = ui.treeWidget->topLevelItem(i);
                if (item->checkState(0) == Qt::Checked) {
                    selectedCount++;
                    FileBrowserLineEdit *widget =
                        qobject_cast<FileBrowserLineEdit*>(ui.treeWidget->itemWidget(item, 1));
                    if (widget->fileInfo().fileName().isEmpty()) {
                        QMessageBox::warning(0, tr("Filename not set"),
                                tr("Please set file names for untitled documents"));
                        d->newFilePaths.clear();
                        return;
                    }

                    d->newFilePaths << qMakePair(d->modifiedDocuments[i],
                            widget->fileInfo().absoluteFilePath());
                }
            }
        }
        setResult(ui.buttonBox->buttonRole(button));
        hide();
    }

    void SaveDocumentsDialog::slotHandleClick(const QModelIndex& index)
    {
        if (index.row() < 0 || index.row() >= d->modifiedDocuments.count()) {
            return;
        }
        DocumentViewManager *manager = DocumentViewManager::instance();
        QList<IView*> views = manager->viewsForDocument(d->modifiedDocuments[index.row()]);
        if (!views.isEmpty()) {
            TabWidget *tabWidget = MainWindow::instance()->tabWidget();
            Tab *tab = tabWidget->tabForView(views.first());
            tab->setFocus();
        }
    }

    void SaveDocumentsDialog::reject()
    {
        QAbstractButton *cancel = ui.buttonBox->button(QDialogButtonBox::Cancel);
        slotButtonClicked(cancel);
    }

} // namespace Caneda
