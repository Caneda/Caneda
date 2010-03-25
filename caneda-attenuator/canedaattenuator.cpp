/****************************************************************************
**     Qucs Attenuator Synthesis
**     qucsattenuator.cpp
**
**
**
**
**
**
**
*****************************************************************************/

#include "qucs-tools/global.h"
#include "attenuatorfunc.h"
#include "qucsattenuator.h"
#include "helpdialog.h"
#include <QtCore/QString>

#include <QtGui/QPixmap>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QValidator>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QStatusBar>
#include <QtGui/QFrame>
#include <QtCore/QTimer>

QucsAttenuator::QucsAttenuator()
{
   setWindowIcon(QPixmap(Qucs::bitmapDirectory() + "big.qucs.xpm"));
   setWindowTitle(tr("Qucs Attenuator") + " " + Qucs::version);

  QMenuBar *bar = new QMenuBar(this);
  QMenu *fileMenu = bar->addMenu(tr("&File"));
  bar->addSeparator ();
  QMenu *helpMenu = bar->addMenu(tr("&Help"));
  bar->setFixedHeight(bar->sizeHint().height());
  
  fileMenu->addAction(tr("E&xit"), this, SLOT(close()), Qt::CTRL+Qt::Key_Q);
  
  helpMenu->addAction(tr("Help..."), this, SLOT(slotHelpIntro()), Qt::Key_F1);
  helpMenu->addSeparator();
  helpMenu->addAction(tr("&About QucsAttenuator..."), this, SLOT(slotHelpAbout()), 0);
  helpMenu->addAction(tr("About Qt..."), this, SLOT(slotHelpAboutQt()), 0);
  
  statusBar = new QStatusBar(this);
  statusBar->setFixedHeight(statusBar->sizeHint().height());
  // First set up layouts and button groups
  QVBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->setMargin(0);
  topLayout->addWidget(bar);
  QFrame *f = new QFrame(this);
  topLayout->addWidget(f);
  //topLayout->addLayout(mainLayout);
  topLayout->addWidget(statusBar);
  QHBoxLayout * mainLayout = new QHBoxLayout(f);
  QGroupBox * TopoGroup = new QGroupBox (tr("Topology"));
  TopoGroup->setAlignment(Qt::AlignHCenter);
  
  mainLayout->addWidget(TopoGroup);
  QVBoxLayout *topoLayout = new QVBoxLayout(TopoGroup);
  ComboTopology = new QComboBox(TopoGroup);//Topology Combobox
  topoLayout->addWidget(ComboTopology);
  pixTopology = new QLabel(TopoGroup);//Pixmap for Topology
  pixTopology->setMinimumSize(250,175);
  topoLayout->addWidget(pixTopology);

  QVBoxLayout *ioLayout = new QVBoxLayout();
  mainLayout->addLayout(ioLayout);
  
  QGroupBox * InputGroup = new QGroupBox (tr("Input"));
  InputGroup->setAlignment(Qt::AlignHCenter);
  ioLayout->addWidget(InputGroup);
  Calculate = new QPushButton(tr("Calculate and put into Clipboard"));
  
  ioLayout->addWidget(Calculate);
  QGroupBox * OutputGroup = new QGroupBox (tr("Output"));
  OutputGroup->setAlignment(Qt::AlignHCenter);
  ioLayout->addWidget(OutputGroup);
  
  QGridLayout * inpLayout = new QGridLayout(InputGroup);
  QGridLayout * outLayout = new QGridLayout(OutputGroup);

  // Set up the widgets now
  ComboTopology->addItems(QStringList() << tr("Pi") << tr("Tee") << tr("Bridged Tee"));
  connect(ComboTopology, SIGNAL(activated(int)),this,SLOT(slotTopologyChanged()));
    
  pixTopology->setPixmap(QPixmap(Qucs::bitmapDirectory() + "att_pi.png"));

  IntVal = new QIntValidator(this);
  DoubleVal = new QDoubleValidator(this);

  LabelAtten = new QLabel(tr("Attenuation:"));
  lineEdit_Attvalue = new QLineEdit();
  lineEdit_Attvalue->setValidator(DoubleVal);
  QLabel *Label1 = new QLabel(tr("dB"));
  inpLayout->addWidget(LabelAtten,0,0);
  inpLayout->addWidget(lineEdit_Attvalue,0,1);
  inpLayout->addWidget(Label1,0,2);
  
  
  LabelImp1 = new QLabel(tr("Zin:"));
  lineEdit_Zin = new QLineEdit();
  lineEdit_Zin->setValidator(DoubleVal);
  connect(lineEdit_Zin, SIGNAL(textChanged(const QString&)), this,
	  SLOT(slotSetText_Zin(const QString&)) );
  QLabel *Label2 = new QLabel(tr("Ohm"), InputGroup);
  inpLayout->addWidget(LabelImp1,1,0);
  inpLayout->addWidget(lineEdit_Zin,1,1);
  inpLayout->addWidget(Label2,1,2);

  LabelImp2 = new QLabel(tr("Zout:"));
  lineEdit_Zout = new QLineEdit();
  lineEdit_Zout->setValidator(DoubleVal);
  connect(lineEdit_Zout, SIGNAL(textChanged(const QString&)), this,
	  SLOT(slotSetText_Zout(const QString&)) );
  QLabel *Label3 = new QLabel(tr("Ohm"));
  inpLayout->addWidget(LabelImp2,2,0);
  inpLayout->addWidget(lineEdit_Zout,2,1);
  inpLayout->addWidget(Label3,2,2);
  
  connect(Calculate, SIGNAL(clicked()), this, SLOT(slotCalculate()));

  
  LabelR1 = new QLabel(tr("R1:"));
  lineEdit_R1 = new QLineEdit(tr("--"));
  lineEdit_R1->setReadOnly(true);
  QLabel *Label4 = new QLabel(tr("Ohm"));
  outLayout->addWidget(LabelR1, 0,0);
  outLayout->addWidget(lineEdit_R1, 0,1);
  outLayout->addWidget(Label4, 0,2);

  LabelR2 = new QLabel(tr("R2:"));
  lineEdit_R2 = new QLineEdit(tr("--"));
  lineEdit_R2->setReadOnly(true);
  QLabel *Label5 = new QLabel(tr("Ohm"));
  outLayout->addWidget(LabelR2, 1,0);
  outLayout->addWidget(lineEdit_R2, 1,1);
  outLayout->addWidget(Label5, 1,2);

  LabelR3 = new QLabel(tr("R3:"));
  lineEdit_R3 = new QLineEdit(tr("--"));
  lineEdit_R3->setReadOnly(true);
  LabelR3_Ohm = new QLabel(tr("Ohm"));
  outLayout->addWidget(LabelR3, 2,0);
  outLayout->addWidget(lineEdit_R3, 2,1);
  outLayout->addWidget(LabelR3_Ohm, 2,2);
  
  // Make sure this is at last, else may cause seg fault
  readSettings();
}

