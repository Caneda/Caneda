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

#include "schematicview.h"
#include "schematicscene.h"
#include "qucsmainwindow.h"
#include "components/resistor.h"
#include "wire.h"
#include "node.h"
#include "qucsprimaryformat.h"

#include <QtGui/QWheelEvent>
#include <QtCore/QDebug>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QFileInfo>

const qreal SchematicView::zoomFactor = 1.2f;

SchematicView::SchematicView(SchematicScene *sc,QucsMainWindow *parent) :
   QGraphicsView((QGraphicsScene*)sc,(QWidget*)parent), QucsView(parent)
{
   if(sc == 0)
      setScene(new SchematicScene(0,0,800,600));
   setDragMode(RubberBandDrag);
   setAcceptDrops(true);
   setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
   setWindowTitle("Untitled");
   init();
}

void SchematicView::init()
{
   SchematicScene *s = schematicScene();

   for( int j=1;j<3;++j)
      for(int i=1; i <11;i++)
      {
         Resistor *r = new Resistor(s);
         r->setPos(j*200,i*50);
      }
}

SchematicScene* SchematicView::schematicScene() const
{
   SchematicScene* s = qobject_cast<SchematicScene*>(scene());
   Q_ASSERT(s);// This should never fail!
   return s;
}

void SchematicView::setFileName(const QString& name)
{
   if(name == fileName)
      return;
   QucsView::setFileName(name);
   QFileInfo info(name);
   setTitle(info.fileName());
}

bool SchematicView::load()
{
   //This assumes filename is set before!
   QucsPrimaryFormat format(this);
   QFile file(fileName);
   if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Cannot load document ")+fileName);
      return false;
   }
   QTextStream stream(&file);
   format.loadFromText(stream.readAll());
   return true;
}

bool SchematicView::save()
{
   QFile file(fileName);
   if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Cannot save document!"));
      return false;
   }
   QTextStream stream(&file);
   QucsPrimaryFormat format(this);
   QString saveText = format.saveText();
   if(saveText.isNull()) {
      qDebug("Null data to save! Was this expected ??");
   }
   stream << saveText;
   file.close();
   return true;
}

void SchematicView::print(QPainter *p, bool printAll, bool fitToPage)
{
   if(p && printAll && fitToPage); // To avoid warning
}

void SchematicView::zoomIn()
{
   scale(zoomFactor,zoomFactor);
}

void SchematicView::zoomOut()
{
   qreal zf = 1.0/zoomFactor;
   scale(zf,zf);
}

void SchematicView::showAll()
{
   QRectF intersect;
   QList<QGraphicsItem*> _items = items();
   if ( !_items.isEmpty() ) {
      // It's ineficient??????
      intersect = _items.first()->sceneBoundingRect();
      foreach( QGraphicsItem* it, _items ) {
         intersect |= it->sceneBoundingRect();
      }
      intersect.adjust( -10, -10, 10, 10);
      fitInView( intersect, Qt::KeepAspectRatio );
   }
}

void SchematicView::showNoZoom()
{
   resetMatrix();
}

void SchematicView::setTitle(const QString& title)
{
   setWindowTitle(title);
   setWindowModified(false);
   emit titleChanged(windowTitle());
}
