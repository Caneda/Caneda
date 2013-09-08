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
#include "settings.h"
#include "xmlutilities.h"

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

        m_name = "Port";
        m_paintingRect = QRectF(-10, -10, 20, 20);

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
    void PortSymbol::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
    {
        port()->paint(painter, option);

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
        //! \todo Display the port name

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \brief Set name of the port.
    void PortSymbol::setName(QString str)
    {
        prepareGeometryChange();
        m_name = str;
        updateGeometry();
    }

    //! \brief Updates the geometry once a font is set or it is mirrored.
    void PortSymbol::updateGeometry()
    {
        //! \todo Make space for the port name to be displayed
//        QFontMetrics fm(font());
//        qreal height = qMax(portEllipse.bottom(), qreal(fm.height() + fm.descent()));

//        QPointF topLeft(0, -height/2.);
//        QPointF bottomRight(0, height/2.);

//        topLeft.rx() = portEllipse.left();
//        bottomRight.rx() = portEllipse.right() + portSymbolOffset + fm.width(text());

//        QRectF rect(topLeft, bottomRight);
//        setPaintingRect(rect);

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
        portSymbol->updateGeometry();
        PortSymbol::copyDataTo(portSymbol);
        return portSymbol;
    }

    void PortSymbol::copyDataTo(PortSymbol *portSymbol) const
    {
        //! \todo Check this
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(portSymbol));
        portSymbol->update();
    }

    //! \brief Saves data to xml \a writer.
    void PortSymbol::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("port");

        writer->writeAttribute("name", name());
        writer->writePointAttribute(pos(), "pos");

        writer->writeEndElement(); // < /port>
    }

    //! \brief Loads portSymbol from xml \a reader.
    void PortSymbol::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "port");

        setName(reader->attributes().value("name").toString());
        setPos(reader->readPointAttribute("pos"));
    }

} // namespace Caneda
