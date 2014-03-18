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
        name = QString();
        value = QString();
    }

    //! \brief Copy constructor
    SimulationData::SimulationData(const SimulationData& p) : QSharedData(p)
    {
        name = p.name;
        value = p.value;
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
    Simulation::Simulation(const QString& _name,
                       const QString& _value)
    {
        d = new SimulationData;
        d->name = _name;
        d->value = _value;
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

        writer->writeAttribute("name", name());
        writer->writeAttribute("value", value());

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

        data->name = attributes.value("name").toString();
        data->value = attributes.value("value").toString();

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
    SimulationGroup::SimulationGroup(CGraphicsScene *scene, const SimulationMap &simMap)
    {
        m_simulationMap = simMap;
        m_simulationGroupEnabled = true;

        if(scene) {
            scene->addItem(this);
        }

        // Set items flags
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);
    }

    //! \brief Adds a new simulation to the SimulationMap.
    void SimulationGroup::addSimulation(const Simulation &sim)
    {
        m_simulationMap.insert(sim.name(), sim);
        updateSimulationDisplay();  // This is necessary to update the simulations display on a scene
    }

    //! \brief Sets simulation \a key to \a value in the SimulationMap.
    void SimulationGroup::setSimulationValue(const QString& key, const QString& value)
    {
        if(m_simulationMap.contains(key)) {
            m_simulationMap[key].setValue(value);
            updateSimulationDisplay();  // This is necessary to update the simulations display on a scene
        }
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
        QString newValue;  // New value to set

        // Iterate through all simulations to add its values
        foreach(const Simulation simulation, m_simulationMap) {

            // Current simulation text
            QString simulationText = "";

            // Add simulation name
            simulationText = simulation.name() + " = ";

            // Add simulation value
            simulationText.append(simulation.value());

            // Add the simulation to the group
            if(!newValue.isEmpty()) {
                newValue.append("\n");  // If already has simulations, add newline
            }
            newValue.append(simulationText);
        }

        // Set new simulations values
        setText(newValue);

        // Make item visible
        show();
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
        }
        else if(simulationGroupEnabled()) {
            painter->setPen(QPen(settings->currentValue("gui/lineColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }
        else {
            painter->setPen(QPen(settings->currentValue("gui/foregroundColor").value<QColor>(),
                                 settings->currentValue("gui/lineWidth").toInt()));
        }

        // Paint the simulation text
        painter->drawText(boundingRect(), text());

        // Restore pen
        painter->setPen(savedPen);
    }

    //! \brief Helper method to write all simulations in \a m_simulationMap to xml.
    void SimulationGroup::saveSimulationGroup(Caneda::XmlWriter *writer)
    {
        writer->writeStartElement("simulations");
        writer->writePointAttribute(pos(), "pos");

        foreach(Simulation p, m_simulationMap) {
            p.saveSimulation(writer);
        }

        writer->writeEndElement(); // </simulations>
    }

    //! \brief Helper method to read xml saved simulations into \a m_simulationMap.
    void SimulationGroup::loadSimulationGroup(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "simulations");
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

    //! \copydoc CGraphicsItem::launchPropertyDialog()
    int SimulationGroup::launchSimulationDialog()
    {
        SimulationDialog *dia = new SimulationDialog();
        int status = dia->exec();
        delete dia;

        return status;
    }

    //! \brief On mouse click deselect selected items other than this.
    void SimulationGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(scene()) {
            foreach(QGraphicsItem *item, scene()->selectedItems()) {
                if(item != this) {
                    item->setSelected(false);
                }
            }
        }

        QGraphicsSimpleTextItem::mousePressEvent(event);
    }

    //! \brief Launches simulation dialog on double click.
    void SimulationGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        launchSimulationDialog();
    }

} // namespace Caneda
