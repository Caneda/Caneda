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

#include "component.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "library.h"
#include "propertygroup.h"

#include "dialogs/propertydialog.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QPainter>

namespace Caneda
{
    //! \brief Constructs and initializes a default empty component item.
    Component::Component(CGraphicsScene *scene) :
        CGraphicsItem(0, scene),
        d(new ComponentData()), m_propertyGroup(0)
    {
        init();
    }

    //! \brief Constructs a component from \a other data.
    Component::Component(const QSharedDataPointer<ComponentData>& other, CGraphicsScene *scene) :
        CGraphicsItem(0, scene),
        d(other), m_propertyGroup(0)
    {
        init();
    }

    //! Destructor.
    Component::~Component()
    {
        delete m_propertyGroup;
        qDeleteAll(m_ports);
    }

    /*!
     * \brief Intialize the component.
     *
     * This method sets component's flags, adds initial label based on
     * default prefix value and adds the component's ports.
     *
     * The symbol corresponding to this component should already be
     * registered with LibraryManager using LibraryManager::registerComponent().
     */
    void Component::init()
    {
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        Property _label("label", tr("Label"), QVariant::String, true,
                false, labelPrefix().append('1'));
        d->propertyMap.insert("label", _label);

        const QList<PortData*> portDatas = d.constData()->ports;
        foreach(const PortData *data, portDatas) {
            m_ports << new Port(this, data->pos, data->name);
        }

        updateBoundingRect();
        updatePropertyGroup();
    }

    /*!
     * \brief This method updates the property display on schematic.
     *
     * This also takes care of creating new property group if it didn't
     * exist before and also deletes it if none of property are visible.
     */
    void Component::updatePropertyGroup()
    {
        bool itemsVisible = false;
        PropertyMap::const_iterator it = propertyMap().constBegin(),
            end = propertyMap().constEnd();
        // Determine if any item is visible.
        while(it != end) {
            if(it->isVisible()) {
                itemsVisible = true;
                break;
            }
            ++it;
        }
        // Delete the group if none of the properties are visible.
        if(!itemsVisible) {
            delete m_propertyGroup;
            m_propertyGroup = 0;
            return;
        }

        if(!m_propertyGroup) {
            createPropertyGroup();
        }
        else {
            m_propertyGroup->realignItems();
        }
    }

    //! \brief Creates property group for the first time.
    void Component::createPropertyGroup()
    {
        // Delete the old group if it exists.
        delete m_propertyGroup;
        m_propertyGroup = new PropertiesGroup(cGraphicsScene());
        m_propertyGroup->setParentItem(this);
        m_propertyGroup->setTransform(transform().inverted());
        m_propertyGroup->realignItems();
    }

    /*!
     * \brief Method used to set property's value.
     *
     * This also handles the property change of special properties such as
     * symbol, label and forwards the call to those methods on match.
     * It also updates the textual display of property on schematic.
     *
     * \param propName The property which is to be set.
     * \param value The new value to be set.
     * \return Returns true if successful, else returns false.
     */
    bool Component::setProperty(const QString& propName, const QVariant& value)
    {
        if(!propertyMap().contains(propName)) {
            qDebug() << "Component::setPropertyValue(): Property '" << propName
                     << "' doesn't exist!";
            return false;
        }
        if(propName == "label") {
            return setLabel(value.toString());
        }

        bool state = d->propertyMap[propName].setValue(value);
        if(state) {
            updatePropertyGroup();
        }
        return state;
    }

    //! \brief Takes care of visibility of property text on schematic.
    void Component::setPropertyVisible(const QString& propName, bool visiblity)
    {
        if(!propertyMap().contains(propName)) {
            qWarning() << "Component::setPropertyVisible() : Property " << propName
                       << " doesn't exist!";
            return;
        }
        d->propertyMap[propName].setVisible(visiblity);
        updatePropertyGroup();
    }

    /*!
     * \brief Sets the label of component.
     *
     * This also handles lable prefix and number suffix appropriately.
     * \param newLabel The label to be set now.
     * \return Returns true on success and false on failure.
     * \todo Yet to implement label prefix and number suffixing.
     */
    bool Component::setLabel(const QString& newLabel)
    {
        Q_ASSERT_X(propertyMap().contains("label"),
                __FUNCTION__, "label property not found");

        if(!newLabel.startsWith(labelPrefix())) {
            return false;
        }

        bool state = d->propertyMap["label"].setValue(newLabel);
        if(state) {
            updatePropertyGroup();
        }
        return state;
    }

    //! \brief Returns the label's suffix part.
    QString Component::labelSuffix() const
    {
        QString _label = label();
        return _label.mid(labelPrefix().length());
    }

    //! \brief Sets the propertyMap of this component to \a propMap
    void Component::setPropertyMap(const PropertyMap& propMap)
    {
        d->propertyMap = propMap;
    }

