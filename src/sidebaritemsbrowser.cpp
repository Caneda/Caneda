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

#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace Caneda
{
    /*************************************************************************
     *                          FilterProxyModel                             *
     *************************************************************************/
    //! \brief Constructor.
    FilterProxyModel::FilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
    {
    }

    /*!
     * \brief Returns true if the item should be included in the model
     * (filtered); false otherwise.
     *
     * This method must be reimplemented from QSortFilterProxyModel to be able
     * to perform multicolumn filtering. It returns true if the item in the row
     * indicated by the given sourceRow and sourceParent should be included in
     * the model; otherwise returns false.
     */
    bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

        // Do bottom to top filtering
        if(sourceModel()->hasChildren(index0)) {
            for(int i=0; i < sourceModel()->rowCount(index0); ++i) {
                if(filterAcceptsRow(i, index0)) {
                    return true;
                }
            }
        }

        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }


    /*************************************************************************
     *                       SidebarItemsBrowser                             *
     *************************************************************************/
    //! \brief Constructor.
    SidebarItemsBrowser::SidebarItemsBrowser(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Set lineEdit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        m_filterEdit->installEventFilter(this);
        layout->addWidget(m_filterEdit);

        // Create a new model
        m_model = new QStandardItemModel(this);

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
        connect(m_filterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged()));
        connect(m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotOnClicked(QModelIndex)));
        connect(m_treeView, SIGNAL(activated(QModelIndex)), this, SLOT(slotOnClicked(QModelIndex)));

        setWindowTitle(tr("Components Browser"));
    }

    //! \brief Destructor.
    SidebarItemsBrowser::~SidebarItemsBrowser()
    {
        m_treeView->setModel(0);
    }

    /*!
     * \brief Add a library to the sidebar
     *
     * \param libraryName Library name
     * \param category Category to place the library
     */
    void SidebarItemsBrowser::plugLibrary(QString libraryName, QString category)
    {
        LibraryManager *manager = LibraryManager::instance();
        const Library *libItem = manager->library(libraryName);

        if(!libItem) {
            return;
        }

        QStandardItem *libRoot;
        if(category == "root") {
            libRoot = new QStandardItem(libItem->libraryName());
            libRoot->setSizeHint(QSize(150, 32));
            m_model->invisibleRootItem()->appendRow(libRoot);
        }
        else {
            libRoot = new QStandardItem(libItem->libraryName());
            m_model->findItems(category).first()->appendRow(libRoot);
        }

        // Get the components list and plug each component into the sidebar browser
        QStringList components(libItem->componentsList());
        foreach(const QString component, components) {
            ComponentDataPtr data = libItem->component(component);
            QIcon icon = QIcon(manager->pixmapCache(data->name, data->library));
            QStandardItem *item = new QStandardItem(icon, data->name);
            libRoot->appendRow(item);
        }

        m_treeView->expandAll();
    }

    /*!
     * \brief Remove a library from the sidebar
     *
     * \param libraryName Library name
     * \param category Category of the library to be removed
     */
    void SidebarItemsBrowser::unPlugLibrary(QString libraryName, QString category)
    {
        if(category == "root") {
            if(!m_model->findItems(libraryName).isEmpty()) {
                QStandardItem *catItem = m_model->findItems(libraryName).first();
                m_model->invisibleRootItem()->removeRow(catItem->row());
            }
        }
        else {
            if(!m_model->findItems(category).isEmpty()) {

                QStandardItem *catItem = m_model->findItems(category).first();

                if(!m_model->findItems(libraryName, Qt::MatchExactly, catItem->column()).isEmpty()) {
                    QStandardItem *libItem = m_model->findItems(libraryName, Qt::MatchExactly, catItem->column()).first();
                    catItem->removeRow(libItem->row());
                }
            }
        }

        m_treeView->expandAll();
    }

    /*!
     * \brief Add an item to the sidebar
     *
     * \param itemName Item name
     * \param itemPixmap Item icon
     * \param category Category to place the item
     */
    void SidebarItemsBrowser::plugItem(QString itemName, const QPixmap& itemPixmap,
            QString category)
    {
        QStandardItem *catItem = 0;

        if(category == "root") {
            catItem = new QStandardItem(itemName);
            catItem->setSizeHint(QSize(150, 32));
            m_model->invisibleRootItem()->appendRow(catItem);
        }
        else {
            if(m_model->findItems(category).isEmpty()) {
                catItem = new QStandardItem(category);
                m_model->invisibleRootItem()->appendRow(catItem);
            }
            else {
                catItem = m_model->findItems(category).first();
            }

            QStandardItem *item = new QStandardItem(QIcon(itemPixmap), itemName);
            catItem->appendRow(item);
        }

        m_treeView->expandAll();
    }

    /*!
     * \brief Add multiple items to the sidebar
     *
     * \param items List of items with their icons
     * \param category Category to place the items
     */
    void SidebarItemsBrowser::plugItems(const QList<QPair<QString, QPixmap> > &items,
            QString category)
    {
        QStandardItem *catItem = 0;

        if(m_model->findItems(category).isEmpty()) {
            catItem = new QStandardItem(category);
            m_model->invisibleRootItem()->appendRow(catItem);
        }
        else {
            catItem = m_model->findItems(category).first();
        }


        QList<QPair<QString, QPixmap> >::const_iterator it = items.begin(),
            end = items.end();

        while(it != end) {
            QStandardItem *item = new QStandardItem(QIcon(it->second), it->first);
            catItem->appendRow(item);
            ++it;
        }

        m_treeView->expandAll();
    }

    /*!
     * \brief Filters available items in the sidebar.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. This method allows for an external object to request
     * the selection of the sidebar focus and filtering, for example when
     * inserting items.
     */
    void SidebarItemsBrowser::focusFilter()
    {
        m_filterEdit->setFocus();
        m_filterEdit->clear();
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
    void SidebarItemsBrowser::slotOnClicked(const QModelIndex& index)
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
