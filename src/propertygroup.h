/***************************************************************************
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

#ifndef PROPERTY_GROUP_H
#define PROPERTY_GROUP_H

#include "property.h"

#include <QMap>
#include <QString>

namespace Caneda
{
    //Forward declarations
    class PropertyDisplay;
    class XmlWriter;
    class XmlReader;

    //! \def PropertyMap This is typedef for map of string and property.
    typedef QMap<QString, Property> PropertyMap;

    /*!
     * \brief This class groups properties all together.
     *
     * Gouping all properties of a component into a QMap (m_propertyMap)
     * provides a convenient way of handling them all together. In this
     * way, the properties of a component can be selected and moved
     * all at once.
     *
     * While Property class holds actual properties, PropertyGroup class
     * groups them all together and PropertyDisplay class is the object
     * that renders them on a scene, allowing selection and moving of all
     * properties at once.
     *
     * \sa PropertyData, Property, PropertyDisplay
     */
    class PropertyGroup
    {
    public:
        PropertyGroup(PropertyMap propertyMap = PropertyMap());
        ~PropertyGroup();

        //! Returns selected property from property map.
        QString property(const QString& value) { return m_propertyMap[value].value(); }
        void addProperty(const QString& key, const Property& prop);
        void setProperty(const QString& key, const QString& value);

        //! Returns the property map (actually a copy of property map).
        PropertyMap propertyMap() const { return m_propertyMap; }
        void setPropertyMap(const PropertyMap& propMap);

        //! Returns property group of the component.
        PropertyDisplay* propertyDisplay() const { return m_propertyDisplay; }
        void updatePropertyDisplay();

        void writeProperties(Caneda::XmlWriter *writer);
        void readProperties(Caneda::XmlReader *reader);

    private:
        //! \brief QMap holding actual properties.
        PropertyMap m_propertyMap;

        //! \brief Property display of this PropertyGroup
        PropertyDisplay *m_propertyDisplay;
    };

    void writeProperties(Caneda::XmlWriter *writer, const PropertyMap& propMap);
    void readProperties(Caneda::XmlReader *reader, PropertyMap &propMap);

} // namespace Caneda

#endif //PROPERTY_GROUP_H
