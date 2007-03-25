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

#include "componentproperty.h"
#include "propertytext.h"
#include "component.h"

ComponentProperty::ComponentProperty( Component *c,const QString& name, const QString& value,
                                     const QString& description, bool visible,const QStringList& options)
   : m_component(c),
     m_name(name),
     m_value(value),
     m_description(description),
     m_options(options),
     m_item(0)
{
   if(visible)
      show();
}

ComponentProperty::~ComponentProperty()
{
   hide();
}

void ComponentProperty::show()
{
   if(m_item)
   {
      m_item->setPos(m_pos);
      m_item->show();
      return;
   }

   m_item = new PropertyText(this,m_component->schematicScene());
}

void ComponentProperty::hide()
{
   if(m_item)
      m_pos = m_item->pos();
   delete m_item;
   m_item = 0;
}

void ComponentProperty::operator=(const QString& value)
{
   if(m_options.isEmpty())
      m_value = value;
   else if(m_options.contains(value))
   {
      m_value = value;
   }
   else
   {
      qDebug() << "Trying to set value out of given options" ;
      qDebug() << m_name << m_options << "Given val is" << value;
   }
   update();
}

void ComponentProperty::update()
{
   if(m_item)
      m_item->updateValue();
}
