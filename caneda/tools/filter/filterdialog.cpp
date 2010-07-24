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

#include "filterdialog.h"

#include "lc_filter.h"
#include "qf_poly.h"
#include "qf_filter.h"
#include "qf_cauer.h"

#include <QClipboard>

#include <math.h>

namespace Caneda
{
    FilterDialog::FilterDialog(QWidget *parent) :
            QDialog(parent)
    {
        ui.setupUi(this);

        connect(ui.comboType, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotTypeChanged()));
        connect(ui.comboClass, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotClassChanged()));
        connect(ui.btnCalculate, SIGNAL(clicked()), this, SLOT(slotCalculate()));

        // Set initial state
        slotTypeChanged();
        slotClassChanged();
    }

    FilterDialog::~FilterDialog()
    {
    }

    void FilterDialog::slotCalculate()
    {
        // Get numerical values from input widgets
        double CornerFreq   = ui.spinStart->value();
        double StopFreq     = ui.spinStop->value();
        double BandStopFreq = ui.spinStopBand->value();

        int    Order       = ui.spinOrder->value();
        double Ripple      = ui.spinRipple->value();
        double Attenuation = ui.spinAttenuation->value();
        double Impedance   = ui.spinImpedance->value();

        // Add exponent
        CornerFreq   *= pow(10, double(3*ui.comboStart->currentIndex()));
        StopFreq     *= pow(10, double(3*ui.comboStop->currentIndex()));
        BandStopFreq *= pow(10, double(3*ui.comboStopBand->currentIndex()));


        // Check the input values
        if ( ui.spinStop->isEnabled() ) {
            if ( CornerFreq >= StopFreq ) {
                ui.statusBar->setText(tr("Error: Stop frequency must be greater than start frequency"));
                return;
            }
        }
        if ( ui.spinOrder->isEnabled() ) {
            if ( Order < 2) {
                ui.statusBar->setText(tr("Filter order must not be less than two"));
                return;
            }
            if ( Order > 19 && ui.comboType->currentText() == "Bessel" ) {
                ui.statusBar->setText(tr("Bessel filter order must not be greater than 19"));
                return;
            }
        }

        // Calculate the filter
        QString *s;
        if ( ui.comboType->currentText() == "Cauer" ) {

            qf_cauer *Filter = NULL;
            double fc   = CornerFreq;
            double amin = Ripple;
            double fs   = BandStopFreq;
            double r    = Impedance;
            double amax = Attenuation;
            double bw   = StopFreq - fc;

            if ( ui.comboClass->currentText() == "Low pass" ) {
                Filter = new qf_cauer (amin, amax, fc, fs, r, 0, LOWPASS);
            }
            else if ( ui.comboClass->currentText() == "High pass" ) {
                Filter = new qf_cauer (amin, amax, fc, fs, r, 0, HIGHPASS);
            }
            else if ( ui.comboClass->currentText() == "Band pass" ) {
                Filter = new qf_cauer (amin, amax, fc + bw / 2, fs, r, bw, BANDPASS);
            }
            else if ( ui.comboClass->currentText() == "Band stop" ) {
                Filter = new qf_cauer (amin, amax, fc + bw / 2, fs, r, bw, BANDSTOP);
            }

            if (Filter) {
                ui.spinOrder->setValue(Filter->order());
                s = new QString(Filter->to_caneda().c_str());
                delete Filter;
            }
            else {
                return;
            }
        }
        else {
            tFilter Filter;
            Filter.Type = ui.comboType->currentIndex();
            Filter.Class = ui.comboType->currentIndex();
            Filter.Order = Order;
            Filter.Ripple = Ripple;
            Filter.Attenuation = Attenuation;
            Filter.Impedance = Impedance;
            Filter.Frequency = CornerFreq;
            Filter.Frequency2 = StopFreq;
            Filter.Frequency3 = BandStopFreq;
            s = LC_Filter::createSchematic(&Filter);
        }

        // Output the result
        if (!s) {
            ui.statusBar->setText(tr("Result: Couldn't calculate filter"));
        }
        else {
            ui.statusBar->setText(tr("Result: Successful"));

            // Put resulting filter schematic into clipboard
            QClipboard *cb = QApplication::clipboard();
            cb->setText(*s);
        }

        delete s;
    }

    void FilterDialog::slotTypeChanged()
    {
        if ( ui.comboType->currentText() == "Bessel" ) {
            ui.spinRipple->setEnabled(false);
            ui.spinOrder->setEnabled(true);
            ui.spinAttenuation->setEnabled(false);
            ui.spinStopBand->setEnabled(false);
        }
        else if ( ui.comboType->currentText() == "Butterworth" ) {
            ui.spinRipple->setEnabled(false);
            ui.spinOrder->setEnabled(true);
            ui.spinAttenuation->setEnabled(false);
            ui.spinStopBand->setEnabled(false);
        }
        else if ( ui.comboType->currentText() == "Chebyshev" ) {
            ui.spinRipple->setEnabled(true);
            ui.spinOrder->setEnabled(true);
            ui.spinAttenuation->setEnabled(false);
            ui.spinStopBand->setEnabled(false);
        }
        else if ( ui.comboType->currentText() == "Cauer" ) {
            ui.spinRipple->setEnabled(true);
            ui.spinOrder->setEnabled(false);
            ui.spinAttenuation->setEnabled(true);
            ui.spinStopBand->setEnabled(true);
        }
    }

    void FilterDialog::slotClassChanged()
    {
        if ( ui.comboClass->currentText() == "Low pass" ) {
            ui.spinStop->setEnabled(false);
            ui.labelStart->setText("Corner frequency");
        }
        else if ( ui.comboClass->currentText() == "High pass" ) {
            ui.spinStop->setEnabled(false);
            ui.labelStart->setText("Corner frequency");
        }
        else if ( ui.comboClass->currentText() == "Band pass" ) {
            ui.spinStop->setEnabled(true);
            ui.labelStart->setText("Start frequency");
            ui.labelStopBand->setText("Stop band frequency");
            ui.labelRipple->setText("Pass band ripple");
        }
        else if ( ui.comboClass->currentText() == "Band stop" ) {
            ui.spinStop->setEnabled(true);
            ui.labelStart->setText("Start frequency");
            ui.labelStopBand->setText("Pass band frequency");
            ui.labelRipple->setText("Pass band attenuation");
        }
    }

} // namespace Caneda
