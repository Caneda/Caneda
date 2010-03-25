/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sun Feb 27 2005
    copyright            : (C) 2005 by Stefan Jahn
    email                : stefan@lkcc.org
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

#include "canedatrans.h"

#include <QtGui/QApplication>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QLocale>
#include <QtGui/QFont>
#include "caneda-tools/propertygrid.h"


tCanedaSettings CanedaSettings;
extern QDir CanedaWorkDir;

// Loads the settings file and stores the settings.
bool loadSettings()
{
  bool result = true;

  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/transrc"));
  if(!file.open(QFile::ReadOnly))
    result = false; // settings file doesn't exist
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1);
      if(Setting == "Mode") {
        CanedaSettings.Mode = Line.simplified();
      }
      else if(Setting == "Frequency") {
	Line = Line.simplified();
        CanedaSettings.freq_unit = Units::toInt(Line);
      }
      else if(Setting == "Length") {
	Line = Line.simplified();
        CanedaSettings.length_unit = Units::toInt(Line);
      }
      else if(Setting == "Resistance") {
	Line = Line.simplified();
        CanedaSettings.res_unit = Units::toInt(Line);
      }
      else if(Setting == "Angle") {
	Line = Line.simplified();
        CanedaSettings.ang_unit = Units::toInt(Line);
      }
      else if(Setting == "TransWindow") {
        CanedaSettings.x  = Line.section(",",0,0).toInt();
        CanedaSettings.y  = Line.section(",",1,1).toInt();
        CanedaSettings.dx = Line.section(",",2,2).toInt();
        CanedaSettings.dy = Line.section(",",3,3).toInt();
	break;
      }
    }
    file.close();
  }

  file.setFileName(QDir::homePath()+QDir::convertSeparators ("/.caneda/canedarc"));
  if(!file.open(QFile::ReadOnly))
    result = true; // caneda settings not necessary
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

// Saves the settings in the settings file.
bool saveApplSettings(CanedaTranscalc *caneda)
{
  QFile file(QDir::homePath()+QDir::convertSeparators ("/.caneda/transrc"));
  if(!file.open(QFile::WriteOnly)) {
    QMessageBox::warning(0, QObject::tr("Warning"),
			QObject::tr("Cannot save settings !"));
    return false;
  }

  QString Line;
  QTextStream stream(&file);

  stream << "Settings file, CanedaTranscalc " PACKAGE_VERSION "\n"
         << "Mode=" << caneda->currentModeString() << "\n"
         << "Frequency=" << Units::toString(CanedaSettings.freq_unit,Units::Frequency) << "\n"
         << "Length=" << Units::toString(CanedaSettings.length_unit,Units::Length) << "\n"
         << "Resistance=" << Units::toString(CanedaSettings.res_unit,Units::Resistance) << "\n"
         << "Angle=" << Units::toString(CanedaSettings.ang_unit,Units::Angle) << "\n"
         << "TransWindow=" << caneda->x() << ',' << caneda->y() << ','
         << caneda->width() << ',' << caneda->height() << '\n';
  caneda->saveToStream(stream);

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
  CanedaSettings.dx = 540;
  CanedaSettings.dy = 400;
  CanedaSettings.font = QFont("Helvetica", 12);
  CanedaSettings.length_unit = 0;
  CanedaSettings.res_unit = 0;
  CanedaSettings.ang_unit = 0;
  CanedaSettings.freq_unit = 0;

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
  CanedaWorkDir.setPath (QDir::homePath()+QDir::convertSeparators ("/.caneda"));
  loadSettings();

  QApplication a(argc, argv);
  a.setFont(CanedaSettings.font);

  QTranslator tor( 0 );
  QString lang = CanedaSettings.Language;
  if(lang.isEmpty())
    lang = QLocale().name();
  tor.load( QString("caneda_") + lang, CanedaSettings.LangDir);
  a.installTranslator( &tor );

  CanedaTranscalc *caneda = new CanedaTranscalc();
  caneda->resize(CanedaSettings.dx, CanedaSettings.dy); // size and position ...
  caneda->move(CanedaSettings.x, CanedaSettings.y);     // ... before "show" !!!
  caneda->show();

  caneda->loadFile(QDir::homePath()+"/.caneda/transrc");
  caneda->setCurrentMode(CanedaSettings.Mode);

  // optional file argument
  if (argc > 1) {
    QString File = argv[1];
    caneda->loadFile(File);
  }

  int result = a.exec();
  saveApplSettings(caneda);
  delete caneda;
  return result;
}
