/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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
        QStringList libraries;
        libraries << Caneda::libDirectory() + "components/active";
        libraries << Caneda::libDirectory() + "components/passive";
        libraries << Caneda::libDirectory() + "components/semiconductor";
        settingsData["libraries/schematic"] = VariantPair(QStringList(libraries));
        settingsData["libraries/hdl"] = VariantPair(QStringList(Caneda::libDirectory() + "hdl"));

        settingsData["gui/geometry"] = VariantPair(QByteArray());
        settingsData["gui/dockPositions"] = VariantPair(QByteArray());
        settingsData["gui/gridVisible"] = VariantPair(bool(true));
        settingsData["gui/foregroundColor"] = VariantPair(QColor(Qt::darkGray));
        settingsData["gui/backgroundColor"] = VariantPair(QColor(Qt::white));
        settingsData["gui/simulationBackgroundColor"] = VariantPair(QColor(Qt::white));
        settingsData["gui/lineColor"] = VariantPair(QColor(Qt::blue));
        settingsData["gui/selectionColor"] = VariantPair(QColor(255, 128, 0)); // Dark orange
        settingsData["gui/lineWidth"] = VariantPair(int(1));
        settingsData["gui/font"] = VariantPair(QFont());
        settingsData["gui/largeFontSize"] = VariantPair(qreal(16.0));
        settingsData["gui/iconSize"] = VariantPair(QSize(24, 24));
        settingsData["gui/maxUndo"] = VariantPair(int(20));

        settingsData["gui/hdl/keyword"]= VariantPair(QVariant(QColor(Qt::black)));
        settingsData["gui/hdl/type"]= VariantPair(QVariant(QColor(Qt::blue)));
        settingsData["gui/hdl/attribute"]= VariantPair(QVariant(QColor(Qt::darkCyan)));
        settingsData["gui/hdl/block"] = VariantPair(QColor(Qt::darkBlue));
        settingsData["gui/hdl/class"] = VariantPair(QColor(Qt::darkMagenta));
        settingsData["gui/hdl/data"]= VariantPair(QVariant(QColor(Qt::darkGreen)));
        settingsData["gui/hdl/comment"] = VariantPair(QColor(Qt::red));
        settingsData["gui/hdl/system"] = VariantPair(QColor(Qt::darkYellow));

        settingsData["gui/layout/metal1"] = VariantPair(QColor(Qt::blue));
        settingsData["gui/layout/metal2"] = VariantPair(QColor(Qt::gray));
        settingsData["gui/layout/poly1"] = VariantPair(QColor(Qt::red));
        settingsData["gui/layout/poly2"] = VariantPair(QColor(Qt::darkRed));
        settingsData["gui/layout/active"] = VariantPair(QColor(Qt::green));
        settingsData["gui/layout/contact"] = VariantPair(QColor(Qt::black));
        settingsData["gui/layout/nwell"] = VariantPair(QColor(Qt::darkYellow));
        settingsData["gui/layout/pwell"] = VariantPair(QColor(Qt::darkCyan));
    }

    Settings::~Settings()
    {
    }

    QVariant Settings::currentValue(const QString& key) const
    {
        if (!settingsData[key].currentValue.isValid()) {
            return settingsData[key].defaultValue;
        }
        return settingsData[key].currentValue;
    }

    QVariant Settings::defaultValue(const QString& key) const
    {
        return settingsData[key].defaultValue;
    }

    void Settings::setCurrentValue(const QString& key, const QVariant& value)
    {
        settingsData[key].currentValue = value.isValid() ? value : settingsData[key].defaultValue;
    }

    bool Settings::load(QSettings &settings)
    {
        QStringList childKeys = settingsData.keys();
        foreach (const QString& childKey, childKeys) {
            setCurrentValue(childKey, settings.value(childKey));
        }

        return true;
    }

    bool Settings::save(QSettings &settings)
    {
        QStringList childKeys = settingsData.keys();
        foreach (const QString& childKey, childKeys) {
            settings.setValue(childKey, currentValue(childKey));
        }
    }

    Settings* Settings::instance()
    {
        static Settings *instance = 0;
        if (!instance) {
            instance = new Settings();
        }
        return instance;
    }

} // namespace Caneda
