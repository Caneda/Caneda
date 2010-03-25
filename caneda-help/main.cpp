/***************************************************************************
                                main.cpp
                               ----------
    begin                : Thu Jun 24  2004
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "canedahelp.h"

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QFont>

#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/QLocale>



QDir CanedaHelpDir; // directory to find helps files
tCanedaSettings CanedaSettings; // application settings

// #########################################################################
// Loads the settings file and stores the settings.
bool loadSettings()
{
  bool result = true;

  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/helprc"));
  if(!file.open(QIODevice::ReadOnly))
    result = false; // settings file doesn't exist
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1);
      if(Setting == "HelpWindow") {
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
      Line = Line.section('=',1,1).trimmed();
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
bool saveApplSettings(CanedaHelp *caneda)
{
  if(caneda->x() == CanedaSettings.x && caneda->y() == CanedaSettings.y &&
     caneda->width() == CanedaSettings.dx && caneda->height() == CanedaSettings.dy)
    return true;   // nothing has changed


  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/helprc"));
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(0, QObject::tr("Warning"),
			QObject::tr("Cannot save settings !"));
    return false;
  }

  QString Line;
  QTextStream stream(&file);

  stream << "Settings file, Caneda Help System " PACKAGE_VERSION "\n"
         << "HelpWindow=" << caneda->x() << ',' << caneda->y() << ','
         << caneda->width() << ',' << caneda->height() << '\n';

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
  CanedaSettings.x = 60;
  CanedaSettings.y = 30;
  CanedaSettings.dx = 640;
  CanedaSettings.dy = 400;
  CanedaSettings.font = QFont("Helvetica", 12);

  // is application relocated?
  char * var = getenv ("CANEDADIR");
  if (var != NULL) {
    QDir CanedaDir = QDir (var);
    QString CanedaDirStr = CanedaDir.canonicalPath ();
    CanedaSettings.DocDir =
      QDir::convertSeparators (CanedaDirStr + "/share/caneda/docs/");
    CanedaSettings.BitmapDir =
      QDir::convertSeparators (CanedaDirStr + "/share/caneda/bitmaps/");
    CanedaSettings.LangDir =
      QDir::convertSeparators (CanedaDirStr + "/share/caneda/lang/");
  } else {
    CanedaSettings.DocDir = DOCDIR;
    CanedaSettings.BitmapDir = BITMAPDIR;
    CanedaSettings.LangDir = LANGUAGEDIR;
  }

  loadSettings();

  QApplication a(argc, argv);
  a.setFont(CanedaSettings.font);

  QTranslator tor( 0 );
  QString locale = CanedaSettings.Language;
  if(locale.isEmpty())
    locale = QLocale().name();
  tor.load( QString("caneda_") + locale, CanedaSettings.LangDir);
  a.installTranslator( &tor );

  CanedaHelpDir = CanedaSettings.DocDir + locale;
  if (!CanedaHelpDir.exists () || !CanedaHelpDir.isReadable ()) {
    int p = locale.indexOf(QChar('_'));
    if (p != -1) {
      CanedaHelpDir = CanedaSettings.DocDir + locale.left (p);
      if (!CanedaHelpDir.exists () || !CanedaHelpDir.isReadable ()) {
        CanedaHelpDir = CanedaSettings.DocDir + "en";
      }
    }
    else CanedaHelpDir = CanedaSettings.DocDir + "en";
  }

  QString Page;
  if(argc > 1) Page = argv[1];

  CanedaHelp *caneda = new CanedaHelp(Page);
  caneda->resize(CanedaSettings.dx, CanedaSettings.dy); // size and position ...
  caneda->move(CanedaSettings.x, CanedaSettings.y);     // ... before "show" !!!
  caneda->show();
  int result = a.exec();
  saveApplSettings(caneda);
  return result;
}
