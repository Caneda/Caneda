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

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>

//! Default constructor
PropertyData::PropertyData()
{
    options = 0;
    visible = false;
    netlistProperty = true;
    //default property type is string.
    valueType = QVariant::String;
    value = QVariant(QString());
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

/*!
 * \brief Constructs a property object
 *
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
 *
 * \note The value's data type is assumed to be string type if it has options.
 */
Property::Property(const QString& _name, const QString& _description,
        QVariant::Type _valueType,
        bool _visible, bool _isNetlistProp, const QVariant& _value,
        const QStringList& _options)
{
    d = new PropertyData;
    d->name = _name;
    d->description = _description;
    d->visible = _visible;
    d->netlistProperty = _isNetlistProp;
    d->value = _value;
    d->valueType = _valueType;

    bool conversion = d->value.convert(_valueType);
    Q_ASSERT(conversion == true);

    d->options = _options.isEmpty() ? 0 : new QStringList(_options);
}

//! \brief Construct property from shared data.
Property::Property(QSharedDataPointer<PropertyData> data) : d(data)
{
}

/*!
 * \brief Sets the value of property to \a newValue if it is valid to do so.
 * \return Returns whether the property could be succesfully set or not.
 */
bool Property::setValue(const QVariant& newValue)
{
    if(d.constData()->options == 0 ||
            d.constData()->options->contains(newValue.toString())) {

        d->value = newValue;
        Q_ASSERT(d->value.convert(d->valueType));
        return true;
    }
    qDebug() << "Property::setValue(): " << "Trying to assign value out of"
                                             "options";
    return false;
}

//! \brief Helper function to write all properties in \a propMap in xml.
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

//! \brief Helper function to read the saved properties into \a propMap.
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

//! \brief A function which returns corresponding variant type from given \a atring.
QVariant::Type stringToType(const QString& _string)
{
    char first = _string.at(0).toAscii();
    QString remain = _string.right(_string.size() - 1);
    QVariant::Type retVal = QVariant::Invalid;
    switch(first) {
        case 's':
            if(remain == "tring") {
                retVal = QVariant::String;
            }
            break;

        case 'b':
            if(remain == "oolean") {
                retVal = QVariant::Bool;
            }
            break;

        case 'i':
            if(remain == "nt") {
                retVal = QVariant::Int;
            }
            break;

        case 'd':
            if(remain == "ouble") {
                retVal = QVariant::Double;
            }
            break;
    };

    if(retVal == QVariant::Invalid) {
        qDebug() << "stringToType() : Invalid qvariant type found" << _string;
    }

    return retVal;
}

//! \brief Returns string corresponding to \a type.
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

//! \brief This static object is used to represent common empty property.
Property PropertyFactory::sharedNull;

/*!
 * \brief This factory method is used to create property from xml.
 * \param reader XmlReader which is in use for parsing.
 */
Property PropertyFactory::createProperty(Qucs::XmlReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "property");
    QSharedDataPointer<PropertyData> data(new PropertyData);

    QXmlStreamAttributes attributes = reader->attributes();

    data->name = attributes.value("name").toString();
    if(data->name.isEmpty()) {
        reader->raiseError("Couldn't find name attribute in property description");
        return sharedNull;
    }

    if(attributes.value("type").isEmpty()) {
        reader->raiseError("Couldn't find 'type' attribute in property description");
        return sharedNull;
    }
    QString vt(attributes.value("type").toString());
    data->valueType = stringToType(vt);
    if(data->valueType == QVariant::Invalid) {
        reader->raiseError(QObject::tr("Invalid property type %1 found").arg(vt));
        return sharedNull;
    }

    QString visible = attributes.value("visible").toString();
    data->visible = !(visible.isEmpty() || visible == "false");

    QString options = attributes.value("options").toString();
    if(!options.isEmpty()) {
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

        if(reader->isEndElement()) {
            break;
        }

        if(reader->isStartElement()) {
            if(reader->name() == "description") {
                data->description = reader->readLocaleText(Qucs::localePrefix());
            }
            else {
                reader->readUnknownElement();
            }
        }
    }

    if(!defaultVal.isNull() && defaultVal.convert(data->valueType)) {
        data->value = defaultVal;
    }

    return Property(data);
}
