/***************************************************************************
                               helpdialog.cpp
                             ------------------
    begin                : Fri Mar 04 2005
    copyright            : (C) 2005 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
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

#include "helpdialog.h"

#include <QtCore/QString>
#include <QtGui/qlayout.h>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>


HelpDialog::HelpDialog(QWidget *parent)
   : QDialog(parent)
{
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("Qucs Attenuator Help"));


  // --------  set help text into dialog  ------------
  QString s(tr("QucsAttenuator is an attenuator synthesis program. "
	       "To create a attenuator, simply enter all "
	       "the input parameters and press the calculation button. "
	       "Immediatly, the "
	       "schematic of the attenuator is calculated and "
	       "put into the clipboard. Now go to Qucs, "
	       "open an schematic and press "
	       "CTRL-V (paste from clipboard). The attenuator "
	       "schematic can now be inserted. "
	       "Have lots of fun!"));


  // --------  create dialog widgets  ------------
  resize(250, 230);

  vLayout = new QVBoxLayout(this);

  Text = new QTextEdit(s, this);
  Text->setReadOnly(true);
  Text->setMinimumSize(200,200);
  vLayout->addWidget(Text);

  QHBoxLayout *h = new QHBoxLayout;
  vLayout->addLayout(h);

  h->insertStretch(0,40);

  QPushButton *ButtonClose = new QPushButton(tr("Close"));
  connect(ButtonClose, SIGNAL(clicked()), this, SLOT(accept()));
  ButtonClose->setFocus();
  h->addWidget(ButtonClose);
  h->insertStretch(2,40);
}

HelpDialog::~HelpDialog()
{
   delete vLayout;
}
