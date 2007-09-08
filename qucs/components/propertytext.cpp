/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "components/propertytext.h"
#include "components/componentproperty.h"
#include "components/component.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "qucsmainwindow.h"


#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QFontMetricsF>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QTextCursor>

#include <QtCore/QtDebug>

PropertyText::PropertyText(ComponentProperty *prop,SchematicScene *sc) :
   QGraphicsTextItem(0, sc), property(prop)
{
   m_staticText = prop->name();
   m_staticText.append(" = ");
   setTextInteractionFlags(Qt::TextEditorInteraction);
   setFlags(ItemIsMovable | ItemIsFocusable);
   calculatePos();
   updateValue();
   setAcceptsHoverEvents(false);
}

QRectF PropertyText::boundingRect() const
{
   QRectF br = QGraphicsTextItem::boundingRect();
   br.setLeft((staticPos.x()));
   return br;
}

QPainterPath PropertyText::shape() const
{
   QPainterPath p;
   p.addRect(boundingRect());
   return p;
}

void PropertyText::setFont(const QFont& f)
{
   QGraphicsTextItem::setFont(f);
   calculatePos();
}

void PropertyText::validateText()
{
   //TODO: Validate the text here
}

void PropertyText::updateValue()
{
   setPlainText(property->value());
}

SchematicView* PropertyText::activeView() const
{
   QGraphicsView *gview = property->component()->activeView();
   SchematicView *view = gview ? static_cast<SchematicView*>(gview) : 0;
   return view;
}

//HACK: This solves the bug where keyboard shortcuts were interfering with textcontrol.
bool PropertyText::eventFilter(QObject* object, QEvent* event)
{
   if (event->type() == QEvent::Shortcut ||
       event->type() == QEvent::ShortcutOverride) {
      if (!object->inherits("QGraphicsView")) {
         event->accept();
         return true;
      }
   }
   return false;
}


void PropertyText::paint(QPainter *painter, const QStyleOptionGraphicsItem *o,
                         QWidget *widget)
{
   painter->setFont(font());
   painter->setBrush(Qt::NoBrush);
   painter->setPen(QPen(Qt::black,0));
   painter->drawText(staticPos,m_staticText);

   QStyleOptionGraphicsItem *option = const_cast<QStyleOptionGraphicsItem*>(o);
   QStyle::State state = option->state;

   if(option->exposedRect.left() < 0)
      option->exposedRect.setLeft(0);
   option->state &= ~QStyle::State_Selected;
   option->state &= ~QStyle::State_HasFocus;
   QGraphicsTextItem::paint(painter,option,widget);

   option->state = state;

   if (option->state & QStyle::State_HasFocus) {
      painter->setPen(QPen(Qt::black, 1));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(QGraphicsTextItem::boundingRect());
   }
}

bool PropertyText::sceneEvent(QEvent *event)
{
   if(!group())
      return QGraphicsTextItem::sceneEvent(event);

   //Eat some unnecessary
   switch (event->type()) {
      case QEvent::GraphicsSceneDragEnter:
      case QEvent::GraphicsSceneDragMove:
      case QEvent::GraphicsSceneDragLeave:
      case QEvent::GraphicsSceneDrop:
      case QEvent::GraphicsSceneHoverEnter:
      case QEvent::GraphicsSceneHoverMove:
      case QEvent::GraphicsSceneHoverLeave:
      case QEvent::InputMethod:
      case QEvent::GraphicsSceneWheel:
         event->ignore();
         return true;
      default: ;
   };

   //Call event handlers of this and not send them to the parent - group
   if(hasFocus()) {
      switch (event->type()) {

         case QEvent::KeyPress:
            keyPressEvent(static_cast<QKeyEvent *>(event));
            return true;

         case QEvent::KeyRelease:
            keyReleaseEvent(static_cast<QKeyEvent *>(event));
            return true;

         case QEvent::FocusIn:
            focusInEvent(static_cast<QFocusEvent *>(event));
            return true;

         case QEvent::FocusOut:
            focusOutEvent(static_cast<QFocusEvent *>(event));
            return true;

         case QEvent::GraphicsSceneMousePress:
            if(!isSendable(static_cast<QGraphicsSceneMouseEvent *>(event))) {
               mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
               return true;
            }
            //Remove foucs of this
            clearFocus();
            //Ignoring the event facilitates selection of new grabber, which is most likely group
            event->ignore();
            //This prevents double sending of events when the scene is selecting mouse grabber
            if(!group()->hasFocus())
               return true;
            break;

         case QEvent::GraphicsSceneMouseMove:
            if(!isSendable(static_cast<QGraphicsSceneMouseEvent *>(event))) {
               mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
               return true;
            }
            break;

         case QEvent::GraphicsSceneMouseRelease:
            if(!isSendable(static_cast<QGraphicsSceneMouseEvent *>(event))) {
               mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
               return true;
            }
            break;

         case QEvent::GraphicsSceneMouseDoubleClick:
            if(!isSendable(static_cast<QGraphicsSceneMouseEvent *>(event))) {
               mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
               return true;
            }
            break;

         case QEvent::GraphicsSceneContextMenu:
            contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
            break;

         default: break;
      };
   }
   else {
      event->accept();
      //TODO:  Review this code thoroughly
   }

   return QGraphicsTextItem::sceneEvent(event);
}

void PropertyText::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if(scene()) {
      // Unselect other items once focus is received
      foreach(QGraphicsItem *item, scene()->selectedItems()) {
         if(item != this)
            item->setSelected(false);
      }
   }
   QGraphicsTextItem::mousePressEvent(event);
}

void PropertyText::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   QGraphicsTextItem::mouseMoveEvent(event);
}

void PropertyText::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   QGraphicsTextItem::mouseReleaseEvent(event);
}

void PropertyText::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void PropertyText::focusInEvent(QFocusEvent *event)
{
   QGraphicsTextItem::focusInEvent(event);
   qApp->installEventFilter(this);
}

void PropertyText::focusOutEvent(QFocusEvent *event)
{
   QGraphicsTextItem::focusOutEvent(event);
   validateText();
   updateGroupGeometry();
   property->updateValueFromItem();
   qApp->removeEventFilter(this);

   // Clear the text selection
   QTextCursor c = textCursor();
   c.clearSelection();
   setTextCursor(c);
}

void PropertyText::keyPressEvent(QKeyEvent *e)
{
   if(e->key() == Qt::Key_Escape)
      clearFocus();
   else
      QGraphicsTextItem::keyPressEvent(e);
}

void PropertyText::calculatePos()
{
   QFontMetricsF fm(font());
   prepareGeometryChange();
   staticPos = QPointF(-fm.width(m_staticText),fm.ascent()+2);
}

bool PropertyText::isSendable(QGraphicsSceneMouseEvent *event) const
{
   if(event->type() == QEvent::GraphicsSceneMousePress) {
      if( event->pos().x() < 0.0)
         return true;
      return false;
   }
   if(event->buttonDownPos(Qt::LeftButton).x() < 0.0)
      return true;
   return false;
}

void PropertyText::updateGroupGeometry()
{
   if(group()) {
      property->group()->forceUpdate();
   }
}
