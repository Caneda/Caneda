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

#include "qucsmainwindow.h"
#include "qucsview.h"
#include "settings.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>

struct FileBrowserLineEditPrivate
{
    QTreeWidgetItem *item;
    QFileInfo fileInfo;

    QLineEdit *lineEdit;
    QToolButton *browseButton;
};

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
                Settings::instance()->currentValue("nosave/qucsFilter").toString());
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
    QSet<QPair<QucsView*, int> > modifiedViews;
    QMap<int, int> treeIndexToTabIndex;
    QSet<QPair<QucsView*, QString> > newFilePaths;
};

SaveDocumentsDialog::SaveDocumentsDialog(const QSet<QPair<QucsView*, int> > &modifiedViews,
        QWidget *parent) : QDialog(parent)
{
    d = new SaveDocumentsDialogPrivate;
    d->modifiedViews = modifiedViews;

    ui.setupUi(this);

    ui.buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save Selected"));
    ui.buttonBox->button(QDialogButtonBox::Discard)->setText(tr("Do not Save"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Abort Closing"));

    populateItems();

    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(slotButtonClicked(QAbstractButton*)));
    connect(ui.treeWidget, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(slotHandleClick(const QModelIndex&)));
}

SaveDocumentsDialog::~SaveDocumentsDialog()
{
    delete d;
}

QSet<QPair<QucsView*, QString> > SaveDocumentsDialog::newFilePaths() const
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

                QucsMainWindow *mw = QucsMainWindow::instance();
                QTabWidget *tw = mw->tabWidget();
                QucsView *view = mw->viewFromWidget(tw->widget(d->treeIndexToTabIndex[i]));
                if (view) {
                    d->newFilePaths.insert(qMakePair(view,
                                widget->fileInfo().absoluteFilePath()));
                }
            }
        }
    }
    setResult(ui.buttonBox->buttonRole(button));
    hide();
}

void SaveDocumentsDialog::slotHandleClick(const QModelIndex& index)
{
    QucsMainWindow *mw = QucsMainWindow::instance();
    mw->tabWidget()->setCurrentIndex(d->treeIndexToTabIndex[index.row()]);
}

void SaveDocumentsDialog::reject()
{
    QAbstractButton *cancel = ui.buttonBox->button(QDialogButtonBox::Cancel);
    slotButtonClicked(cancel);
}

void SaveDocumentsDialog::populateItems()
{
    QSet<QPair<QucsView*, int> >::iterator it = d->modifiedViews.begin();
    for (int i = 0 ; it != d->modifiedViews.end(); ++it, ++i) {
        QucsView *view = it->first;
        QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget);
        item->setCheckState(0, Qt::Checked);
        d->treeIndexToTabIndex[i] = it->second;

        QFileInfo fileInfo(view->fileName());
        FileBrowserLineEdit *widget = new FileBrowserLineEdit(item, fileInfo);
        ui.treeWidget->setItemWidget(item, 1, widget);
        widget->updateTexts();
    }
}
