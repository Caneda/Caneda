/***************************************************************************
                              librarydialog.cpp
                             -------------------
    begin                : Sun Jun 04 2006
    copyright            : (C) 2006 by Michael Margraf
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

#include "librarydialog.h"
#include "qucslib.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QLabel>
#include <Qt/qfile.h>
#include <Qt/qlabel.h>
#include <Qt/qlineedit.h>
#include <Qt/qvalidator.h>
#include <Qt/qmessagebox.h>
#include <Qt/qpushbutton.h>
#include <Qt/qradiobutton.h>
#include <Qt/qbuttongroup.h>


LibraryDialog::LibraryDialog(QWidget *App_)
			: QDialog(App_)
{

  setWindowTitle(tr("Manage User Libraries"));
  setAttribute( Qt::WA_DeleteOnClose  );

  Expr.setPattern("[\\w_]+");
  Validator = new QRegExpValidator(Expr, this);

  // ...........................................................
  QVBoxLayout *all = new QVBoxLayout(this);
  all->setMargin(5);
  all->setSpacing(6);

  QLabel *lb1 = new QLabel( tr("<font color=#000FFF>Choose library:</font>"), this );
  all->addWidget( lb1 );
  listUserLib = new QListWidget(this);
  all->addWidget(listUserLib);

  QHBoxLayout *h1 = new QHBoxLayout();
  all->addLayout(h1);
  theLabel = new QLabel(tr("New Name:"), this);
	h1->addWidget( theLabel );
  NameEdit = new QLineEdit(this);
	h1->addWidget( NameEdit );
  NameEdit->setValidator(Validator);

  // ...........................................................
  QHBoxLayout *h2 = new QHBoxLayout();
  all->addLayout(h2);
  ButtDelete = new QPushButton(tr("Delete"), this);
  h2->addWidget(ButtDelete);
  connect(ButtDelete, SIGNAL(clicked()), SLOT(slotDelete()));
  ButtRename = new QPushButton(tr("Rename"), this);
  h2->addWidget(ButtRename);
  connect(ButtRename, SIGNAL(clicked()), SLOT(slotRename()));
  ButtClose = new QPushButton(tr("Close"), this);
  h2->addWidget(ButtClose);
  connect(ButtClose, SIGNAL(clicked()), SLOT(reject()));
  ButtClose->setDefault(true);

  // ...........................................................
  // insert all user libraries
  QStringList LibFiles = UserLibDir.entryList(QStringList("*.lib"), QDir::Files, QDir::Name);
  QStringList::iterator it;
  // inserts all project directories
  for(it = LibFiles.begin(); it != LibFiles.end(); it++) {
    listUserLib->addItem( ((*it).left((*it).length()-4) ) );
  }
}

LibraryDialog::~LibraryDialog()
{
  delete Validator;
}

// ---------------------------------------------------------------
// Renames selected user library.
void LibraryDialog::slotRename()
{
  if(NameEdit->text().isEmpty()) {
    QMessageBox::critical(this, tr("Error"), tr("Please insert a new library name!"));
    return;
  }

  QListWidgetItem *r = listUserLib->currentItem();
  if(r == 0) {
    QMessageBox::critical(this, tr("Error"), tr("Please choose a library!"));
    return;
  }
  QFile NewLibFile(QucsSettings.LibDir + NameEdit->text() + ".lib");
  if(NewLibFile.exists()) {
    QMessageBox::critical(this, tr("Error"), tr("A system library with this name already exists!"));
    return;
  }

  NewLibFile.setFileName(UserLibDir.absoluteFilePath(NameEdit->text() + ".lib"));
  if(NewLibFile.exists()) {
    QMessageBox::critical(this, tr("Error"), tr("A library with this name already exists!"));
    return;
  }

  QFile LibFile(UserLibDir.absoluteFilePath(r->text() + ".lib"));
  if(!LibFile.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, tr("Error"), tr("Cannot open library!"));
    return;
  }

  QByteArray FileContent = LibFile.readAll();
//  LibFile.close();

  // rename library name within file
  char *p, *Name;
  char *Config = FileContent.data();
  for(;;) {
    p = strstr(Config, "<Qucs Library ");
    if(p == 0) break;
    Name = strstr(p, " \"");
    if(Name == 0) break;
    Name += 2;
    p = strstr(Name, "\">");
    if(p == 0) break;

    if(!NewLibFile.open(QFile::WriteOnly)) {
      QMessageBox::critical(this, tr("Error"), tr("No permission to modify library!"));
      return;
    }
    int count = 0;
    count += NewLibFile.write(Config, Name-Config);
    count += NewLibFile.write(NameEdit->text().toLatin1(), NameEdit->text().length());
    count += NewLibFile.write(p, FileContent.count() - (p-Config) );
    NewLibFile.close();
    count -= FileContent.count() + NameEdit->text().length() - (p-Name);
    if(count != 0) {
      QMessageBox::critical(this, tr("Error"), tr("Writing new library not successful!"));
      return;
    }

    if(!LibFile.remove()) {
      QMessageBox::critical(this, tr("Error"), tr("Cannot delete old library."));
			listUserLib->addItem( NameEdit->text() );
      //toggleGroup->insert(new QRadioButton(NameEdit->text(), Dia_Box));
      NameEdit->clear();
      return;
    }

    r->setText(NameEdit->text());
    NameEdit->clear();
    return;
  }

  QMessageBox::critical(this, tr("Error"), tr("Library file is corrupt!"));
}

// ---------------------------------------------------------------
// Deletes the selected user library.
void LibraryDialog::slotDelete()
{

  QListWidgetItem *r = listUserLib->currentItem();
  if(r == 0) {
    QMessageBox::critical(this, tr("Error"), tr("Please choose a library!"));
    return;
  }

  QFile LibFile(UserLibDir.absoluteFilePath(r->text() + ".lib"));
  if(!LibFile.remove()) {
    QMessageBox::critical(this, tr("Error"),
                 tr("No permission to delete library \"%1\".").arg(r->text()));
    return;
  }
  delete r;
}
