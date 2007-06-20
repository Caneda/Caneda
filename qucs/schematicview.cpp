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
#include "xmlformat.h"
#include <QtGui/QWheelEvent>
#include <QtCore/QDebug>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QFileInfo>

const qreal SchematicView::zoomFactor = 1.2f;

SchematicView::SchematicView(SchematicScene *sc,QucsMainWindow *parent) :
   QGraphicsView((QGraphicsScene*)sc,(QWidget*)parent), QucsView(parent)
{
   if(sc == 0) {
      sc = new SchematicScene(0.0,0.0,1024.0,768.0);
      setScene(sc);
      DragMode dragMode = (sc->currentMouseAction() == SchematicScene::Normal) ? RubberBandDrag : NoDrag;
      setDragMode(dragMode);
   }

   setAcceptDrops(true);
   setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
   setWindowTitle("Untitled");
#if QT_VERSION >= 0x040300
   setViewportUpdateMode(SmartViewportUpdate);
#endif
   //init();
}

void SchematicView::init()
{
   SchematicScene *s = schematicScene();
   for( int j=1;j<6;++j)
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

QString SchematicView::fileName() const
{
   return schematicScene()->fileName();
}

void SchematicView::setFileName(const QString& name)
{
   schematicScene()->setFileName(name);
   QFileInfo info(fileName());
   setTitle(info.fileName());
}

bool SchematicView::load()
{
   //This assumes filename is set before!
   FileFormatHandler *format = 0;
   QFileInfo info(fileName());
   if(info.suffix() == "sch")
      format = new QucsPrimaryFormat(this);
   else if(info.suffix() == "xsch")
      format = new XmlFormat(this);
   if(!format) {
      QMessageBox::critical(0, tr("Error"), tr("Unknown file format!"));
      return false;
   }
   QFile file(fileName());
   if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::critical(0, tr("Error"), tr("Cannot load document ")+fileName());
      return false;
   }
   QTextStream stream(&file);
   return format->loadFromText(stream.readAll());
}

bool SchematicView::save()
{
   QFile file(fileName());
   if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Cannot save document!"));
      return false;
   }
   QTextStream stream(&file);
   QFileInfo info(fileName());
   FileFormatHandler *format = 0;
   if(info.suffix() == "sch")
      format = new QucsPrimaryFormat(this);
   else if(info.suffix() == "xsch")
      format = new XmlFormat(this);

   if(!format) {
      QMessageBox::critical(0, tr("Error"), tr("Unknown file format!"));
      return false;
   }

   QString saveText = format->saveText();
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
   repaintWires();
}

void SchematicView::zoomOut()
{
   qreal zf = 1.0/zoomFactor;
   scale(zf,zf);
   repaintWires();
}

void SchematicView::showAll()
{
   fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
   repaintWires();
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

void SchematicView::drawForeground(QPainter *painter, const QRectF &rect)
{
   QGraphicsView::drawBackground(painter, rect);
}

void SchematicView::repaintWires()
{
   foreach(Wire *w, schematicScene()->wires())
      w->rebuild();
}
