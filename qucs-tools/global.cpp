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

   QString boolToString(bool b)
   {
      return b ? QString("true") : QString("false");
   }
   // #########################################################################
   QString complexRect(double real, double imag, int Precision)
   {
      QString Text;
      if(fabs(imag) < 1e-250) Text = QString::number(real,'g',Precision);
      else {
         Text = QString::number(imag,'g',Precision);
         if(Text.at(0) == '-') {
            Text[0] = 'j';
            Text = '-'+Text;
         }
         else  Text = "+j"+Text;
         Text = QString::number(real,'g',Precision) + Text;
      }
      return Text;
   }

   QString complexDeg(double real, double imag, int Precision)
   {
      QString Text;
      if(fabs(imag) < 1e-250) Text = QString::number(real,'g',Precision);
      else {
         Text  = QString::number(sqrt(real*real+imag*imag),'g',Precision) + " / ";
         Text += QString::number(double(180.0/M_PI*atan2(imag,real)),'g',Precision);
         Text += QChar('°');
      }
      return Text;
   }

   QString complexRad (double real, double imag, int Precision)
   {
      QString Text;
      if(fabs(imag) < 1e-250) Text = QString::number(real,'g',Precision);
      else {
         Text  = QString::number(sqrt(real*real+imag*imag),'g',Precision);
         Text += " / " + QString::number(atan2(imag,real),'g',Precision) + "rad";
      }
      return Text;
   }

// #########################################################################
   QString StringNum(double num, char form, int Precision)
   {
      int a = 0;
      char *p, Buffer[512], Format[6] = "%.00g";

      if(Precision < 0) {
         Format[1]  = form;
         Format[2]  = 0;
      }
      else {
         Format[4]  = form;
         Format[2] += Precision / 10;
         Format[3] += Precision % 10;
      }
      sprintf(Buffer, Format, num);
      p = strchr(Buffer, 'e');
      if(p) {
         p++;
         if(*(p++) == '+') { a = 1; }   // remove '+' of exponent
         if(*p == '0') { a++; p++; }    // remove leading zeros of exponent
         if(a > 0)
            do {
               *(p-a) = *p;
            } while(*(p++) != 0);    // override characters not needed
      }

      return QString(Buffer);
   }

// #########################################################################
   QString StringNiceNum(double num)
   {
      char Format[6] = "%.8e";
      if(fabs(num) < 1e-250)  return QString("0");  // avoid many problems
      if(fabs(log10(fabs(num))) < 3.0)  Format[3] = 'g';

      int a = 0;
      char *p, *pe, Buffer[512];

      sprintf(Buffer, Format, num);
      p = pe = strchr(Buffer, 'e');
      if(p) {
         if(*(++p) == '+') { a = 1; }    // remove '+' of exponent
         if(*(++p) == '0') { a++; p++; } // remove leading zeros of exponent
         if(a > 0)
            do {
               *(p-a) = *p;
            } while(*(p++) != 0);  // override characters not needed

         // In 'g' format, trailing zeros are already cut off !!!
         p = strchr(Buffer, '.');
         if(p) {
            if(!pe)  pe = Buffer + strlen(Buffer);
            p = pe-1;
            while(*p == '0')   // looking for unneccessary zero characters
               if((--p) <= Buffer)  break;
            if(*p != '.')  p++;  // no digit after decimal point ?
            while( (*(p++) = *(pe++)) != 0 ) ;  // overwrite zero characters
         }
      }

      return QString(Buffer);
   }

// #########################################################################
   void str2num(const QString& s_, double& Number, QString& Unit, double& Factor)
   {
      QString str = s_.simplified();

      QRegExp Expr( QRegExp("[^0-9\\x2E\\x2D\\x2B]") );
      int i = str.indexOf( Expr );
      if(i >= 0)
         if((str.at(i).toLatin1() | 0x20) == 'e') {
            int j = str.indexOf( Expr , ++i);
            if(j == i)  j--;
            i = j;
         }

      Number = str.left(i).toDouble();
      Unit   = str.mid(i).simplified();

      switch(Unit.at(0).toLatin1()) {
         case 'T': Factor = 1e12;  break;
         case 'G': Factor = 1e9;   break;
         case 'M': Factor = 1e6;   break;
         case 'k': Factor = 1e3;   break;
         case 'c': Factor = 1e-2;  break;
         case 'm': Factor = 1e-3;  break;
         case 'u': Factor = 1e-6;  break;
         case 'n': Factor = 1e-9;  break;
         case 'p': Factor = 1e-12; break;
         case 'f': Factor = 1e-15; break;
//    case 'd':
         default:  Factor = 1.0;
      }

      return;
   }