    /*!
     * \brief Convenience static method to load component saved as xml.
     *
     * \param reader The xmlreader used to read xml data.
     * \param scene CGraphicsScene to which component should be parented to.
     * \return Returns new component pointer on success and null on failure.
     */
    Component* Component::loadComponentData(Caneda::XmlReader *reader, CGraphicsScene *scene)
    {
        Component *retVal = 0;
        Q_ASSERT(reader->isStartElement() && reader->name() == "component");

        QString compName = reader->attributes().value("name").toString();
        QString libName = reader->attributes().value("library").toString();

        Q_ASSERT(!compName.isEmpty());

        retVal = LibraryManager::instance()->newComponent(compName, scene, libName);
        if(retVal) {
            retVal->loadData(reader);
        }
        else {
            // Read to the end of the file if not found in any of Caneda libraries.
            qWarning() << "Warning: Found unknown element" << compName << ", skipping";
            reader->readUnknownElement();
        }
        return retVal;
    }

    //! \reimp
    void Component::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "component");

        d->name = reader->attributes().value("name").toString();
        d->library = reader->attributes().value("library").toString();

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "pos") {
                    setPos(reader->readPoint());
                }
                else if(reader->name() == "propertyPos") {
                    QPointF point = reader->readPoint();
                    if(m_propertyGroup) {
                        m_propertyGroup->setPos(point);
                    }
                }
                else if(reader->name() == "transform") {
                    setTransform(reader->readTransform());
                }
                else if(reader->name() == "properties") {
                    // Note the usage as it expects reference of property map.
                    readProperties(reader, d->propertyMap);
                    // This updates the visual representation appropriately.
                    setPropertyMap(d->propertyMap);
                }
                else {
                    qWarning() << "Warning: Found unknown element" << reader->name().toString();
                    reader->readUnknownElement();
                }
            }
        }
    }

    //! \reimp
    void Component::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("component");
        writer->writeAttribute("name", name());
        writer->writeAttribute("library", library());

        writer->writePoint(pos(), "pos");
        if(m_propertyGroup) {
            writer->writePoint(m_propertyGroup->pos(), "propertyPos");
        }

        writer->writeTransform(transform());
        writeProperties(writer, d.constData()->propertyMap);

        writer->writeEndElement();  //</component>
    }

    /*!
     * \brief Paints a registered component, peforms sanity checks and takes
     * care of drawing the selection rectangle.
     */
    void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *w)
    {
        // Paint the component symbol
        LibraryManager *libraryManager = LibraryManager::instance();
        QPainterPath *symbol = libraryManager->symbolCache(name());

        if(painter->worldTransform().isScaling()) {
            // If zooming, the paint is performed without the pixmap cache.
            painter->drawPath(symbol->simplified());
        }
        else {
            // Else, a pixmap cached is used.
            QPixmap pix = libraryManager->pixmapCache(name());
            QRect rect =  symbol->boundingRect().toRect();
            painter->drawPixmap(rect, pix);
        }


        // Paint the selection rectangle
        if(option->state & QStyle::State_Selected) {
            Caneda::drawHighlightRect(painter, this->boundingRect(), 1.0, option);
        }

        // Paint the ports
        foreach(Port *port, m_ports) {
            port->paint(painter, option);
        }
    }

    //! \brief Returns a copy of this component.
    Component* Component::copy(CGraphicsScene *scene) const
    {
        Component *retVal = new Component(d, scene);
        // No need for Component::copyDataTo() because data is already copied from d pointer.
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(retVal));
        retVal->updatePropertyGroup();
        return retVal;
    }

    //! \brief Copies the data to \a component.
    void Component::copyDataTo(Component *component) const
    {
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(component));
        component->d = d;
        component->updatePropertyGroup();
        component->update();
    }

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int Component::launchPropertyDialog(Caneda::UndoOption)
    {
        PropertyDialog *dia = new PropertyDialog(this, Caneda::PushUndoCmd);
        int status = dia->exec();
        delete dia;

        return status;
    }

    //! \brief Returns the rect adjusted to accomodate ports too.
    QRectF Component::adjustedBoundRect(const QRectF& rect)
    {
        QRectF adjustedRect = rect;
        foreach(Port *port, m_ports) {
            adjustedRect |= portEllipse.translated(port->pos());
        }
        return adjustedRect;
    }

    //! \brief React to change of item position.
    QVariant Component::itemChange(GraphicsItemChange change,
            const QVariant &value)
    {
        if(change == ItemTransformHasChanged && m_propertyGroup) {
            // Set the inverse of component's matrix to property group so that
            // it maintains identity when transformed.
            m_propertyGroup->setTransform(transform().inverted());
        }
        return CGraphicsItem::itemChange(change, value);
    }

    //! \brief Updates the bounding rect of this item.
    void Component::updateBoundingRect()
    {
        // Get the bounding rect of the symbol
        QPainterPath *symbol = LibraryManager::instance()->symbolCache(name());

        // Get an adjusted rect for accomodating extra stuff like ports.
        QRectF adjustedRect = adjustedBoundRect(symbol->boundingRect());

        // Set symbol bounding rect
        setShapeAndBoundRect(QPainterPath(), adjustedRect, 1.0);
    }

} // namespace Caneda
