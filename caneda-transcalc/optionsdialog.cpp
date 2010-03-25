/***************************************************************************
                              optionsdialog.cpp
                             --------------------
    begin                : Sun Apr 03 2005
    copyright            : (C) 2005 by Stefan Jahn
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

#include "optionsdialog.h"
#include "qucstrans.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include "qucs-tools/propertygrid.h"

OptionsDialog::OptionsDialog(QWidget *parent)
                     : QDialog(parent)
{
  setWindowTitle("QucsTranscalc "+tr("Options"));

  // --------  create dialog widgets  ------------
  QVBoxLayout *vLayout = new QVBoxLayout(this);
  vLayout->setMargin(3);
  vLayout->setSpacing(3);

  QGroupBox * h = new QGroupBox(tr("Units"), this);
  vLayout->addWidget(h);
  QGridLayout *l = new QGridLayout(h);
  l->setSpacing(3);
  QLabel * lfr = new QLabel(tr("Frequency"));
  lfr->setAlignment (Qt::AlignRight);
  QLabel * lle = new QLabel(tr("Length"));
  lle->setAlignment (Qt::AlignRight);
  QLabel * lre = new QLabel(tr("Resistance"));
  lre->setAlignment (Qt::AlignRight);
  QLabel * lan = new QLabel(tr("Angle"));
  lan->setAlignment (Qt::AlignRight);
  l->addWidget(lfr,0,0);
  l->addWidget(lle,1,0);
  l->addWidget(lre,2,0);
  l->addWidget(lan,3,0);
  
  for(int i=0; i < 4; i++) {
    units[i] = new QComboBox();
    l->addWidget(units[i],i,1);
  }
  
  units[0]->addItems(Units::freqList);
  units[1]->addItems(Units::lenList);
  units[2]->addItems(Units::resList);
  units[3]->addItems(Units::angleList);
  
  /*units[0]->setCurrentIndex (QucsSettings.freq_unit);
  units[1]->setCurrentIndex (QucsSettings.length_unit);
  units[2]->setCurrentIndex (QucsSettings.res_unit);
  units[3]->setCurrentIndex (QucsSettings.ang_unit);*/
  
  QHBoxLayout * h2 = new QHBoxLayout();
  vLayout->addLayout(h2);

  QPushButton *ButtonSave = new QPushButton(tr("Save as Default"));
  connect(ButtonSave, SIGNAL(clicked()), SLOT(slotSave()));

  QPushButton *ButtonClose = new QPushButton(tr("Dismiss"));
  connect(ButtonClose, SIGNAL(clicked()), SLOT(slotClose()));
  
  h2->addWidget(ButtonSave);
  h2->addWidget(ButtonClose);
  ButtonClose->setFocus();
}

OptionsDialog::~OptionsDialog()
{
}

void OptionsDialog::slotClose()
{
  accept();
}

void OptionsDialog::slotSave()
{
  QucsSettings.freq_unit = units[0]->currentIndex();
  QucsSettings.length_unit = units[1]->currentIndex();
  QucsSettings.res_unit = units[2]->currentIndex();
  QucsSettings.ang_unit = units[3]->currentIndex();
  accept();
}
