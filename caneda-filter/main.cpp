/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Thu Aug 28 18:17:41 CEST 2003
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

#include <stdlib.h>

#include "canedafilter.h"

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

struct tCanedaSettings CanedaSettings;

// #########################################################################
// Loads the settings file and stores the settings.
bool loadSettings()
{
  bool result = true;

  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/filterrc"));
  if(!file.open(QIODevice::ReadOnly))
    result = false; // settings file doesn't exist
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1);
      if(Setting == "FilterWindow") {
        CanedaSettings.x = Line.section(",",0,0).toInt();
        CanedaSettings.y = Line.section(",",1,1).toInt();
	break;
      }
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
bool saveApplSettings(CanedaFilter *caneda)
{
  if(caneda->x() == CanedaSettings.x)
    if(caneda->y() == CanedaSettings.y)
      return true;   // nothing has changed


  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/filterrc"));
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(0, QObject::tr("Warning"),
			QObject::tr("Cannot save settings !"));
    return false;
  }

  QString Line;
  QTextStream stream(&file);

  stream << "Settings file, Caneda Filter " PACKAGE_VERSION "\n"
         << "FilterWindow=" << caneda->x() << ',' << caneda->y() << '\n';

  file.close();
  return true;
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
    lang = QLocale().name();
  tor.load( QString("caneda_") + lang, CanedaSettings.LangDir);
  a.installTranslator( &tor );

  CanedaFilter *caneda = new CanedaFilter();
  caneda->move(CanedaSettings.x, CanedaSettings.y);  // position before "show" !!!
  caneda->show();
  int result = a.exec();
  saveApplSettings(caneda);
  return result;
}
