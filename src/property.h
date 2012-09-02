/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#ifndef PROPERTY_H
#define PROPERTY_H

#include <QSharedData>
#include <QStringList>
#include <QVariant>

namespace Caneda
{
    //Forward declarations
    class XmlWriter;
    class XmlReader;

    class Property;

    //! \def PropertyMap This is typedef for map of string and property.
    typedef QMap<QString, Property> PropertyMap;

    /*!
     * \brief This struct hold data to be shared implicitly of a property.
     * \details This inherits QSharedData which takes care of reference counting.
     *
     * \sa Property
     */
    struct PropertyData : public QSharedData
    {
        PropertyData();
        PropertyData(const PropertyData& p);

        ~PropertyData() { delete options; }

        QString name;
        QVariant value;
        QVariant::Type valueType;
        QString description;
        QStringList *options;
        bool visible;
        bool netlistProperty;
    };

    /*!
     * \brief This class represents the property of a component.
     *
     * Actual properties inside the Property class, are implemented as
     * an implicitly shared class named PropertyData, thereby allowing
     * the use of the property objects directly instead of using pointers.
     *
     * While Property class holds actual properties, PropertyItem
     * class is the object that renders them on a scene. Finally, PropertiesGroup
     * is the class that groups all PropertyItems to allow selection and
     * moving of all properties at once.
     *
     * \sa PropertyData, PropertiesGroup, PropertyItem
     * \note Assumes that options list is always strings!
     */
    class Property
    {
    public:
        Property(const QString& _name = QString(),
                 const QString& _description = QString(),
                 QVariant::Type _valueType = QVariant::String,
                 bool _visible=false,
                 bool _isNetlistProp=true,
                 const QVariant& _defaultValue = QVariant(QString()),
                 const QStringList& _options = QStringList());

        //! Returns the description of property.
        QString description() const { return d->description; }

        //! Returns the property name.
        QString name() const { return d->name; }

        //! Returns the options for the value of property.
        QStringList options() const {
            return d->options ? *d->options : QStringList();
        }

        bool setValue(const QVariant& newValue);

        //! Returns the value of property.
        QVariant value() const { return d->value; }

        //! Returns the value's data type.
        QVariant::Type valueType() const { return d->valueType; }

        //! Sets the visibility of property tp \a visible .
        void setVisible(bool visible) { d->visible = visible; }

        //! Returns the visibility of property.
        bool isVisible() const { return d->visible; }

        //! Returns whether this property should be written to netlist or not.
        bool isNetlistProperty() const { return d->netlistProperty; }

    private:
        //! d pointer enabling sharing of data implicitly.
        QSharedDataPointer<PropertyData> d;
        Property(QSharedDataPointer<PropertyData> data);
        friend class PropertyFactory;
    };

    void writeProperties(Caneda::XmlWriter *writer, const PropertyMap& prMap);
    void readProperties(Caneda::XmlReader *reader, PropertyMap &propMap);

    //! This is factory class used to construct properties.
    struct PropertyFactory
    {
        static Property createProperty(Caneda::XmlReader *reader);
        static Property sharedNull;
    };

} // namespace Caneda

#endif //PROPERTY_H
