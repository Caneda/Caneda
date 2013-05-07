/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "sidebarbrowser.h"

#include "clineedit.h"
#include "component.h"
#include "global.h"
#include "sidebarmodel.h"

#include <QAction>
#include <QDebug>
#include <QDrag>
#include <QHeaderView>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

namespace Caneda
{
    /*************************************************************************
     *                                TreeView                               *
     *************************************************************************/
    //! \brief Constructor.
    TreeView::TreeView(QWidget *parent) :
        QTreeView(parent),
        invalidPressed(false)
    {
        header()->hide();

        setDragDropMode(QAbstractItemView::DragOnly);
        setDragEnabled(true);
        setAlternatingRowColors(true);
        setAnimated(true);
        setIconSize(QSize(24, 24));
    }

    //! \brief Destructor.
    TreeView::~TreeView()
    {
    }

    void TreeView::mousePressEvent(QMouseEvent *event)
    {
        invalidPressed = !indexAt(event->pos()).isValid();
        QTreeView::mousePressEvent(event);
    }

    void TreeView::mouseMoveEvent(QMouseEvent *event)
    {
        QTreeView::mouseMoveEvent(event);
    }

    void TreeView::mouseReleaseEvent(QMouseEvent *event)
    {
        if(invalidPressed && !indexAt(event->pos()).isValid()) {
            emit invalidAreaClicked(QModelIndex());
        }
        QTreeView::mouseReleaseEvent(event);
    }

    void TreeView::startDrag(Qt::DropActions supportedActions)
    {
        QModelIndex index = selectedIndexes().first();
        QPixmap pix = qVariantValue<QPixmap>(model()->data(index, SidebarModel::DragPixmapRole));

        QDrag *drag = new QDrag(this);

        drag->setPixmap(pix);
        drag->setMimeData(model()->mimeData(selectedIndexes()));
        drag->setHotSpot(QPoint(pix.width()/2, pix.height()/2));
        drag->exec(supportedActions);
    }


    /*************************************************************************
     *                          FilterProxyModel                             *
     *************************************************************************/
    //! \brief This helps in filtering sidebar display corresponding to lineedit.
    class FilterProxyModel : public QSortFilterProxyModel
    {
    public:
        FilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent)
        {
        }

        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
        {
            QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
            SidebarModel *sm = static_cast<SidebarModel*>(sourceModel());

            if(sm->isLeaf(index0) == false) {
                return true;
            }
            return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
        }
    };


    /*************************************************************************
     *                            SidebarBrowser                             *
     *************************************************************************/
    //! \brief Constructor.
    SidebarBrowser::SidebarBrowser(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        m_filterEdit = new CLineEdit(this);
        layout->addWidget(m_filterEdit);

        m_treeView = new TreeView();
        layout->addWidget(m_treeView);

        m_model = new SidebarModel(this);
        m_proxyModel = new FilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_treeView->setModel(m_proxyModel);

        connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));

        connect(m_model, SIGNAL(modelReset()), m_treeView, SLOT(expandAll()));
        connect(m_treeView, SIGNAL(clicked(const QModelIndex&)), this,
                SLOT(slotOnClicked(const QModelIndex&)));
        connect(m_treeView, SIGNAL(invalidAreaClicked(const QModelIndex&)), this,
                SLOT(slotOnClicked(const QModelIndex&)));
        connect(m_treeView, SIGNAL(activated(const QModelIndex&)), this,
                SLOT(slotOnDoubleClicked(const QModelIndex&)));

        setWindowTitle(tr("Schematic Items"));
        m_currentComponent = "";
    }

    //! \brief Destructor.
    SidebarBrowser::~SidebarBrowser()
    {
        m_treeView->setModel(0);
    }

    void SidebarBrowser::plugLibrary(QString libraryName, QString category)
    {
        m_model->plugLibrary(libraryName, category);
    }

    void SidebarBrowser::unPlugLibrary(QString libraryName, QString category)
    {
        m_model->unPlugLibrary(libraryName, category);
    }

    void SidebarBrowser::plugItem(QString itemName, const QPixmap& itemPixmap,
            QString category)
    {
        m_model->plugItem(itemName, itemPixmap, category);
    }

    void SidebarBrowser::plugItems(const QList<QPair<QString, QPixmap> > &items,
            QString category)
    {
        m_model->plugItems(items, category);
    }

    QString SidebarBrowser::currentComponent()
    {
        return m_currentComponent;
    }

    void SidebarBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    void SidebarBrowser::slotOnClicked(const QModelIndex& index)
    {
        if(index.isValid()) {
            QMimeData *mime = index.model()->mimeData(QModelIndexList() << index);
            if(mime) {
                QByteArray encodedData = mime->data("application/caneda.sidebarItem");
                QDataStream stream(&encodedData, QIODevice::ReadOnly);
                QString item, category;
                stream >> item >> category;
                emit itemClicked(item, category);
                m_currentComponent = item;
            }
        }
    }

    void SidebarBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        if(index.isValid()) {
            QMimeData *mime = index.model()->mimeData(QModelIndexList() << index);
            if(mime) {
                QByteArray encodedData = mime->data("application/caneda.sidebarItem");
                QDataStream stream(&encodedData, QIODevice::ReadOnly);
                QString item, category;
                stream >> item >> category;
                emit itemDoubleClicked(item, category);
                m_currentComponent = item;
            }
        }
    }

} // namespace Caneda
