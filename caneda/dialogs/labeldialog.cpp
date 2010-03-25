/***************************************************************************
                               labeldialog.cpp
                              ----------------
    begin                : Thu Dec 09 2004
    copyright            : (C) 2004 by Michael Margraf
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

#include "labeldialog.h"
#include "../wirelabel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qvalidator.h>


LabelDialog::LabelDialog(WireLabel *pl, QWidget *parent)
                     : QDialog(parent, 0, true)
{
  setCaption(tr("Insert Nodename"));

  pLabel = pl;
  gbox = new QGridLayout(this,4,3,5,5);

  // valid expression for LineEdit: alpha-numeric, but must start with
  // letter and never two "_" together
  Expr1.setPattern("[a-zA-Z]([0-9a-zA-Z]|_(?!_))+");
  Validator1 = new QRegExpValidator(Expr1, this);

  QLabel *Label1 = new QLabel(tr("Enter the label:"),this);
  gbox->addWidget(Label1,0,0);

  NodeName = new QLineEdit(this);
  if(pLabel)  NodeName->setText(pLabel->Name);
  NodeName->setValidator(Validator1);
  gbox->addMultiCellWidget(NodeName,1,1,0,3);

  Expr2.setPattern("[^\"=]+");  // valid expression for LineEdit
  Validator2 = new QRegExpValidator(Expr2, this);

  Label2 = new QLabel(tr("Initial node voltage:"),this);
  gbox->addWidget(Label2,2,0);
  InitValue = new QLineEdit(this);
  if(pLabel)  InitValue->setText(pLabel->initValue);
  InitValue->setValidator(Validator2);
  gbox->addMultiCellWidget(InitValue,2,2,1,3);

  ButtonMore = new QPushButton(tr("Less..."),this);
  gbox->addWidget(ButtonMore,3,1);
  ButtonOk = new QPushButton(tr("Ok"),this);
  gbox->addWidget(ButtonOk,3,2);
  ButtonCancel = new QPushButton(tr("Cancel"),this);
  gbox->addWidget(ButtonCancel,3,3);

  for(;;) {
    if(pLabel)  if(!pLabel->initValue.isEmpty())  break;
    Label2->hide();
    InitValue->hide();
    ButtonMore->setText(tr("More..."));
    break;
  }

  connect(ButtonMore, SIGNAL(clicked()), SLOT(slotExtend()));
  connect(ButtonOk, SIGNAL(clicked()), SLOT(slotOk()));
  connect(ButtonCancel, SIGNAL(clicked()), SLOT(slotCancel()));

  ButtonOk->setDefault(true);
  setFocusProxy(NodeName);
}

LabelDialog::~LabelDialog()
{
  delete gbox;
  delete Validator1;
  delete Validator2;
}

void LabelDialog::slotExtend()
{
  if(Label2->isHidden()) {
    Label2->setHidden(false);
    InitValue->setHidden(false);
    ButtonMore->setText(tr("Less..."));
  }
  else {
    Label2->hide();
    InitValue->hide();
    ButtonMore->setText(tr("More..."));
  }

}

void LabelDialog::slotCancel()
{
  done(0);
}

void LabelDialog::slotOk()
{
  NodeName->setText(NodeName->text().stripWhiteSpace());
  InitValue->setText(InitValue->text().stripWhiteSpace());

  bool changed = false;
  if(pLabel) {
    if(pLabel->Name != NodeName->text()) {
      pLabel->Name = NodeName->text();
      changed = true;
    }

    if(pLabel->initValue != InitValue->text()) {
      pLabel->initValue = InitValue->text();
      changed = true;
    }
  }

  if(changed) done(2);
  else done(1);
}
