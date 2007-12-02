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

#include "property.h"
#include "xmlutilities.h"
#include "qucs-tools/global.h"
#include <QtCore/QDebug>

//! Default constructor
PropertyData::PropertyData()
{
   options = 0;
   visible = false;
   netlistProperty = true;
   valueType = QVariant::String;
}

//! Copy constructor
PropertyData::PropertyData(const PropertyData& p) : QSharedData(p)
{
   name = p.name;
   value = p.value;
   valueType = p.valueType;
   Q_ASSERT(value.type() == valueType);
   description = p.description;
   visible = p.visible;
   netlistProperty = p.netlistProperty;

   options = p.options ? new QStringList(*p.options) : 0;
}

//! Assignment opearator.
PropertyData& PropertyData::operator=(const PropertyData& p)
{
   name = p.name;
   value = p.value;
   valueType = p.valueType;
   Q_ASSERT(value.type() == valueType);
   description = p.description;
   visible = p.visible;
   netlistProperty = p.netlistProperty;

   options = p.options ? new QStringList(*p.options) : 0;
   return *this;
}

/*! Constructs a property object
 * \param _name Name of property object.
 * \param _description Description of property.
 * \param _valueType The type of \a _value.
 * \param _visible Visibility of property object(item)
 * \param _isNetlistProp This represents whether this property should be
 *  written to netlist or not.
 * \param _value The default value of property.
 * \param _options Represents a list of available options for the value.
 * If _options is empty, then any value can be set otherwise the value should
 * be only one of _options.
 */
Property::Property(const QString& _name, const QString& _description,
                   QVariant::Type _valueType,
                   bool _visible, bool _isNetlistProp, const QVariant& _value,
                   const QStringList& _options)
{
   d = new PropertyData;
   PropertyData *data = d.data();
   data->name = _name;
   data->description = _description;
   data->visible = _visible;
   data->netlistProperty = _isNetlistProp;
   data->value = _value;
   data->valueType = _valueType;
   if(!data->value.isNull()) {
      bool conversion = data->value.convert(_valueType);
      Q_ASSERT(conversion == true);
   }

   if(!_options.isEmpty())
      data->options = new QStringList(_options);
   else
      data->options = 0;
}

/*! \brief Convienience constructor.
 * This constructor fetches the property details from \a reader.
 */
Property::Property(Qucs::XmlReader *reader)
{
   d = new PropertyData;
   readXml(reader);
}

//! Copy constructor
Property::Property(const Property& prop) : d(prop.d)
{
}

//! Assingment operator.
Property& Property::operator=(const Property& prop)
{
   d = prop.d;
   return *this;
}

//! Sets the value of property to \a newValue
void Property::setValue(const QVariant& newValue)
{
   if(d.constData()->options == 0 ||
      d.constData()->options->contains(newValue.toString())) {

      d->value = newValue;
      Q_ASSERT(d->value.convert(d->valueType));
      return;
   }
   qWarning() << "Property::setValue(): " << "Trying to assign value out of"
      "options";
}

//! Reads the property from component's description xml file.
void Property::readXml(Qucs::XmlReader *reader)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "property");
   QXmlStreamAttributes attributes = reader->attributes();

   PropertyData *data = d.data();
   data->name = attributes.value("name").toString();
   Q_ASSERT(!data->name.isEmpty());

   Q_ASSERT(!attributes.value("type").isEmpty());
   data->valueType = stringToType(attributes.value("type").toString());

   QString visible = attributes.value("visible").toString();
   data->visible = !(visible.isEmpty() || visible == "false");

   QString options = attributes.value("options").toString();
   if(!options.isEmpty()) {
      delete data->options;
      options.replace(' ', "");
      QStringList splitList = options.split(',', QString::SkipEmptyParts);
      data->options = new QStringList(splitList);
   }

   QString defaultValStr = attributes.value("default").toString();
   QVariant defaultVal;
   if(!defaultValStr.isEmpty()) {
      defaultVal.setValue(defaultValStr);
   }

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement())
         break;

      if(reader->isStartElement()) {
         if(reader->name() == "description") {
            data->description = reader->readLocaleText(Qucs::localePrefix());
         }
         else {
            reader->readUnknownElement();
         }
      }
   }

   if(!defaultVal.isNull())
      setValue(defaultVal);
}

//! Helper function to write all properties in \a propMap in xml.
void writeProperties(Qucs::XmlWriter *writer, const PropertyMap& propMap)
{
   writer->writeStartElement("properties");
   foreach(const Property p, propMap) {
      writer->writeEmptyElement("property");
      writer->writeAttribute("name", p.name());
      writer->writeAttribute("value", p.value().toString());
      writer->writeAttribute("visible", Qucs::boolToString(p.isVisible()));
   }
   writer->writeEndElement(); // </properties>
}

//! Helper function to read the saved properties into \a propMap.
void readProperties(Qucs::XmlReader *reader, PropertyMap &propMap)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "properties");

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "property") {
            QXmlStreamAttributes attribs(reader->attributes());
            QString propName = attribs.value("name").toString();
            if(!propMap.contains(propName)) {
               qWarning() << "readProperties() : " << "Property " << propName
                          << "not found in map!";
            }
            else {
               Property &prop = propMap[propName];
               prop.setValue(QVariant(attribs.value("value").toString()));
               prop.setVisible(attribs.value("visible") == "true");
            }
            // now read till end element
            reader->readUnknownElement();
         }
         else {
            reader->readUnknownElement();
         }
      }
   }
}

//! A function which returns corresponding variant type from given \a atring.
QVariant::Type stringToType(const QString& _string)
{
   char first = _string.at(0).toAscii();
   QString remain = _string.right(_string.size() - 1);
   QVariant::Type retVal = QVariant::Invalid;
   switch(first) {
      case 's':
         if(remain == "tring")
            retVal = QVariant::String;
         break;
      case 'b':
         if(remain == "oolean")
            retVal = QVariant::Bool;
         break;
      case 'i':
         if(remain == "nt")
            retVal = QVariant::Int;
         break;
      case 'd':
         if(remain == "ouble")
            retVal = QVariant::Double;
         break;
   };
   if(retVal == QVariant::Invalid) {
      qDebug() << "stringToType() : Invalid qvariant type found" << _string;
   }
   return retVal;
}

//! Returns string corresponding to \a type.
QString typeToString(QVariant::Type type)
{
   switch(type)
   {
      case QVariant::String:
         return QString("string");
      case QVariant::Bool:
         return QString("boolean");
      case QVariant::Int:
         return QString("int");
      case QVariant::Double:
         return QString("double");
      default: ;
   };
   qDebug() << "typeToString() : Invalid type" << type;
   return QString();
}
