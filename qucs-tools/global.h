/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __GLOBAL_H
#define __GLOBAL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QLocale>

#include <QtGui/QFont>
#include <QtGui/QPixmap>

namespace Qucs
{
   const QString baseDir(BASEDIR);
   const QString binaryDir(BINARYDIR);
   const QString bitmapDir(BITMAPDIR);
   const QString docDir(DOCDIR);
   const QString langDir(LANGUAGEDIR);
   const QString libDir(LIBRARYDIR);
   const QString version(PACKAGE_VERSION);
   const QString versionString(PACKAGE_STRING);

   QString pathForQucsFile(const QString& fileName);

   QString getenv();

   QString baseDirectory();
   QString binaryDirectory();
   QString bitmapDirectory();
   QString langDirectory();

   inline QPixmap pixmapFor(const QString& name) {
      return QPixmap(bitmapDirectory() + name + ".png");
   }

   QString language();
   QString localePrefix();
   QFont font();

   bool checkVersion(const QString& line);

   inline QString boolToString(bool boolean) {
      return boolean ? QString("true") : QString("false");
   }

   inline bool stringToBool(const QString& str) {
      return str == "true" ? true : false;
   }

   inline QString realToString(qreal val) {
      return QString::number(val,'f',2);
   }
}

#endif //__GLOBAL_H
