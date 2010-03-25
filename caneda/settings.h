/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

class QSettings;

struct VariantPair
{
    QVariant defaultValue;
    QVariant currentValue;

    VariantPair(const QVariant& def = QVariant(),
            const QVariant& cur = QVariant());
};

typedef QMap<QString, VariantPair> SettingsData;

struct Settings
{
    ~Settings();

    QVariant currentValue(const QString& key) const;
    QVariant defaultValue(const QString& key) const;

    void setCurrentValue(const QString& key, const QVariant& value);

    bool load(QSettings &settings);
    bool save(QSettings &settings);

    static Settings* instance();
private:
    SettingsData data;
    Settings();
};

#endif //SETTINGS_H
