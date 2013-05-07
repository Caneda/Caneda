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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

// Forward declarations
class QSettings;

namespace Caneda
{
    /*!
     * \brief This class handles all of Caneda's settings.
     *
     * This is used to provide a unified way of accesing and storing the user
     * settings, as well as the default values of those settings.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa SettingsDialog
     */
    class Settings
    {
    public:
        ~Settings();

        QVariant currentValue(const QString& key) const;
        QVariant defaultValue(const QString& key) const;

        void setCurrentValue(const QString& key, const QVariant& value);

        bool load();
        bool save();

        static Settings* instance();

    private:
        Settings();

        QMap<QString, QVariant> defaultSettings;
        QMap<QString, QVariant> currentSettings;
    };

} // namespace Caneda

#endif //SETTINGS_H
