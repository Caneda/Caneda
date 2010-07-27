/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#ifndef ATTENUATOR_H
#define ATTENUATOR_H

#include "ui_attenuator.h"

namespace Caneda
{
    class Attenuator : public QDialog
    {
        Q_OBJECT

    public:
        Attenuator(QWidget *parent = 0);
        ~Attenuator();

    private slots:
        void slotCalculate();
        void slotCreateSchematic();
        void slotTopologyChanged();

    private:
        Ui::Attenuator ui;
    };

} // namespace Caneda

#endif //ATTENUATOR_H
