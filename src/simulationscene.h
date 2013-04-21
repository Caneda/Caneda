/***************************************************************************
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef SIMULATION_SCENE_H
#define SIMULATION_SCENE_H

#include <QWidget>

// Forward declarations
class QUndoStack;

class QwtPlotCurve;

namespace Caneda
{
    class SimulationScene : public QWidget
    {
        Q_OBJECT

    public:
        SimulationScene(QWidget *parent = 0);
        ~SimulationScene();

        //! \brief Returns a list of all items in the scene in descending stacking
        QList<QwtPlotCurve*> items() const { return m_items; }
        void addItem(QwtPlotCurve *item);

        //! Return current undo stack
        QUndoStack* undoStack() { return m_undoStack; }
        bool isModified() const { return m_modified; }

    public Q_SLOTS:
        void setModified(const bool m = true);

    Q_SIGNALS:
        void changed();

    private:
        QList<QwtPlotCurve*> m_items;  //! \brief Items available in the scene (curves, markers, etc)

        /*!
         * \brief Flag to hold whether a simulation is modified or not
         * i.e to determine whether a file should be saved or not on closing.
         *
         * \sa setModified
         */
        bool m_modified;

        //! Undo stack state
        QUndoStack *m_undoStack;
    };

} // namespace Caneda

#endif // SIMULATION_SCENE_H
