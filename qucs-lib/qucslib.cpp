/***************************************************************************
                               qucslib.cpp
                              -------------
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

#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QTextEdit>
#include <QtGui/QListWidget>
#include <QtCore/QRegExp>
#include <QtGui/QCloseEvent>
#include <QtCore/QTextStream>

#include "qucslib.h"
#include "librarydialog.h"
#include "displaydialog.h"
#include "symbolwidget.h"
#include "searchdialog.h"

/* Constructor setups the GUI. */
QucsLib::QucsLib()
{
  // set application icon
  setWindowIcon (QPixmap(QucsSettings.BitmapDir + "big.qucs.xpm"));
  setWindowTitle("Qucs Library Tool " PACKAGE_VERSION);

  QMenuBar * menuBar = new QMenuBar (this);

  // create file menu
  QMenu * fileMenu = menuBar->addMenu(tr("&File"));//new QMenu ();
  QAction * manageLib = new QAction (tr("Manage User &Libraries..."), this);
  manageLib->setShortcut(QKeySequence("CTRL+M"));

  fileMenu->addAction ( manageLib );
  connect(manageLib, SIGNAL(activated()), SLOT(slotManageLib()));

  fileMenu->addSeparator();

  QAction * fileQuit = new QAction (tr("&Quit"), this);
  fileQuit->setShortcut(QKeySequence("CTRL+Q"));

  fileMenu->addAction(fileQuit);
  connect(fileQuit, SIGNAL(activated()), SLOT(slotQuit()));

  // Insert separator
  menuBar->addSeparator ();

  // create help menu
  QMenu * helpMenu = menuBar->addMenu(tr("&Help"));
  QAction * helpHelp = new QAction (tr("&Help"), this);
  helpHelp->setShortcut(QKeySequence("F1"));

  helpMenu->addAction (helpHelp);
  connect(helpHelp, SIGNAL(activated()), SLOT(slotHelp()));
  QAction * helpAbout = new QAction(tr("About"),helpMenu);

  helpMenu->addAction (helpAbout);
  connect(helpAbout, SIGNAL(activated()), SLOT(slotAbout()));

  // main box
  QVBoxLayout * all = new QVBoxLayout (this);
  all->setSpacing (0);
  all->setMargin (0);

  // reserve space for menubar
  QWidget * Space = new QWidget (this);
  Space->setFixedSize(5, menuBar->height() + 2);
  all->addWidget (Space);

  // main layout
  QHBoxLayout * h = new QHBoxLayout();
  h->setSpacing (5);
  h->setMargin (3);
  all->addLayout (h);

  // library and component choice
  QGroupBox * LibGroup = new QGroupBox (tr("Component Selection"),this);
  h->addWidget(LibGroup);
  QVBoxLayout *grpl = new QVBoxLayout( LibGroup );
  Library = new QComboBox (LibGroup);
  grpl->addWidget( Library );
  connect(Library, SIGNAL(activated(int)), SLOT(slotSelectLibrary(int)));
  CompList = new QListWidget(LibGroup);
  grpl->addWidget( CompList );
//  connect(CompList, SIGNAL(highlighted(QListBoxItem*)),SLOT(slotShowComponent(QListBoxItem*)));
  connect(CompList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
	  this, SLOT(slotShowComponent(QListWidgetItem*,QListWidgetItem*)));

  QHBoxLayout * h1 = new QHBoxLayout();
  grpl->addLayout( h1 );
  QPushButton * SearchButton = new QPushButton (tr("Search..."), LibGroup);
  h1->addWidget( SearchButton );
  connect(SearchButton, SIGNAL(clicked()), SLOT(slotSearchComponent()));
  QWidget * w1 = new QWidget(LibGroup);
  h1->addWidget(w1);
  h1->setStretchFactor(w1, 5); // stretchable placeholder

  // component display
  QGroupBox *CompGroup = new QGroupBox (tr("Component"), this);
  h->addWidget( CompGroup );
  QVBoxLayout * compLayout = new QVBoxLayout(CompGroup);
  CompDescr = new QTextEdit(CompGroup);
  compLayout->addWidget(CompDescr);
  CompDescr->setReadOnly(true);
  CompDescr->setWordWrapMode(QTextOption::NoWrap);

  Symbol = new SymbolWidget (CompGroup);
  compLayout->addWidget( Symbol );

  QHBoxLayout * h2 = new QHBoxLayout();
  compLayout->addLayout( h2  );
  QPushButton * CopyButton = new QPushButton (tr("Copy to clipboard"), CompGroup);
  h2->addWidget( CopyButton );
  connect(CopyButton, SIGNAL(clicked()), SLOT(slotCopyToClipBoard()));
  QPushButton * ShowButton = new QPushButton (tr("Show Model"), CompGroup);
  h2->addWidget( ShowButton );
  connect(ShowButton, SIGNAL(clicked()), SLOT(slotShowModel()));

  // ......................................................
  putLibrariesIntoCombobox();
}

