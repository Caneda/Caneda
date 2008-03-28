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
/*!\file
  \author Gopala Krishna A <krishna.ggk@gmail.com>

  Implement QucsItem class
*/

#include "item.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "xmlutilities/xmlutilities.h"
#include "qucsmainwindow.h"

#include <QtXml/QXmlStreamWriter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsSceneEvent>

namespace Qucs
{
   void drawHighlightRect(QPainter *painter, QRectF rect, qreal pw,
                          const QStyleOptionGraphicsItem *option)
   {
      const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
      if (qFuzzyCompare(qMax(murect.width(), murect.height()), qreal(0.0)))
         return;

      const QPen savePen = painter->pen();
      const QBrush saveBrush = painter->brush();

      const QRectF mbrect = painter->transform().mapRect(rect);
      if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
         return;

      qreal itemStrokeWidth = pw;
      const qreal pad = itemStrokeWidth / 2;
      const qreal strokeWidth = 0; // cosmetic pen

      const QColor fgcolor = option->palette.windowText().color();
      const QColor bgcolor( // ensure good contrast against fgcolor
         fgcolor.red()   > 127 ? 0 : 255,
         fgcolor.green() > 127 ? 0 : 255,
         fgcolor.blue()  > 127 ? 0 : 255);

      rect.adjust(pad, pad, -pad, -pad);

      painter->setPen(QPen(bgcolor, strokeWidth, Qt::SolidLine));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(rect);

      painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(rect);

      painter->setPen(savePen);
      painter->setBrush(saveBrush);
   }

   void drawResizeHandle(const QPointF &centrePos, QPainter *painter)
   {
      QPen savedPen = painter->pen();
      QBrush savedBrush = painter->brush();

      painter->setPen(Qucs::handlePen);
      painter->setBrush(Qucs::handleBrush);

      painter->drawRect(Qucs::handleRect.translated(centrePos));

      painter->setPen(savedPen);
      painter->setBrush(savedBrush);
   }

   void drawResizeHandles(ResizeHandles handles, const QRectF& rect, QPainter *painter)
   {
      if(handles.testFlag(Qucs::TopLeftHandle))
         drawResizeHandle(rect.topLeft(), painter);

      if(handles.testFlag(Qucs::TopRightHandle))
         drawResizeHandle(rect.topRight(), painter);

      if(handles.testFlag(Qucs::BottomLeftHandle))
         drawResizeHandle(rect.bottomLeft(), painter);

      if(handles.testFlag(Qucs::BottomRightHandle))
         drawResizeHandle(rect.bottomRight(), painter);
   }

   ResizeHandle handleHitTest(const QPointF& point, ResizeHandles handles, const QRectF& rect)
   {
      if(handles == Qucs::NoHandle)
         return Qucs::NoHandle;

      if(handles.testFlag(Qucs::TopLeftHandle)) {
         if(Qucs::handleRect.translated(rect.topLeft()).contains(point)) {
            return Qucs::TopLeftHandle;
         }
      }

      if(handles.testFlag(Qucs::TopRightHandle)) {
         if(Qucs::handleRect.translated(rect.topRight()).contains(point)) {
            return Qucs::TopRightHandle;
         }
      }

      if(handles.testFlag(Qucs::BottomLeftHandle)){
         if(Qucs::handleRect.translated(rect.bottomLeft()).contains(point)) {
            return Qucs::BottomLeftHandle;
         }
      }

      if(handles.testFlag(Qucs::BottomRightHandle)){
         if(Qucs::handleRect.translated(rect.bottomRight()).contains(point)) {
            return Qucs::BottomRightHandle;
         }
      }

      return Qucs::NoHandle;
   }
}

//! Create a new item and add to scene.
QucsItem::QucsItem(QGraphicsItem* parent, SchematicScene* scene)
   : QGraphicsItem(parent),
     m_boundingRect(-2., -2., 4., 4.) /* Non empty default bound rect.*/
{
   m_shape.addRect(m_boundingRect);
   if(scene) {
      scene->addItem(this);
   }
}

//!\brief Destructor
QucsItem::~QucsItem()
{
}

void QucsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   if(event->buttons().testFlag(Qt::LeftButton)) {
      launchPropertyDialog(Qucs::PushUndoCmd);
   }
}

