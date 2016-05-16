/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "settings.h"

#include "global.h"

#include <QColor>
#include <QFont>
#include <QSettings>
#include <QSize>
#include <QVariant>

namespace Caneda
{
    //! \brief Constructor.
    Settings::Settings(QObject *parent) : QObject(parent)
    {
        QStringList libraries;
        libraries << Caneda::libDirectory() + "components/active";
        libraries << Caneda::libDirectory() + "components/cmos";
        libraries << Caneda::libDirectory() + "components/miscellaneous";
        libraries << Caneda::libDirectory() + "components/models";
        libraries << Caneda::libDirectory() + "components/passive";
        libraries << Caneda::libDirectory() + "components/semiconductor";
        libraries << Caneda::libDirectory() + "components/simulations";
        libraries << Caneda::libDirectory() + "components/sources";
        libraries << Caneda::libDirectory() + "components/transmission";

        defaultSettings["libraries/schematic"] = QVariant(QStringList(libraries));
        defaultSettings["libraries/hdl"] = QVariant(QStringList(Caneda::libDirectory() + "hdl"));

        defaultSettings["gui/showMenuBar"] = QVariant(bool(true));
        defaultSettings["gui/showToolBar"] = QVariant(bool(true));
        defaultSettings["gui/showStatusBar"] = QVariant(bool(true));
        defaultSettings["gui/showSideBarBrowser"] = QVariant(bool(true));
        defaultSettings["gui/showFolderBrowser"] = QVariant(bool(true));
        defaultSettings["gui/showFullScreen"] = QVariant(bool(false));

        defaultSettings["gui/gridVisible"] = QVariant(bool(true));
        defaultSettings["gui/foregroundColor"] = QVariant(QColor(Qt::darkGray));
        defaultSettings["gui/backgroundColor"] = QVariant(QColor(Qt::white));
        defaultSettings["gui/simulationBackgroundColor"] = QVariant(QColor(Qt::white));
        defaultSettings["gui/lineColor"] = QVariant(QColor(Qt::blue));
        defaultSettings["gui/selectionColor"] = QVariant(QColor(255, 128, 0)); // Dark orange
        defaultSettings["gui/lineWidth"] = QVariant(int(1));

        defaultSettings["gui/hdl/keyword"]= QVariant(QVariant(QColor(Qt::black)));
        defaultSettings["gui/hdl/type"]= QVariant(QVariant(QColor(Qt::blue)));
        defaultSettings["gui/hdl/attribute"]= QVariant(QVariant(QColor(Qt::darkCyan)));
        defaultSettings["gui/hdl/block"] = QVariant(QColor(Qt::darkBlue));
        defaultSettings["gui/hdl/class"] = QVariant(QColor(Qt::darkMagenta));
        defaultSettings["gui/hdl/data"]= QVariant(QVariant(QColor(Qt::darkGreen)));
        defaultSettings["gui/hdl/comment"] = QVariant(QColor(Qt::red));
        defaultSettings["gui/hdl/system"] = QVariant(QColor(Qt::darkYellow));

        defaultSettings["gui/layout/metal1"] = QVariant(QColor(Qt::blue));
        defaultSettings["gui/layout/metal2"] = QVariant(QColor(Qt::gray));
        defaultSettings["gui/layout/poly1"] = QVariant(QColor(Qt::red));
        defaultSettings["gui/layout/poly2"] = QVariant(QColor(Qt::darkRed));
        defaultSettings["gui/layout/active"] = QVariant(QColor(Qt::green));
        defaultSettings["gui/layout/contact"] = QVariant(QColor(Qt::black));
        defaultSettings["gui/layout/nwell"] = QVariant(QColor(Qt::darkYellow));
        defaultSettings["gui/layout/pwell"] = QVariant(QColor(Qt::darkCyan));

        defaultSettings["sim/simulationEngine"] = QVariant(QString("ngspice"));  //! \todo In the future this could be replaced by an enum, to avoid problems
        defaultSettings["sim/simulationCommand"] = QVariant(QString("ngspice -b -r %filename.raw %filename.net"));
        defaultSettings["sim/outputFormat"] = QVariant(QString("binary"));  //! \todo In the future this could be replaced by an enum, to avoid problems

        currentSettings = defaultSettings;
    }

    //! \copydoc MainWindow::instance()
    Settings* Settings::instance()
    {
        static Settings *instance = 0;
        if (!instance) {
            instance = new Settings();
        }
        return instance;
    }

    /*!
     * \brief Get the current value of the selected setting.
     *
     * \param key Setting to look for.
     * \return QVariant value of the setting.
     *
     * \sa setCurrentValue
     */
    QVariant Settings::currentValue(const QString& key) const
    {
        if (!currentSettings[key].isValid()) {
            return defaultSettings[key];
        }
        return currentSettings[key];
    }

    /*!
     * \brief Get the default value of the selected setting.
     *
     * \param key Setting to look for.
     * \return QVariant default value of the setting.
     */
    QVariant Settings::defaultValue(const QString& key) const
    {
        return defaultSettings[key];
    }

    /*!
     * \brief Set a new value for the selected setting.
     *
     * \param key Setting to change its value.
     * \param value New value of the setting.
     *
     * \sa currentValue
     */
    void Settings::setCurrentValue(const QString& key, const QVariant& value)
    {
        currentSettings[key] = value.isValid() ? value : defaultSettings[key];
    }

    /*!
     * \brief Load stored settings values.
     *
     * \return True on success, false otherwise.
     */
    bool Settings::load()
    {
        QSettings settings;

        QStringList childKeys = settings.allKeys();
        foreach (const QString& childKey, childKeys) {
            setCurrentValue(childKey, settings.value(childKey));
        }

        return true;
    }

    /*!
     * \brief Store settings values.
     *
     * \return True on success, false otherwise.
     */
    bool Settings::save()
    {
        QSettings settings;

        QStringList childKeys = currentSettings.keys();
        foreach (const QString& childKey, childKeys) {
            settings.setValue(childKey, currentValue(childKey));
        }
    }

} // namespace Caneda
