/***************************************************************************
                          qucstrans.cpp  -  description
                             -------------------
    begin                : Sun Feb 27 2005
    copyright            : (C) 2005, 2006 by Stefan Jahn
    email                : stefan@lkcc.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qucstrans.h"

#include "helpdialog.h"
#include "optionsdialog.h"
#include "qucs-tools/units.h"
#include "microstrip.h"
#include "coax.h"
#include "rectwaveguide.h"
#include "c_microstrip.h"

#include <QtGui/QFrame>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QStackedWidget>
#include <QtGui/QStatusBar>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QClipboard>
#include <QtGui/QKeySequence>
#include <QtGui/QApplication>
#include <QtGui/QSplitter>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtCore/QTime>

#include "qucs-tools/propertygrid.h"
QDir QucsWorkDir;

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

/* Constructor setups the GUI. */
QucsTranscalc::QucsTranscalc() : QWidget()
{
  // set application icon
  setWindowIcon(QPixmap(QucsSettings.BitmapDir + "big.qucs.xpm"));
  setWindowTitle("Qucs Transcalc " PACKAGE_VERSION);
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(0);
  setupMenu();

  // main layout
  QHBoxLayout *midLayout = new QHBoxLayout();
  mainLayout->addLayout(midLayout);
    
  // transmission line type choice
  QGroupBox * lineGroup = new QGroupBox ("           "+tr("Transmission Line Type")+"                       ");
  QVBoxLayout *lineLayout = new QVBoxLayout(lineGroup);
  lineLayout->setSizeConstraint(QLayout::SetMinimumSize);
  tranType = new QComboBox ();
  lineLayout->addWidget(tranType);
  tranType->addItem (tr("Microstrip"));
  tranType->addItem (tr("Rectangular Waveguide"));
  tranType->addItem (tr("Coaxial Line"));
  tranType->addItem (tr("Coupled Microstrip"));
  //tranType->setFixedSize(280,tranType->sizeHint().height());
  // setup transmission line picture
  tranPix = new QLabel ();
  tranPix->setMinimumSize(220,220);
  lineLayout->addWidget(tranPix);
  tranPix->setPixmap(QPixmap(QucsSettings.BitmapDir + "microstrip.png"));
  midLayout->addWidget(lineGroup);
  connect(tranType, SIGNAL(activated(int)), this, SLOT(slotSelectType(int)));
  
  QVBoxLayout *rl = new QVBoxLayout();
  midLayout->addLayout(rl);
  rl->setMargin(0);
  widStack = new QStackedWidget();
  connect(tranType, SIGNAL(activated(int)), widStack, SLOT(setCurrentIndex(int)));
  rl->addWidget(widStack);
  QHBoxLayout *butLayout = new QHBoxLayout();
  rl->addLayout(butLayout);
  butLayout->addStretch(2);
  QPushButton * analyze = new QPushButton (tr("Analyze"));
  butLayout->addWidget(analyze,2);
  analyze->setToolTip(tr("Derive Electrical Parameters"));
  connect(analyze, SIGNAL(clicked()), SLOT(slotAnalyze()));
  QPushButton * synthesize = new QPushButton (tr("Synthesize"));
  butLayout->addWidget(synthesize,2);
  butLayout->addStretch(2);
  synthesize->setToolTip(tr("Compute Physical Parameters"));
  connect(synthesize, SIGNAL(clicked()), SLOT(slotSynthesize()));
  
  setupFrames();
  statBar = new QStatusBar();
  statBar->showMessage (tr("Ready."));
  statBar->setFixedHeight (statBar->sizeHint().height());
  mainLayout->addWidget (statBar);
  
}

/* Destructor destroys the application. */
QucsTranscalc::~QucsTranscalc()
{
}

