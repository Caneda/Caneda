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

#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include "ui_transmissiondialog.h"

// Forward declarations
class transline;
class PropertyBox;
class ResultBox;

struct TransWidgets
{
  PropertyBox *subParams;
  PropertyBox *phyParams;
  PropertyBox *comParams;
  PropertyBox *eleParams;
  ResultBox *result;
  transline *line;
  TransWidgets();
  PropertyBox* boxWithProperty(const QString& name);
};

namespace Caneda
{
    class TransmissionDialog : public QDialog
    {
        Q_OBJECT

    public:
        TransmissionDialog(QWidget *parent = 0);

    private:
        void setupMicrostrip();
        void setupRectWaveGuide();
        void setupCoaxialLine();
        void setupCoupledMicrostrip();

    private slots:
        void slotTypeChanged();
        void slotAnalyze();
        void slotSynthesize();
        void slotCreateSchematic();

        void slotFileLoad();
        void slotFileSave();

    private:
        TransWidgets *transWidgets[4];

        Ui::TransmissionDialog ui;
    };

} // namespace Caneda

#endif //TRANSMISSION_H
