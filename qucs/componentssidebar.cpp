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

#include "qucs-tools/global.h"
#include "componentssidebar.h"
#include "sidebarmodel.h"
#include "component.h"

#include <QtGui/QHeaderView>
#include <QtGui/QDrag>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTreeView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QAction>

#include <QtCore/QMimeData>
#include <QtCore/QDebug>

class TreeView : public QTreeView
{
   public:
      TreeView(QWidget *parent = 0);
      ~TreeView() {}

      void startDrag( Qt::DropActions supportedActions);
      QPixmap renderToPixmap(const QMimeData *d, QRect *r, QPointF& hotSpot);
};

TreeView::TreeView(QWidget *parent) : QTreeView(parent)
{
   header()->hide();
   setDragDropMode(QAbstractItemView::DragOnly);
   setDragEnabled(true);
   setAlternatingRowColors(true);
   setIconSize(QSize(32, 32));
   expandAll();
}

//! Custom drag The drag pixmap is drawn from svg.
void TreeView::startDrag(Qt::DropActions supportedActions)
{
   //there can never be more that one index dragged at a time.
   Q_ASSERT(selectedIndexes().size() == 1);
   QModelIndex index = selectedIndexes().first();
   QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel*>(model());
   if(!proxyModel) {
      qDebug() << "TreeView::startDrag() : Failed to identify filter model";
      QTreeView::startDrag(supportedActions);
      return;
   }
   SidebarModel *sm = qobject_cast<SidebarModel*>(proxyModel->sourceModel());
   if(!sm) {
      qDebug() << "TreeView::startDrag() : Failed to identify sidebar model";
      QTreeView::startDrag(supportedActions);
      return;
   }
   QModelIndex proxyIndex = proxyModel->mapToSource(index);
   QPixmap pix = sm->pixmap(proxyIndex);
   QPointF translateHint = model()->data(index, Qt::EditRole).toPointF();

   QDrag *drag = new QDrag(this);

   QPixmap plainPixmap(32, 32);
   plainPixmap.fill(Qt::transparent);
   drag->setDragCursor(plainPixmap, Qt::MoveAction);
   drag->setDragCursor(plainPixmap, Qt::CopyAction);

   drag->setPixmap(pix);
   drag->setMimeData(model()->mimeData(selectedIndexes()));
   drag->setHotSpot(translateHint.toPoint());
   if (drag->exec(supportedActions) == Qt::MoveAction)
      clearSelection();
}

//! This helps in filtering sidebar display corresponding to lineedit.
class FilterProxyModel : public QSortFilterProxyModel
{
   public:
      FilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent)
      {}

      bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
      {
         QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
         SidebarModel *sm = static_cast<SidebarModel*>(sourceModel());
         if(sm->isComponent(index0) == false)
            return true;
         return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
      }
};

ComponentsSidebar::ComponentsSidebar(QWidget *parent) : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout(this);
   QHBoxLayout *hl = new QHBoxLayout();
   layout->addLayout(hl);
   m_filterEdit = new QLineEdit();
   hl->addWidget(m_filterEdit);
   m_clearButton = new QToolButton();
   m_clearButton->setIcon(QIcon(Qucs::bitmapDirectory() + "clearFilterText.png"));
   m_clearButton->setShortcut(Qt::ALT + Qt::Key_C);
   m_clearButton->setWhatsThis(tr("Clear Filter Text\n\nClears the filter text thus reshowing all components"));
   hl->addWidget(m_clearButton);
   m_clearButton->setEnabled(false);

   m_treeView = new TreeView();
   layout->addWidget(m_treeView);

   m_model = new SidebarModel();
   m_proxyModel = new FilterProxyModel();
   m_proxyModel->setDynamicSortFilter(true);
   m_proxyModel->setSourceModel(m_model);
   m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
   m_treeView->setModel(m_proxyModel);
   m_treeView->expandAll();

   connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
           this, SLOT(filterTextChanged()));

   connect(m_clearButton,SIGNAL(clicked()),m_filterEdit,SLOT(clear()));
   connect(m_model, SIGNAL(modelReset()), m_treeView, SLOT(expandAll()));
   setWindowTitle(tr("Components"));
}

void ComponentsSidebar::filterTextChanged()
{
   QString text = m_filterEdit->text();
   m_clearButton->setEnabled(!text.isEmpty());
   QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
   m_proxyModel->setFilterRegExp(regExp);
}

/*!
 * \brief Plugs library \a lib to sidebar display.
 */
void ComponentsSidebar::plugLibrary(QString lib)
{
   m_model->plugLibrary(lib);
}
