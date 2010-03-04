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

#include "global.h"
#include <cmath>
#include <QtCore/QDebug>

using namespace std;

namespace Qucs
{
   QString pathForQucsFile(const QString& fileName)
   {
      QString retVal = QDir::homePath();
      retVal += QDir::convertSeparators(QString("/.qucs/")+fileName);
      return retVal;
   }

   QString getenv()
   {
      // is application relocated?
      static QString var(::getenv("QUCSDIR"));
      return var;
   }

   QString baseDirectory()
   {
      QString var = Qucs::getenv();
      if(!var.isEmpty())
      {
         QDir QucsDir = QDir (var);
         return QDir::convertSeparators (QucsDir.canonicalPath () + "/share/qucs/");
      }
      return Qucs::baseDir;
   }

   QString binaryDirectory()
   {
      QString var = Qucs::getenv();
      if(!var.isEmpty())
      {
         QDir QucsDir = QDir (var);
         return QDir::convertSeparators (QucsDir.canonicalPath () + "/bin/");
      }
      return Qucs::binaryDir;
   }

   QString bitmapDirectory()
   {
      QString var = Qucs::getenv();
      if(!var.isEmpty())
      {
         QDir QucsDir = QDir (var);
         return QDir::convertSeparators (QucsDir.canonicalPath () + "/share/qucs/bitmaps/");
      }
      return Qucs::bitmapDir;
   }

   QString langDirectory()
   {

      QString var = Qucs::getenv();
      if(!var.isEmpty())
      {
         QDir QucsDir = QDir (var);
         return QDir::convertSeparators (QucsDir.canonicalPath () + "/share/qucs/lang/");
      }
      return Qucs::langDir;
   }

   QString language()
   {
      QString _default = QLocale().name();
      QSettings settings;
      settings.beginGroup("MainWindow");
      QString retVal = settings.value("language",_default).toString();
      settings.endGroup();
      return retVal;
   }

   QString localePrefix()
   {
      QString retVal = QLocale::system().name();
      retVal = retVal.left(retVal.indexOf('_'));
      return retVal;
   }

   QFont font()
   {
      QSettings settings;
      settings.beginGroup("MainWindow");
      QString fontStr = settings.value("font").toString();
      QFont fnt;
      fnt.fromString(fontStr);
      settings.endGroup();
      return fnt;
   }


   bool checkVersion(const QString& Line)
   {
      QStringList sl = Qucs::version.split('.', QString::SkipEmptyParts);
      QStringList ll = Line.split('.',QString::SkipEmptyParts);
      if (ll.count() != 3 || sl.count() != 3)
         return false;
      int sv = (sl.at(0)).toInt() * 10000 + (sl.at(1)).toInt() * 100 +
         (sl.at(2)).toInt();
      int lv = (ll.at(0)).toInt() * 10000 + (ll.at(1)).toInt() * 100 +
         (ll.at(2)).toInt();
      if(lv > sv) // wrong version number ? (only backward compatible)
         return false;
      return true;
   }

}
