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

#ifndef CHART_ITEM_H
#define CHART_ITEM_H

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
    class ChartSeries : public QwtPlotCurve
    {
    public:
        explicit ChartSeries(const QString &title = QString::null);
        explicit ChartSeries(const QwtText &title);

        //! \brief Returns the type of curve
        QString type() const { return m_type; }
        //! \brief Sets the type of curve
        void setType(const QString& type) { m_type = type; }

    private:
        QString m_type;  //! \brief Type of curve (voltage, current, etc)
    };

} // namespace Caneda

#endif //CHART_ITEM_H
