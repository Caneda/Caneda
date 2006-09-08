/****************************************************************************
**     Qucs Attenuator Synthesis
**     main.cpp
**
**
**
**
**
**
**
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "qucsattenuator.h"
#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QFont>


#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/QLocale>
struct tQucsSettings QucsSettings;


// #########################################################################
// Loads the settings file and stores the settings.
bool loadSettings()
{
  bool result = true;

  QFile file(QDir::homePath()+QDir::convertSeparators ("/.qucs/attenuatorrc"));
  if(!file.open(QIODevice::ReadOnly))
    result = false; // settings file doesn't exist
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1);
      if(Setting == "AttenuatorWindow") {
	QucsSettings.x = Line.section(",",0,0).toInt();
	QucsSettings.y = Line.section(",",1,1).toInt();
	break;
      }
    }
    file.close();
  }

  file.setFileName(QDir::homePath()+QDir::convertSeparators ("/.qucs/qucsrc"));
  if(!file.open(QIODevice::ReadOnly))
    result = true; // qucs settings not necessary
  else {
    QTextStream stream(&file);
    QString Line, Setting;
    while(!stream.atEnd()) {
      Line = stream.readLine();
      Setting = Line.section('=',0,0);
      Line = Line.section('=',1,1).simplified();
      if(Setting == "Font")
	QucsSettings.font.fromString(Line);
      else if(Setting == "Language")
	QucsSettings.Language = Line;
    }
    file.close();
  }
  return result;
}

// #########################################################################
// Saves the settings in the settings file.
bool saveApplSettings(QucsAttenuator *qucs)
{
  if(qucs->x() == QucsSettings.x)
    if(qucs->y() == QucsSettings.y)
      return true;   // nothing has changed


  QFile file(QDir::homePath()+QDir::convertSeparators ("/.qucs/attenuatorrc"));
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(0, QObject::tr("Warning"),
			 QObject::tr("Cannot save settings !"));
    return false;
  }

  QString Line;
  QTextStream stream(&file);
  
  stream << "Settings file, Qucs Attenuator " PACKAGE_VERSION "\n"
    	 << "AttenuatorWindow=" << qucs->x() << ',' << qucs->y() << '\n';
  
  file.close();
  return true;
}



int main( int argc, char ** argv )
{
  // apply default settings
  QucsSettings.x = 200;
  QucsSettings.y = 100;
  QucsSettings.font = QFont("Helvetica", 12);

  // is application relocated?
  char * var = getenv ("QUCSDIR");
  if (var != NULL) {
    QDir QucsDir = QDir (var);
    QString QucsDirStr = QucsDir.canonicalPath ();
    QucsSettings.BitmapDir =
      QDir::convertSeparators (QucsDirStr + "/share/qucs/bitmaps/");
    QucsSettings.LangDir =
      QDir::convertSeparators (QucsDirStr + "/share/qucs/lang/");
  } else {
    QucsSettings.BitmapDir = BITMAPDIR;
    QucsSettings.LangDir = LANGUAGEDIR;
  }

  loadSettings();

  QApplication a( argc, argv );
  a.setFont(QucsSettings.font);
  QTranslator tor( 0 );
  QString lang = QucsSettings.Language;
  if(lang.isEmpty())
    lang = QLocale().name();
  tor.load( QString("qucs_") + lang, QucsSettings.LangDir);
  a.installTranslator( &tor );

  QucsAttenuator *qucs = new QucsAttenuator();
  qucs->move(QucsSettings.x, QucsSettings.y);  // position before "show" !!!
  qucs->show();
  int result = a.exec();
  saveApplSettings(qucs);
  return result;
}