void QucsTranscalc::setupMenu()
{
  using namespace Qt;
  QMenuBar * menuBar = new QMenuBar ();
  layout()->addWidget(menuBar);
  // create file menu
  QMenu * fileMenu = menuBar->addMenu(tr("&File"));
  fileMenu->addAction(tr("&Load"), this, SLOT(slotFileLoad()), CTRL+Key_L);
  fileMenu->addAction(tr("&Save"), this, SLOT(slotFileSave()), CTRL+Key_S);
  fileMenu->addSeparator ();
  fileMenu->addAction(tr("&Options"), this, SLOT(slotOptions()), CTRL+Key_O);
  fileMenu->addSeparator ();
  fileMenu->addAction(tr("&Quit"), qApp, SLOT(quit()), CTRL+Key_Q);
  
  // create execute menu
  QMenu * execMenu = menuBar->addMenu(tr("&Execute"));
  execMenu->addAction(tr("&Copy to Clipboard"), this, SLOT(slotCopyToClipboard()), Key_F2);
  execMenu->addAction(tr("&Analyze"), this, SLOT(slotAnalyze()), Key_F3);
  execMenu->addAction(tr("&Synthesize"), this, SLOT(slotSynthesize()), Key_F4);
  
  // create help menu
  QMenu * helpMenu = menuBar->addMenu(tr("&Help"));
  helpMenu->addAction(tr("&Help"), this, SLOT(slotHelpIntro()), Key_F1);
  helpMenu->addAction(tr("About"), this, SLOT(slotAbout()));

  menuBar->setFixedHeight(menuBar->sizeHint().height());
}

void QucsTranscalc::setupFrames()
{
  for(int i=0; i<4; i++) 
  {
    tranContainers[i] = new QSplitter();
    transWidgets[i] = new TransWidgets();
    
    transWidgets[i]->subParams = new PropertyBox("       "+tr("Substrate Parameters")+"      ");
    transWidgets[i]->phyParams = new PropertyBox("       "+tr("Physical Parameters")+"      ");
    transWidgets[i]->eleParams = new PropertyBox("       "+tr("Electrical Parameters")+"      ");
    transWidgets[i]->comParams = new PropertyBox("       "+tr("Component Parameters")+"      ");
    tranContainers[i]->setOrientation(Qt::Vertical);
    
    QWidget *upperPart = new QWidget();
    QHBoxLayout *h = new QHBoxLayout(upperPart);
    h->addWidget(transWidgets[i]->subParams);
    QVBoxLayout *v = new QVBoxLayout();
    h->addLayout(v);
    v->addWidget(transWidgets[i]->phyParams);
    v->addWidget(transWidgets[i]->comParams);
    v->addWidget(transWidgets[i]->eleParams);
    tranContainers[i]->addWidget(upperPart);
    transWidgets[i]->result = new ResultBox(0l);
    tranContainers[i]->addWidget(transWidgets[i]->result);

    widStack->insertWidget(i,tranContainers[i]);
  }

  tranContainers[0]->setObjectName("Microstrip");
  tranContainers[1]->setObjectName("RectWaveGuide");
  tranContainers[2]->setObjectName("CoaxialLine");
  tranContainers[3]->setObjectName("CoupledMicrostrip");
  
  setupMicrostrip();
  setupRectWaveGuide();
  setupCoaxialLine();
  setupCoupledMicrostrip();
}

void QucsTranscalc::setupMicrostrip()
{
  transWidgets[0]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),2.94);
  transWidgets[0]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1.0);
  transWidgets[0]->subParams->addDoubleProperty("H",tr("Height of Substrate"),10.0,Units::Length,Units::mil);
  transWidgets[0]->subParams->addDoubleProperty("H_t",tr("Height of Box Top"),1e20,Units::Length,Units::mil);
  transWidgets[0]->subParams->addDoubleProperty("T",tr("Strip Thickness"),0.1,Units::Length,Units::mil);
  transWidgets[0]->subParams->addDoubleProperty("Cond",tr("Strip Conductivity"),4.1e7);
  transWidgets[0]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0);
  transWidgets[0]->subParams->addDoubleProperty("Rough",tr("Conductor Roughness"),0,Units::Length,Units::mil);

  transWidgets[0]->comParams->addDoubleProperty("Freq",tr("Frequency"),1,Units::Frequency,Units::GHz);

  transWidgets[0]->phyParams->addDoubleProperty("W",tr("Line Width"),10,Units::Length,Units::mil);
  transWidgets[0]->phyParams->addDoubleProperty("L",tr("Line Length"),100,Units::Length,Units::mil);

  transWidgets[0]->eleParams->addDoubleProperty("Z0",tr("Characteristic Impedance"),50,Units::Resistance,Units::Ohm);
  transWidgets[0]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),90,Units::Angle,Units::Deg);

  transWidgets[0]->result->addResultItem(tr("ErEff"));
  transWidgets[0]->result->addResultItem(tr("Conductor Losses"));
  transWidgets[0]->result->addResultItem(tr("Dielectric Losses"));
  transWidgets[0]->result->addResultItem(tr("Skin Depth"));
  transWidgets[0]->result->adjustSize();

  transWidgets[0]->line = new microstrip();
  transWidgets[0]->line->setTransWidgets(transWidgets[0]);
}

