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
#include <QString>

namespace Caneda
{
    //Forward declarations
    class XmlWriter;
    class XmlReader;

    /*!
     * \brief This struct hold data to be shared implicitly of a property.
     *
     * This inherits QSharedData which takes care of reference counting.
     *
     * \sa Property
     */
    struct PropertyData : public QSharedData
    {
        PropertyData();
        PropertyData(const PropertyData& p);

        ~PropertyData() { }

        QString name;
        QString value;
        QString description;
        bool visible;
    };

    /*!
     * \brief This class represents the property of a component.
     *
     * Actual properties inside the Property class, are implemented as
     * an implicitly shared class named PropertyData, thereby allowing
     * the use of the property objects directly instead of using pointers.
     *
     * Properties should be always strings. While more specific types
     * could be used, string types allow the use of suffixes and parameters
     * like p for pico, u for micro, and {R} for parameter, for example.
     *
     * While Property class holds actual properties, PropertyGroup class
     * groups them all together and PropertyDisplay class is the object
     * that renders them on a scene, allowing selection and moving of all
     * properties at once.
     *
     * \sa PropertyData, PropertyGroup, PropertyDisplay
     */
    class Property
    {
    public:
        Property(const QString &_name = QString(),
                 const QString &_defaultValue = QString(),
                 const QString &_description = QString(),
                 bool _visible=false);
        Property(QSharedDataPointer<PropertyData> data);

        //! Returns the property name.
        QString name() const { return d->name; }

        //! Returns the value of property.
        QString value() const { return d->value; }
        //! Sets the value of property to \a newValue.
        void setValue(const QString &newValue) { d->value = newValue; }

        //! Returns the description of property.
        QString description() const { return d->description; }

        //! Returns the visibility of property.
        bool isVisible() const { return d->visible; }
        //! Sets the visibility of property to \a visible .
        void setVisible(bool visible) { d->visible = visible; }

        static Property loadProperty(Caneda::XmlReader *reader);

    private:
        //! Pointer enabling implicit sharing of data.
        QSharedDataPointer<PropertyData> d;
    };

} // namespace Caneda

#endif //PROPERTY_H
