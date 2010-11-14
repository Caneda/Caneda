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

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <QDebug>
#include <QVariant>

// Forward declarations
class QIcon;

namespace Caneda
{
    QString baseDirectory();
    QString binaryDirectory();
    QString bitmapDirectory();
    QString docDirectory();
    QString langDirectory();
    QString libDirectory();

    QString version();
    QString versionString();

    QIcon icon(const QString& iconName);
    QString pathForCanedaFile(const QString& fileName);

    QString localePrefix();

    bool checkVersion(const QString& line);

    QVariant::Type stringToType(const QString& string);
    QString typeToString(QVariant::Type type);

    inline QString boolToString(bool boolean) {
        return boolean ? QString("true") : QString("false");
    }

    inline bool stringToBool(const QString& str) {
        return str == "true" ? true : false;
    }

    inline QString realToString(qreal val) {
        return QString::number(val,'f',2);
    }

    QString latexToUnicode(const QString& input);
    QString unicodeToLatex(QString unicode);

    struct ZoomRange
    {
        const qreal min;
        const qreal max;

        ZoomRange(qreal _min = 0., qreal _max = 1.0) :
                min(_min), max(_max)
        {
            if (max < min) {
                qWarning() << Q_FUNC_INFO << "Invalid range" << min << max;
            }
        }

        bool contains(qreal value) const {
            return value >= min && value <= max;
        }
    };

} // namespace Caneda

#endif //GLOBAL_H
