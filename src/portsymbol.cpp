/***************************************************************************
 * Copyright (C) 2013-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "graphicsitem.h"
#include "portsymboldialog.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a port symbol item.
     *
     * This method initializes the main port parameters.
     *
     * \param parent Parent of the PortSymbol item.
     *
     * \sa updateGeometry()
     */
    PortSymbol::PortSymbol(QGraphicsItem *parent) : GraphicsItem(parent)
    {
        // Set component flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Initialize port label with the default name
        m_label = new QGraphicsSimpleTextItem("label", this);

        // Create ports
        m_ports << new Port(this);

        updateGeometry();
    }

    //! \brief Destructor.
    PortSymbol::~PortSymbol()
    {
        qDeleteAll(m_ports);
    }

    /*!
     * \brief Sets the label of the PortSymbol.
     *
     * \param newLabel The label to be set.
     * \return True on success and false on failure.
     */
    bool PortSymbol::setLabel(const QString &newLabel)
    {
        m_label->setText(newLabel);
        updateGeometry();

        return true;
    }

    //! \brief Updates the geometry of the port symbol.
    void PortSymbol::updateGeometry()
    {
        m_symbol = QPainterPath();

        // Define the port symbol acording to its label
        if(m_label->text().toLower() == "ground" ||
                m_label->text().toLower() == "gnd") {

            m_symbol.lineTo(0,10);
            m_symbol.moveTo(-10,10);
            m_symbol.lineTo(10,10);
            m_symbol.moveTo(-5,15);
            m_symbol.lineTo(5,15);
            m_symbol.moveTo(-2,20);
            m_symbol.lineTo(2,20);

            m_label->setVisible(false);
        }
        else {
            m_symbol.addRoundRect(-10, -10, 20, 20, 25, 25);

            m_label->setVisible(true);
        }

        QPointF labelPos = m_symbol.boundingRect().bottomLeft();
        m_label->setPos(labelPos);

        // Set the bounding rect to contain both the m_symbol shape and the
        // label. Use a rectangular shape (path) in setShapeAndBoundRect
        // to allow easy selection of the item. Otherwise, the ground symbol
        // would be very difficult to select (the selection would only work
        // when picking the lines).
        QRectF _boundRect = m_symbol.boundingRect() | m_label->boundingRect().translated(labelPos);
        QPainterPath _path = QPainterPath();
        _path.addRect(_boundRect);

        setShapeAndBoundRect(_path, _boundRect);
        update();
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

            // Set the label font settings
            m_label->setBrush(QBrush(settings->currentValue("gui/selectionColor").value<QColor>()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            // Set the label font settings
            m_label->setBrush(QBrush(settings->currentValue("gui/foregroundColor").value<QColor>()));
        }

        // Draw the port symbol if it is a termination point or ground
        if(m_label->text().toLower() == "ground" ||
                m_label->text().toLower() == "gnd" ||
                port()->connections()->size() <= 2) {
            painter->drawPath(m_symbol);
        }

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \copydoc GraphicsItem::copy()
    PortSymbol* PortSymbol::copy() const
    {
        PortSymbol *portSymbol = new PortSymbol(parentItem());
        portSymbol->setLabel(label());

        GraphicsItem::copyDataTo(portSymbol);
        return portSymbol;
    }

    //! \copydoc GraphicsItem::saveData()
    void PortSymbol::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("port");

        writer->writeAttribute("name", m_label->text());
        writer->writePointAttribute(pos(), "pos");

        writer->writeEndElement(); // < /port>
    }

    //! \copydoc GraphicsItem::loadData()
    void PortSymbol::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "port");

        setPos(reader->readPointAttribute("pos"));
        setLabel(reader->attributes().value("name").toString());

        // Read until end of element
        reader->readUnknownElement();
    }

    //! \copydoc GraphicsItem::launchPropertiesDialog()
    void PortSymbol::launchPropertiesDialog()
    {
        PortSymbolDialog dialog(this);
        dialog.exec();
    }

} // namespace Caneda
