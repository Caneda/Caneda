/***************************************************************************
                       Caneda Attenuator Synthesis
                                 main.cpp
                               ------------
    begin                : Jun 14 2006

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "caneda-tools/global.h"
#include <stdlib.h>

#include "canedaattenuator.h"
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
  a.setOrganizationName("Caneda");
  a.setApplicationName("CanedaAttenuator");
  a.setFont(Caneda::font());
  QTranslator tor( 0 );
  QString lang = Caneda::language();
  tor.load( QString("caneda_") + lang, Caneda::langDirectory());
  a.installTranslator( &tor );

  CanedaAttenuator *caneda = new CanedaAttenuator();
  caneda->show();
  return a.exec();
}