/*!
 * \brief Sets the shape cache as well as boundbox cache
 *
 * This method abstracts the method of changing the geometry with support for
 * cache as well.
 * \param path The path to be cached. If empty, bound rect is added.
 * \param rect The bound rect to be cached.
 * \param pw Pen width of pen used to paint outer stroke of item.
 */
void QucsItem::setShapeAndBoundRect(const QPainterPath& path,
                                    const QRectF& rect, qreal pw)
{
   // Inform scene about change in geometry.
   prepareGeometryChange();
   m_boundingRect = rect;
   // Adjust the bounding rect by half pen width as required by graphicsview.
   m_boundingRect.adjust(-pw/2, -pw/2, pw, pw);
   m_shape = path;
   if(m_shape.isEmpty()) {
      //if path is empry just add the bounding rect itself to the path.
      m_shape.addRect(m_boundingRect);
   }
}

//!\brief returns a pointer to the schematic scene to which the item belongs.
SchematicScene* QucsItem::schematicScene() const
{
   return qobject_cast<SchematicScene*>(scene());
}


/*!
 * \brief Returns a pointer to the view which is currently active(having foucs)
 * \sa SchematicScene::activeView()
 */
SchematicView* QucsItem::activeView() const
{
   return schematicScene() ? schematicScene()->activeView() : 0;
}

//!\brief Returns a pointer to the applications main window.
QucsMainWindow* QucsItem::mainWindow() const
{
   QGraphicsView *view = activeView();
   return view ? qobject_cast<QucsMainWindow*>(view->parent()) : 0;
}

/*!
 * \brief Convienience method to just get the saved text as string.
 *
 * Though this is simple, this method shouldn't be used in too many places as
 * there will be unnecessary creation of xml writer and reader instances which
 * will render the program inefficient.
 */
QString QucsItem::saveDataText() const
{
   QString retVal;
   Qucs::XmlWriter writer(&retVal);
   saveData(&writer);
   return retVal;
}

/*!
 * \brief Convienience method to just load data from string.
 *
 * Though this is simple, this method shouldn't be used in too many places as
 * there will be unnecessary creation of xml writer and reader instances which
 * will render the program inefficient.
 */
void QucsItem::loadDataFromText(const QString &text)
{
  Qucs::XmlReader reader(text.toUtf8());
   while(!reader.atEnd()) {
      reader.readNext();

      if(reader.isEndElement())
         break;

      if(reader.isStartElement()) {
         loadData(&reader);
      }
   }
}

/*!
 * \brief Graphically mirror item according to x axis
 * \note Items can be mirrored only along x and y axis.
 */
void QucsItem::mirrorAlong(Qt::Axis axis)
{
   update();
   if(axis == Qt::ZAxis) {
      qWarning("Using unsupported mirroring axis - zaxis. Falling back"
               " to x axis");
      axis = Qt::XAxis;
   }
   if(axis == Qt::XAxis) {
      scale(1.0, -1.0);
   }
   else /*axis = Qt::YAxis*/ {
      scale(-1.0, 1.0);
   }
}

//!\brief Rotate item by -90°
void QucsItem::rotate90(Qucs::AngleDirection dir)
{
   rotate(dir == Qucs::AntiClockwise ? -90.0 : 90.0);
}

/*!
 * \brief This returns a copy of the current item parented to \a scene.
 *
 * Now it returns null but subclasses should reimplement this to return the
 * appropriate copy of that reimplemented item.
 */
QucsItem* QucsItem::copy(SchematicScene *) const
{
   return 0;
}

/*!
 * \brief Copies data of current-item to \a item.
 *
 * Sublasses should reimplement it to copy their data.
 */
void QucsItem::copyDataTo(QucsItem *item) const
{
   item->setTransform(transform());
   item->prepareGeometryChange();
   item->m_boundingRect = m_boundingRect;
   item->m_shape = m_shape;
}

/*!
 * \brief Constructs and returns a menu with default actions inderted.
 * \todo Implement this function.
 */
QMenu* QucsItem::defaultContextMenu() const
{
   return 0;
}

/*!
 * \brief Stores the item's current position in data field of item.
 *
 * This method is required for handling undo/redo.
 */
void storePos(QGraphicsItem *item)
{
   item->setData(PointKey, QVariant(item->scenePos()));
}

/*!
 * \brief Returns the stored point by fetching from item's data field.
 *
 * This method is required for handling undo/redo.
 */
QPointF storedPos(QGraphicsItem *item)
{
   return item->data(PointKey).toPointF();
}
