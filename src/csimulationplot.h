/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#ifndef C_SIMULATION_PLOT_H
#define C_SIMULATION_PLOT_H

#include <QString>

#include <qwt_plot_curve.h>

namespace Caneda
{
    /*!
     * \brief This class extends the QwtPlotCurve class, providing some
     * special properties needed for Caneda.
     *
     * \sa QwtPlotCurve
     */
    class CSimulationPlot : public QwtPlotCurve
    {
    public:
        CSimulationPlot(const QString &title = QString::null);
        CSimulationPlot(const QwtText &title);

        QString type() const { return m_type; }  //! \brief Returns the type of curve
        void setType(const QString type) { m_type = type; }    //! \brief Sets the type of curve

    private:
        QString m_type;  //! \brief Type of curve (voltage, current, etc)
    };

} // namespace Caneda

#endif // C_SIMULATION_PLOT_H
