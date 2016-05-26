/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include "ui_settingsdialog.h"

#include <QDialog>

namespace Caneda
{
    /*!
     * \brief This class constructs a configuration dialog.
     *
     * This class constructs a configuration dialog, showing settings pages.
     * Each settings page should provide an icon and a title, with a set of
     * different configuration options.
     */
    class SettingsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SettingsDialog(QWidget *parent = 0);

    private Q_SLOTS:
        void colorButtonDialog();
        QColor getButtonColor(QPushButton *button);
        void setButtonColor(QPushButton *button, QColor color);

        void slotAddLibrary();
        void slotRemoveLibrary();
        void slotAddHdlLibrary();
        void slotRemoveHdlLibrary();
        void slotGetNewLibraries();

        void simulationEngineChanged();

        void changePage(int index);
        void applySettings();

    private:
        void init();

        Ui::SettingsDialog ui;
    };

} // namespace Caneda

#endif //SETTINGS_DIALOG_H
