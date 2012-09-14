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
#include "propertydisplay.h"
#include "settings.h"
#include "xmlutilities.h"

#include "dialogs/propertydialog.h"

#include <QDebug>
#include <QFile>
#include <QPainter>

namespace Caneda
{
    //! \brief Constructs and initializes a default empty component item.
    Component::Component(CGraphicsScene *scene) :
        CGraphicsItem(0, scene),
        d(new ComponentData()), m_propertyDisplay(0)
    {
        init();
    }

    //! \brief Constructs a component from \a other data.
    Component::Component(const QSharedDataPointer<ComponentData>& other, CGraphicsScene *scene) :
        CGraphicsItem(0, scene),
        d(other), m_propertyDisplay(0)
    {
        init();
    }

    //! \brief Destructor.
    Component::~Component()
    {
        delete m_propertyDisplay;
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

        Property _label("label", labelPrefix().append('1'), tr("Label"), true);
        d->propertyMap.insert("label", _label);

        const QList<PortData*> portDatas = d.constData()->ports;
        foreach(const PortData *data, portDatas) {
            m_ports << new Port(this, data->pos, data->name);
        }

        updateBoundingRect();
        updatePropertyDisplay();
    }

    /*!
     * \brief Updates properties' display on a scene (schematic).
     *
     * This method updates properties' display on a scene. It also takes
     * care of creating a new PropertyDisplay if it didn't exist before
     * and deletes it if none of the properties are visible.
     *
     * \sa PropertyDisplay, PropertyDisplay::update()
     */
    void Component::updatePropertyDisplay()
    {
        bool itemsVisible = false;

        // Determine if any item is visible.
        PropertyMap::const_iterator it = propertyMap().constBegin(),
            end = propertyMap().constEnd();
        while(it != end) {
            if(it->isVisible()) {
                itemsVisible = true;
                break;
            }
            ++it;
        }

        // Delete the group if none of the properties are visible.
        if(!itemsVisible) {
            delete m_propertyDisplay;
            m_propertyDisplay = 0;
            return;
        }

        // If m_propertyDisplay=0 create a new PopertyDisplay, else
        // just update it calling PopertyDisplay::updateProperties()
        if(!m_propertyDisplay) {
            m_propertyDisplay = new PropertyDisplay(cGraphicsScene());
            m_propertyDisplay->setParentItem(this);
            m_propertyDisplay->setTransform(transform().inverted());
        }

        m_propertyDisplay->updateProperties();
    }

    /*!
     * \brief Method to obtain property's value.
     *
     * \param propName The name of property.
     * \return Returns corresponding property if it exists otherwise
     * returns empty QString().
     */
    QString Component::property(const QString& propName) const
    {
        if(d->propertyMap.contains(propName)){
            return d->propertyMap[propName].value();
        }

        return QString();
    }

    /*!
     * \brief Sets the label of component.
     *
     * This method also handles label prefix and number suffix appropriately.
     * In case the label doesn't start with the correct number prefix, the
     * return value is false.
     *
     * \param newLabel The label to be set.
     * \return True on success and false on failure.
     */
    bool Component::setLabel(const QString& newLabel)
    {
        Q_ASSERT_X(propertyMap().contains("label"),
                __FUNCTION__, "label property not found");

        if(!newLabel.startsWith(labelPrefix())) {
            return false;
        }

        d->propertyMap["label"].setValue(newLabel);
        updatePropertyDisplay();
        return true;
    }

    //! \brief Returns the label's suffix part.
    QString Component::labelSuffix() const
    {
        QString _label = label();
        return _label.mid(labelPrefix().length());
    }

    /*!
     * \brief Set the component's properties' values.
     *
     * This method sets the component's properties' values by updating
     * its propertyMap to \a propMap.
     *
     * First it sets the propertyMap of this component and then takes
     * care of updating the PropertyDisplay (which is the class that displays
     * properties on a scene).
     *
     * \param propMap The new property map to be set.
     *
     * \sa Property, PropertyMap, updatePropertyDisplay(), PropertyDisplay
     */
    void Component::setPropertyMap(const PropertyMap& propMap)
    {
        d->propertyMap = propMap;
        updatePropertyDisplay();  // This is neccessary to update the properties display on a scene
    }

