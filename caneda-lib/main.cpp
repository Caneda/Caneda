/***************************************************************************
                                main.cpp
                               ----------
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

#include <stdlib.h>

#include <Qt/qapplication.h>
#include <Qt/qstring.h>
#include <Qt/qtextcodec.h>
#include <Qt/qtranslator.h>
#include <Qt/qfile.h>
#include <Qt/qtextstream.h>
#include <Qt/qmessagebox.h>
#include <Qt/qdir.h>
#include <Qt/qfont.h>

#include <QtCore/QLocale>

#include "canedalib.h"

tCanedaSettings CanedaSettings;
QDir UserLibDir;

// Loads the settings file and stores the settings.
bool loadSettings()
{
  bool result = true;

  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/librc"));
  if(!file.open(QIODevice::ReadOnly))
    result = false; // settings file doesn't exist
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1);
      if(Setting == "Position") {
        CanedaSettings.x = Line.section(",",0,0).toInt();
        CanedaSettings.y = Line.section(",",1,1).toInt(); }
      else if(Setting == "Size") {
        CanedaSettings.dx = Line.section(",",0,0).toInt();
        CanedaSettings.dy = Line.section(",",1,1).toInt(); }
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

// Saves the settings in the settings file.
bool saveApplSettings(CanedaLib *caneda)
{
  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/librc"));
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(0, QObject::tr("Warning"),
			QObject::tr("Cannot save settings !"));
    return false;
  }

  QString Line;
  QTextStream stream(&file);

  stream << "Settings file, CanedaLib " PACKAGE_VERSION "\n"
    << "Position=" << caneda->x() << "," << caneda->y() << "\n"
    << "Size=" << caneda->width() << "," << caneda->height() << "\n";

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
  CanedaSettings.x = 100;
  CanedaSettings.y = 50;
  CanedaSettings.dx = 600;
  CanedaSettings.dy = 350;
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
    CanedaSettings.LibDir =
      QDir::convertSeparators (CanedaDirStr + "/share/caneda/library/");
  } else {
    CanedaSettings.BitmapDir = BITMAPDIR;
    CanedaSettings.LangDir = LANGUAGEDIR;
    CanedaSettings.LibDir = LIBRARYDIR;
  }
  UserLibDir.setPath (QDir::homePath()+QDir::convertSeparators ("/.caneda/user_lib"));
  loadSettings();

  QApplication a(argc, argv);
  a.setFont(CanedaSettings.font);

  QTranslator tor( 0 );
  QString lang = CanedaSettings.Language;
  if(lang.isEmpty())
    lang = QLocale::languageToString( QLocale::system().language());
		//lang = QTextCodec::locale();
  tor.load( QString("caneda_") + lang, CanedaSettings.LangDir);
  a.installTranslator( &tor );

  CanedaLib *caneda = new CanedaLib();
  //a.setMainWidget(caneda);
  caneda->resize(CanedaSettings.dx, CanedaSettings.dy); // size and position ...
  caneda->move(CanedaSettings.x, CanedaSettings.y);     // ... before "show" !!!
  caneda->show();

  int result = a.exec();
  saveApplSettings(caneda);
  delete caneda;
  return result;
}
