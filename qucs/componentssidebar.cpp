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

#include "componentssidebar.h"
#include "sidebarmodel.h"
#include "components/component.h"
#include "node.h"

#include <QtGui/QHeaderView>
#include <QtGui/QDrag>
#include <QtGui/QPainter>

ComponentsSidebar::ComponentsSidebar(QWidget *parent) : QTreeView(parent)
{
   setModel(new SidebarModel());
   setWindowTitle(tr("Components"));
   header()->hide();
   setDragDropMode(QAbstractItemView::DragOnly);
   setDragEnabled(true);
   setAlternatingRowColors(true);
   expandAll();
}

void ComponentsSidebar::startDrag( Qt::DropActions supportedActions)
{
   QModelIndexList indexes = selectedIndexes();
   Q_ASSERT(indexes.count() == 1);
   QMimeData *data = model()->mimeData(indexes);
   if (!data)
      return;
   QRect rect;
   QPixmap pixmap = renderToPixmap(indexes, &rect);
   if(pixmap.isNull())
      return QAbstractItemView::startDrag(supportedActions);
   QDrag *drag = new QDrag(this);
   drag->setPixmap(pixmap);
   drag->setMimeData(data);
   drag->setHotSpot(rect.center());
   if (drag->start(supportedActions) == Qt::MoveAction)
      clearSelection();
}

QPixmap ComponentsSidebar::renderToPixmap(const QModelIndexList &indexes, QRect *r)
{
   Q_ASSERT(indexes.size() == 1);
   QRectF rect = visualRect(indexes.at(0));
   QString modelString = model()->data(indexes[0],Qt::DisplayRole).toString();
   Component *c = Component::componentFromName(modelString,0);
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
   
   QPainter p;
   QStyleOptionGraphicsItem so;
   so.state |= QStyle::State_Open;
   QPixmap pix(int(rect.width()),int(rect.height()));
   pix.fill(QColor(255,255,255,0));
   p.begin(&pix);
   QPen pen = QPen(Qt::darkGray);
   pen.setStyle(Qt::DashDotLine);
   pen.setWidth(2);
   p.setPen(pen);
   p.setRenderHint(QPainter::Antialiasing,true);
   p.translate(rect.width()/2,rect.height()/2);
   c->paint(&p,&so,0);
   p.end();

   if(r)
   {
      qreal dx = -rect.left();
      qreal dy = -rect.top();
      rect.translate(dx,dy);
      *r = rect.toRect();
   }
   delete c;
   return pix;
}
