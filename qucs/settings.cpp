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

#include "settings.h"

#include "config.h"

#include <QColor>
#include <QFont>
#include <QRect>
#include <QSettings>
#include <QSize>

VariantPair::VariantPair(const QVariant& def, const QVariant &cur)
{
    defaultValue = def;
    currentValue = cur.isValid() ? cur : def;
}

Settings::Settings()
{
    // The assumed default installation path of qucscomponents is
    // <INSTALL>/share/qucs/qucscomponents
    data["sidebarLibrary"] = VariantPair(QString(BASEDIR"/qucscomponents/"));

    data["gui/geometry"] = VariantPair(QByteArray());
    data["gui/dockPositions"] = VariantPair(QByteArray());
    data["gui/backgroundColor"] = VariantPair(QColor(Qt::white));
    data["gui/font"] = VariantPair(QFont());
    data["gui/largeFontSize"] = VariantPair(qreal(16.0));
    data["gui/iconSize"] = VariantPair(QSize(24, 24));
    data["gui/maxUndo"] = VariantPair(int(20));
    data["gui/textEditor"] = VariantPair(QString(BINARYDIR"/qucsedit"));
    data["gui/language"] = VariantPair(QString());
    data["gui/fileTypes"] = VariantPair(QStringList());

    data["gui/vhdl/comment"] = VariantPair(QColor(Qt::gray));
    data["gui/vhdl/string"]= VariantPair(QVariant(QColor(Qt::red)));
    data["gui/vhdl/integer"]= VariantPair(QVariant(QColor(Qt::blue)));
    data["gui/vhdl/real"]= VariantPair(QVariant(QColor(Qt::darkMagenta)));
    data["gui/vhdl/character"]= VariantPair(QVariant(QColor(Qt::magenta)));
    data["gui/vhdl/types"]= VariantPair(QVariant(QColor(Qt::darkRed)));
    data["gui/vhdl/attributes"]= VariantPair(QVariant(QColor(Qt::darkCyan)));
}

Settings::~Settings()
{
}

QVariant Settings::currentValue(const QString& key) const
{
    if (!data[key].currentValue.isValid()) {
        return data[key].defaultValue;
    }
    return data[key].currentValue;
}

QVariant Settings::defaultValue(const QString& key) const
{
    return data[key].defaultValue;
}

void Settings::setCurrentValue(const QString& key, const QVariant& value)
{
    data[key].currentValue = value.isValid() ? value : data[key].defaultValue;
}

bool Settings::load(QSettings &settings)
{
    QStringList childKeys = data.keys();
    childKeys.removeAll("gui/fileTypes"); // Requires special treatment
    foreach (const QString& childKey, childKeys) {
        setCurrentValue(childKey, settings.value(childKey));
    }

    QStringList fileTypes;
    int size = settings.beginReadArray("gui/fileTypes");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString suffix = settings.value("suffix").toString();
        QString program = settings.value("program").toString();
        fileTypes.append(suffix + QChar(':') + program);
    }
    settings.endArray();
    setCurrentValue("gui/fileTypes", fileTypes);
    return true;
}

bool Settings::save(QSettings &settings)
{
    QStringList childKeys = data.keys();
    childKeys.removeAll("gui/fileTypes"); // Requires special treatment
    foreach (const QString& childKey, childKeys) {
        settings.setValue(childKey, currentValue(childKey));
    }

    QStringList fileTypes = currentValue("gui/fileTypes").toStringList();
    settings.beginWriteArray("gui/fileTypes");
    for (int i = 0; i < fileTypes.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("suffix", fileTypes.at(i).section(':', 0, 0));
        settings.setValue("program", fileTypes.at(i).section(':', 1, 1));
    }
    settings.endArray();
}

Settings* Settings::instance()
{
    static Settings *instance = 0;
    if (!instance) {
        instance = new Settings;
    }
    return instance;
}
