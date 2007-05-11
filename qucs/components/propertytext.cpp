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

#include "propertytext.h"
#include "componentproperty.h"
#include "schematicscene.h"

#include <QtGui/QTextCursor>
#include <QtGui/QCursor>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QGraphicsScene>
#include <QtGui/QFontMetricsF>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QPainter>

#include <QtCore/QTimer>
#include <QtCore/QtDebug>

PropertyText::PropertyText(ComponentProperty *prop,SchematicScene *sc) :
   QGraphicsTextItem(0,(QGraphicsScene *)sc), property(prop)
{
   m_staticText = prop->name();
   m_staticText.append(" = ");
   setPlainText(prop->value());
   setTextInteractionFlags(Qt::TextEditorInteraction);
   setFlags(ItemIsMovable | ItemIsFocusable);
   calculatePos();
}

void PropertyText::calculatePos()
{
   QFontMetricsF fm(font());
   prepareGeometryChange();
   staticPos = QPointF(-fm.width(m_staticText),fm.ascent()+2);
}

void PropertyText::trimText()
{
   QString text = toPlainText().remove(QRegExp("[\n\r]"));
   if(text.isEmpty())
      text.append('0');
   setPlainText(text);
}

void PropertyText::updateValue()
{
   setPlainText(property->value());
}

void PropertyText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
   painter->save();
   painter->setFont(font());
   painter->drawText(staticPos,m_staticText);

   QStyleOptionGraphicsItem *o = new QStyleOptionGraphicsItem(*option);
   o->exposedRect.setLeft(0.);
   o->state &= ~QStyle::State_Selected;
   o->state &= ~QStyle::State_HasFocus;
   QGraphicsTextItem::paint(painter,o,widget);

   if (option->state & QStyle::State_HasFocus)
   {
      painter->setPen(QPen(Qt::black, 1));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(QGraphicsTextItem::boundingRect());
   }

   painter->restore();
   delete o;
}

bool PropertyText::sceneEvent(QEvent *event)
{
   if(!group() || !hasFocus())
      return QGraphicsTextItem::sceneEvent(event);

   // The call to parent is avoided to remove group behaviour
   // and allow items to be edited.
   switch (event->type()) {
    case QEvent::FocusIn:
        focusInEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::FocusOut:
        focusOutEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::GraphicsSceneContextMenu:
        contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragEnter:
        dragEnterEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragMove:
        dragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragLeave:
        dragLeaveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDrop:
        dropEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverEnter:
        hoverEnterEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverMove:
        hoverMoveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverLeave:
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMousePress:
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneWheel:
        wheelEvent(static_cast<QGraphicsSceneWheelEvent *>(event));
        break;
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::InputMethod:
        inputMethodEvent(static_cast<QInputMethodEvent *>(event));
        break;
    default:
        return false;
    }

   return true;
}

void PropertyText::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if (event->pos().x() > 0) {
      if(scene())
         scene()->clearSelection();
      QGraphicsTextItem::mousePressEvent(event);
   }
   else
      clearFocus();
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
   QTextCursor tc = textCursor();
   tc.clearSelection();
   setTextCursor(tc);
}

void PropertyText::focusOutEvent(QFocusEvent *event)
{
   QGraphicsTextItem::focusOutEvent(event);
   trimText();
}

void PropertyText::keyPressEvent(QKeyEvent *e)
{
   if(e->key() == Qt::Key_Escape)
      clearFocus();
   else
      QGraphicsTextItem::keyPressEvent(e);
}

QVariant PropertyText::itemChange(GraphicsItemChange change, const QVariant &value)
{
   return QGraphicsTextItem::itemChange(change,value);
}
