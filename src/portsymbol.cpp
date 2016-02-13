/***************************************************************************
 * Copyright (C) 2013-2014 by Pablo Daniel Pareja Obregon                  *
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
#include "portsymboldialog.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a port symbol item.
     *
     * \param scene CGraphicsScene on which the port symbol should be added.
     *
     * \sa init()
     */
    PortSymbol::PortSymbol(CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        // Create a port with the default name
        init(QString("label"));
    }

    /*!
     * \brief Constructs a port symbol item.
     *
     * \param label Label of the port. Special ports as GND, should pass the
     *        corresponding label.
     * \param scene CGraphicsScene on which the port symbol should be added.
     *
     * \sa init()
     */
    PortSymbol::PortSymbol(const QString &label, CGraphicsScene *scene) : CGraphicsItem(0, scene)
    {
        init(label);
    }

    //! \brief Destructor.
    PortSymbol::~PortSymbol()
    {
        qDeleteAll(m_ports);
    }

    /*!
     * \brief Initialize the ports parameters
     *
     * This method initializes the main port parameters. It is done separately
     * from the contructors methods to allow reutilization of the code in
     * different existing contructors.
     *
     * \param label Label of the port. Special ports as GND, should pass the
     *        corresponding label.
     */
    void PortSymbol::init(const QString &label)
    {
        // Set component flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        // Initialize port label
        m_label = new QGraphicsSimpleTextItem(label, this);

        // Create ports
        m_ports << new Port(this, mapFromScene(pos()));

        updateGeometry();
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

    //! \brief Returns a copy of port symbol item parented to scene \a scene.
    PortSymbol* PortSymbol::copy(CGraphicsScene *scene) const
    {
        PortSymbol *portSymbol = new PortSymbol(scene);
        portSymbol->m_label->setText(m_label->text());
        PortSymbol::copyDataTo(portSymbol);
        return portSymbol;
    }

    void PortSymbol::copyDataTo(PortSymbol *portSymbol) const
    {
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(portSymbol));
        portSymbol->updateGeometry();
        portSymbol->update();
    }

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int PortSymbol::launchPropertyDialog(Caneda::UndoOption)
    {
        QString newLabel = QString(m_label->text());

        PortSymbolDialog *dia = new PortSymbolDialog(&newLabel);
        int status = dia->exec();
        delete dia;

        m_label->setText(newLabel);
        updateGeometry();

        return status;
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

        writer->writeAttribute("name", m_label->text());
        writer->writePointAttribute(pos(), "pos");

        writer->writeEndElement(); // < /port>
    }

    //! \brief Loads portSymbol from xml \a reader.
    void PortSymbol::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "port");

        m_label->setText(reader->attributes().value("name").toString());
        setPos(reader->readPointAttribute("pos"));
        updateGeometry();

        // Read until end of element
        reader->readUnknownElement();
    }

} // namespace Caneda