    /*!
     * \brief Convenience static method to load a component saved as xml.
     *
     * This method loads a component saved as xml. Once the component name
     * and library are retrieved, a new component is created from LibraryManager
     * and its data is filled using the loadData() method.
     *
     * \param reader The xmlreader used to read xml data.
     * \param scene CGraphicsScene to which component should be parented to.
     * \return Returns new component pointer on success and null on failure.
     *
     * \sa LibraryManager::newComponent(), loadData()
     */
    Component* Component::loadComponent(Caneda::XmlReader *reader, CGraphicsScene *scene)
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

    /*!
     * \brief Loads current component data from \a Caneda::XmlReader.
     *
     * Loads current component data (name, library, position, properties
     * and transform) from \a Caneda::XmlReader.
     *
     * \sa loadComponentData(), saveData()
     */
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
                    if(m_propertyDisplay) {
                        m_propertyDisplay->setPos(point);
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

    /*!
     * \brief Saves current component data to \a Caneda::XmlWriter.
     *
     * Saves current component data (name, library, position, properties
     * and transform) to \a Caneda::XmlWriter.
     *
     * \sa loadComponentData(), loadData()
     */
    void Component::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("component");
        writer->writeAttribute("name", name());
        writer->writeAttribute("library", library());

        writer->writePoint(pos(), "pos");
        if(m_propertyDisplay) {
            writer->writePoint(m_propertyDisplay->pos(), "propertyPos");
        }

        writer->writeTransform(transform());
        writeProperties(writer, d.constData()->propertyMap);

        writer->writeEndElement();  //</component>
    }

    /*!
     * \brief Paints a previously registered component.
     *
     * Takes care of painting a component on a scene. The component must be
     * previously registered using LibraryManager::registerComponent(). This
     * method also takes care of setting the correct global settings pen
     * according to its selection state.
     *
     * \sa LibraryManager::registerComponent()
     */
    void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *w)
    {
        // Paint the component symbol
        Settings *settings = Settings::instance();
        LibraryManager *libraryManager = LibraryManager::instance();
        QPainterPath symbol = libraryManager->symbolCache(name());

        // Save pen
        QPen savedPen = painter->pen();

        if(option->state & QStyle::State_Selected) {
            // If selected, the paint is performed without the pixmap cache
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            painter->drawPath(symbol);  // Draw symbol
        }
        else if(painter->worldTransform().isScaling()) {
            // If zooming, the paint is performed without the pixmap cache
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            painter->drawPath(symbol);  // Draw symbol
        }
        else {
            // Else, a pixmap cached is used
            QPixmap pix = libraryManager->pixmapCache(name());
            QRect rect =  symbol.boundingRect().toRect();
            rect.adjust(-1.0, -1.0, 1.0, 1.0);  // Adjust rect to avoid clipping when size = 1px in any dimension
            painter->drawPixmap(rect, pix);
        }

        // Restore pen
        painter->setPen(savedPen);

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
        retVal->updatePropertyDisplay();
        return retVal;
    }

    //! \brief Copies the data to \a component.
    void Component::copyDataTo(Component *component) const
    {
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(component));
        component->d = d;
        component->updatePropertyDisplay();
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
        if(change == ItemTransformHasChanged && m_propertyDisplay) {
            // Set the inverse of component's matrix to property group so that
            // it maintains identity when transformed.
            m_propertyDisplay->setTransform(transform().inverted());
        }
        return CGraphicsItem::itemChange(change, value);
    }

    //! \brief Updates the bounding rect of this item.
    void Component::updateBoundingRect()
    {
        // Get the bounding rect of the symbol
        QPainterPath symbol = LibraryManager::instance()->symbolCache(name());

        // Get an adjusted rect for accomodating extra stuff like ports.
        QRectF adjustedRect = adjustedBoundRect(symbol.boundingRect());

        // Set symbol bounding rect
        setShapeAndBoundRect(QPainterPath(), adjustedRect);
    }

} // namespace Caneda