/* Destructor destroys the application. */
QucsLib::~QucsLib()
{
}

// ----------------------------------------------------
// Put all available libraries into ComboBox.
void QucsLib::putLibrariesIntoCombobox()
{
  Library->clear();

  UserLibCount = 0;
  QStringList LibFiles;
  QStringList::iterator it;
  if(UserLibDir.cd(".")) { // user library directory exists ?
    LibFiles = UserLibDir.entryList(QStringList("*.lib"), QDir::Files, QDir::Name);
  UserLibCount = LibFiles.count();

    for(it = LibFiles.begin(); it != LibFiles.end(); it++)
      Library->addItem((*it).left((*it).length()-4));
  }

  QDir LibDir(QucsSettings.LibDir);
  LibFiles = LibDir.entryList(QStringList("*.lib"), QDir::Files, QDir::Name);

  for(it = LibFiles.begin(); it != LibFiles.end(); it++)
    Library->addItem((*it).left((*it).length()-4));

  slotSelectLibrary(0);
}

// ----------------------------------------------------
void QucsLib::slotAbout()
{
  QMessageBox::about(this, tr("About..."),
    "QucsLib Version " PACKAGE_VERSION "\n"+
    tr("Library Manager for Qucs\n")+
    tr("Copyright (C) 2005 by Michael Margraf\n")+
    "\nThis is free software; see the source for copying conditions."
    "\nThere is NO warranty; not even for MERCHANTABILITY or "
    "\nFITNESS FOR A PARTICULAR PURPOSE.");
}

// ----------------------------------------------------
void QucsLib::slotQuit()
{
  int tmp;
  tmp = x();		// call size and position function in order to ...
  tmp = y();		// ... set them correctly before closing the ...
  tmp = width();	// dialog !!!  Otherwise the frame of the window ...
  tmp = height();	// will not be recognized (a X11 problem).

  accept();
}

// ----------------------------------------------------
void QucsLib::closeEvent(QCloseEvent *Event)
{
  int tmp;
  tmp = x();		// call size and position function in order to ...
  tmp = y();		// ... set them correctly before closing the ...
  tmp = width();	// dialog !!!  Otherwise the frame of the window ...
  tmp = height();	// will not be recognized (a X11 problem).

  Event->accept();
}

// ----------------------------------------------------
void QucsLib::slotManageLib()
{
  (new LibraryDialog(this))->exec();
  putLibrariesIntoCombobox();
}

// ----------------------------------------------------
void QucsLib::slotHelp()
{
  DisplayDialog *d = new DisplayDialog(this);
  d->setWindowTitle("QucsLib Help");
  d->resize(250, 325);
  d->Text->setText(
     tr("QucsLib is a program to manage Qucs component libraries. "
	"On the left side of the application window the available "
	"libraries can be browsed to search the wanted component. "
	"By clicking on the component name its description can be "
	"seen on the right side. The selected component is "
	"transported to the Qucs application by clicking on the "
	"button \"Copy to Clipboard\". Being back in the schematic "
	"window the component can be inserted by pressing CTRL-V "
	" (paste from clipboard).") + "\n" +
     tr("A more comfortable way: The component can also be placed "
        "onto the schematic by using Drag n'Drop."));
  d->show();
}

// ----------------------------------------------------
void QucsLib::slotCopyToClipBoard()
{
  QString s = "<Qucs Schematic " PACKAGE_VERSION ">\n";
  s += "<Components>\n  " +
       Symbol->theModel() +
       "\n</Components>\n";

  // put resulting schematic into clipboard
  QClipboard *cb = QApplication::clipboard();
  cb->setText(s);
}

// ----------------------------------------------------
void QucsLib::slotShowModel()
{
  DisplayDialog *d = new DisplayDialog(this);
  d->setWindowTitle("Model");
  d->resize(500, 150);
  d->Text->setText(Symbol->ModelString);
  d->Text->setWordWrapMode(QTextOption::NoWrap);
  d->show();
}

