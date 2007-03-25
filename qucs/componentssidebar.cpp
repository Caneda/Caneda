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
#include <QtCore/QMimeData>

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
   QPixmap pixmap = renderToPixmap(data, &rect);
   if(pixmap.isNull())
      return QAbstractItemView::startDrag(supportedActions);
   QDrag *drag = new QDrag(this);
   drag->setPixmap(pixmap);
   drag->setMimeData(data);
   drag->setHotSpot(rect.center());
   if (drag->start(supportedActions) == Qt::MoveAction)
      clearSelection();
}

QPixmap ComponentsSidebar::renderToPixmap(const QMimeData* data, QRect *r)
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
   p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |QPainter::SmoothPixmapTransform );
   p.translate(rect.width()/2,rect.height()/2);
   //p.setMatrix(c->matrix());
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
