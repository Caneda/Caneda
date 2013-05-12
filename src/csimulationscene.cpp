/***************************************************************************
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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

#include "csimulationscene.h"

#include <qwt_plot_curve.h>

namespace Caneda
{
    /*!
     * \brief Constructs a new simulation scene.
     *
     * \param parent Parent of the scene.
     */
    CSimulationScene::CSimulationScene(QWidget *parent) :
        QWidget(parent)
    {
    }

    /*!
     * \brief Adds or moves the item and all its childen to this scene. This
     * scene takes ownership of the item.
     */
    void CSimulationScene::addItem(QwtPlotCurve *item)
    {
        m_items.append(item);
    }

} // namespace Caneda
