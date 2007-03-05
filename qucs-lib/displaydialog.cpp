/***************************************************************************
                               displaydialog.cpp
                              -------------------
    begin                : Sat May 28 2005
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

#include "displaydialog.h"

#include <QtGui/QLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>


DisplayDialog::DisplayDialog(QWidget *parent)
                     : QDialog(parent)
{
  vLayout = new QVBoxLayout(this);
  setAttribute( Qt::WA_DeleteOnClose  );
  Text = new QTextEdit(this);
  //Text->setTextFormat(Qt::PlainText);
  Text->setReadOnly(true);
  Text->setMinimumSize(200, 100);
  vLayout->addWidget(Text);

  QHBoxLayout *h = new QHBoxLayout(this);
  vLayout->addLayout(h);

	QWidget *w1 = new QWidget(this);
	h->addWidget( w1 );
  h->setStretchFactor(w1,5); // stretchable placeholder

  QPushButton *ButtonClose = new QPushButton(tr("Close"), this);
	h->addWidget( ButtonClose );
  connect(ButtonClose, SIGNAL(clicked()), SLOT(slotClose()));
  ButtonClose->setFocus();

	QWidget *w2= new QWidget(this);
	h->addWidget( w2 );
  h->setStretchFactor(w2,5); // stretchable placeholder
}

DisplayDialog::~DisplayDialog()
{
  delete vLayout;
}

// ************************************************************
void DisplayDialog::slotClose()
{
  accept();
}
