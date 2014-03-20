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

#ifndef SIMULATION_H
#define SIMULATION_H

#include "cgraphicsitem.h"

#include <QSharedData>

namespace Caneda
{
    //Forward declarations
    class CGraphicsScene;
    class XmlWriter;
    class XmlReader;

    /*!
     * \brief This struct holds data of a Simulation to be shared implicitly.
     *
     * This inherits QSharedData which takes care of reference counting.
     *
     * \sa Simulation
     */
    struct SimulationData : public QSharedData
    {
        SimulationData();
        SimulationData(const SimulationData& p);

        QString type;
        QString properties;
    };

    /*!
     * \brief This class represents a single simulation.
     *
     * Actual simulations inside the Simulation class, are implemented as
     * an implicitly shared class named SimulationData, thereby allowing
     * the use of the simulation objects directly instead of using pointers.
     *
     * While Simulation class holds actual simulations, SimulationGroup class
     * groups them all together and renders them on a scene, allowing
     * selection and moving of all a simulation group (or profile) at once.
     *
     * \sa SimulationData, SimulationGroup
     */
    class Simulation
    {
    public:
        Simulation(const QString &_type = QString(),
                   const QString &_properties = QString());
        Simulation(QSharedDataPointer<SimulationData> data);

        //! Returns the simulation type. Can be trans, ac, op, etc.
        QString type() const { return d->type; }
        //! Sets the type of simulation to \a newType. Can be trans, ac, op, etc.
        void setType(const QString &newType) { d->type = newType; }

        //! Returns the properties of the simulation.
        QString properties() const { return d->properties; }
        //! Sets the properties of the simulation to \a newValue.
        void setProperties(const QString &newValue) { d->properties = newValue; }

        //! Returns the (spice) model of the simulation.
        QString model() const { return type() + " " + properties(); }

        void saveSimulation(Caneda::XmlWriter *writer);
        static Simulation loadSimulation(Caneda::XmlReader *reader);

    private:
        //! Pointer enabling implicit sharing of data.
        QSharedDataPointer<SimulationData> d;
    };


    //! \def SimulationList This is a typedef to list simulations.
    typedef QList<Simulation> SimulationList;

    /*!
     * \brief Class used to group simulations all together and render
     * them on a scene.
     *
     * Gouping several simulations into a QList (m_simulationList) provides
     * a convenient way of handling them all together. In this way, for
     * example, a simulation group (or profile) can be selected and moved
     * all at once.
     *
     * While Simulation class holds actual simulations, SimulationGroup class
     * groups them all together and renders them on a scene, allowing
     * selection and moving of a simulation group at once.
     *
     * \sa SimulationData, Simulation
     */
    class SimulationGroup : public CGraphicsItem
    {
    public:
        SimulationGroup(CGraphicsScene* scene = 0, const SimulationList& simList = SimulationList());
        ~SimulationGroup();

        //! \copydoc CGraphicsItem::Type
        enum { Type = CGraphicsItem::SimulationType };
        //! \copydoc CGraphicsItem::type()
        int type() const { return Type; }

        void addSimulation(const Simulation& sim);

        //! Returns the simulation list (actually a copy of m_simulationList).
        SimulationList simulationList() const { return m_simulationList; }
        void setSimulationList(const SimulationList& simList);

        //! Returns if the simulation group is enabled for the next simulation.
        bool simulationGroupEnabled() const { return m_simulationGroupEnabled; }
        void setSimulationGroupEnabled(const bool enable);

        void updateSimulationDisplay();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget = 0 );

        static SimulationGroup* loadSimulationGroup(Caneda::XmlReader *reader, CGraphicsScene *scene);
        void saveData(Caneda::XmlWriter *writer) const;
        void loadData(Caneda::XmlReader *reader);

        SimulationGroup* copy(CGraphicsScene *scene = 0) const;
        void copyDataTo(SimulationGroup *simulationGroup) const;

        int launchPropertyDialog(Caneda::UndoOption opt);

    private:
        //! QList holding actual simulations.
        SimulationList m_simulationList;

        /*!
         * \brief Holds simulationGroup enable status.
         *
         * \sa simulationGroupEnabled(), setSimulationGroupEnabled()
         */
        bool m_simulationGroupEnabled;

        //! String which holds the text to display on the scene
        QGraphicsSimpleTextItem *m_text;
        //! QPainterPath which holds the symbol to display on the scene
        QPainterPath m_symbol;
    };

} // namespace Caneda

#endif //SIMULATION_H
