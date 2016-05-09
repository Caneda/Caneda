/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2014 by Pablo Daniel Pareja Obregon                  *
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

#include <QGraphicsSimpleTextItem>

namespace Caneda
{
    //Forward declarations
    class GraphicsScene;
    class XmlWriter;
    class XmlReader;

    /*!
     * \brief This struct holds data of a property to be shared implicitly.
     *
     * This inherits QSharedData which takes care of reference counting.
     *
     * \sa Property
     */
    struct PropertyData : public QSharedData
    {
        explicit PropertyData();
        explicit PropertyData(const PropertyData& p);

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
     * groups them all together and renders them on a scene, allowing
     * selection and moving of all properties at once.
     *
     * \sa PropertyData, PropertyGroup
     */
    class Property
    {
    public:
        explicit Property(const QString &_name = QString(),
                          const QString &_defaultValue = QString(),
                          const QString &_description = QString(),
                          bool _visible=false);
        explicit Property(QSharedDataPointer<PropertyData> data);

        //! Returns the property name.
        QString name() const { return d->name; }
        //! Sets the value of property to \a newValue.
        void setName(const QString &newName) { d->name = newName; }

        //! Returns the value of property.
        QString value() const { return d->value; }
        //! Sets the value of property to \a newValue.
        void setValue(const QString &newValue) { d->value = newValue; }

        //! Returns the description of property.
        QString description() const { return d->description; }
        //! Sets the description of property to \a newDescription.
        void setDescription(const QString &newDescription) { d->description = newDescription; }

        //! Returns the visibility of property.
        bool isVisible() const { return d->visible; }
        //! Sets the visibility of property to \a visible .
        void setVisible(bool visible) { d->visible = visible; }

        static Property loadProperty(Caneda::XmlReader *reader);
        void saveProperty(Caneda::XmlWriter *writer) const;

    private:
        //! Pointer enabling implicit sharing of data.
        QSharedDataPointer<PropertyData> d;
    };


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
    class PropertyGroup : public QGraphicsSimpleTextItem
    {
    public:
        explicit PropertyGroup(QGraphicsItem *parent = 0);

        void addProperty(const QString& key, const Property& prop);
        //! Returns selected property from property map.
        QString propertyValue(const QString& key) const { return m_propertyMap[key].value(); }
        void setPropertyValue(const QString& key, const QString& value);

        //! Returns the property map (actually a copy of property map).
        PropertyMap propertyMap() const { return m_propertyMap; }
        void setPropertyMap(const PropertyMap& propMap);

        //! Returns if the user is enabled to add or remove properties.
        bool userPropertiesEnabled() const { return m_userPropertiesEnabled; }
        void setUserPropertiesEnabled(const bool enable);

        void updatePropertyDisplay();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget = 0 );

        void writeProperties(Caneda::XmlWriter *writer);
        void readProperties(Caneda::XmlReader *reader);

    public Q_SLOTS:
        int launchPropertiesDialog();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    private:
        //! QMap holding actual properties.
        PropertyMap m_propertyMap;

        /*!
         * \brief Holds the user created properties enable status.
         *
         * This tells the property dialog if the user should be
         * enabled to add or remove properties, and if the existing
         * properties should be allowed to change its key (property
         * name)
         *
         * \sa userPropertiesEnabled(), setUserPropertiesEnabled()
         */
        bool m_userPropertiesEnabled;
    };

} // namespace Caneda

#endif //PROPERTY_H
