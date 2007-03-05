/***************************************************************************
                               qucsedit.cpp
                              --------------
    begin                : Mon Nov 17 2003
    copyright            : (C) 2003 by Michael Margraf
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

/* Modified on 28/02/07 by Martin "Ruso" Ribelotta at porting to Qt4 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qucsedit.h"

#include <QtGui/QTextEdit>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QtGui/QImage>
#include <QtGui/QFileDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTextCharFormat>

QucsEdit::QucsEdit(const QString& FileName_, bool readOnly)
{
  // set application icon
  setWindowIcon (QPixmap(QucsSettings.BitmapDir + "big.qucs.xpm"));
  setWindowTitle("Qucs Editor " PACKAGE_VERSION " - " + tr("File: "));

  QVBoxLayout *v = new QVBoxLayout(this);

  QHBoxLayout *h = new QHBoxLayout();
  v->addLayout(h);

  QToolButton *ButtLoad = new QToolButton(this);
	h->addWidget(ButtLoad);
	QString s1 = QucsSettings.BitmapDir + "fileopen.png";
  ButtLoad->setIcon(QIcon(s1));
  connect(ButtLoad, SIGNAL(clicked()), SLOT(slotLoad()));

  QToolButton *ButtSave = new QToolButton(this);
	h->addWidget(ButtSave);
  ButtSave->setIcon(QIcon(QucsSettings.BitmapDir + "filesave.png"));
  connect(ButtSave, SIGNAL(clicked()), SLOT(slotSave()));
  ButtSave->setDisabled(readOnly);

	QWidget *gost_w1 = new QWidget( this );
	h->addWidget(gost_w1);
  h->setStretchFactor(gost_w1,5); // stretchable placeholder
  PosText = new QLabel(tr("Line: %1  -  Column: %2").arg(1).arg(1), this);
	h->addWidget( PosText );
	QWidget *gost_w2 = new QWidget( this );
	h->addWidget( gost_w2 );
  h->setStretchFactor(gost_w2,5); // stretchable placeholder

  QPushButton *ButtAbout = new QPushButton(tr("About"),this);
	h->addWidget( ButtAbout );
  connect(ButtAbout, SIGNAL(clicked()), SLOT(slotAbout()));

  QPushButton *ButtOK = new QPushButton(tr("Quit"),this);
	h->addWidget( ButtOK );
  connect(ButtOK, SIGNAL(clicked()), SLOT(slotQuit()));
  ButtOK->setFocus();

  text = new QTextEdit(this);
  //text->setTextFormat(Qt::PlainText);
  //text->setReadOnly(readOnly);
  //text->setWordWrap(QTextEdit::NoWrap);
	text->setLineWrapMode( QTextEdit::NoWrap );
	text->setWordWrapMode( QTextOption::NoWrap );
  text->setMinimumSize(300,200);
  v->addWidget(text);
  connect(text, SIGNAL(cursorPositionChanged()),
          SLOT(slotPrintCursorPosition()));

  // .................................................
  loadFile(FileName_);
}

QucsEdit::~QucsEdit()
{
}

// ************************************************************
void QucsEdit::slotPrintCursorPosition()
{
	/// Commented out until we find how to implement it.
// 	QTextCursor textCursor = text->textCursor();
//
//   PosText->setText(tr("Line: %1  -  Column: %2").arg(Para+1).arg(Pos+1));
}

// ************************************************************
void QucsEdit::slotAbout()
{
  QMessageBox::about(this, tr("About..."),
    "QucsEdit Version " PACKAGE_VERSION+
    tr("\nVery simple text editor for Qucs\n")+
    tr("Copyright (C) 2004, 2005 by Michael Margraf\n")+
    "\nThis is free software; see the source for copying conditions."
    "\nThere is NO warranty; not even for MERCHANTABILITY or "
    "\nFITNESS FOR A PARTICULAR PURPOSE.");
}

// ************************************************************
void QucsEdit::slotLoad()
{
  static QString lastDir;  // to remember last directory and file
/*
  QString s = QFileDialog::getOpenFileName(
    lastDir.isEmpty() ? QString(".") : lastDir,
    "*", this, "", tr("Enter a Filename"));
	*/
	QString s = QFileDialog::getOpenFileName(
			this, tr("Enter a Filename"), lastDir.isEmpty()? ".":lastDir, tr("Any files (*)")
																					);
  if(s.isEmpty()) return;
  lastDir = s;   // remember last directory and file
  if(!closeFile()) return;
  loadFile(s);
}

// ************************************************************
void QucsEdit::slotSave()
{
/*
	if(FileName.isEmpty()) {
    FileName = QFileDialog::getSaveFileName(".", QString::null,
	this, "", tr("Enter a Document Name"));
    if(FileName.isEmpty())  return;
  }
*/
  if (FileName.isEmpty())
		FileName = QFileDialog::getSaveFileName(this, tr("Enter a Document Name"),
		                                        ".", tr("Any files (*)"));
	if(FileName.isEmpty())
		return;
	QFile file(FileName);
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::critical(this, tr("Error"),
		tr("Cannot write file: ")+FileName);
    return;
  }

  QTextStream stream(&file);
  stream << text->toPlainText();
  text->document()->setModified(false);
  file.close();
}

// ************************************************************
void QucsEdit::slotQuit()
{
  if(!closeFile()) return;

  int tmp;
  tmp = x();		// call size and position function in order to ...
  tmp = y();		// ... set them correctly before closing the ...
  tmp = width();	// dialog !!!  Otherwise the frame of the window ...
  tmp = height();	// will not be recognized (a X11 problem).

  accept();
}

// ************************************************************
// To get all close events.
void QucsEdit::closeEvent(QCloseEvent*)
{
  slotQuit();
}

// ************************************************************
bool QucsEdit::loadFile(const QString& Name)
{
  if(Name.isEmpty()) return false;
  QFile file(Name);
  if(!file.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(this, tr("Error"),
		tr("Cannot read file: ")+Name);
    return false;
  }

  QTextStream stream(&file);
  text->setPlainText(stream.readAll());
  file.close();

  FileName = Name;
//  QFileInfo info(Name);
//  FileName = info.fileName();
  setWindowTitle("Qucs Editor " PACKAGE_VERSION " - " + tr("File: ")+FileName);
  return true;
}


// ************************************************************
bool QucsEdit::closeFile()
{
  if(text->document()->isModified()) {
    switch(QMessageBox::warning(this,tr("Closing document"),
      tr("The text contains unsaved changes!\n")+
      tr("Do you want to save the changes?"),
      tr("&Save"), tr("&Discard"), tr("&Cancel"), 0, 2)) {
      case 0: slotSave();
	      if(FileName.isEmpty()) return false;
	      return true;
      case 2: return false;
    }
  }
  return true;
}
