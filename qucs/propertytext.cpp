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

#include "propertytext.h"

#include <QtGui/QTextCursor>
#include <QtGui/QCursor>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QGraphicsScene>

PropertyTextValue::PropertyTextValue(const QString& text,QGraphicsItem *p,QGraphicsScene *scene) :
   QGraphicsTextItem(text,p,scene)
{
   setCursor(Qt::IBeamCursor);
}

void PropertyTextValue::formatText()
{
   QString text = toPlainText().remove(QRegExp("[\n\r ]"));
   if(text.isEmpty())
      text.append('0');
   setPlainText(text);
}

void PropertyTextValue::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
   if(e->button() == Qt::LeftButton)
   {
      setTextInteractionFlags(Qt::TextEditorInteraction);
      setFocus();
   }
   QGraphicsTextItem::mousePressEvent(e);
}

void PropertyTextValue::keyPressEvent(QKeyEvent *e)
{
   if(e->key() == Qt::Key_Escape)
      clearFocus();
   QGraphicsTextItem::keyPressEvent(e);
}

void PropertyTextValue::focusOutEvent(QFocusEvent *e)
{
   setTextInteractionFlags(Qt::NoTextInteraction);
   QTextCursor tc = textCursor();
   tc.clearSelection();
   setTextCursor(tc);
   formatText();
   QGraphicsTextItem::focusOutEvent(e);
}

PropertyText::PropertyText(const QString& name, const QString& initialValue,
			   const QString& description, QGraphicsItem* par, QGraphicsScene *scene)
   : QGraphicsTextItem(name + QString(" = "),par,scene)
{
   m_description = description;
   m_name = name;
   m_valueItem = new PropertyTextValue(initialValue,this,scene);
   m_valueItem->setPos(40,0);
   setFlag(ItemIsMovable,true);
}

QVariant PropertyText::itemChange(GraphicsItemChange c,const QVariant& value)
{
   return QGraphicsTextItem::itemChange(c,value);
}

void PropertyText::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
   scene()->clearSelection();
   QGraphicsTextItem::mousePressEvent(e);
}
