/***************************************************************************
 * Copyright (C) 2013-2015 by Pablo Daniel Pareja Obregon                  *
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

#include <QWidget>

// Forward declarations
class QwtPlotCurve;

namespace Caneda
{
    class CSimulationScene : public QWidget
    {
        Q_OBJECT

    public:
        CSimulationScene(QWidget *parent = 0);

        //! \brief Returns a list of all items in the scene in descending stacking
        QList<QwtPlotCurve*> items() const { return m_items; }
        void addItem(QwtPlotCurve *item);

    private:
        QList<QwtPlotCurve*> m_items;  //! \brief Items available in the scene (curves, markers, etc)
    };

} // namespace Caneda

#endif // C_SIMULATION_SCENE_H
