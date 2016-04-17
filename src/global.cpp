/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#include <QDir>
#include <QIcon>

using namespace std;

namespace Caneda
{
    QString baseDirectory()
    {
        const QString var(BASEDIR);
        QDir CanedaDir = QDir(var);
        return QDir::toNativeSeparators(CanedaDir.canonicalPath() + "/");
    }

    QString binaryDirectory()
    {
        const QString var(BINARYDIR);
        QDir CanedaDir = QDir(var);
        return QDir::toNativeSeparators(CanedaDir.canonicalPath() + "/");
    }

    QString imageDirectory()
    {
        const QString var(IMAGEDIR);
        QDir CanedaDir = QDir(var);
        return QDir::toNativeSeparators(CanedaDir.canonicalPath() + "/");
    }

    QString langDirectory()
    {
        const QString var(LANGUAGEDIR);
        QDir CanedaDir = QDir(var);
        return QDir::toNativeSeparators(CanedaDir.canonicalPath() + "/");
    }

    QString libDirectory()
    {
        const QString var(LIBRARYDIR);
        QDir CanedaDir = QDir(var);
        return QDir::toNativeSeparators(CanedaDir.canonicalPath() + "/");
    }

    QString version()
    {
        const QString var(PACKAGE_VERSION);
        return var;
    }

    QString versionString()
    {
        const QString var(PACKAGE_STRING);
        return var;
    }

    /*!
    * \brief Returns an icon from current theme or a fallback default.
    */
    QIcon icon(const QString& iconName)
    {
        return QIcon::fromTheme(iconName, QIcon(Caneda::imageDirectory() + iconName + ".png"));
    }

    QString localePrefix()
    {
        QString retVal = QLocale::system().name();
        retVal = retVal.left(retVal.indexOf('_'));
        return retVal;
    }

    bool checkVersion(const QString& Line)
    {
        QStringList sl = Caneda::version().split('.', QString::SkipEmptyParts);
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

    /*!
    * \brief Special characters used in LaTeX to unicode and unicode
    * to LaTeX conversions.
    */
    struct tSpecialChar {
        char Mnemonic[16];
        unsigned short Unicode;
    };

    struct tSpecialChar SpecialChars[] = {
        {"alpha", 0x03B1}, {"beta", 0x03B2}, {"gamma", 0x03B3},
        {"delta", 0x03B4}, {"epsilon", 0x03B5}, {"zeta", 0x03B6},
        {"eta", 0x03B7}, {"theta", 0x03B8}, {"iota", 0x03B9},
        {"kappa", 0x03BA}, {"lambda", 0x03BB}, {"mu", 0x03BC},
        {"nu", 0x03BD}, {"xi", 0x03BE}, {"pi", 0x03C0},
        {"rho", 0x03C1}, {"sigma", 0x03C3}, {"tau", 0x03C4},
        {"upsilon", 0x03C5}, {"phi", 0x03C6}, {"chi", 0x03C7},
        {"psi", 0x03C8}, {"omega", 0x03C9},

        {"varpi", 0x03D6}, {"varrho", 0x03F1},

        {"Gamma", 0x0393}, {"Delta", 0x0394}, {"Theta", 0x0398},
        {"Lambda", 0x039B}, {"Xi", 0x039E}, {"Pi", 0x03A0},
        {"Sigma", 0x03A3}, {"Upsilon", 0x03A5}, {"Phi", 0x03A6},
        {"Psi", 0x03A8}, {"Omega", 0x03A9},

        {"textmu", 0x00B5}, {"cdot", 0x00B7}, {"times", 0x00D7},
        {"pm", 0x00B1}, {"mp", 0x2213}, {"partial", 0x2202},
        {"nabla", 0x2207}, {"infty", 0x221E}, {"int", 0x222B},
        {"approx", 0x2248}, {"neq", 0x2260}, {"in", 0x220A},
        {"leq", 0x2264}, {"geq", 0x2265}, {"sim", 0x223C},
        {"propto", 0x221D}, {"onehalf", 0x00BD}, {"onequarter", 0x00BC},
        {"twosuperior", 0x00B2}, {"threesuperior", 0x00B3},
        {"diameter", 0x00F8}, {"ohm", 0x03A9},

        {"", 0}  // end mark
    };

    /*!
    * \brief This function replaces the LaTeX tags for special characters
    * into its unicode value.
    */
    QString latexToUnicode(const QString& Input)
    {
        int Begin = 0, End = 0;
        struct tSpecialChar *p;

        QString Output = "";
        Output.reserve(Input.size());

        while((Begin=Input.indexOf('\\', Begin)) >= 0) {
            Output += Input.mid(End, Begin - End);
            End = Begin++;

            p = SpecialChars;
            while(p->Unicode != 0) {  // test all special characters
                if(Input.mid(Begin).startsWith(p->Mnemonic)) {
                    Output += QChar(p->Unicode);
                    End = Begin + qstrlen(p->Mnemonic);
                    break;
                }
                else {
                    p++;
                }
            }
        }
        Output += Input.mid(End);
        return Output;
    }

    /*!
    * \brief This function replaces the unicode of special characters
    * by its LaTeX tags.
    */
    QString unicodeToLatex(QString Output)
    {
        struct tSpecialChar *p = SpecialChars;
        while(p->Unicode != 0) {   // test all special characters
            Output.replace(QChar(p->Unicode), "\\" + QString(p->Mnemonic));
            p++;
        }
        return Output;
    }

    /*!
     * \brief Get nearest grid point (grid snapping)
     *
     * \param pos: position to be rounded
     * \return rounded position
     */
    QPointF smartNearingGridPoint(const QPointF &pos)
    {
        const QPoint point = pos.toPoint();

        int x = qAbs(point.x());
        x += (Caneda::DefaultGridSpace >> 1);
        x -= x % Caneda::DefaultGridSpace;
        x *= sign(point.x());

        int y = qAbs(point.y());
        y += (Caneda::DefaultGridSpace >> 1);
        y -= y % Caneda::DefaultGridSpace;
        y *= sign(point.y());

        return QPointF(x, y);
    }

} // namespace Caneda
