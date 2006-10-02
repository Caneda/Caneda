/***************************************************************************
                               helpdialog.cpp
                             ------------------
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

#include "helpdialog.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>


HelpDialog::HelpDialog(QWidget *parent)
                     : QDialog(parent)
{
  setWindowTitle("QucsTranscalc "+tr("Help"));

  // --------  set help text into dialog  ------------
  QString s(tr("QucsTranscalc is an analysis and synthesis tool for "
       "calculating the electrical and physical properties of "
       "different kinds of RF and microwave transmission lines.")+
       "\n\n"+
       tr("For each type of transmission line, using dialog boxes, you can "
       "enter values for the various parameters, and either calculate its "
       "electrical properties, or use the given electrical requirements to "
       "synthesize physical parameters of the required transmission line.")+
       "\n\n"+
       tr("Available transmission lines are: Microstrip, Rectangular "
      "Waveguide, Coaxial Line and Coupled Microstrips."));


  // --------  create dialog widgets  ------------
  resize(350, 230);

  QVBoxLayout *vLayout = new QVBoxLayout(this);
  vLayout->setMargin(3);
  vLayout->setSpacing(3);

  Text = new QTextEdit(s, this);
  //Text->setTextFormat(Qt::PlainText);
  Text->setReadOnly(true);
  Text->setMinimumSize(300,200);
  vLayout->addWidget(Text);

  QPushButton *ButtonClose = new QPushButton(tr("Dismiss"), this);
  vLayout->addWidget(ButtonClose);
  connect(ButtonClose, SIGNAL(clicked()), SLOT(slotClose()));
  ButtonClose->setFocus();
}

HelpDialog::~HelpDialog()
{
  //delete vLayout;
}

void HelpDialog::slotClose()
{
  accept();
}
