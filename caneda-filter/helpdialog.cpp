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

#include <QtGui/qlayout.h>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
HelpDialog::HelpDialog(QWidget *parent)
                     : QDialog(parent)
{
  setWindowTitle(tr("Qucs Filter Help"));


  // --------  set help text into dialog  ------------
  QString s(tr("QucsFilter is a filter synthesis program. "
	       "To create a filter, simply enter all "
	       "parameters and press the big button at the "
	       "bottom of the main window. Immediatly, the "
	       "schematic of the filter is calculated and "
	       "put into the clipboard. Now go to Qucs, "
	       "open an empty schematic and press "
	       "CTRL-V (paste from clipboard). The filter "
	       "schematic can now be inserted and "
	       " simulated. Have lots of fun!"));


  // --------  create dialog widgets  ------------
  resize(250, 230);

  QVBoxLayout *vLayout = new QVBoxLayout(this);

  Text = new QTextEdit(s,this);
  Text->setReadOnly(true);
  Text->setMinimumSize(200,200);
  vLayout->addWidget(Text);

  QHBoxLayout *h = new QHBoxLayout();
  vLayout->addLayout(h);

  h->addStretch( 5 );
  QPushButton *ButtonClose = new QPushButton(tr("Close"));
  connect(ButtonClose, SIGNAL(clicked()),this, SLOT(accept()));
  ButtonClose->setFocus();
  h->addStretch( 5 );
}
