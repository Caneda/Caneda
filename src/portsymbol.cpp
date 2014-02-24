/***************************************************************************
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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

#include "portsymbol.h"

#include "cgraphicsitem.h"
#include "propertygroup.h"
#include "settings.h"
#include "xmlutilities.h"

#include "dialogs/propertydialog.h"

#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a port symbol item.
     *
     * \param scene CGraphicsScene on which the port symbol should be added.
     */
    PortSymbol::PortSymbol(CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        // Set component flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        m_paintingRect = QRectF(-10, -10, 20, 20);

        // Create port properties and add port label
        Property _label("label", "Label", QObject::tr("Port Label"), true);
        properties = new PropertyGroup();
        properties->setParentItem(this);
        properties->addProperty("label", _label);
        properties->setTransform(transform().inverted());
        properties->setPos(m_paintingRect.bottomLeft());

        // Create ports
        m_ports << new Port(this, mapFromScene(pos()));

        updateGeometry();
    }

    //! \brief Destructor.
    PortSymbol::~PortSymbol()
    {
        qDeleteAll(m_ports);
    }

    //! \brief Draw port symbol
    void PortSymbol::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(option->state & QStyle::State_Selected) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        // Draw the port symbol
        painter->drawRoundRect(m_paintingRect);

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \brief Updates the geometry once a font is set or it is mirrored.
    void PortSymbol::updateGeometry()
    {
        QRectF boundRect = m_paintingRect;
        QPainterPath path;
        path.addRect(boundRect);

        setShapeAndBoundRect(path, boundRect);

        update();
    }

    //! \brief Returns a copy of port symbol item parented to scene \a scene.
    PortSymbol* PortSymbol::copy(CGraphicsScene *scene) const
    {
        //! \todo Check this
        PortSymbol *portSymbol = new PortSymbol(scene);
        portSymbol->properties->setPropertyMap(properties->propertyMap());
        PortSymbol::copyDataTo(portSymbol);
        return portSymbol;
    }

    void PortSymbol::copyDataTo(PortSymbol *portSymbol) const
    {
        //! \todo Check this
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(portSymbol));
        portSymbol->update();
    }

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int PortSymbol::launchPropertyDialog(Caneda::UndoOption)
    {
        return properties->launchPropertyDialog();
    }

    /*!
     * \brief Convenience static method to load a PortSymbol saved as xml.
     *
     * This method loads a PortSymbol saved as xml. Once the PortSymbol is
     * created, its data is filled using the loadData() method.
     *
     * \param reader The xmlreader used to read xml data.
     * \param scene CGraphicsScene to which PortSymbol should be parented to.
     * \return Returns new PortSymbol pointer on success and null on failure.
     *
     * \sa loadData()
     */
    PortSymbol* PortSymbol::loadPortSymbol(Caneda::XmlReader *reader, CGraphicsScene *scene)
    {
        PortSymbol *retVal = new PortSymbol(scene);
        retVal->loadData(reader);

        return retVal;
    }

    //! \brief Saves data to xml \a writer.
    void PortSymbol::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("port");

        writer->writeAttribute("name", properties->propertyValue("label"));
        writer->writePointAttribute(pos(), "pos");

        writer->writeEndElement(); // < /port>
    }

    //! \brief Loads portSymbol from xml \a reader.
    void PortSymbol::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "port");

        QString label = reader->attributes().value("name").toString();
        properties->setPropertyValue("label", label);
        setPos(reader->readPointAttribute("pos"));

        // Read until end of element
        reader->readUnknownElement();
    }

} // namespace Caneda
