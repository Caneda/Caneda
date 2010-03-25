/***************************************************************************
 * Copyright 2006-2009 Xavier Guerrin                                      *
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
 * This file was part of QElectroTech and modified by Pablo Daniel Pareja  *
 * Obregon to be included in Qucs.                                         *
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

#include "settingsdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>

/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
SettingsDialog::SettingsDialog(QList<SettingsPage *> wantedPages, const char *title,
        QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr(title, "window title"));

    pages = wantedPages;

    // List of pages
    pages_list = new QListWidget();
    pages_list->setViewMode(QListView::ListMode);
    pages_list->setIconSize(QSize(32, 32));
    pages_list->setMovement(QListView::Static);
    pages_list->setMaximumWidth(150);
    pages_list->setSpacing(4);

    // pages
    pages_widget = new QStackedWidget();
    foreach(SettingsPage *page, pages) {
        pages_widget->addWidget(page);
    }
    buildPagesList();

    // buttons
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    // layouts
    QHBoxLayout *hlayout1 = new QHBoxLayout();
    hlayout1->addWidget(pages_list);
    hlayout1->addWidget(pages_widget);

    QVBoxLayout *vlayout1 = new QVBoxLayout();
    vlayout1->addLayout(hlayout1);
    vlayout1->addWidget(buttons);
    setLayout(vlayout1);

    // signals/slots
    connect(buttons, SIGNAL(accepted()), SLOT(applyConf()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
    connect(pages_list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}

//! Destructor
SettingsDialog::~SettingsDialog()
{
}

//! Changes the page in the configuration dialog
void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(!current) {
        current = previous;
    }
    pages_widget->setCurrentIndex(pages_list->row(current));
}

//! Builds a list of pages on the left
void SettingsDialog::buildPagesList()
{
    pages_list->clear();
    foreach(SettingsPage *page, pages) {
        QListWidgetItem *new_button = new QListWidgetItem(pages_list);
        new_button->setIcon(page->icon());
        new_button->setText(page->title());
        new_button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
}

//! Applies the configurations of all pages
void SettingsDialog::applyConf()
{
    foreach(SettingsPage *page, pages) {
        page->applyConf();
    }

    accept();
}
