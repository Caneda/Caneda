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

namespace Qucs
{

   static inline QString pathForFile(const QString& fileName)
   {
      QString retVal = QDir::homePath();
      retVal += QDir::convertSeparators(QString("/.qucs/")+fileName);
      return retVal;
   }

   
   class Settings : public QSettings
   {
      public:
         Settings(const QString& filename):
            QSettings(Qucs::pathForFile(filename),QSettings::IniFormat)
         {}
   };

   const QString binaryDir = QString(BINARYDIR);
   const QString bitmapDir = QString(BITMAPDIR);
   const QString docDir = QString(DOCDIR);
   const QString langDir = QString(LANGUAGEDIR);
   const QString libDir = QString(LIBRARYDIR);
   const QString version = QString(PACKAGE_VERSION);
   const QString versionString = QString(PACKAGE_STRING);

   inline QString bitmapDirectory()
   {
      // is application relocated?
      char *var = getenv("QUCSDIR");
      if(var)
      {
         QDir QucsDir = QDir (var);
         return QDir::convertSeparators (QucsDir.canonicalPath () + "/share/qucs/bitmaps/");
      }
      return Qucs::bitmapDir;
   }
   
   inline QString langDirectory()
   {
      // is application relocated?
      char *var = getenv("QUCSDIR");
      if(var)
      {
         QDir QucsDir = QDir (var);
         return QDir::convertSeparators (QucsDir.canonicalPath () + "/share/qucs/lang/");
      }
      return Qucs::langDir;
   }
   
   inline QString language()
   {
      QString _default = QLocale().name();
      Qucs::Settings settings("qucsrc");
      QString retVal = settings.value("language",_default).toString();
      return retVal;
   }

   inline QFont font()
   {
      Qucs::Settings settings("qucsrc");
      QString fontStr = settings.value("font","Helvetica").toString();
      QFont fnt;
      fnt.fromString(fontStr);
      return fnt;
   }
}
      


#endif //__GLOBAL_H
