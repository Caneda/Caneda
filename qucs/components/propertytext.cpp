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
   setFlag(ItemIsMovable,true);
   setFlag(ItemIsSelectable,false);
   setFlag(ItemIsFocusable,false);
   setSelected(false);
   clickedIn = false;
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
   if(!clickedIn)
   {         
      o->state &= ~QStyle::State_Selected;
      o->state &= ~QStyle::State_HasFocus;
   }
   QGraphicsTextItem::paint(painter,o,widget);
	
   if (!clickedIn && option->state & (QStyle::State_Selected | QStyle::State_HasFocus))
   {
      painter->setPen(QPen(Qt::black, 1));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(boundingRect());
   }
	
   painter->restore();
   delete o;
}

void PropertyText::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if(isSelected() && event->modifiers() & Qt::ControlModifier)
   {
      setSelected(false);
      return;
   }
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   if (event->pos().x() < 0 || (!clickedIn && !isSelected()))
   {
      QGraphicsItem::mousePressEvent(event);
      clickedIn = false;
      //clearFocus();     
   }
   else
   {
      clickedIn = true;
      setFlag(ItemIsMovable,false);
      QGraphicsTextItem::mousePressEvent(event);
   }
}

void PropertyText::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   if (!clickedIn)
      QGraphicsItem::mouseMoveEvent(event);
   else
      QGraphicsTextItem::mouseMoveEvent(event);
}

void PropertyText::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   if (clickedIn)
      QGraphicsTextItem::mouseReleaseEvent(event);
}

void PropertyText::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   QGraphicsTextItem::mouseDoubleClickEvent(event);
	
   if(!clickedIn)
   {
      QTextCursor tc = textCursor();
      tc.clearSelection();
      setTextCursor(tc);
   }
   clickedIn = true;
}

void PropertyText::focusOutEvent(QFocusEvent *event)
{
   clickedIn = false;
   QTextCursor tc = textCursor();
   tc.clearSelection();
   setTextCursor(tc);
   trimText();
   QGraphicsTextItem::focusOutEvent(event);      
   setFlag(ItemIsMovable,true);
   setFlag(ItemIsSelectable,false);
   setFlag(ItemIsFocusable,false);
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
   if(change == QGraphicsItem::ItemSelectedChange)
   {
      if(value.toBool() == false)
         QTimer::singleShot(50,this,SLOT(resetFlags()));
      return value;
   }
   return QGraphicsTextItem::itemChange(change,value);
}

void PropertyText::resetFlags()
{
   setFlag(ItemIsMovable,true);
   setFlag(ItemIsSelectable,false);
   setFlag(ItemIsFocusable,false);
}
