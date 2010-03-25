/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "helpdialog.h"
#include <QtGui/QTextEdit>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

HelpDialog::HelpDialog(const QString& cap,QWidget *p) : QDialog(p)
{
  setWindowTitle(cap);
  QVBoxLayout *l = new QVBoxLayout(this);
  edit = new QTextEdit();
  l->addWidget(edit);
  QHBoxLayout *h = new QHBoxLayout();
  l->addLayout(h);
  h->addStretch(5);
  QPushButton *pb = new QPushButton(tr("Dismiss"));
  pb->setFocus();
  h->addWidget(pb);
  h->addStretch(5);
  connect(pb,SIGNAL(clicked()),this,SLOT(accept()));
}

void HelpDialog::setText(const QString &text)
{
  edit->setPlainText(text);
}
