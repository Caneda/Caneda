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
#include "global.h"

#include <QColor>
#include <QFont>
#include <QRect>
#include <QSettings>
#include <QSize>

namespace Caneda
{
    VariantPair::VariantPair(const QVariant& def, const QVariant &cur)
    {
        defaultValue = def;
        currentValue = cur.isValid() ? cur : def;
    }

    Settings::Settings()
    {
        // The assumed default installation path of caneda components is
        // <INSTALL>/share/caneda/components
        data["sidebarLibrary"] = VariantPair(QString(Caneda::baseDirectory() + "components/"));

        data["gui/geometry"] = VariantPair(QByteArray());
        data["gui/dockPositions"] = VariantPair(QByteArray());
        data["gui/gridVisible"] = VariantPair(bool(true));
        data["gui/foregroundColor"] = VariantPair(QColor(Qt::darkGray));
        data["gui/backgroundColor"] = VariantPair(QColor(Qt::white));
        data["gui/font"] = VariantPair(QFont());
        data["gui/largeFontSize"] = VariantPair(qreal(16.0));
        data["gui/iconSize"] = VariantPair(QSize(24, 24));
        data["gui/maxUndo"] = VariantPair(int(20));

        data["gui/hdl/keyword"]= VariantPair(QVariant(QColor(Qt::black)));
        data["gui/hdl/type"]= VariantPair(QVariant(QColor(Qt::blue)));
        data["gui/hdl/attribute"]= VariantPair(QVariant(QColor(Qt::darkCyan)));
        data["gui/hdl/block"] = VariantPair(QColor(Qt::darkBlue));
        data["gui/hdl/class"] = VariantPair(QColor(Qt::darkMagenta));
        data["gui/hdl/data"]= VariantPair(QVariant(QColor(Qt::darkGreen)));
        data["gui/hdl/comment"] = VariantPair(QColor(Qt::red));
        data["gui/hdl/system"] = VariantPair(QColor(Qt::darkYellow));

        QString canedaFilter =
                QObject::tr("Schematic-xml")+" (*.xsch);;"+
                QObject::tr("Symbol-xml")+" (*.xsym);;"+
                QObject::tr("Caneda Project")+" (*.xpro);;"+
                QObject::tr("Data Display")+" (*.dpl);;"+
                QObject::tr("HDL Sources")+" (*.vhdl *.vhd *.v);;"+
                QObject::tr("Any File")+" (*)";
        data["nosave/canedaFilter"] = VariantPair(canedaFilter);
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
        foreach (const QString& childKey, childKeys) {
            setCurrentValue(childKey, settings.value(childKey));
        }

        return true;
    }

    bool Settings::save(QSettings &settings)
    {
        QStringList childKeys = data.keys();
        foreach (const QString& childKey, childKeys) {
            // Save only keys which aren't having "nosave/" prefix.
            if (!childKey.startsWith("nosave/")) {
                settings.setValue(childKey, currentValue(childKey));
            }
        }
    }

    Settings* Settings::instance()
    {
        static Settings *instance = 0;
        if (!instance) {
            instance = new Settings;
        }
        return instance;
    }

} // namespace Caneda
