/***************************************************************************
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

#include "runneritems.h"

#include "sidebaritemsbrowser.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTreeView>
#include <QVBoxLayout>

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object.
     */
    RunnerItems::RunnerItems(SidebarItemsModel *model, QWidget *parent) :
        QDialog(parent),
        m_model(model)
    {
        // Set a popup window type to be able to close it clicking outside
        setWindowFlags(Qt::Popup);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // Set lineEdit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        m_filterEdit->installEventFilter(this);
        layout->addWidget(m_filterEdit);

        // Create proxy model and set its properties.
        m_proxyModel = new FilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setSourceModel(m_model);

        // Create view, set properties and proxy model
        m_treeView = new QTreeView(this);
        m_treeView->header()->hide();
        m_treeView->setAlternatingRowColors(true);
        m_treeView->setAnimated(true);
        m_treeView->setUniformRowHeights(true);
        m_treeView->setIconSize(QSize(24, 24));
        m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_treeView->setModel(m_proxyModel);
        m_treeView->expandAll();
        layout->addWidget(m_treeView);

        // Signals and slots connections
        connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)), m_treeView, SLOT(expandAll()));
        connect(m_filterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged()));
        connect(m_filterEdit, SIGNAL(returnPressed()), this, SLOT(insertItem()));
        connect(m_treeView, SIGNAL(activated(QModelIndex)), this, SLOT(insertItem()));

        // Start with the focus on the filter
        m_filterEdit->setFocus();
    }

    //! \brief Filter event to select the listView on down arrow key event
    bool RunnerItems::eventFilter(QObject *object, QEvent *event)
    {
        if(object == m_filterEdit) {
            if(event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if(keyEvent->key() == Qt::Key_Down) {
                    // Set the row next to the currently selected one
                    int index = m_treeView->currentIndex().row() + 1;
                    m_treeView->setCurrentIndex(m_proxyModel->index(index,0));
                    m_treeView->setFocus();

                    return true;
                }
            }

            return false;
        }

        return QDialog::eventFilter(object, event);
    }

    //! \brief Filters actions according to user input on a QLineEdit.
    void RunnerItems::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
        m_treeView->setCurrentIndex(m_proxyModel->index(0,0));
        m_treeView->expandAll();
    }

    //! \brief Accept the dialog and insert the selected action.
    void RunnerItems::insertItem()
    {
        if(m_treeView->currentIndex().isValid()) {

            QStandardItem *currentItem = m_model->itemFromIndex(
                        m_proxyModel->mapToSource(m_treeView->currentIndex()));

            QString item = currentItem->text();
            QString category = currentItem->parent() ? currentItem->parent()->text() : "root";

            emit itemClicked(item, category);
        }

        accept();
    }

} // namespace Caneda