QucsAttenuator::~QucsAttenuator()
{
  delete IntVal;
  delete DoubleVal;
}

void QucsAttenuator::slotHelpIntro()
{
  HelpDialog *d = new HelpDialog(this);
  d->show();
}

void QucsAttenuator::slotHelpAboutQt()
{
      QMessageBox::aboutQt(this, tr("About Qt"));
}

void QucsAttenuator::slotHelpAbout()
{
    QMessageBox::about(this, tr("About..."),
		       QString("QucsAttenuator Version " PACKAGE_VERSION)+
		       tr("\nAttenuator synthesis program\n")+
		       tr("Copyright (C) 2006 by")+" Toyoyuki Ishikawa"
		       "\n"+
		       tr("Copyright (C) 2006 by")+" Stefan Jahn"
		       "\n"
		       "\nThis is free software; see the source for copying conditions."
		       "\nThere is NO warranty; not even for MERCHANTABILITY or "
		       "\nFITNESS FOR A PARTICULAR PURPOSE.\n\n");
}

void QucsAttenuator::slotSetText_Zin( const QString &text )
{
  if(ComboTopology->currentIndex() == BRIDGE_TYPE) {
    lineEdit_Zout->blockSignals( TRUE );
    lineEdit_Zout->setText( text );
    lineEdit_Zout->blockSignals( FALSE );
  }
}