// ----------------------------------------------------
void QucsLib::slotSelectLibrary(int Index)
{
  int Start, End, NameStart, NameEnd;
  End = Library->count()-1;
  if(Library->itemText(End) == tr("Search result"))
    if(Index < End)
      Library->removeItem(End); // if search result still there -> remove it
    else  return;


  CompList->clear();
  LibraryComps.clear();
  DefaultSymbol = "";

  QFile file;
  if(Index < UserLibCount)  // Is it user library ?
    file.setFileName(UserLibDir.absolutePath() + QDir::separator() + Library->itemText(Index) + ".lib");
  else
    file.setFileName(QucsSettings.LibDir + Library->itemText(Index) + ".lib");

  if(!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(this, tr("Error"),
        tr("Cannot open \"%1\".").arg(
           QucsSettings.LibDir + Library->itemText(Index) + ".lib"));
    return;
  }

  QTextStream ReadWhole(&file);
  QString LibraryString = ReadWhole.readAll();
  file.close();

  Start = LibraryString.indexOf("<Qucs Library ");
  if(Start < 0) {
    QMessageBox::critical(this, tr("Error"), tr("Library is corrupt."));
    return;
  }
  End = LibraryString.indexOf('>', Start);
  if(End < 0) {
    QMessageBox::critical(this, tr("Error"), tr("Library is corrupt."));
    return;
  }
  QString LibName = LibraryString.mid(Start, End-Start).section('"', 1, 1);

  Start = LibraryString.indexOf("\n<", End);
  if(Start < 0) return;
  if(LibraryString.mid(Start+2, 14) == "DefaultSymbol>") {
    End = LibraryString.indexOf("\n</DefaultSymbol>");
    if(End < 0) {
      QMessageBox::critical(this, tr("Error"), tr("Library is corrupt."));
      return;
    }

    DefaultSymbol = LibraryString.mid(Start+16, End-Start-16);
    Start = End + 3;
  }

  while((Start=LibraryString.indexOf("\n<Component ", Start)) > 0) {
    Start++;
    NameStart = Start + 11;
    NameEnd = LibraryString.indexOf('>', NameStart);
    if(NameEnd < 0)  continue;

    End = LibraryString.indexOf("\n</Component>", NameEnd);
    if(End < 0)  continue;
    End += 13;

    CompList->addItem(LibraryString.mid(NameStart, NameEnd-NameStart));
    LibraryComps.append(LibName+'\n'+LibraryString.mid(Start, End-Start));
    Start = End;
  }

  ///FIXME (Is this alright?)
  CompList->setCurrentRow(0);  // select first item
}

// ----------------------------------------------------
void QucsLib::slotSearchComponent()
{
  SearchDialog *d = new SearchDialog(this);
  if(d->exec() == QDialog::Accepted)
    QMessageBox::information(this, tr("Result"),
                             tr("No appropriate component found."));
}

// ----------------------------------------------------
void QucsLib::slotShowComponent(QListWidgetItem *Item, QListWidgetItem*)
{
  if(!Item) return;

  QString CompString = LibraryComps.at(CompList->row(Item));
  QString LibName = CompString.section('\n', 0, 0);
  CompDescr->setText("Name: " + Item->text());
  CompDescr->append("Library: " + LibName);
  CompDescr->append("----------------------------");

  if(Library->currentIndex() < UserLibCount)
    LibName = UserLibDir.absolutePath() + QDir::separator() + LibName;


  int Start, End;
  Start = CompString.indexOf("<Description>");
  if(Start > 0) {
    Start += 13;
    End = CompString.indexOf("</Description>", Start);
    if(End < 0) {
      QMessageBox::critical(this, tr("Error"), tr("Library is corrupt."));
      return;
    }
    CompDescr->append(
      CompString.mid(Start, End-Start).replace(QRegExp("\\n\\x20+"), "\n").remove(0, 1));
  }


  Start = CompString.indexOf("<Model>");
  if(Start > 0) {
    Start += 7;
    End = CompString.indexOf("</Model>", Start);
    if(End < 0) {
      QMessageBox::critical(this, tr("Error"), tr("Library is corrupt."));
      return;
    }
    Symbol->ModelString =
      CompString.mid(Start, End-Start).replace(QRegExp("\\n\\x20+"), "\n").remove(0, 1);

    if((Symbol->ModelString.count('\n')) < 2)
      Symbol->createSymbol(LibName, Item->text());
  }


  Start = CompString.indexOf("<Symbol>");
  if(Start > 0) {
    Start += 8;
    End = CompString.indexOf("</Symbol>", Start);
    if(End < 0) {
      QMessageBox::critical(this, tr("Error"), tr("Library is corrupt."));
      return;
    }
    Symbol->setSymbol(CompString.mid(Start, End-Start),
                      LibName, Item->text());
  }
  else if(!DefaultSymbol.isEmpty())   // has library a default symbol ?
    Symbol->setSymbol(DefaultSymbol, LibName, Item->text());
}
