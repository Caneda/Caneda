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

#include "qucs-tools/global.h"
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

int main( int argc, char ** argv )
{

  QApplication a( argc, argv );
  a.setOrganizationName("Qucs");
  a.setApplicationName("QucsAttenuator");
  a.setFont(Qucs::font());
  QTranslator tor( 0 );
  QString lang = Qucs::language();
  tor.load( QString("qucs_") + lang, Qucs::langDirectory());
  a.installTranslator( &tor );

  QucsAttenuator *qucs = new QucsAttenuator();
  qucs->show();
  return a.exec();
}
