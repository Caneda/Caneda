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
#include "components/component.h"
#include "node.h"

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
   expandAll();
}

void TreeView::startDrag( Qt::DropActions supportedActions)
{
   QModelIndexList indexes = selectedIndexes();
   QMimeData *data = model()->mimeData(indexes);
   if (!data) return;

   QRect rect;
   QPointF hotSpot;
   QPixmap pixmap = renderToPixmap(data, &rect, hotSpot);
   if(pixmap.isNull())
      return QAbstractItemView::startDrag(supportedActions);
   QDrag *drag = new QDrag(this);
   drag->setPixmap(pixmap);
   drag->setMimeData(data);
   drag->setHotSpot(hotSpot.toPoint());
   if (drag->start(supportedActions) == Qt::MoveAction)
      clearSelection();
}

QPixmap TreeView::renderToPixmap(const QMimeData* data, QRect *r, QPointF& hotSpot)
{
   QRectF rect;// = visualRect(indexes.at(0));
   Component *c = 0;
   if(data->formats().contains("application/qucs.sidebarItem"))
   {
      QByteArray encodedData = data->data("application/qucs.sidebarItem");
      QDataStream stream(&encodedData, QIODevice::ReadOnly);
      QString text;
      stream >> text;
      c = Component::componentFromName(text,0);
   }
   if(!c)
   {
      if(r)
         *r = rect.toRect();
      return QPixmap();
   }
   rect = c->boundingRect();

   QList<ComponentPort*>::const_iterator it = c->componentPorts().constBegin();
   const QList<ComponentPort*>::const_iterator end = c->componentPorts().constEnd();
   for(; it != end; ++it)
      rect |= (*it)->node()->boundingRect().translated((*it)->centrePos());
   rect = rect.adjusted(-1,-1,1,1);
   rect = c->matrix().mapRect(rect);
   QPainter p;
   QStyleOptionGraphicsItem so;
   so.state |= QStyle::State_Open;
   QPixmap pix(int(rect.width()),int(rect.height()));
   pix.fill(QColor(255,255,255,0));
   p.begin(&pix);
   QPen pen = QPen(Qt::darkGray);
   pen.setStyle(Qt::DashLine);
   pen.setWidth(1);
   p.setPen(pen);

   p.setRenderHint(QPainter::Antialiasing,true);
   p.setRenderHint(QPainter::TextAntialiasing,true);
   p.setRenderHint(QPainter::SmoothPixmapTransform,true);
   p.translate(-rect.left(),-rect.top());
   #if QT_VERSION >= 0x040300
   p.setTransform(c->transform(),true);
   #else
   p.setMatrix(c->matrix(),true);
   #endif
   c->paint(&p,&so,0);

   p.end();

   if(r)
   {
      hotSpot = QPointF(-rect.left(),-rect.top());
      rect.translate(-rect.left(),-rect.top());
      *r = rect.toRect();
   }
   delete c;
   return pix;
}


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

   setWindowTitle(tr("Components"));
}

void ComponentsSidebar::filterTextChanged()
{
   QString text = m_filterEdit->text();
   m_clearButton->setEnabled(!text.isEmpty());
   QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
   m_proxyModel->setFilterRegExp(regExp);
}
