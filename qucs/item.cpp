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
#include "undocommands.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "qucsmainwindow.h"
#include "xmlutilities.h"

#include <QtXml/QXmlStreamWriter>

/*!\brief Create a new item and add to scene */
QucsItem::QucsItem(QGraphicsItem* parent, SchematicScene* scene)
   : QGraphicsItem(parent)
{
   init();
   if(scene)
      scene->addItem(this);
}

void QucsItem::init()
{
   // An empty bounding rect sometime's cause 100% cpu usage.
   m_boundingRect = QRectF(-2, -2, 4, 4);
   m_shape = QPainterPath();
   m_shape.addRect(m_boundingRect);
}

//!\brief Destructor
QucsItem::~QucsItem()
{
}

//!\brief Sets the shape cache as well as boundbox cache
void QucsItem::setShapeAndBoundRect(const QPainterPath& path,
                                    const QRectF& rect, qreal pw)
{
   prepareGeometryChange();
   m_boundingRect = rect;
   m_boundingRect.adjust(-pw/2, -pw/2, pw, pw);
   m_shape = path;
   if(m_shape.isEmpty()) {
      m_shape.addRect(m_boundingRect);
   }
}

//!\brief returns a pointer to the schematic scene to which the item belongs.
SchematicScene* QucsItem::schematicScene() const
{
   SchematicScene *s = qobject_cast<SchematicScene*>(this->scene());
   return s;
}


//!\brief Returns a pointer to the view which is currently active(having foucs)
QGraphicsView* QucsItem::activeView() const
{
   if(!scene() || scene()->views().isEmpty())
      return 0;
   //for now return the first view in the views list since multiple view
   //support is not yet there.
   return scene()->views()[0];
}

//!\brief Returns a pointer to the applications main window.
QucsMainWindow* QucsItem::mainWindow() const
{
   QGraphicsView *view = activeView();
   if(!view) return 0;
   //Fetch indirectly.
   QucsMainWindow *mw = qobject_cast<QucsMainWindow*>(view->parent());
   return mw;
}

//!\brief Graphically mirror item according to x axis
void QucsItem::mirrorAlong(Qt::Axis axis)
{
   update();
   if(axis == Qt::ZAxis) {
      qWarning("Using unsupported mirroring axis - zaxis. Falling back"
               " to x axis");
      axis = Qt::XAxis;
   }
   if(axis == Qt::XAxis) {
      scale(1.0,-1.0);
   }
   else /*axis = Qt::YAxis*/ {
      scale(-1.0,1.0);
   }
}

//!\brief Rotate item by -90°
void QucsItem::rotate90()
{
   update();
   rotate(-90.0);
}

/*!\brief Constructs and returns a menu with default actions inderted.
 * \todo Implement this function. */

QMenu* QucsItem::defaultContextMenu() const
{
   //TODO:
   return 0;
}