void QucsTranscalc::setupRectWaveGuide()
{
  transWidgets[1]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),1);
  transWidgets[1]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1);
  transWidgets[1]->subParams->addDoubleProperty("Cond",tr("Conductivity of Metal"),4.1e7);
  transWidgets[1]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0);
  transWidgets[1]->subParams->addDoubleProperty("TanM",tr("Magnetic Loss Tangent"),0);

  transWidgets[1]->comParams->addDoubleProperty("Freq",tr("Frequency"),10,Units::Frequency,Units::GHz);

  transWidgets[1]->phyParams->addDoubleProperty("a",tr("Width of Waveguide"),1000,Units::Length,Units::mil,true);
  transWidgets[1]->phyParams->addDoubleProperty("b",tr("Height of Waveguide"),500,Units::Length,Units::mil,true);
  transWidgets[1]->phyParams->addDoubleProperty("L",tr("Waveguide Length"),4000,Units::Length,Units::mil);
  transWidgets[1]->phyParams->setSelected("b",true);
  
  transWidgets[1]->eleParams->addDoubleProperty("Z0",tr("Characteristic Impedance"),0,Units::Resistance,Units::Ohm);
  transWidgets[1]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),0,Units::Angle,Units::Deg);

  transWidgets[1]->result->addResultItem(tr("ErEff"));
  transWidgets[1]->result->addResultItem(tr("Conductor Losses"));
  transWidgets[1]->result->addResultItem(tr("Dielectric Losses"));
  transWidgets[1]->result->addResultItem(tr("TE-Modes"));
  transWidgets[1]->result->addResultItem(tr("TM-Modes"));
  transWidgets[1]->result->adjustSize();

  transWidgets[1]->line = new rectwaveguide();
  transWidgets[1]->line->setTransWidgets(transWidgets[1]);
}

void QucsTranscalc::setupCoaxialLine()
{
  transWidgets[2]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),2.1);
  transWidgets[2]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1);
  transWidgets[2]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0.002);
  transWidgets[2]->subParams->addDoubleProperty("Sigma",tr("Conductivity of Metal"),4.1e7);

  transWidgets[2]->comParams->addDoubleProperty("Freq",tr("Frequency"),10,Units::Frequency,Units::GHz);
  
  transWidgets[2]->phyParams->addDoubleProperty("din",tr("Inner Diameter"),40,Units::Length,Units::mil,true);
  transWidgets[2]->phyParams->addDoubleProperty("dout",tr("Outer Diameter"),134,Units::Length,Units::mil,true);
  transWidgets[2]->phyParams->addDoubleProperty("L",tr("Length"),1000,Units::Length,Units::mil);
  transWidgets[2]->phyParams->setSelected("dout",true);

  transWidgets[2]->eleParams->addDoubleProperty("Z0",tr("Characteristic Impedance"),0,Units::Resistance,Units::Ohm);
  transWidgets[2]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),0,Units::Angle,Units::Deg);

  transWidgets[2]->result->addResultItem(tr("Conductor Losses"));
  transWidgets[2]->result->addResultItem(tr("Dielectric Losses"));
  transWidgets[2]->result->addResultItem(tr("TE-Modes"));
  transWidgets[2]->result->addResultItem(tr("TM-Modes"));
  transWidgets[2]->result->adjustSize();
  
  transWidgets[2]->line = new coax();
  transWidgets[2]->line->setTransWidgets(transWidgets[2]);
}

