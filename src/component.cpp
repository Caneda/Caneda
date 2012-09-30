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
#include "settings.h"
#include "xmlutilities.h"

#include "dialogs/propertydialog.h"

#include <QDebug>
#include <QFile>
#include <QPainter>

namespace Caneda
{
    //! \brief Constructs default empty ComponentData.
    ComponentData::ComponentData(CGraphicsScene *scene)
    {
        if(scene) {
            properties = new PropertyGroup(scene);
        }
        else {
            properties = new PropertyGroup();
        }
    }

    /*!
     * \brief Constructs ComponentData from ComponentDataPtr.
     *
     * Contructs a new ComponentData object from ComponentDataPtr. Special
     * care is taken to avoid copying the properties pointer, and copying
     * properties content (PropertyMap) instead. Otherwise, one would be
     * copying the reference to the PropertyGroup (properties) and all
     * components would share only one reference, modifying only one set
     * of properties data.
     */
    ComponentData::ComponentData(const QSharedDataPointer<ComponentData> &other, CGraphicsScene *scene)
    {
        // Copy all data from given ComponentDataPtr
        name = other->name;
        filename = other->filename;
        displayText = other->displayText;
        labelPrefix = other->labelPrefix;
        description = other->description;
        library = other->library;
        ports = other->ports;

        // Recreate PropertyGroup (properties) as it is a pointer
        // and only internal data must be copied.
        if(scene) {
            properties = new PropertyGroup(scene);
        }
        else {
            properties = new PropertyGroup();
        }

        properties->setPropertyMap(other->properties->propertyMap());
    }

    //! \brief Constructs and initializes a default empty component item.
    Component::Component(CGraphicsScene *scene) :
        CGraphicsItem(0, scene)
    {
        d = new ComponentData(cGraphicsScene());
        init();
    }

    //! \brief Constructs a component from \a other data.
    Component::Component(const QSharedDataPointer<ComponentData>& other, CGraphicsScene *scene) :
        CGraphicsItem(0, scene)
    {
        d = new ComponentData(other, cGraphicsScene());
        init();
    }

    //! \brief Destructor.
    Component::~Component()
    {
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
        // Set component flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Add component label
        Property _label("label", labelPrefix().append('1'), tr("Label"), true);
        d->properties->addProperty("label", _label);

        // Add component ports
        const QList<PortData*> portDatas = d.constData()->ports;
        foreach(const PortData *data, portDatas) {
            m_ports << new Port(this, data->pos, data->name);
        }

        // Update component geometry
        updateBoundingRect();

        // Update properties text position
        d->properties->setParentItem(this);
        d->properties->setTransform(transform().inverted());
        d->properties->setPos(boundingRect().bottomLeft());
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
        if(!newLabel.startsWith(labelPrefix())) {
            return false;
        }

        d->properties->setPropertyValue("label", newLabel);
        return true;
    }

    //! \brief Returns the label's suffix part.
    QString Component::labelSuffix() const
    {
        QString _label = label();
        return _label.mid(labelPrefix().length());
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
                else if(reader->name() == "transform") {
                    setTransform(reader->readTransform());
                }
                else if(reader->name() == "properties") {
                    d->properties->readProperties(reader);
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
        writer->writeTransform(transform());
        d->properties->writeProperties(writer);

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
        return retVal;
    }

    //! \brief Copies the data to \a component.
    void Component::copyDataTo(Component *component) const
    {
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(component));
        component->d = new ComponentData(d, cGraphicsScene());
        component->update();
    }

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int Component::launchPropertyDialog(Caneda::UndoOption)
    {
        PropertyDialog *dia = new PropertyDialog(d->properties);
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
        if(change == ItemTransformHasChanged) {
            // Set the inverse of component's matrix to the PropertyGroup
            // (properties) so that it maintains identity when transformed.
            d->properties->setTransform(transform().inverted());
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
