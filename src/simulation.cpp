/***************************************************************************
 * Copyright (C) 2014 by Pablo Daniel Pareja Obregon                       *
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

#include "simulation.h"

#include "cgraphicsscene.h"
#include "global.h"
#include "settings.h"
#include "xmlutilities.h"

#include "dialogs/simulationdialog.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*************************************************************************
     *                          SimulationData                               *
     *************************************************************************/
    //! \brief Constructor.
    SimulationData::SimulationData()
    {
        type = QString();
        properties = QString();
    }

    //! \brief Copy constructor
    SimulationData::SimulationData(const SimulationData& p) : QSharedData(p)
    {
        type = p.type;
        properties = p.properties;
    }


    /*************************************************************************
     *                            Simulation                                 *
     *************************************************************************/
    /*!
     * \brief Constructs a simulation object
     *
     * \param _name Name of simulation object.
     * \param _value The default value of simulation.
     */
    Simulation::Simulation(const QString& _type,
                       const QString& _properties)
    {
        d = new SimulationData;
        d->type = _type;
        d->properties = _properties;
    }

    //! \brief Construct simulation from shared data.
    Simulation::Simulation(QSharedDataPointer<SimulationData> data) : d(data)
    {
    }

    /*!
     * \brief Method used to save a simulation to an xml file.
     *
     * \param reader XmlReader which is used for writing.
     */
    void Simulation::saveSimulation(Caneda::XmlWriter *writer)
    {
        writer->writeStartElement("simulation");

        writer->writeAttribute("type", type());
        writer->writeAttribute("properties", properties());

        writer->writeEndElement(); // </simulation>
    }

    /*!
     * \brief Method used to create a simulation from an xml file.
     *
     * \param reader XmlReader which is in use for parsing.
     */
    Simulation Simulation::loadSimulation(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "simulation");

        QSharedDataPointer<SimulationData> data(new SimulationData);
        QXmlStreamAttributes attributes = reader->attributes();

        data->type = attributes.value("type").toString();
        data->properties = attributes.value("properties").toString();

        return Simulation(data);
    }


    /*************************************************************************
     *                          SimulationGroup                              *
     *************************************************************************/
    /*!
     * \brief Constructs a SimulationGroup from a given scene and SimulationMap.
     *
     * \param scene The graphics scene to which this simulation should belong.
     * \param simMap The SimulationMap to use on initialization.
     */
    SimulationGroup::SimulationGroup(CGraphicsScene *scene, const SimulationMap &simMap) : CGraphicsItem(0, scene)
    {
        // Set items flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);

        m_simulationMap = simMap;
        m_simulationGroupEnabled = true;
        m_text = new QGraphicsSimpleTextItem("", this);
    }

    //! \brief Destructor.
    SimulationGroup::~SimulationGroup()
    {
        qDeleteAll(m_ports);
    }

    //! \brief Adds a new simulation to the SimulationMap.
    void SimulationGroup::addSimulation(const Simulation &sim)
    {
        m_simulationMap.insert(sim.type(), sim);
        updateSimulationDisplay();  // This is necessary to update the simulations display on a scene
    }

    /*!
     * \brief Set all the simulations values through a SimulationMap.
     *
     * This method sets the simulations values by updating the simulationMap
     * to \a simMap. After setting the simulationMap, this method also takes
     * care of updating the simulations display on the scene.
     *
     * \param simMap The new simulation map to be set.
     *
     * \sa Simulation, SimulationMap, updateSimulationDisplay()
     */
    void SimulationGroup::setSimulationMap(const SimulationMap& simMap)
    {
        m_simulationMap = simMap;
        updateSimulationDisplay();  // This is necessary to update the simulations display on a scene
    }

    /*!
     * \brief Sets the simulationGroupEnabled status.
     *
     * This method sets if the simulationGroup is enabled to be executed in the
     * next simulation run. This way, the user can have multiple simulation
     * groups simultaneusly and decide what groups are to be simulated.
     *
     * \sa m_simulationGroupEnabled, \sa simulationGroupEnabled()
     */
    void SimulationGroup::setSimulationGroupEnabled(const bool enable)
    {
        m_simulationGroupEnabled = enable;
    }

    /*!
     * \brief Updates the visual display of all the simulations in the
     * SimulationGroup.
     *
     * This method is key to alter the visual display text of given
     * simulations. It should be called wherever a simulation changes.
     *
     * To update the visual display, it recreates all individual simulations
     * display from the group and then adds them to the plaintext property of
     * this item.
     */
    void SimulationGroup::updateSimulationDisplay()
    {
        // Define the simulation symbol
        m_symbol = QPainterPath();
        m_symbol.addRoundRect(0, 0, 40, 40, 25);
        m_symbol.addRoundRect(5, 5, 30, 20, 25);
        m_symbol.moveTo(10,15);
        m_symbol.lineTo(30,15);
        m_symbol.moveTo(10,15);
        m_symbol.arcTo(10,10,10,10,180,-180);
        m_symbol.arcTo(20,10,10,10,180,180);

        // Define the simulation text
        QString newValue = "Simulation Profile";  // New value to set

        // Iterate through all simulations to add its values
        foreach(const Simulation simulation, m_simulationMap) {

            // Current simulation text
            QString simulationText = "";

            // Add simulation name
            simulationText = simulation.type() + " = ";

            // Add simulation value
            simulationText.append(simulation.model());

            // Add the simulation to the group
            if(!newValue.isEmpty()) {
                newValue.append("\n");  // If already has simulations, add newline
            }
            newValue.append(simulationText);
        }

        // Set new simulations values
        m_text->setText(newValue);

        // Set the text position
        QPointF labelPos = m_symbol.boundingRect().topRight() + QPointF(5,0);
        m_text->setPos(labelPos);

        // Set the bounding rect to contain both the m_symbol shape and the
        // simulation text. Use a rectangular shape (path) in
        // setShapeAndBoundRect to allow easy selection of the item.
        QRectF _boundRect = m_symbol.boundingRect() | m_text->boundingRect().translated(labelPos);
        QPainterPath _path = QPainterPath();
        _path.addRect(_boundRect);

        setShapeAndBoundRect(_path, _boundRect);
        update();
    }

    /*!
     * \brief Draws the SimulationGroup to painter.
     *
     * This method draws the SimulationGroup contents on a scene. The pen color
     * changes according to the selection state, thus giving state feedback to
     * the user.
     *
     * The selection rectangle around all SimulationGroup contents is handled
     * by this method. Currently, no selection rectangle around simulation
     * items is drawn, although it could change in the future (acording to
     * user's feedback). In that case, this class bounding rect should be used.
     * The selection state is instead handled by changing the simulations' pen
     * color according to the global selection pen.
     */
    void SimulationGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
    {
        // Save pen
        QPen savedPen = painter->pen();

        // Set global pen settings
        Settings *settings = Settings::instance();
        if(isSelected()) {
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            m_text->setBrush(QBrush(settings->currentValue("gui/selectionColor").value<QColor>()));
        }
        else if(simulationGroupEnabled()) {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            m_text->setBrush(QBrush(settings->currentValue("gui/lineColor").value<QColor>()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/foregroundColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));

            m_text->setBrush(QBrush(settings->currentValue("gui/foregroundColor").value<QColor>()));
        }

        // Draw the simulation symbol
        painter->drawPath(m_symbol);

        // Restore pen
        painter->setPen(savedPen);
    }

    /*!
     * \brief Convenience static method to load a SimulationGroup saved as xml.
     *
     * This method loads a SimulationGroup saved as xml. Once the
     * SimulationGroup is created, its data is filled using the loadData()
     * method.
     *
     * \param reader The xmlreader used to read xml data.
     * \param scene CGraphicsScene to which SimulationGroup should be parented to.
     * \return Returns new SimulationGroup pointer on success and null on failure.
     *
     * \sa loadData()
     */
    SimulationGroup* SimulationGroup::loadSimulationGroup(Caneda::XmlReader *reader, CGraphicsScene *scene)
    {
        SimulationGroup *retVal = new SimulationGroup(scene);
        retVal->loadData(reader);

        return retVal;
    }

    //! \brief Helper method to write all simulations in \a m_simulationMap to xml.
    void SimulationGroup::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("simulationsGroup");
        writer->writePointAttribute(pos(), "pos");

        foreach(Simulation p, m_simulationMap) {
            p.saveSimulation(writer);
        }

        writer->writeEndElement(); // </simulationsGroup>
    }

    //! \brief Helper method to read xml saved simulations into \a m_simulationMap.
    void SimulationGroup::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "simulationsGroup");
        setPos(reader->readPointAttribute("pos"));

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "simulation") {

                    // Read an individual simulation
                    Simulation sim = Simulation::loadSimulation(reader);
                    addSimulation(sim);

                    // Read till end element
                    reader->readUnknownElement();
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }

        updateSimulationDisplay();
    }

    //! \brief Returns a copy of a simulationGroup item parented to scene \a scene.
    SimulationGroup* SimulationGroup::copy(CGraphicsScene *scene) const
    {
        SimulationGroup *simulationGroup = new SimulationGroup(scene);
        simulationGroup->setSimulationMap(simulationMap());
        simulationGroup->setSimulationGroupEnabled(m_simulationGroupEnabled);
        SimulationGroup::copyDataTo(simulationGroup);
        return simulationGroup;
    }

    void SimulationGroup::copyDataTo(SimulationGroup *simulationGroup) const
    {
        CGraphicsItem::copyDataTo(static_cast<CGraphicsItem*>(simulationGroup));
        simulationGroup->updateSimulationDisplay();
        simulationGroup->update();
    }

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int SimulationGroup::launchPropertyDialog(Caneda::UndoOption opt)
    {
        SimulationDialog *dia = new SimulationDialog();
        int status = dia->exec();
        delete dia;

        return status;
    }

} // namespace Caneda
