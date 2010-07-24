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

#include "attenuator.h"

#include "caneda-tools/global.h"

#include <QString>
#include <QClipboard>

#include <math.h>

namespace Caneda
{
    Attenuator::Attenuator(QWidget *parent) :
            QDialog(parent)
    {
        ui.setupUi(this);

        connect(ui.btnCalculate, SIGNAL(clicked()), this, SLOT(slotCalculate()));
        connect(ui.comboTopology, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotTopologyChanged()));
        ui.labelTopologyImage->setPixmap(QPixmap(Caneda::bitmapDirectory() + "att_pi.png"));
    }

    Attenuator::~Attenuator()
    {
    }

    void Attenuator::slotCalculate()
    {
        double Zin = ui.spinZin->value();
        double Zout = ui.spinZout->value();
        double Attenuation = ui.spinAttenuation->value();
        double R1;
        double R2;
        double R3;

        double L = pow(10, Attenuation/10);
        double A = (L + 1) / (L - 1);

        double Lmin;
        if ( Zin > Zout ) {
            Lmin = (2*Zin/Zout) - 1 + 2*sqrt( Zin/Zout*(Zin/Zout - 1) );
        }
        else {
            Lmin = (2*Zout/Zin) - 1 + 2*sqrt( Zout/Zin*(Zout/Zin - 1) );
        }

        double MinimumATT = 10*log10(Lmin);
        int result = 0;
        if ( MinimumATT > Attenuation ) {
            result = -1;
        }
        else {
            if ( ui.comboTopology->currentText() == "Pi" ) {
                R2 = (L - 1)/2 * sqrt( Zin*Zout/L );
                R1 = 1/( (A/Zin) - (1/R2) );
                R3 = 1/( (A/Zout) - (1/R2) );
            }
            else if ( ui.comboTopology->currentText() == "Tee" ) {
                R2 = (2 * sqrt(L * Zin * Zout)) / (L - 1);
                R1 = Zin*A - R2;
                R3 = Zout*A - R2;
            }
            else if ( ui.comboTopology->currentText() == "Bridged Tee" ) {
                L = pow(10, Attenuation/20);
                R1 = Zin*(L - 1);
                R2 = Zin/(L - 1);
                R3 = Zin;
            }
        }

        if (result != -1) {
            ui.statusBar->setText(tr("Result: Success!"));
            ui.spinR1->setValue(R1);
            ui.spinR2->setValue(R2);
            ui.spinR3->setValue(R3);

            slotCreateSchematic();
        }
        else {
            ui.statusBar->setText(tr("Error: Set Attenuation more than %1 dB")
                                  .arg(QString::number(MinimumATT, 'f', 3)));
            ui.spinR1->setValue(0);
            ui.spinR2->setValue(0);
            ui.spinR3->setValue(0);
        }
    }

    void Attenuator::slotCreateSchematic()
    {
        // Create the Caneda schematic
        QString *s = new QString("<Caneda Schematic " + Caneda::version + ">\n");
        *s += "<Components>\n";

        if ( ui.comboTopology->currentText() == "Pi" ) {
            *s += QString("<R R1 1 180 160 -30 -14 0 1 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR1->value());
            *s += QString("<R R2 1 210 130 -19 -42 0 0 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR2->value());
            *s += QString("<R R3 1 240 160 11 -14 0 1 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR3->value());
            *s += "<GND * 1 180 190 0 0 0 0>\n";
            *s += "<GND * 1 240 190 0 0 0 0>\n";
            *s += "</Components>\n";
            *s += "<Wires>\n";
            *s += "<240 130 280 130 \"\" 0 0 0 \"\">\n";
            *s += "<140 130 180 130 \"\" 0 0 0 \"\">\n";
            *s += "</Wires>\n";
            *s += "<Diagrams>\n";
            *s += "</Diagrams>\n";
            *s += "<Paintings>\n";
            *s += QString("<Text 125 60 12 #000000 0 \"%1 dB Pi-Type Attenuator\">\n").arg(ui.spinAttenuation->value());
            *s += QString("<Text 290 122 10 #000000 0 \"Z2: %1 Ohm\">\n").arg(ui.spinZout->value());
            *s += QString("<Text 50 122 10 #000000 0 \"Z1: %1 Ohm\">\n").arg(ui.spinZin->value());
            *s += "</Paintings>\n";
        }
        else if ( ui.comboTopology->currentText() == "Tee" ) {
            *s += QString("<R R1 1 180 130 -25 -42 0 2 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR1->value());
            *s += QString("<R R2 1 210 160 10 -4 0 3 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR2->value());
            *s += QString("<R R3 1 240 130 -25 -42 0 2 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR3->value());
            *s += "<GND * 1 210 190 0 0 0 0>\n";
            *s +="</Components>\n";
            *s += "<Wires>\n";
            *s += "<140 130 150 130 \"\" 0 0 0 \"\">\n";
            *s += "<270 130 280 130 \"\" 0 0 0 \"\">\n";
            *s += "</Wires>\n";
            *s += "<Diagrams>\n";
            *s += "</Diagrams>\n";
            *s += "<Paintings>\n";
            *s += QString("<Text 115 60 12 #000000 0 \"%1 dB Tee-Type Attenuator\">\n").arg(ui.spinAttenuation->value());
            *s += QString("<Text 290 122 10 #000000 0 \"Z2: %1 Ohm\">\n").arg(ui.spinZout->value());
            *s += QString("<Text 50 122 10 #000000 0 \"Z1: %1 Ohm\">\n").arg(ui.spinZin->value());
            *s += "</Paintings>\n";
        }
        else if ( ui.comboTopology->currentText() == "Bridged Tee" ) {
            *s += QString("<R R1 1 210 130 -19 -42 0 0 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR1->value());
            *s += QString("<R R2 1 180 160 -30 -14 0 1 \"%1 Ohm\" 0 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR3->value());
            *s += QString("<R R3 1 240 160 11 -14 0 1 \"%1 Ohm\" 0 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR3->value());
            *s += QString("<R R4 1 210 220 11 -14 0 1 \"%1 Ohm\" 1 \"26.85\" 0 \"0.0\" 0 \"0.0\" 0 \"26.85\" 0 \"US\" 0>\n").arg(ui.spinR2->value());
            *s += "<GND * 1 210 250 0 0 0 0>\n";
            *s += "</Components>\n";
            *s += "<Wires>\n";
            *s += "<240 130 280 130 \"\" 0 0 0 \"\">\n";
            *s += "<140 130 180 130 \"\" 0 0 0 \"\">\n";
            *s += "<180 190 240 190 \"\" 0 0 0 \"\">\n";
            *s += "</Wires>\n";
            *s += "<Diagrams>\n";
            *s += "</Diagrams>\n";
            *s += "<Paintings>\n";
            *s += QString("<Text 100 60 12 #000000 0 \"%1 dB Bridged-Tee-Type Attenuator\">\n").arg(ui.spinAttenuation->value());
            *s += QString("<Text 290 122 10 #000000 0 \"Z2: %1 Ohm\">\n").arg(ui.spinZout->value());
            *s += QString("<Text 50 122 10 #000000 0 \"Z1: %1 Ohm\">\n").arg(ui.spinZin->value());
            *s += "</Paintings>\n";
        }

        QClipboard *cb = QApplication::clipboard();
        cb->setText(*s);
        delete s;
    }

    void Attenuator::slotTopologyChanged()
    {
        if ( ui.comboTopology->currentText() == "Pi" ) {
            ui.labelTopologyImage->setPixmap(QPixmap(Caneda::bitmapDirectory() + "att_pi.png"));
            ui.spinZout->setEnabled(true);
        }
        else if ( ui.comboTopology->currentText() == "Tee" ) {
            ui.labelTopologyImage->setPixmap(QPixmap(Caneda::bitmapDirectory() + "att_tee.png"));
            ui.spinZout->setEnabled(true);
        }
        else if ( ui.comboTopology->currentText() == "Bridged Tee" ) {
            ui.labelTopologyImage->setPixmap(QPixmap(Caneda::bitmapDirectory() + "att_bridge.png"));
            ui.spinZout->setValue(0);
            ui.spinZout->setEnabled(false);
        }
    }

} // namespace Caneda