void QucsTranscalc::setupCoupledMicrostrip()
{
  transWidgets[3]->subParams->addDoubleProperty("Er",tr("Relative Permittivity"),4.3);
  transWidgets[3]->subParams->addDoubleProperty("Mur",tr("Relative Permeability"),1);
  transWidgets[3]->subParams->addDoubleProperty("H",tr("Height of Substrate"),8.27,Units::Length,Units::mil);
  transWidgets[3]->subParams->addDoubleProperty("H_t",tr("Height of Box Top"),1e20,Units::Length,Units::mil);
  transWidgets[3]->subParams->addDoubleProperty("T",tr("Strip Thickness"),0.68,Units::Length,Units::mil);
  transWidgets[3]->subParams->addDoubleProperty("Cond",tr("Strip Conductivity"),4.1e7);
  transWidgets[3]->subParams->addDoubleProperty("Tand",tr("Dielectric Loss Tangent"),0);
  transWidgets[3]->subParams->addDoubleProperty("Rough",tr("Conductor Roughness"),0,Units::Length,Units::mil);

  transWidgets[3]->comParams->addDoubleProperty("Freq",tr("Frequency"),10,Units::Frequency,Units::GHz);

  transWidgets[3]->phyParams->addDoubleProperty("W",tr("Line Width"),8.66,Units::Length,Units::mil);
  transWidgets[3]->phyParams->addDoubleProperty("S",tr("Gap Width"),5.31,Units::Length,Units::mil);
  transWidgets[3]->phyParams->addDoubleProperty("L",tr("Length"),1000.0,Units::Length,Units::mil);

  transWidgets[3]->eleParams->addDoubleProperty("Z0e",tr("Even-Mode Impedance"),0,Units::Resistance,Units::Ohm);
  transWidgets[3]->eleParams->addDoubleProperty("Z0o",tr("Odd-Mode Impedance"),0,Units::Resistance,Units::Ohm);
  transWidgets[3]->eleParams->addDoubleProperty("Ang_l",tr("Electrical Length"),0,Units::Angle,Units::Deg);

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


void QucsTranscalc::slotAbout()
{
  QMessageBox::about(this, tr("About..."),
                     "QucsTranscalc Version " PACKAGE_VERSION "\n"+
                         tr("Transmission Line Calculator for Qucs\n")+
                         tr("Copyright (C) 2001 by Gopal Narayanan\n")+
                         tr("Copyright (C) 2002 by Claudio Girardi\n")+
                         tr("Copyright (C) 2005 by Stefan Jahn\n")+
                         "\nThis is free software; see the source for copying conditions."
                         "\nThere is NO warranty; not even for MERCHANTABILITY or "
                         "\nFITNESS FOR A PARTICULAR PURPOSE.");
}

void QucsTranscalc::slotSelectType (int Type)
{
  switch (Type) {
    case 0:
      tranPix->setPixmap (QPixmap (QucsSettings.BitmapDir + "microstrip.png"));
      break;
    case 1:
      tranPix->setPixmap (QPixmap (QucsSettings.BitmapDir + "rectwaveguide.png"));
      break;
    case 2:
      tranPix->setPixmap (QPixmap (QucsSettings.BitmapDir +  "coax.png"));
      break;
    case 3:
      tranPix->setPixmap (QPixmap (QucsSettings.BitmapDir +  "c_microstrip.png"));
      break;
    default:
      return;
  }
  statBar->showMessage(tr("Ready."));
}

int QucsTranscalc::currentModeIndex() const
{
  return tranType->currentIndex();
}

void QucsTranscalc::slotAnalyze()
{
  transWidgets[currentModeIndex()]->line->analyze();
  statBar->showMessage(tr("Values are consistent."));
}

void QucsTranscalc::slotSynthesize()
{
  transWidgets[currentModeIndex()]->line->synthesize();
  statBar->showMessage(tr("Values are consistent."));
}

// Load transmission line values from the given file.
bool QucsTranscalc::loadFile(QString fname)
{
  QFile file(QDir::convertSeparators (fname));
  if(!file.open(QIODevice::ReadOnly)) return false; // file doesn't exist

  QTextStream stream(&file);
  QString Line, Name, Unit;
  double Value;

  while(!stream.atEnd()) {
    Line = stream.readLine();
    for (int i = 0; i < 4; i++)
    {
      if (Line == "<" + QString(transWidgets[i]->line->description) + ">")
      {
        tranType->setCurrentIndex(i);
        while(!stream.atEnd())
        {
          Line = stream.readLine();
          if (Line == "</" + QString(transWidgets[i]->line->description) + ">")
            break;
          Line = Line.simplified();
          Name = Line.section(' ',0,0);
          Value = Line.section(' ',1,1).toDouble();
          Unit = Line.section(' ',2,2);
	  PropertyBox *paramExist = transWidgets[i]->boxWithProperty(Name);
	  Q_ASSERT(paramExist != 0l);
	  int intUnit = Units::toInt(Unit);
          bool hasUnit = (intUnit != Units::None);
	  if(hasUnit)
            paramExist->setDoubleValue(Name,Value,intUnit);
          else
            paramExist->setDoubleValue(Name,Value);
        }
        break;
      }
    }
  }
  file.close();
  return true;
}

// Saves current transmission line values into the given file.
bool QucsTranscalc::saveFile(QString fname) {
  QFile file (QDir::convertSeparators (fname));
  if(!file.open (QIODevice::WriteOnly)) return false; // file not writable
  QTextStream stream (&file);

  // some lines of documentation
  stream << "# QucsTranscalc " << PACKAGE_VERSION << "  " << fname << "\n";
  stream << "#   Generated on " << QDate::currentDate().toString()
      << " at " << QTime::currentTime().toString() << ".\n";
  stream << "#   It is not suggested to edit the file, use QucsTranscalc "
      << "instead.\n\n";

  int mode = tranType->currentIndex();
  stream << "<" << transWidgets[mode]->line->description << ">\n";
  saveToStream(stream);
  stream << "</" << transWidgets[mode]->line->description << ">\n";
  file.close ();
  return true;
}

void QucsTranscalc::saveToStream(QTextStream &stream)
{
  int mode = tranType->currentIndex();
  stream << *(transWidgets[mode]->subParams);
  stream << *(transWidgets[mode]->comParams);
  stream << *(transWidgets[mode]->phyParams);
  stream << *(transWidgets[mode]->eleParams);
}

void QucsTranscalc::slotFileLoad()
{
  statBar->showMessage(tr("Loading file..."));
  QString s = QFileDialog::getOpenFileName(this,tr("Enter a Filename"),QucsWorkDir.path(),
                                           tr("Transcalc File")+" (*.trc)");
  if (!s.isEmpty())  {
    QucsWorkDir.setPath(QDir::cleanPath(s));
    if (!loadFile (s)) {
      QMessageBox::critical (this, tr("Error"),
                             tr("Cannot load file:")+" '"+s+"'!");
    }
  }
  else statBar->showMessage(tr("Loading aborted."), 2000);

  statBar->showMessage(tr("Ready."));
}

void QucsTranscalc::slotFileSave()
{
  statBar->showMessage(tr("Saving file..."));

  QString s = QFileDialog::getSaveFileName(this,tr("Enter a Filename"), QucsWorkDir.path(),
                                           tr("Transcalc File")+" (*.trc)");
  if (!s.isEmpty())  {
    QucsWorkDir.setPath(QDir::cleanPath(s));
    if (!saveFile (s)) {
      QMessageBox::critical (this, tr("Error"),
                             tr("Cannot save file:")+" '"+s+"'!");
    }
  }
  else statBar->showMessage(tr("Saving aborted."), 2000);

  statBar->showMessage(tr("Ready."));
}

void QucsTranscalc::slotHelpIntro()
{
  HelpDialog *d = new HelpDialog(this);
  d->show();
}

void QucsTranscalc::slotOptions()
{
  OptionsDialog *d = new OptionsDialog(this);
  d->exec();
}

void QucsTranscalc::slotCopyToClipboard()
{
  int created = 0;
  QString s = "<Qucs Schematic " PACKAGE_VERSION ">\n";

  // create microstrip schematic
  if (tranType->currentIndex() == 0) {
    TransWidgets *tw = transWidgets[0];
    s += "<Components>\n";
    s += "  <Pac P1 1 90 150 -74 -26 1 1 \"1\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
    s +="  <Pac P2 1 270 150 18 -26 0 1 \"2\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
    s += "  <GND * 1 90 180 0 0 0 0>\n";
    s += "  <GND * 1 270 180 0 0 0 0>\n";
    s += QString("  <SUBST SubstTC1 1 390 140 -30 24 0 0 \"%1\" 1 \"%2 mm\" 1 \"%3 um\" 1 \"%4\" 1 \"%5\" 1 \"%6\" 1>\n").
      arg(tw->subParams->doubleValue("Er")).
        arg(tw->subParams->doubleValue("H", Units::mm)).
        arg(tw->subParams->doubleValue("T", Units::um)).
        arg(tw->subParams->doubleValue("Tand")).
        arg(1 / tw->subParams->doubleValue("Cond")).
        arg(tw->subParams->doubleValue("Rough",Units::m));
    s += "  <.SP SPTC1 1 90 240 0 51 0 0 ";
    double freq = tw->comParams->doubleValue("Freq",Units::GHz);
    if (freq > 0)
      s += QString("\"log\" 1 \"%1 GHz\" 1 \"%2 GHz\" 1 ").
          arg(freq / 10).arg(freq * 10);
    else
      s += "\"lin\" 1 \"0 GHz\" 1 \"10 GHz\" 1 ";
    s += "\"51\" 1 \"no\" 0 \"1\" 0 \"2\" 0>\n";
    s += QString("  <MLIN MSTC1 1 180 100 -26 15 0 0 \"SubstTC1\" 1 \"%1 mm\" 1 \"%2 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"26.85\" 0>\n").
        arg(tw->phyParams->doubleValue("W",Units::mm)).
        arg(tw->phyParams->doubleValue("L",Units::mm));
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
  else if (tranType->currentIndex() == 3) {
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
      arg(tw->subParams->doubleValue("H", Units::mm)).
        arg(tw->subParams->doubleValue("T", Units::um)).
        arg(tw->subParams->doubleValue("Tand")).
        arg(1 / tw->subParams->doubleValue("Cond")).
        arg(tw->subParams->doubleValue("Rough", Units::m));
    s += "  <.SP SPTC1 1 100 290 0 51 0 0 ";
    double freq = tw->comParams->doubleValue("Freq", Units::GHz);
    if (freq > 0)
      s += QString("\"log\" 1 \"%1 GHz\" 1 \"%2 GHz\" 1 ").
          arg(freq / 10).arg(freq * 10);
    else
      s += "\"lin\" 1 \"0 GHz\" 1 \"10 GHz\" 1 ";
    s += "\"51\" 1 \"no\" 0 \"1\" 0 \"2\" 0>\n";
    s += QString("  <MCOUPLED MSTC1 1 190 110 -26 37 0 0 \"SubstTC1\" 1 \"%1 mm\" 1 \"%2 mm\" 1 \"%3 mm\" 1 \"Kirschning\" 0 \"Kirschning\" 0 \"26.85\" 0>\n").
        arg(tw->phyParams->doubleValue("W", Units::mm)).
        arg(tw->phyParams->doubleValue("L", Units::mm)).
        arg(tw->phyParams->doubleValue("S", Units::mm));
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
  else if (tranType->currentIndex() == 2) {
    TransWidgets *tw = transWidgets[2];
    s += "<Components>\n";
    s += "  <Pac P1 1 90 150 -74 -26 1 1 \"1\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
    s +="  <Pac P2 1 270 150 18 -26 0 1 \"2\" 1 \"50 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0 \"26.85\" 0>\n";
    s += "  <GND * 1 90 180 0 0 0 0>\n";
    s += "  <GND * 1 270 180 0 0 0 0>\n";
    s += "  <.SP SPTC1 1 90 240 0 51 0 0 ";
    double freq = tw->comParams->doubleValue("Freq", Units::GHz);
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
        arg(tw->phyParams->doubleValue("dout", Units::mm)).
        arg(tw->phyParams->doubleValue("din", Units::mm)).
        arg(tw->phyParams->doubleValue("L", Units::mm)).
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
  if (created)
    statBar->showMessage(tr("Schematic copied into clipboard."), 2000);
  else
    statBar->showMessage(tr("Transmission line type not available."), 2000);
}

QString QucsTranscalc::currentModeString()
{
  return transWidgets[currentModeIndex()]->line->description;
}

void QucsTranscalc::setCurrentMode(const QString& mode)
{
  for(int i=0;i<4;i++) {
    if(transWidgets[i]->line->description == mode) {
      tranType->setCurrentIndex(i);
      return;
    }
  }
}
