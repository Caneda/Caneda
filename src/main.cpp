/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2012 by Pablo Daniel Pareja Obregon                  *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "mainwindow.h"

#include "global.h"

#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QTranslator>

int main(int argc,char *argv[])
{
   QApplication app(argc,argv);
   app.setOrganizationName("Caneda");
   app.setApplicationName("Caneda");

   // Load the translations
   QTranslator translator;
   translator.load(QLocale::system(), "caneda", "_", Caneda::langDirectory(), ".qm");
   app.installTranslator(&translator);

   // We create a splash screen
   QPixmap pixmap(Caneda::bitmapDirectory() + "splash.jpg");
   QSplashScreen splash(pixmap);
   splash.show();
   app.processEvents();

   Caneda::MainWindow *mw = Caneda::MainWindow::instance();
   QTimer::singleShot(100, mw, SLOT(show()));

   splash.finish(mw);
   return app.exec();
}
