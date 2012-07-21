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

#include "xmlutilities/transformers.h"
#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QPainter>

namespace Caneda
{
    /*
    ##########################################################################
    #                            HELPER METHODS                              #
    ##########################################################################
    */
    /*!
     * \brief This code draws the highlighted rect around the item
     * \note This code is stolen from source of qt ;-)
     */
    static void highlightSelectedSvgItem(
            Component *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
    {
        const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
        if(qFuzzyCompare(qMax(murect.width(), murect.height()), qreal(0.0))) {
            return;
        }

        const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
        if(qMin(mbrect.width(), mbrect.height()) < qreal(1.0)) {
            return;
        }

        const qreal pad = 0.5;
        const qreal strokeWidth = 0; // cosmetic pen

        const QColor fgcolor = option->palette.windowText().color();
        const QColor bgcolor( // ensure good contrast against fgcolor
                fgcolor.red()   > 127 ? 0 : 255,
                fgcolor.green() > 127 ? 0 : 255,
                fgcolor.blue()  > 127 ? 0 : 255);

        painter->setPen(QPen(bgcolor, strokeWidth, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

        painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
    }

    /*!
     * \brief Constructs and initializes a default empty component item.
     *
     * The item is an unregistered, initially unrenderable component. To
     * render the item it should be first connected to SvgPainter and the
     * svg id should already be registered with SvgPainter.
     * \sa SvgItem::registerConnections, SvgPainter::registerSvg()
     */
    Component::Component(CGraphicsScene *scene) :
        CGraphicsItem(0, scene), m_svgPainter(0),
        d(new ComponentData()), m_propertyGroup(0)
    {
        init();
    }

    /*!
     * \brief Constructs a component from \a other data.
     *
     * The item is an unregistered, initially unrenderable component. To
     * render the item it should be first connected to SvgPainter and the
     * svg id should already be registered with SvgPainter.
     * \sa SvgItem::registerConnections, SvgPainter::registerSvg()
     */
    Component::Component(const QSharedDataPointer<ComponentData>& other,
            SvgPainter *svgPainter_, CGraphicsScene *scene) :
        CGraphicsItem(0, scene), m_svgPainter(svgPainter_),
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

    //! \brief Intialize the component.
    void Component::init()
    {
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        Property _label("label", tr("Label"), QVariant::String, true,
                false, labelPrefix().append('1'));
        d->propertyMap.insert("label", _label);
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
        // determine if any item is visible.
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
        //delete the old group if it exists.
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
        if(propName == "symbol") {
            return setSymbol(value.toString());
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
     * \brief Sets the symbol of component to newSymbol if it exists.
     *
     * This method sets the symbol property's value and then takes care
     * of geometry changes as well.
     *
     * \param newSymbol The symbol to be set now
     * \return True on success, and false on failure.
     * \todo Take care of undo\redo as well as connection changes.
     */
    bool Component::setSymbol(const QString& newSymbol)
    {
        QString svgid = newSymbol;
        QString prefix(name());
        prefix.append('/');

        Q_ASSERT_X(propertyMap().contains("symbol"),
                __FUNCTION__,"Component::setSymbol() : 'symbol' property not found");

        if(!d->propertyMap["symbol"].setValue(svgid)) {
            return false;
        }

        qDeleteAll(m_ports);
        m_ports.clear();

        const QList<PortData*> portDatas = d.constData()->schematicPortMap[svgid];
        foreach(const PortData *data, portDatas) {
            m_ports << new Port(this, data->pos, data->name);
        }
        svgid.prepend(prefix);

        registerConnections(svgid, m_svgPainter);

        updatePropertyGroup();

        return true;
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

        //TODO: Yet to implement label prefix and number suffixing.
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
        setSymbol(propMap["symbol"].value().toString());
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

        retVal = LibraryLoader::instance()->newComponent(compName, scene, libName);
        if(retVal) {
            retVal->loadData(reader);
        }
        else {
            //read upto end if component is not found in any of Caneda identified libraries.
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
                    //note the usage as it expects reference of property map.
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

        writer->writeEndElement();
    }

    /*!
     * \brief Paints the component, peforms sanity checks and takes
     * care of drawing the selection rectangle.
     */
    void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *w)
    {
        // Paint the component symbol
        if(!isRegistered()) {
            qWarning() << "Component::paint() : called when unregistered";
            return;
        }

        m_svgPainter->paint(painter, m_svgId);

        if(option->state & QStyle::State_Selected) {
            highlightSelectedSvgItem(this, painter, option);
        }

        // Paint the ports
        foreach(Port *port, m_ports) {
            port->paint(painter, option);
        }
    }

    //! \brief Returns a copy of this component.
    Component* Component::copy(CGraphicsScene *scene) const
    {
        Component *retVal = new Component(d, m_svgPainter, scene);
        //no need for Component::copyDataTo() because the data is already copied from d pointer.
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(retVal));
        retVal->setSymbol(symbol()); //to register svg connections
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

    /*!
     * \brief Registers connections of this item with SvgPainter \a painter.
     *
     * Unless this item is connected this way, it won't be rendered. The svg
     * corresponding to svg id \a svg_id should already be registered with
     * SvgPainter \a painter using SvgPainter::registerSvg. This method also
     * unregisters any previously connected SvgPainter.
     *
     * \sa SvgPainter::registerSvg()
     */
    void Component::registerConnections(const QString& svg_id, SvgPainter *painter)
    {
        Q_ASSERT(!svg_id.isEmpty());
        Q_ASSERT(painter);

        if(!painter->isSvgRegistered(svg_id)) {
            qWarning() << "Component::registerConnections()  :  "
                       << "Cannot register for ungregisted svgs. Register svg first";
            return;
        }

        // Disconnect if this was connected to a different SvgPainter before.
        if(isRegistered()) {
            disconnect(m_svgPainter->rendererFor(m_svgId), SIGNAL(repaintNeeded()),
                       this, SLOT(updateBoundingRect()));
        }

        m_svgId = svg_id;
        m_svgPainter = painter;

        connect(painter->rendererFor(svg_id), SIGNAL(repaintNeeded()), this,
                SLOT(updateBoundingRect()));
        updateBoundingRect();
    }

    //! \brief Returns whether item is registered to an svg or not.
    bool Component::isRegistered() const
    {
        return m_svgPainter && !m_svgId.isEmpty() && m_svgPainter->isSvgRegistered(m_svgId);
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
            //set the inverse of component's matrix to property group so that
            //it maintains identity when transformed.
            m_propertyGroup->setTransform(transform().inverted());
        }
        return CGraphicsItem::itemChange(change, value);
    }

    /*!
     * \brief Updates the bounding rect of this item.
     *
     * This public slot is connected to the QSvgRenderer::repaintNeeded()
     * signal.
     */
    void Component::updateBoundingRect()
    {
        if(!isRegistered()) {
            qWarning() << "SvgItem::updateBoundingRect()  : Can't update"
                       << "unregistered items";
            return;
        }

        QRectF bound = m_svgPainter->boundingRect(m_svgId);
        if(bound.isNull()) {
            qWarning() << "SvgItem::calcBoundingRect() : Data parse error";
        }

        // Now get an adjusted rect for accomodating extra stuff like ports.
        QRectF adjustedRect = adjustedBoundRect(bound);

        setShapeAndBoundRect(QPainterPath(), adjustedRect, 1.0);
    }

    /*!
     * \brief Read an svg schematic
     *
     * \param svgContent svg content as utf8
     * \param schName Schematic name
     * \param svgPainter The SvgPainter object to which the symbols should be exported to.
     * \param d (Output variable) The data ptr where data should be uploaded.
     *  \todo Check error
     */
    static bool readSchematicSvg(const QByteArray &svgContent,
            const QString &schName,
            SvgPainter *svgPainter,
            QSharedDataPointer<ComponentData> &d)
    {
        /* process using xslt */
        Caneda::QXmlStreamReaderExt QXmlSvg(svgContent, 0,
                Caneda::transformers::defaultInstance()->componentsvg());

        QString svgId = d.constData()->name + "/" + schName;
        svgPainter->registerSvg(svgId, QXmlSvg.constData());
        if(QXmlSvg.hasError()) {
            qWarning() << "Could not read svg file" << schName << ": " << QXmlSvg.errorString();
            return false;
        }

        return true;;
    }


    /*!
     * \brief Reads the schematic port tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param schName Schematic name
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    static void readSchematicPort(Caneda::XmlReader *reader, const QString & schName,
            QSharedDataPointer<ComponentData> &d)
    {
        Q_ASSERT(reader->isStartElement());
        QXmlStreamAttributes attribs = reader->attributes();
        bool ok;

        qreal x = attribs.value("x").toString().toDouble(&ok);
        Q_ASSERT(ok);

        qreal y = attribs.value("y").toString().toDouble(&ok);
        Q_ASSERT(ok);

        QString portName = attribs.value("name").toString();
        d->schematicPortMap[schName] <<
            new PortData(QPointF(x, y), portName);

        // Read until end element as all data we require is already obtained.
        reader->readUnknownElement();
    }

    /*!
     * \brief Reads the schematic tag of component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param path The path of the xml file being processed.
     * \param svgPainter The SvgPainter object to which the symbols should be exported to.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    static bool readSchematic(Caneda::XmlReader *reader, const QString& svgPath, SvgPainter *svgPainter,
            QSharedDataPointer<ComponentData> &d)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "schematic");

        QString schName = reader->attributes().value("name").toString();
        QString schType = reader->attributes().value("href").toString();
        bool readok;

        /* if external svg file */
        if(!schType.isEmpty()) {
            QFile svgFile(svgPath + "/" + schType);
            if(!svgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return false;
            }

            QByteArray svgContent(svgFile.readAll());
            if(svgContent.isEmpty()) {
                return false;
            }

            readok = readSchematicSvg(svgContent, schName, svgPainter,d);
            if(!readok) {
                return false;
            }
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                /* internal svg */
                if(reader->name() == "svg") {
                    Q_ASSERT(schType.isEmpty());
                    QByteArray svgContent = reader->readXmlFragment().toUtf8();
                    // todo return error
                    readok = readSchematicSvg(svgContent, schName, svgPainter, d);
                    if(!readok) {
                        return false;
                    }
                }
                else if(reader->name() == "port") {
                    readSchematicPort(reader, schName, d);
                }
                else {
                    Q_ASSERT(!sizeof("unknow element in schematic element"));
                }
            }
        }
        return true;
    }

    /*!
     * \brief Read schematics section of component description xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param path The path of the xml file being processed.
     * \param svgPainter The SvgPainter object to which the symbols should be exported to.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    static bool readSchematics(Caneda::XmlReader *reader, const QString& svgPath, SvgPainter *svgPainter,
            QSharedDataPointer<ComponentData> &d)
    {
        /* list of symbols */
        QStringList parsedSymbols;

        /* get default value */
        QString defaultSchematic =
            reader->attributes().value("default").toString();

        Q_ASSERT(reader->isStartElement() && reader->name() == "schematics");
        Q_ASSERT(!defaultSchematic.isEmpty());

        /* read different schematic */
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "schematic") {
                    QString schName =
                        reader->attributes().value("name").toString();

                    Q_ASSERT(!schName.isEmpty());

                    parsedSymbols << schName;
                    if(!readSchematic(reader, svgPath, svgPainter, d)) {
                        return false;
                    }
                }
                else {
                    Q_ASSERT(!sizeof("unknow element in schematics element"));
                }
            }
        }

        /* check if default is present */
        Q_ASSERT(parsedSymbols.contains(defaultSchematic));

        /* add symbols to property list */
        QString symbolDescription =
            QObject::tr("Represents the current symbol of component.");
        QVariant defValue(defaultSchematic);
        Q_ASSERT(defValue.convert(QVariant::String));
        Property symb("symbol", symbolDescription, QVariant::String, false,
                false, defValue, parsedSymbols);
        d->propertyMap.insert("symbol", symb);

        return true;
    }

    /*!
     * \brief Reads component properties data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param d (Output variable) The data ptr where data should be uploaded.
     */
    static void readComponentProperties(Caneda::XmlReader *reader,
            QSharedDataPointer<ComponentData> &d)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            else if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    Property prop = PropertyFactory::createProperty(reader);
                    d->propertyMap.insert(prop.name(), prop);
                }
                /* default */
                else {
                    Q_ASSERT(!sizeof("unknow element in properties element"));
                }
            }
        }
    }

    /*!
     * \brief Reads component data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     * \param path The path of the xml file being processed.
     * \param svgPainter The SvgPainter object to which the symbols should be exported to.
     * \param d (Output variable) The data ptr where data should be uploaded.
     * \note Policy is to assert well formed xml.
     */
    bool readComponentData(Caneda::XmlReader *reader, const QString& path,
            SvgPainter *svgPainter, QSharedDataPointer<ComponentData> &d)
    {
        QXmlStreamAttributes attributes = reader->attributes();

        Q_ASSERT(reader->isStartElement() && reader->name() == "component");

        //check version compatibility first.
        Q_ASSERT(Caneda::checkVersion(attributes.value("version").toString()));

        /* get name */
        d->name = attributes.value("name").toString();
        Q_ASSERT(!d->name.isEmpty());

        /* get label */
        d->labelPrefix = attributes.value("label").toString();
        Q_ASSERT(!d->labelPrefix.isEmpty());

        /* read the component body */
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                /* read display text */
                if(reader->name() == "displaytext") {
                    d->displayText = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement());
                }

                /* Read description */
                else if(reader->name() == "description") {
                    d->description = reader->readLocaleText(Caneda::localePrefix());
                    Q_ASSERT(reader->isEndElement());
                }

                /* Read schematic */
                else if(reader->name() == "schematics") {
                    if(readSchematics(reader, path, svgPainter, d)==false) {
                        d = static_cast<ComponentData*>(0);
                        return false;
                    }
                }

                /* Read ports*/
                else if(reader->name() == "ports") {
                    while(!reader->atEnd()) {
                        reader->readNext();

                        if(reader->isEndElement()) {
                            Q_ASSERT(reader->name() == "ports");
                            break;
                        }

                        if(reader->isStartElement() && reader->name() == "port") {
                            //TODO: Read ports information
                            reader->readFurther();
                            Q_ASSERT(reader->isEndElement() && reader->name() == "port");
                        }
                    }
                }

                /* Read properties */
                else if(reader->name() == "properties") {
                    readComponentProperties(reader,d);
                }

                else {
                    reader->readUnknownElement();
                }
            }
        }

        if(reader->hasError()) {
            d = static_cast<ComponentData*>(0);
            return false;
        }
        return true;
    }

} // namespace Caneda