void QucsAttenuator::slotSetText_Zout( const QString &text )
{
  if(ComboTopology->currentIndex() == BRIDGE_TYPE) {
    lineEdit_Zin->blockSignals( TRUE );
    lineEdit_Zin->setText( text );
    lineEdit_Zin->blockSignals( FALSE );
  }
}

void QucsAttenuator::slotTopologyChanged()
{
  switch(ComboTopology->currentIndex())
    {
    case PI_TYPE:
      pixTopology->setPixmap(QPixmap(Qucs::bitmapDirectory() + "att_pi.png"));
      LabelR2->setText(tr("R2:"));
      LabelR3->show();
      lineEdit_R3->show();
      LabelR3_Ohm->show();
      break;
    case TEE_TYPE:
      pixTopology->setPixmap(QPixmap(Qucs::bitmapDirectory() + "att_tee.png"));
      LabelR2->setText(tr("R2:"));
      LabelR3->show();
      lineEdit_R3->show();
      LabelR3_Ohm->show();
      break;
    case BRIDGE_TYPE:
      pixTopology->setPixmap(QPixmap(Qucs::bitmapDirectory() + "att_bridge.png"));
      LabelR2->setText(tr("R4:"));
      LabelR3->hide();
      lineEdit_R3->hide();
      LabelR3_Ohm->hide();
      lineEdit_Zout->setText( lineEdit_Zin->text() );
      break;
    }
}

void QucsAttenuator::slotCalculate()
{
    QUCS_Att qatt;
    int result;
    QString * s = NULL;
    struct tagATT Values;


    Values.Topology = ComboTopology->currentIndex();
    Values.Attenuation = lineEdit_Attvalue->text().toDouble();
    Values.Zin = lineEdit_Zin->text().toDouble();
    Values.Zout = lineEdit_Zout->text().toDouble();
    result = qatt.Calc(&Values);

    if(result != -1)
    {
      statusBar->showMessage(tr("Result:")+" "+tr("Success!"));
      lineEdit_R1->setText(QString::number(Values.R1, 'f', 1));
      lineEdit_R2->setText(QString::number(Values.R2, 'f', 1));
      lineEdit_R3->setText(QString::number(Values.R3, 'f', 1));
      
      s = qatt.createSchematic(&Values);
      if(!s) return;
      
      QClipboard *cb = QApplication::clipboard();
      cb->setText(*s);
      delete s;
    }
    else
    {
      statusBar->showMessage(tr("Error: Set Attenuation more than %1 dB").arg(QString::number(Values.MinimumATT, 'f', 3)));
      lineEdit_R1->setText(tr("--"));
      lineEdit_R2->setText(tr("--"));
      lineEdit_R3->setText(tr("--"));
    }

}

void QucsAttenuator::readSettings()
{
   QSettings settings;

   settings.beginGroup("MainWindow");
   resize(settings.value("size", QSize(400, 400)).toSize());
   move(settings.value("pos", QPoint(200, 200)).toPoint());
   settings.endGroup();

   settings.beginGroup("SavedInputValues");
   lineEdit_Attvalue->setText(settings.value("Attenuation","1").toString());
   lineEdit_Zin->setText(settings.value("Zin","50").toString());
   lineEdit_Zout->setText(settings.value("Zout","50").toString());
   ComboTopology->setCurrentIndex(settings.value("Topology",PI_TYPE).toInt());
   slotTopologyChanged();
   settings.endGroup();
}

void QucsAttenuator::writeSettings()
{
   QSettings settings;

   settings.beginGroup("MainWindow");
   settings.setValue("size", size());
   settings.setValue("pos", pos());
   settings.endGroup();

   settings.beginGroup("SavedInputValues");
   settings.setValue("Attenuation",lineEdit_Attvalue->text());
   settings.setValue("Zin",lineEdit_Zin->text());
   settings.setValue("Zout",lineEdit_Zout->text());
   settings.setValue("Topology",ComboTopology->currentIndex());
   settings.endGroup();
}

void QucsAttenuator::closeEvent(QCloseEvent *e)
{
   writeSettings();
   QWidget::closeEvent(e);
}
