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

#include "transmissiondialog.h"

#include "c_microstrip.h"
#include "coax.h"
#include "global.h"
#include "units.h"
#include "microstrip.h"
#include "propertygrid.h"
#include "rectwaveguide.h"

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

TransWidgets::TransWidgets()
{
    subParams = 0l;
    phyParams = 0l;
    comParams = 0l;
    eleParams = 0l;
    result = 0l;
    line = 0l;
}

PropertyBox* TransWidgets::boxWithProperty(const QString& name)
{
    if(subParams->exists(name))
        return subParams;
    else if(comParams->exists(name))
        return comParams;
    else if(phyParams->exists(name))
        return phyParams;
    else if(eleParams->exists(name))
        return eleParams;
    else
        return 0l;
}

namespace Caneda
{
    TransmissionDialog::TransmissionDialog(QWidget *parent) :
            QDialog(parent)
    {
        ui.setupUi(this);

        connect(ui.comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeChanged()));
        // Derive Electrical Parameters
        connect(ui.btnAnalyze, SIGNAL(clicked()), SLOT(slotAnalyze()));
        // Compute Physical Parameters
        connect(ui.btnSynthesize, SIGNAL(clicked()), SLOT(slotSynthesize()));

        for (int i=0; i<4; i++) {
            transWidgets[i] = new TransWidgets();
            transWidgets[i]->subParams = new PropertyBox("       "+tr("Substrate Parameters")+"      ");
            transWidgets[i]->phyParams = new PropertyBox("       "+tr("Physical Parameters")+"      ");
            transWidgets[i]->eleParams = new PropertyBox("       "+tr("Electrical Parameters")+"      ");
            transWidgets[i]->comParams = new PropertyBox("       "+tr("Component Parameters")+"      ");
            transWidgets[i]->result = new ResultBox(0l);
        }

