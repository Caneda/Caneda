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

#ifndef C_SIMULATION_SCENE_H
#define C_SIMULATION_SCENE_H

#include <csimulationitem.h>

#include <QWidget>

namespace Caneda
{
    /*!
     * \brief This class implements the scene class of Qt's Graphics View
     * Architecture, representing the actual document interface (scene),
     * containing the simulation waveform data.
     *
     * Each scene must have at least one associated view (CSimulationView), to
     * display the contents of the scene (waveforms). Several views can be
     * attached to the same scene, providing different viewports into the same
     * data set (for example, when using split views).
     *
     * \sa CSimulationView
     */
    class CSimulationScene : public QWidget
    {
        Q_OBJECT

    public:
        CSimulationScene(QWidget *parent = 0);

        //! \brief Returns a list of all items in the scene in descending stacking
        QList<CSimulationPlotCurve*> items() const { return m_items; }
        void addItem(CSimulationPlotCurve *item);

    private:
        QList<CSimulationPlotCurve*> m_items;  //! \brief Items available in the scene (curves, markers, etc)
    };

} // namespace Caneda

#endif // C_SIMULATION_SCENE_H
