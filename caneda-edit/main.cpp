/***************************************************************************
                                 main.cpp
                                ----------
    begin                : Thu Aug 28 2003
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

/* Modified on 28/02/07 by Martin "Ruso" Ribelotta at porting to Qt4 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <QtGui/QApplication>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtGui/QFont>
#include <QtCore/QLocale>

#include "canedaedit.h"

tCanedaSettings CanedaSettings;

// #########################################################################
// Loads the settings file and stores the settings.
bool loadSettings()
{
  bool result = true;

  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/editrc"));
  if(!file.open(QIODevice::ReadOnly))
    result = false; // settings file doesn't exist
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1);
      if(Setting == "EditWindow") {
        CanedaSettings.x  = Line.section(",",0,0).toInt();
        CanedaSettings.y  = Line.section(",",1,1).toInt();
        CanedaSettings.dx = Line.section(",",2,2).toInt();
        CanedaSettings.dy = Line.section(",",3,3).toInt();
	break; }
    }
    file.close();
  }

  file.setFileName(QDir::homePath()+QDir::convertSeparators ("/.caneda/canedarc"));
  if(!file.open(QIODevice::ReadOnly))
    result = true; // Caneda settings not necessary
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1).simplified();
      if(Setting == "Font")
        CanedaSettings.font.fromString(Line);
      else if(Setting == "Language")
        CanedaSettings.Language = Line;
    }
    file.close();
  }
  return result;
}

// #########################################################################
// Saves the settings in the settings file.
bool saveApplSettings(CanedaEdit *caneda)
{
  if(caneda->x() == CanedaSettings.x)
    if(caneda->y() == CanedaSettings.y)
      if(caneda->width() == CanedaSettings.dx)
        if(caneda->height() == CanedaSettings.dy)
	  return true;   // nothing has changed


  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/editrc"));
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(0, QObject::tr("Warning"),
			QObject::tr("Cannot save settings !"));
    return false;
  }

  QString Line;
  QTextStream stream(&file);

  stream << "Settings file, Caneda Editor " PACKAGE_VERSION "\n"
         << "EditWindow=" << caneda->x() << ',' << caneda->y() << ','
         << caneda->width() << ',' << caneda->height() << '\n';

  file.close();
  return true;
}

// #########################################################################
void showOptions()
{
  fprintf(stdout, QString( QObject::tr("Caneda Editor Version ")+PACKAGE_VERSION+
    QObject::tr("\nVery simple text editor for Caneda\n")+
    QObject::tr("Copyright (C) 2004, 2005 by Michael Margraf\n")+
    QObject::tr("\nUsage:  canedaedit [-r] file\n")+
    QObject::tr("    -h  display this help and exit\n")+
    QObject::tr("    -r  open file read-only\n") ).toAscii());
}


// #########################################################################
// ##########                                                     ##########
// ##########                  Program Start                      ##########
// ##########                                                     ##########
// #########################################################################

int main(int argc, char *argv[])
{
  // apply default settings
  CanedaSettings.x = 200;
  CanedaSettings.y = 100;
  CanedaSettings.dx = 400;
  CanedaSettings.dy = 400;
  CanedaSettings.font = QFont("Helvetica", 12);

  // is application relocated?
  char * var = getenv ("CANEDADIR");
  if (var != NULL) {
    QDir CanedaDir = QDir (var);
    QString CanedaDirStr = CanedaDir.canonicalPath ();
    CanedaSettings.BitmapDir =
      QDir::convertSeparators (CanedaDirStr + "/share/caneda/bitmaps/");
    CanedaSettings.LangDir =
      QDir::convertSeparators (CanedaDirStr + "/share/caneda/lang/");
  } else {
    CanedaSettings.BitmapDir = BITMAPDIR;
    CanedaSettings.LangDir = LANGUAGEDIR;
  }

  loadSettings();

  QApplication a(argc, argv);
  a.setFont(CanedaSettings.font);

  QTranslator tor( 0 );
  QString lang = CanedaSettings.Language;
  if(lang.isEmpty())
    lang = QLocale::languageToString( QLocale::system().language() );
		//lang = QTextCodec::locale();
  tor.load( QString("caneda_") + lang, CanedaSettings.LangDir);
  a.installTranslator( &tor );

  bool readOnly = false;
  QString FileName, s;
  for(int i=1; i<argc; i++) {
    s = argv[i];
    if(s.at(0) == '-') {
      if(s.length() != 2) {
	fprintf(stdout, QString(QObject::tr("Too long command line argument!\n\n")).toAscii());
	showOptions();
	return -1;
      }
      switch(s.at(1).toLatin1()) {
	case 'r': readOnly = true;
		  break;
	case 'h': showOptions();
		  return 0;
	default :
	  fprintf(stderr, QObject::tr("Wrong command line argument!\n\n").toAscii());
	  showOptions();
	  return -1;
      }
    }
    else  if(FileName.isEmpty())  FileName = s;
	  else {
	    fprintf(stderr, QObject::tr("Only one filename allowed!\n\n").toAscii());
	    showOptions();
	    return -1;
	  }
  }

  CanedaEdit *caneda = new CanedaEdit(FileName, readOnly);
  //a.setMainWidget(caneda);
  caneda->resize(CanedaSettings.dx, CanedaSettings.dy); // size and position ...
  caneda->move(CanedaSettings.x, CanedaSettings.y);     // ... before "show" !!!
  caneda->show();
  int result = a.exec();
  saveApplSettings(caneda);
  return result;
}