        slotTypeChanged();
        setupMicrostrip();
        setupRectWaveGuide();
        setupCoaxialLine();
        setupCoupledMicrostrip();
    }

    TransmissionDialog::~TransmissionDialog()
    {
    }

    void TransmissionDialog::setupMicrostrip()
    {
        transWidgets[0]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),2.94);
        transWidgets[0]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1.0);
        transWidgets[0]->subParams->addDoubleProperty("H",tr("Height of Substrate"),10.0,Caneda::Length,Caneda::mil);
        transWidgets[0]->subParams->addDoubleProperty("H_t",tr("Height of Box Top"),1e20,Caneda::Length,Caneda::mil);
        transWidgets[0]->subParams->addDoubleProperty("T",tr("Strip Thickness"),0.1,Caneda::Length,Caneda::mil);
        transWidgets[0]->subParams->addDoubleProperty("Cond",tr("Strip Conductivity"),4.1e7);
        transWidgets[0]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0);
        transWidgets[0]->subParams->addDoubleProperty("Rough",tr("Conductor Roughness"),0,Caneda::Length,Caneda::mil);

        transWidgets[0]->comParams->addDoubleProperty("Freq",tr("Frequency"),1,Caneda::Frequency,Caneda::GHz);

        transWidgets[0]->phyParams->addDoubleProperty("W",tr("Line Width"),10,Caneda::Length,Caneda::mil);
        transWidgets[0]->phyParams->addDoubleProperty("L",tr("Line Length"),100,Caneda::Length,Caneda::mil);

        transWidgets[0]->eleParams->addDoubleProperty("Z0",tr("Characteristic Impedance"),50,Caneda::Resistance,Caneda::Ohm);
        transWidgets[0]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),90,Caneda::Angle,Caneda::Deg);

        transWidgets[0]->result->addResultItem(tr("ErEff"));
        transWidgets[0]->result->addResultItem(tr("Conductor Losses"));
        transWidgets[0]->result->addResultItem(tr("Dielectric Losses"));
        transWidgets[0]->result->addResultItem(tr("Skin Depth"));
        transWidgets[0]->result->adjustSize();

        transWidgets[0]->line = new microstrip();
        transWidgets[0]->line->setTransWidgets(transWidgets[0]);
    }

    void TransmissionDialog::setupRectWaveGuide()
    {
        transWidgets[1]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),1);
        transWidgets[1]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1);
        transWidgets[1]->subParams->addDoubleProperty("Cond",tr("Conductivity of Metal"),4.1e7);
        transWidgets[1]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0);
        transWidgets[1]->subParams->addDoubleProperty("TanM",tr("Magnetic Loss Tangent"),0);

        transWidgets[1]->comParams->addDoubleProperty("Freq",tr("Frequency"),10,Caneda::Frequency,Caneda::GHz);

        transWidgets[1]->phyParams->addDoubleProperty("a",tr("Width of Waveguide"),1000,Caneda::Length,Caneda::mil,true);
        transWidgets[1]->phyParams->addDoubleProperty("b",tr("Height of Waveguide"),500,Caneda::Length,Caneda::mil,true);
        transWidgets[1]->phyParams->addDoubleProperty("L",tr("Waveguide Length"),4000,Caneda::Length,Caneda::mil);
        transWidgets[1]->phyParams->setSelected("b",true);

        transWidgets[1]->eleParams->addDoubleProperty("Z0",tr("Characteristic Impedance"),0,Caneda::Resistance,Caneda::Ohm);
        transWidgets[1]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),0,Caneda::Angle,Caneda::Deg);

        transWidgets[1]->result->addResultItem(tr("ErEff"));
        transWidgets[1]->result->addResultItem(tr("Conductor Losses"));
        transWidgets[1]->result->addResultItem(tr("Dielectric Losses"));
        transWidgets[1]->result->addResultItem(tr("TE-Modes"));
        transWidgets[1]->result->addResultItem(tr("TM-Modes"));
        transWidgets[1]->result->adjustSize();

        transWidgets[1]->line = new rectwaveguide();
        transWidgets[1]->line->setTransWidgets(transWidgets[1]);
    }

    void TransmissionDialog::setupCoaxialLine()
    {
        transWidgets[2]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),2.1);
        transWidgets[2]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1);
        transWidgets[2]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0.002);
        transWidgets[2]->subParams->addDoubleProperty("Sigma",tr("Conductivity of Metal"),4.1e7);

        transWidgets[2]->comParams->addDoubleProperty("Freq",tr("Frequency"),10,Caneda::Frequency,Caneda::GHz);

        transWidgets[2]->phyParams->addDoubleProperty("din",tr("Inner Diameter"),40,Caneda::Length,Caneda::mil,true);
        transWidgets[2]->phyParams->addDoubleProperty("dout",tr("Outer Diameter"),134,Caneda::Length,Caneda::mil,true);
        transWidgets[2]->phyParams->addDoubleProperty("L",tr("Length"),1000,Caneda::Length,Caneda::mil);
        transWidgets[2]->phyParams->setSelected("dout",true);

        transWidgets[2]->eleParams->addDoubleProperty("Z0",tr("Characteristic Impedance"),0,Caneda::Resistance,Caneda::Ohm);
        transWidgets[2]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),0,Caneda::Angle,Caneda::Deg);

        transWidgets[2]->result->addResultItem(tr("Conductor Losses"));
        transWidgets[2]->result->addResultItem(tr("Dielectric Losses"));
        transWidgets[2]->result->addResultItem(tr("TE-Modes"));
        transWidgets[2]->result->addResultItem(tr("TM-Modes"));
        transWidgets[2]->result->adjustSize();

        transWidgets[2]->line = new coax();
        transWidgets[2]->line->setTransWidgets(transWidgets[2]);
    }

    void TransmissionDialog::setupCoupledMicrostrip()
    {
        transWidgets[3]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),4.3);
        transWidgets[3]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1);
        transWidgets[3]->subParams->addDoubleProperty("H",tr("Height of Substrate"),8.27,Caneda::Length,Caneda::mil);
        transWidgets[3]->subParams->addDoubleProperty("H_t",tr("Height of Box Top"),1e20,Caneda::Length,Caneda::mil);
        transWidgets[3]->subParams->addDoubleProperty("T",tr("Strip Thickness"),0.68,Caneda::Length,Caneda::mil);
        transWidgets[3]->subParams->addDoubleProperty("Cond",tr("Strip Conductivity"),4.1e7);
        transWidgets[3]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0);
        transWidgets[3]->subParams->addDoubleProperty("Rough",tr("Conductor Roughness"),0,Caneda::Length,Caneda::mil);

        transWidgets[3]->comParams->addDoubleProperty("Freq",tr("Frequency"),10,Caneda::Frequency,Caneda::GHz);

        transWidgets[3]->phyParams->addDoubleProperty("W",tr("Line Width"),8.66,Caneda::Length,Caneda::mil);
        transWidgets[3]->phyParams->addDoubleProperty("S",tr("Gap Width"),5.31,Caneda::Length,Caneda::mil);
        transWidgets[3]->phyParams->addDoubleProperty("L",tr("Length"),1000.0,Caneda::Length,Caneda::mil);

        transWidgets[3]->eleParams->addDoubleProperty("Z0e",tr("Even-Mode Impedance"),0,Caneda::Resistance,Caneda::Ohm);
        transWidgets[3]->eleParams->addDoubleProperty("Z0o",tr("Odd-Mode Impedance"),0,Caneda::Resistance,Caneda::Ohm);
        transWidgets[3]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),0,Caneda::Angle,Caneda::Deg);

        transWidgets[3]->result->addResultItem(tr("ErEff Even"));
        transWidgets[3]->result->addResultItem(tr("ErEff Odd"));
        transWidgets[3]->result->addResultItem(tr("Conductor Losses Even"));
        transWidgets[3]->result->addResultItem(tr("Conductor Losses Odd"));
        transWidgets[3]->result->addResultItem(tr("Dielectric Losses Even"));
        transWidgets[3]->result->addResultItem(tr("Dielectric Losses Odd"));
        transWidgets[3]->result->addResultItem(tr("Skin Depth"));
        transWidgets[3]->result->adjustSize();

        transWidgets[3]->line = new c_microstrip();
        transWidgets[3]->line->setTransWidgets(transWidgets[3]);
    }

    void TransmissionDialog::slotTypeChanged()
    {
        if ( ui.comboType->currentText() == "Microstrip" ) {
            ui.pixType->setPixmap(QPixmap(Caneda::bitmapDirectory() + "microstrip.png"));
            ui.stackedWidget->setCurrentIndex(0);
        }
        else if ( ui.comboType->currentText() == "Rectangular waveguide" ) {
            ui.pixType->setPixmap(QPixmap(Caneda::bitmapDirectory() + "rectwaveguide.png"));
            ui.stackedWidget->setCurrentIndex(1);
        }
        else if ( ui.comboType->currentText() == "Coaxial line" ) {
            ui.pixType->setPixmap(QPixmap(Caneda::bitmapDirectory() + "coax.png"));
            ui.stackedWidget->setCurrentIndex(2);
        }
        else if ( ui.comboType->currentText() == "Coupled microstrip" ) {
            ui.pixType->setPixmap(QPixmap(Caneda::bitmapDirectory() + "c_microstrip.png"));
            ui.stackedWidget->setCurrentIndex(3);
        }
    }

    void TransmissionDialog::slotAnalyze()
    {
        transWidgets[ui.comboType->currentIndex()]->line->analyze();
        ui.statusBar->setText(tr("Values are consistent."));
    }

    void TransmissionDialog::slotSynthesize()
    {
        transWidgets[ui.comboType->currentIndex()]->line->synthesize();
        ui.statusBar->setText(tr("Values are consistent."));
    }

    void TransmissionDialog::slotCreateSchematic()
    {
        int created = 0;
        QString s = "<Caneda Schematic " + Caneda::version() + ">\n";

        // create microstrip schematic
        if (ui.comboType->currentIndex() == 0) {
            TransWidgets *tw = transWidgets[0];
            s += "<Components>\n";
            s += "  <Pac P1 1 90 150 -74 -26 1 1 \"1\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s +="  <Pac P2 1 270 150 18 -26 0 1 \"2\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s += "  <GND * 1 90 180 0 0 0 0>\n";
            s += "  <GND * 1 270 180 0 0 0 0>\n";
            s += QString("  <SUBST SubstTC1 1 390 140 -30 24 0 0 \"%1\" 1 \"%2 mm\" 1 \"%3 um\" 1 \"%4\" 1 \"%5\" 1 \"%6\" 1>\n").
                 arg(tw->subParams->doubleValue("Er")).
                 arg(tw->subParams->doubleValue("H", Caneda::mm)).
                 arg(tw->subParams->doubleValue("T", Caneda::um)).
                 arg(tw->subParams->doubleValue("Tand")).
                 arg(1 / tw->subParams->doubleValue("Cond")).
                 arg(tw->subParams->doubleValue("Rough",Caneda::m));
            s += "  <.SP SPTC1 1 90 240 0 51 0 0 ";
            double freq = tw->comParams->doubleValue("Freq",Caneda::GHz);
            if (freq > 0)
                s += QString("\"log\" 1 \"%1 GHz\" 1 \"%2 GHz\" 1 ").
                     arg(freq / 10).arg(freq * 10);
            else
                s += "\"lin\" 1 \"0 GHz\" 1 \"10 GHz\" 1 ";
            s += "\"51\" 1 \"no\" 0 \"1\" 0 \"2\" 0>\n";
            s += QString("  <MLIN MSTC1 1 180 100 -26 15 0 0 \"SubstTC1\" 1 \"%1 mm\" 1 \"%2 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"26.85\" 0>\n").
                 arg(tw->phyParams->doubleValue("W",Caneda::mm)).
                 arg(tw->phyParams->doubleValue("L",Caneda::mm));
            s += "  <Eqn EqnTC1 1 240 260 -23 12 0 0 \"A=twoport(S,'S','A')\" 1 \"ZL=real(sqrt(A[1,2]/A[2,1]))\" 1 \"yes\" 0>\n";
            s += "</Components>\n";
            s += "<Wires>\n";
            s += "  <90 100 150 100 \"\" 0 0 0 \"\">\n";
            s += "  <90 100 90 120 \"\" 0 0 0 \"\">\n";
            s += "  <210 100 270 100 \"\" 0 0 0 \"\">\n";
            s += "  <270 100 270 120 \"\" 0 0 0 \"\">\n";
            s += "</Wires>\n";
            created++;
        }
        // create coupled microstrip schematic
        else if (ui.comboType->currentIndex() == 3) {
            TransWidgets *tw = transWidgets[3];
            s += "<Components>\n";
            s += "  <Pac P1 1 100 130 -74 -26 1 1 \"1\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s += "  <Pac P2 1 320 130 18 -26 0 1 \"2\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s += "  <Pac P3 1 280 220 18 -26 0 1 \"3\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s += "  <Pac P4 1 140 200 -74 -26 1 1 \"4\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s += "  <GND * 1 100 160 0 0 0 0>\n";
            s += "  <GND * 1 140 230 0 0 0 0>\n";
            s += "  <GND * 1 320 160 0 0 0 0>\n";
            s += "  <GND * 1 280 250 0 0 0 0>\n";
            s += QString("  <SUBST SubstTC1 1 410 220 -30 24 0 0 \"%1\" 1 \"%2 mm\" 1 \"%3 um\" 1 \"%4\" 1 \"%5\" 1 \"%6\" 1>\n").
                 arg(tw->subParams->doubleValue("Er")).
                 arg(tw->subParams->doubleValue("H", Caneda::mm)).
                 arg(tw->subParams->doubleValue("T", Caneda::um)).
                 arg(tw->subParams->doubleValue("Tand")).
                 arg(1 / tw->subParams->doubleValue("Cond")).
                 arg(tw->subParams->doubleValue("Rough", Caneda::m));
            s += "  <.SP SPTC1 1 100 290 0 51 0 0 ";
            double freq = tw->comParams->doubleValue("Freq", Caneda::GHz);
            if (freq > 0)
                s += QString("\"log\" 1 \"%1 GHz\" 1 \"%2 GHz\" 1 ").
                     arg(freq / 10).arg(freq * 10);
            else
                s += "\"lin\" 1 \"0 GHz\" 1 \"10 GHz\" 1 ";
            s += "\"51\" 1 \"no\" 0 \"1\" 0 \"2\" 0>\n";
            s += QString("  <MCOUPLED MSTC1 1 190 110 -26 37 0 0 \"SubstTC1\" 1 \"%1 mm\" 1 \"%2 mm\" 1 \"%3 mm\" 1 \"Kirschning\" 0 \"Kirschning\" 0 \"26.85\" 0>\n").
                 arg(tw->phyParams->doubleValue("W", Caneda::mm)).
                 arg(tw->phyParams->doubleValue("L", Caneda::mm)).
                 arg(tw->phyParams->doubleValue("S", Caneda::mm));
            s += "</Components>\n";
            s += "<Wires>\n";
            s += "  <100 80 160 80 \"\" 0 0 0 \"\">\n";
            s += "  <100 80 100 100 \"\" 0 0 0 \"\">\n";
            s += "  <140 140 140 170 \"\" 0 0 0 \"\">\n";
            s += "  <140 140 160 140 \"\" 0 0 0 \"\">\n";
            s += "  <320 80 320 100 \"\" 0 0 0 \"\">\n";
            s += "  <220 80 320 80 \"\" 0 0 0 \"\">\n";
            s += "  <280 140 280 190 \"\" 0 0 0 \"\">\n";
            s += "  <220 140 280 140 \"\" 0 0 0 \"\">\n";
            s += "</Wires>\n";
            created++;
        }
        // create coaxial line schematic
        else if (ui.comboType->currentIndex() == 2) {
            TransWidgets *tw = transWidgets[2];
            s += "<Components>\n";
            s += "  <Pac P1 1 90 150 -74 -26 1 1 \"1\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s +="  <Pac P2 1 270 150 18 -26 0 1 \"2\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
            s += "  <GND * 1 90 180 0 0 0 0>\n";
            s += "  <GND * 1 270 180 0 0 0 0>\n";
            s += "  <.SP SPTC1 1 90 240 0 51 0 0 ";
            double freq = tw->comParams->doubleValue("Freq", Caneda::GHz);
            if (freq > 0)
                s += QString("\"log\" 1 \"%1 GHz\" 1 \"%2 GHz\" 1 ").
                     arg(freq / 10).arg(freq * 10);
            else
                s += "\"lin\" 1 \"0 GHz\" 1 \"10 GHz\" 1 ";
            s += "\"51\" 1 \"no\" 0 \"1\" 0 \"2\" 0>\n";
            s += QString("  <COAX CXTC1 1 180 100 -26 15 0 0 \"%1\" 1 \"%2\" 0 \"%3\" 0 \"%4 mm\" 1 \"%5 mm\" 1  \"%6 mm\" 1  \"%7\" 0 \"26.85\" 0>\n").
                 arg(tw->subParams->doubleValue("Er")).
                 arg(1 / tw->subParams->doubleValue("Sigma")).
                 arg(tw->subParams->doubleValue("Mur")).
                 arg(tw->phyParams->doubleValue("dout", Caneda::mm)).
                 arg(tw->phyParams->doubleValue("din", Caneda::mm)).
                 arg(tw->phyParams->doubleValue("L", Caneda::mm)).
                 arg(tw->subParams->doubleValue("Tand"));
            s += "</Components>\n";
            s += "<Wires>\n";
            s += "  <90 100 150 100 \"\" 0 0 0 \"\">\n";
            s += "  <90 100 90 120 \"\" 0 0 0 \"\">\n";
            s += "  <210 100 270 100 \"\" 0 0 0 \"\">\n";
            s += "  <270 100 270 120 \"\" 0 0 0 \"\">\n";
            s += "</Wires>\n";
            created++;
        }

        // put resulting transmission line schematic into clipboard
        QClipboard *cb = QApplication::clipboard();
        cb->setText(s);

        // put a message into status line
        if (created) {
            ui.statusBar->setText(tr("Schematic copied into clipboard"));
        }
        else {
            ui.statusBar->setText(tr("Transmission line type not available"));
        }
    }

    void TransmissionDialog::slotFileLoad()
    {
        //      QString s = QFileDialog::getOpenFileName(this, tr("Open File"), "",
        //                                               tr("Transcalc File (*.trc)");
        //
        //      if (!s.isEmpty())  {
        //
        //
        //
        //          QFile file(QDir::toNativeSeparators(fname));
        //          if(!file.open(QIODevice::ReadOnly)) return false; // file doesn't exist
        //
        //          QTextStream stream(&file);
        //          QString Line, Name, Unit;
        //          double Value;
        //
        //          while(!stream.atEnd()) {
        //            Line = stream.readLine();
        //            for (int i = 0; i < 4; i++)
        //            {
        //              if (Line == "<" + QString(transWidgets[i]->line->description) + ">")
        //              {
        //                tranType->setCurrentIndex(i);
        //                while(!stream.atEnd())
        //                {
        //                  Line = stream.readLine();
        //                  if (Line == "</" + QString(transWidgets[i]->line->description) + ">")
        //                    break;
        //                  Line = Line.simplified();
        //                  Name = Line.section(' ',0,0);
        //                  Value = Line.section(' ',1,1).toDouble();
        //                  Unit = Line.section(' ',2,2);
        //                  PropertyBox *paramExist = transWidgets[i]->boxWithProperty(Name);
        //                  Q_ASSERT(paramExist != 0l);
        //                  int intUnit = Units::toInt(Unit);
        //                  bool hasUnit = (intUnit != Units::None);
        //                  if(hasUnit)
        //                    paramExist->setDoubleValue(Name,Value,intUnit);
        //                  else
        //                    paramExist->setDoubleValue(Name,Value);
        //                }
        //                break;
        //              }
        //            }
        //          }
        //          file.close();
        //          return true;
        //
        //
        //
        //
        //        if (!loadFile (s)) {
        //          QMessageBox::critical (this, tr("Error"),
        //                                 tr("Cannot load file:")+" '"+s+"'!");
        //        }
        //      }

    }

    void TransmissionDialog::slotFileSave()
    {
        //      QString s = QFileDialog::getOpenFileName(this, tr("Save File"), "",
        //                                               tr("Transcalc File (*.trc)");
        //
        //      if (!s.isEmpty())  {
        //
        //
        //
        //
        //          QFile file (QDir::toNativeSeparators(fname));
        //          if(!file.open (QIODevice::WriteOnly)) return false; // file not writable
        //          QTextStream stream (&file);
        //
        //          // some lines of documentation
        //          stream << "# CanedaTranscalc " << PACKAGE_VERSION << "  " << fname << "\n";
        //          stream << "#   It is not suggested to edit the file, use CanedaTranscalc "
        //              << "instead.\n\n";
        //
        //          int mode = tranType->currentIndex();
        //          stream << "<" << transWidgets[mode]->line->description << ">\n";
        //
        //
        //          int mode = tranType->currentIndex();
        //          stream << *(transWidgets[mode]->subParams);
        //          stream << *(transWidgets[mode]->comParams);
        //          stream << *(transWidgets[mode]->phyParams);
        //          stream << *(transWidgets[mode]->eleParams);
        //
        //          stream << "</" << transWidgets[mode]->line->description << ">\n";
        //          file.close ();
        //          return true;
        //
        //
        //
        //
        //        if (!saveFile (s)) {
        //          QMessageBox::critical (this, tr("Error"),
        //                                 tr("Cannot save file:")+" '"+s+"'!");
        //        }
        //      }

    }

} // namespace Caneda
