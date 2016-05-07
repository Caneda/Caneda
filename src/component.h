/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef QCOMPONENT_H
#define QCOMPONENT_H

#include "graphicsitem.h"
#include "property.h"

namespace Caneda
{
    // Forward declarations
    class PortData;

    //! \brief Shareable component's data.
    struct ComponentData : public QSharedData
    {
        explicit ComponentData();

        void setData(const QSharedDataPointer<ComponentData>& other);

        //! Static properties.
        QString name;
        QString filename;
        QString displayText;
        QString labelPrefix;
        QString description;
        QString library;

        /*!
         * Dynamic properties modifiable by the user (in the properties dialog).
         * Special care must be taken to copy the contents of this PropertyGroup
         * and not the pointer itself.
         */
        PropertyGroup *properties;

        //! List of component's ports.
        QList<PortData*> ports;

        //! QMap with all the models available to the component.
        QMap<QString, QString> models;
    };

    typedef QSharedDataPointer<ComponentData> ComponentDataPtr;

    /*!
     * \brief The Component class forms part of one of the GraphicsItem
     * derived classes available on Caneda. It is the base class for all
     * electronic components available through libraries. These components are
     * then inserted on a schematic to form a circuit.
     *
     * The component can either be directly loaded from an xml file or the data
     * manually set if required.
     *
     * This class uses LibraryManager::symbolCache() and
     * LibraryManager::pixmapCache() as a cache to render the component. To be
     * able to render the symbol, the symbol itself must be previously
     * registered (by the LibraryManager::registerComponent() method).
     *
     * \sa GraphicsItem, LibraryManager
     */
    class Component : public QObject, public GraphicsItem
    {
        Q_OBJECT

    public:
        explicit Component(GraphicsScene *scene = 0);
        ~Component();

        //! \copydoc GraphicsItem::Type
        enum { Type = GraphicsItem::ComponentType };
        //! \copydoc GraphicsItem::type()
        int type() const { return Type; }

        //! Returns name of the component (without localization).
        QString name() const { return d->name; }
        //! Returns the filename of the component.
        QString filename() const { return d->filename; }

        //! Returns string to be displayed in sidebar (with localization).
        QString displayText() const { return d->displayText; }

        //! Returns label prefix of component.
        QString labelPrefix() const { return d->labelPrefix; }
        QString labelSuffix() const;

        //! Returns a helpful text corresponding to component.
        QString description() const { return d->description; }

        //! Returns the library to which this component belongs.
        QString library() const { return d->library; }

        //! Returns the label of the component in the form {label_prefix}{number_suffix}
        QString label() const { return d->properties->propertyValue("label"); }
        bool setLabel(const QString& _label);

        //! Returns the component data.
        ComponentDataPtr componentData() const { return d; }
        void setComponentData(const ComponentDataPtr& other);

        //! Returns the property map (actually copy of property map).
        PropertyGroup* properties() const { return d->properties; }

        QString model(const QString& type) const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);

        Component *copy(GraphicsScene *scene = 0) const;

        void saveData(Caneda::XmlWriter *writer) const;
        void loadData(Caneda::XmlReader *reader);

        int launchPropertiesDialog();

    protected:
        QRectF adjustedBoundRect(const QRectF& rect);

    public slots:
        void updateBoundingRect();

    private:
        void updateSharedData();

        //! \brief Component shared data
        ComponentDataPtr d;
    };

} // namespace Caneda

#endif //COMPONENT_H
