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

#include "sidebaritemsbrowser.h"

#include "library.h"
#include "modelviewhelpers.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTreeView>
#include <QVBoxLayout>

namespace Caneda
{
    /*************************************************************************
     *                          SidebarItemsModel                            *
     *************************************************************************/
    //! \brief Constructor.
    SidebarItemsModel::SidebarItemsModel(QObject *parent) : QStandardItemModel(parent)
    {
    }

    /*!
     * \brief Add multiple items to the model, using a category as root.
     *
     * \param items List of items to insert with their icons.
     * \param category Category where to place the items.
     */
    void SidebarItemsModel::plugItems(const QList<QPair<QString, QPixmap> > &items,
            QString category)
    {
        // Search the category inside the tree. If not present, create it.
        QStandardItem *catItem = 0;

        if(findItems(category).isEmpty()) {
            catItem = new QStandardItem(category);
            catItem->setSizeHint(QSize(150, 32));
            invisibleRootItem()->appendRow(catItem);
        }
        else {
            catItem = findItems(category).first();
        }

        // Get the items list and plug each one into the tree
        QList<QPair<QString, QPixmap> >::const_iterator it = items.begin(),
            end = items.end();

        while(it != end) {
            QStandardItem *item = new QStandardItem(QIcon(it->second), it->first);
            catItem->appendRow(item);
            ++it;
        }
    }

    /*!
     * \brief Add a library to the model, using a category as root.
     *
     * \param libraryName Library name to insert.
     * \param category Category where to place the library.
     */
    void SidebarItemsModel::plugLibrary(QString libraryName, QString category)
    {
        // Get the library indicated by libraryName.
        LibraryManager *manager = LibraryManager::instance();
        const Library *libItem = manager->library(libraryName);

        if(!libItem) {
            return;
        }

        // Search the category inside the tree. If not present, create it.
        QStandardItem *catItem = 0;

        if(findItems(category).isEmpty()) {
            catItem = new QStandardItem(category);
            catItem->setSizeHint(QSize(150, 32));
            invisibleRootItem()->appendRow(catItem);
        }
        else {
            catItem = findItems(category).first();
        }

        // Append the library root to the indicated category.
        QStandardItem *libRoot = new QStandardItem(libItem->libraryName());
        catItem->appendRow(libRoot);

        // Get the components list and plug each one into the tree
        QStringList components(libItem->componentsList());

        foreach(const QString component, components) {
            ComponentDataPtr data = libItem->component(component);
            QIcon icon = QIcon(manager->pixmapCache(data->name, data->library));
            QStandardItem *item = new QStandardItem(icon, data->name);
            libRoot->appendRow(item);
        }
    }

    /*!
     * \brief Remove a library from the model.
     *
     * \param libraryName Library name to remove.
     * \param category Category of the library to be removed.
     */
    void SidebarItemsModel::unPlugLibrary(QString libraryName, QString category)
    {
        // Search the category
        if(!findItems(category).isEmpty()) {

            QStandardItem *catItem = findItems(category).first();

            // If found the category, search the library
            if(!findItems(libraryName, Qt::MatchExactly, catItem->column()).isEmpty()) {
                // Remove the library
                QStandardItem *library = findItems(libraryName, Qt::MatchExactly, catItem->column()).first();
                catItem->removeRow(library->row());
            }
        }
    }

    /*************************************************************************
     *                       SidebarItemsBrowser                             *
     *************************************************************************/
    //! \brief Constructor.
    SidebarItemsBrowser::SidebarItemsBrowser(QStandardItemModel *model,
                                             QWidget *parent) :
        QWidget(parent),
        m_model(model)
    {
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

        // Create view, set its properties and proxy model
        m_treeView = new QTreeView(this);
        m_treeView->header()->hide();
        m_treeView->setAlternatingRowColors(true);
        m_treeView->setAnimated(true);
        m_treeView->setUniformRowHeights(true);
        m_treeView->setIconSize(QSize(24, 24));
        m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_treeView->setModel(m_proxyModel);
        layout->addWidget(m_treeView);

        // Signals and slots connections
        connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)), m_treeView, SLOT(expandAll()));
        connect(m_filterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged()));
        connect(m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
        connect(m_treeView, SIGNAL(activated(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));

        setWindowTitle(tr("Components Browser"));
    }

    //! \brief Destructor.
    SidebarItemsBrowser::~SidebarItemsBrowser()
    {
        m_treeView->setModel(0);
    }

    //! \brief Filter event to select the view on down arrow key event
    bool SidebarItemsBrowser::eventFilter(QObject *object, QEvent *event)
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

        return QWidget::eventFilter(object, event);
    }

    //! \brief Filters items according to user input on a QLineEdit.
    void SidebarItemsBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
        m_treeView->expandAll();
    }

    //! \brief Emits the component and category clicked on the model.
    void SidebarItemsBrowser::itemClicked(const QModelIndex& index)
    {
        if(index.isValid()) {
            QStandardItem *currentItem = m_model->itemFromIndex(
                        m_proxyModel->mapToSource(index));

            QString item = currentItem->text();
            QString category = currentItem->parent() ? currentItem->parent()->text() : "root";

            emit itemClicked(item, category);
        }
    }

} // namespace Caneda
