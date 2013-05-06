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

#include <QGraphicsSimpleTextItem>
#include <QMap>
#include <QObject>
#include <QString>

namespace Caneda
{
    //Forward declarations
    class CGraphicsScene;
    class XmlWriter;
    class XmlReader;

    //! \def PropertyMap This is a typedef to map properties with strings.
    typedef QMap<QString, Property> PropertyMap;

    /*!
     * \brief Class used to group properties all together and render
     * them on a scene.
     *
     * Gouping several properties into a QMap (m_propertyMap) provides
     * a convenient way of handling them all together. In this way, for
     * example, the properties of a component can be selected and moved
     * all at once.
     *
     * While Property class holds actual properties, PropertyGroup class
     * groups them all together and renders them on a scene, allowing
     * selection and moving of all properties at once.
     *
     * \sa PropertyData, Property
     */
    class PropertyGroup : public QObject, public QGraphicsSimpleTextItem
    {
        Q_OBJECT

    public:
        PropertyGroup(CGraphicsScene* scene = 0, const PropertyMap& propMap = PropertyMap());
        ~PropertyGroup() {}

        void addProperty(const QString& key, const Property& prop);
        //! Returns selected property from property map.
        QString propertyValue(const QString& value) const { return m_propertyMap[value].value(); }
        void setPropertyValue(const QString& key, const QString& value);

        //! Returns the property map (actually a copy of property map).
        PropertyMap propertyMap() const { return m_propertyMap; }
        void setPropertyMap(const PropertyMap& propMap);

        void updatePropertyDisplay();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget = 0 );

        void writeProperties(Caneda::XmlWriter *writer);
        void readProperties(Caneda::XmlReader *reader);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    private:
        //! \brief QMap holding actual properties.
        PropertyMap m_propertyMap;
    };

} // namespace Caneda

#endif //PROPERTY_GROUP_H