// #########################################################################
   QString num2str(double Num)
   {
      char c = 0;
      double cal = fabs(Num);
      if(cal > 1e-20) {
         cal = log10(cal) / 3.0;
         if(cal < -0.2)  cal -= 0.98;
         int Expo = int(cal);

         if(Expo >= -5) if(Expo <= 4)
                           switch(Expo) {
                              case -5: c = 'f'; break;
                              case -4: c = 'p'; break;
                              case -3: c = 'n'; break;
                              case -2: c = 'u'; break;
                              case -1: c = 'm'; break;
                              case  1: c = 'k'; break;
                              case  2: c = 'M'; break;
                              case  3: c = 'G'; break;
                              case  4: c = 'T'; break;
                           }

         if(c)  Num /= pow(10.0, double(3*Expo));
      }

      QString Str = QString::number(Num);
      if(c)  Str += c;

      return Str;
   }

// #########################################################################
   QString realToString(qreal val)
   {
      return QString::number(val,'f',2);
   }

// #########################################################################
   void convert2Unicode(QString& Text)
   {
      bool ok;
      int i = 0;
      QString n;
      unsigned short ch;
      while((i=Text.indexOf("\\x", i)) >= 0) {
         n = Text.mid(i, 6);
         ch = n.mid(2).toUShort(&ok, 16);
         if(ok)  Text.replace(n, QChar(ch));
         i++;
      }
      Text.replace("\\n", "\n");
      Text.replace("\\\\", "\\");
   }

// #########################################################################
   void convert2ASCII(QString& Text)
   {
      if(Text.isEmpty() || Text.isNull())
         return;
      Text.replace('\\', "\\\\");
      Text.replace('\n', "\\n");

      int i = 0;
      QChar ch;
      char Str[8];
      while(i < Text.size()) { // convert special characters
         ch = Text.at(i++);
         if(ch == QChar(0)) break;
         if(ch > QChar(0x7F)) {
            sprintf(Str, "\\x%04X", ch.unicode());
            Text.replace(ch, Str);
         }
      }
   }

// #########################################################################
// Takes a file name (with path) and replaces all special characters.
   QString properName (const QString& Name)
   {
      QString s = Name;
      QFileInfo Info(s);
      if(Info.suffix() == "sch")
         s = s.left(s.length()-4);
      if(s.at(0) <= '9') if(s.at(0) >= '0')
                            s = 'n' + s;
      s.replace(QRegExp("\\W"), "_"); // none [a-zA-Z0-9] into "_"
      s.replace("__", "_");  // '__' not allowed in VHDL
      if(s.at(0) == '_')
         s = 'n' + s;
      return s;
   }

// #########################################################################
// Checks and corrects a time (number & unit) according VHDL standard.
   bool VHDL_Time(QString& t, const QString& Name)
   {
      char *p;
      double Time = strtod(t.toLatin1().constData(), &p);
      while(*p == ' ') p++;
      for(;;) {
         if(Time >= 0.0) {
            if(strcmp(p, "fs") == 0)  break;
            if(strcmp(p, "ps") == 0)  break;
            if(strcmp(p, "ns") == 0)  break;
            if(strcmp(p, "us") == 0)  break;
            if(strcmp(p, "ms") == 0)  break;
            if(strcmp(p, "sec") == 0) break;
            if(strcmp(p, "min") == 0) break;
            if(strcmp(p, "hr") == 0)  break;
         }
         t = "§" + QObject::tr("Error: Wrong time format in \"%1\". Use positive number with units").arg(Name)
            + " fs, ps, ns, us, ms, sec, min, hr.\n";
         return false;
      }

      t = QString::number(Time) + " " + QString(p);  // the space is mandatory !
      return true;
   }

// #########################################################################
// Checks and corrects a time (number & unit) according Verilog standard.
   bool Verilog_Time(QString& t, const QString& Name)
   {
      char *p;
      double Time = strtod(t.toLatin1().constData(), &p);
      double factor = 1.0;
      while(*p == ' ') p++;
      for(;;) {
         if(Time >= 0.0) {
            if(strcmp(p, "fs") == 0) { factor = 1e-3; break; }
            if(strcmp(p, "ps") == 0) { factor = 1;  break; }
            if(strcmp(p, "ns") == 0) { factor = 1e3;  break; }
            if(strcmp(p, "us") == 0) { factor = 1e6;  break; }
            if(strcmp(p, "ms") == 0) { factor = 1e9;  break; }
            if(strcmp(p, "sec") == 0) { factor = 1e12; break; }
            if(strcmp(p, "min") == 0) { factor = 1e12*60; break; }
            if(strcmp(p, "hr") == 0)  { factor = 1e12*60*60; break; }
         }
         t = "§" + QObject::tr("Error: Wrong time format in \"%1\". Use positive number with units").arg(Name)
            + " fs, ps, ns, us, ms, sec, min, hr.\n";
         return false;
      }

      t = QString::number(Time*factor);  // the space is mandatory !
      return true;
   }

// #########################################################################
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
