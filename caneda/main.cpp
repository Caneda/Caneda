/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "qucsmainwindow.h"

#include "qucs-tools/global.h"

#include <QApplication>
#include <QSplashScreen>
#include <QTimer>

int main(int argc,char *argv[])
{
   QApplication app(argc,argv);
   app.setOrganizationName("Qucs");
   app.setApplicationName("Qucs");

   // We create a splash screen
   QPixmap pixmap(Qucs::bitmapDirectory() + "splash.jpg");
   QSplashScreen splash(pixmap);
   splash.show();
   app.processEvents();

   QucsMainWindow *mw = QucsMainWindow::instance();
   QTimer::singleShot(100, mw, SLOT(show()));

   splash.finish(mw);
   return app.exec();
}

