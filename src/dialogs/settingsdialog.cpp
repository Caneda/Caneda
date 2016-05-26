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

#include "settingsdialog.h"

#include "global.h"
#include "settings.h"

#include <QColorDialog>
#include <QDesktopServices>
#include <QFileDialog>

namespace Caneda
{
    /*!
     * \brief Construct a new Settings dialog.
     *
     * \param parent Parent of this dialog.
     */
    SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
    {
        ui.setupUi(this);

        // List of pages
        ui.pagesList->item(0)->setIcon(Caneda::icon("preferences-other"));
        ui.pagesList->item(1)->setIcon(Caneda::icon("library"));
        ui.pagesList->item(2)->setIcon(Caneda::icon("media-playback-start"));
        ui.pagesList->item(3)->setIcon(Caneda::icon("view-grid"));
        ui.pagesList->item(4)->setIcon(Caneda::icon("code-context"));

        init();

        // Signals/slots connections
        connect(ui.buttons, SIGNAL(accepted()), this, SLOT(applySettings()));
        connect(ui.buttons, SIGNAL(rejected()), this, SLOT(reject()));
        connect(ui.pagesList, SIGNAL(currentRowChanged(int)), this, SLOT(changePage(int)));

        // General group of settings
        connect(ui.buttonBackground, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonSimulationBackground, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonForeground, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonLine, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonSelection, SIGNAL(clicked()), SLOT(colorButtonDialog()));

        // Libraries group of settings
        connect(ui.buttonAddLibrary, SIGNAL(clicked()), SLOT(slotAddLibrary()));
        connect(ui.buttonRemoveLibrary, SIGNAL(clicked()), SLOT(slotRemoveLibrary()));
        connect(ui.buttonAddHdlLibrary, SIGNAL(clicked()), SLOT(slotAddHdlLibrary()));
        connect(ui.buttonRemoveHdlLibrary, SIGNAL(clicked()), SLOT(slotRemoveHdlLibrary()));
        connect(ui.buttonGetNewLibraries, SIGNAL(clicked()), SLOT(slotGetNewLibraries()));

        // Simulation group of settings
        connect(ui.radioNgspiceMode, SIGNAL(clicked()), SLOT(simulationEngineChanged()));
        connect(ui.radioCustomMode, SIGNAL(clicked()), SLOT(simulationEngineChanged()));

        // Layout group of settings
        connect(ui.buttonMetal1, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonMetal2, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonPoly1, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonPoly2, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonActive, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonContact, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonNwell, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonPwell, SIGNAL(clicked()), SLOT(colorButtonDialog()));

        // HDL group of settings
        connect(ui.buttonKeyword, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonType, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonAttribute, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonBlock, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonClass, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonData, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonComment, SIGNAL(clicked()), SLOT(colorButtonDialog()));
        connect(ui.buttonSystem, SIGNAL(clicked()), SLOT(colorButtonDialog()));
    }

    /*!
     * \brief Opens a color selection dialog and sets the selected color to a
     * button.
     */
    void SettingsDialog::colorButtonDialog()
    {
        QPushButton *button = qobject_cast<QPushButton*>(sender());
        if(!button) {
            return;
        }

        QColor color = QColorDialog::getColor(getButtonColor(button), this);
        if(color.isValid()) {
            setButtonColor(button, color);
        }
    }

    //! \brief Get the color of a button.
    QColor SettingsDialog::getButtonColor(QPushButton *button)
    {
        QPalette palette(button->palette());
        return palette.color(button->backgroundRole());
    }

    //! \brief Set the color of a button.
    void SettingsDialog::setButtonColor(QPushButton *button, QColor color)
    {
        QPalette palette(button->palette());
        palette.setColor(button->backgroundRole(), color);
        button->setPalette(palette);
    }

    //! \brief Add a new library to the libraries list
    void SettingsDialog::slotAddLibrary()
    {
        QString dir = QFileDialog::getExistingDirectory(this,
                                                        tr("Select Library"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(!dir.isEmpty()) {
            new QListWidgetItem(dir, ui.listLibraries);
        }

        ui.listLibraries->sortItems(Qt::AscendingOrder);
    }

    //! \brief Remove a library from the libraries list
    void SettingsDialog::slotRemoveLibrary()
    {
        qDeleteAll(ui.listLibraries->selectedItems());
    }

    //! \brief Add a new HDL library to the libraries list
    void SettingsDialog::slotAddHdlLibrary()
    {
        QString dir = QFileDialog::getExistingDirectory(this,
                                                        tr("Select Library"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(!dir.isEmpty()) {
            new QListWidgetItem(dir, ui.listHdlLibraries);
        }

        ui.listHdlLibraries->sortItems(Qt::AscendingOrder);
    }

    //! \brief Remove a HDL library from the libraries list
    void SettingsDialog::slotRemoveHdlLibrary()
    {
        qDeleteAll(ui.listHdlLibraries->selectedItems());
    }

    //! \brief Open the get new libraries repository
    void SettingsDialog::slotGetNewLibraries()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/Caneda/Libraries"));
    }

    //! \brief Updates the lineSimulationCommand enabled status.
    void SettingsDialog::simulationEngineChanged()
    {
        if(ui.radioCustomMode->isChecked()) {
            ui.lineSimulationCommand->setEnabled(true);
        }
        else {
            ui.lineSimulationCommand->setEnabled(false);
        }
    }

    //! \brief Change the currently displayed page in the dialog.
    void SettingsDialog::changePage(int index)
    {
        ui.pagesWidget->setCurrentIndex(index);
    }

    //! \brief Apply the settings of all pages.
    void SettingsDialog::applySettings()
    {
        Settings *settings = Settings::instance();

        // General group of settings
        settings->setCurrentValue("gui/gridVisible", ui.checkShowGrid->isChecked());

        settings->setCurrentValue("gui/backgroundColor", getButtonColor(ui.buttonBackground));
        settings->setCurrentValue("gui/simulationBackgroundColor", getButtonColor(ui.buttonSimulationBackground));
        settings->setCurrentValue("gui/foregroundColor", getButtonColor(ui.buttonForeground));
        settings->setCurrentValue("gui/lineColor", getButtonColor(ui.buttonLine));
        settings->setCurrentValue("gui/selectionColor", getButtonColor(ui.buttonSelection));

        settings->setCurrentValue("gui/lineWidth", ui.spinWidth->value());

        // Libraries group of settings
        QStringList newLibraries;
        for(int i=0; i<ui.listLibraries->count(); i++) {
            newLibraries << ui.listLibraries->item(i)->text();
        }
        settings->setCurrentValue("libraries/schematic", newLibraries);

        QStringList newHdlLibraries;
        for(int i=0; i<ui.listHdlLibraries->count(); i++) {
            newHdlLibraries << ui.listHdlLibraries->item(i)->text();
        }
        settings->setCurrentValue("libraries/hdl", newHdlLibraries);

        // Simulation group of settings
        if(ui.radioNgspiceMode->isChecked()) {
            // If using ngspice simulator, the command to simulate is:
            // ngspice -b -r output.raw input.net
            settings->setCurrentValue("sim/simulationEngine", QString("ngspice"));
            settings->setCurrentValue("sim/simulationCommand", QString("ngspice -b -r %filename.raw %filename.net"));
        }
        else if(ui.radioCustomMode->isChecked()) {
            // If using the custom simulator, take the user provided command
            settings->setCurrentValue("sim/simulationEngine", QString("custom"));
            settings->setCurrentValue("sim/simulationCommand", ui.lineSimulationCommand->text());
        }

        if(ui.radioBinaryMode->isChecked()) {
            settings->setCurrentValue("sim/outputFormat", QString("binary"));
        }
        else if(ui.radioAsciiMode->isChecked()) {
            settings->setCurrentValue("sim/outputFormat", QString("ascii"));
        }

        // Layout group of settings
        settings->setCurrentValue("gui/layout/metal1", getButtonColor(ui.buttonMetal1));
        settings->setCurrentValue("gui/layout/metal2", getButtonColor(ui.buttonMetal2));
        settings->setCurrentValue("gui/layout/poly1", getButtonColor(ui.buttonPoly1));
        settings->setCurrentValue("gui/layout/poly2", getButtonColor(ui.buttonPoly2));
        settings->setCurrentValue("gui/layout/active", getButtonColor(ui.buttonActive));
        settings->setCurrentValue("gui/layout/contact", getButtonColor(ui.buttonContact));
        settings->setCurrentValue("gui/layout/nwell", getButtonColor(ui.buttonNwell));
        settings->setCurrentValue("gui/layout/pwell", getButtonColor(ui.buttonPwell));

        // HDL group of settings
        settings->setCurrentValue("gui/hdl/keyword", getButtonColor(ui.buttonKeyword));
        settings->setCurrentValue("gui/hdl/type", getButtonColor(ui.buttonType));
        settings->setCurrentValue("gui/hdl/attribute", getButtonColor(ui.buttonAttribute));
        settings->setCurrentValue("gui/hdl/block", getButtonColor(ui.buttonBlock));
        settings->setCurrentValue("gui/hdl/class", getButtonColor(ui.buttonClass));
        settings->setCurrentValue("gui/hdl/data", getButtonColor(ui.buttonData));
        settings->setCurrentValue("gui/hdl/comment", getButtonColor(ui.buttonComment));
        settings->setCurrentValue("gui/hdl/system", getButtonColor(ui.buttonSystem));

        // Save settings and accept dialog
        settings->save();

        accept();
    }

    //! \brief Load current settings into each settings page
    void SettingsDialog::init()
    {
        // Read the settings into the dialog widgets
        Settings *settings = Settings::instance();

        QColor currentColor;

        // General group of settings
        ui.checkShowGrid->setChecked(settings->currentValue("gui/gridVisible").value<bool>());

        currentColor = settings->currentValue("gui/backgroundColor").value<QColor>();
        setButtonColor(ui.buttonBackground, currentColor);
        currentColor = settings->currentValue("gui/simulationBackgroundColor").value<QColor>();
        setButtonColor(ui.buttonSimulationBackground, currentColor);
        currentColor = settings->currentValue("gui/foregroundColor").value<QColor>();
        setButtonColor(ui.buttonForeground, currentColor);
        currentColor = settings->currentValue("gui/lineColor").value<QColor>();
        setButtonColor(ui.buttonLine, currentColor);
        currentColor = settings->currentValue("gui/selectionColor").value<QColor>();
        setButtonColor(ui.buttonSelection, currentColor);

        ui.spinWidth->setValue(settings->currentValue("gui/lineWidth").toInt());

        // Libraries group of settings
        QStringList libraries;
        libraries << settings->currentValue("libraries/schematic").toStringList();
        foreach (const QString &str, libraries) {
            ui.listLibraries->addItem(str);
        }
        ui.listLibraries->sortItems(Qt::AscendingOrder);

        QStringList hdlLibraries;
        hdlLibraries << settings->currentValue("libraries/hdl").toStringList();
        foreach (const QString &str, hdlLibraries) {
            ui.listHdlLibraries->addItem(str);
        }
        ui.listHdlLibraries->sortItems(Qt::AscendingOrder);

        // Simulation group of settings
        ui.lineSimulationCommand->setText(settings->currentValue("sim/simulationCommand").toString());

        if(settings->currentValue("sim/simulationEngine").toString() == "ngspice") {
            ui.radioNgspiceMode->setChecked(true);
            ui.lineSimulationCommand->setEnabled(false);
        }
        else if(settings->currentValue("sim/simulationEngine").toString() == "custom") {
            ui.radioCustomMode->setChecked(true);
        }

        if(settings->currentValue("sim/outputFormat").toString() == "binary") {
            ui.radioBinaryMode->setChecked(true);
        }
        else if(settings->currentValue("sim/outputFormat").toString() == "ascii") {
            ui.radioAsciiMode->setChecked(true);
        }

        // Layout group of settings
        currentColor = settings->currentValue("gui/layout/metal1").value<QColor>();
        setButtonColor(ui.buttonMetal1, currentColor);
        currentColor = settings->currentValue("gui/layout/metal2").value<QColor>();
        setButtonColor(ui.buttonMetal2, currentColor);
        currentColor = settings->currentValue("gui/layout/poly1").value<QColor>();
        setButtonColor(ui.buttonPoly1, currentColor);
        currentColor = settings->currentValue("gui/layout/poly2").value<QColor>();
        setButtonColor(ui.buttonPoly2, currentColor);
        currentColor = settings->currentValue("gui/layout/active").value<QColor>();
        setButtonColor(ui.buttonActive, currentColor);
        currentColor = settings->currentValue("gui/layout/contact").value<QColor>();
        setButtonColor(ui.buttonContact, currentColor);
        currentColor = settings->currentValue("gui/layout/nwell").value<QColor>();
        setButtonColor(ui.buttonNwell, currentColor);
        currentColor = settings->currentValue("gui/layout/pwell").value<QColor>();
        setButtonColor(ui.buttonPwell, currentColor);

        // HDL group of settings
        currentColor = settings->currentValue("gui/hdl/keyword").value<QColor>();
        setButtonColor(ui.buttonKeyword, currentColor);
        currentColor = settings->currentValue("gui/hdl/type").value<QColor>();
        setButtonColor(ui.buttonType, currentColor);
        currentColor = settings->currentValue("gui/hdl/attribute").value<QColor>();
        setButtonColor(ui.buttonAttribute, currentColor);
        currentColor = settings->currentValue("gui/hdl/block").value<QColor>();
        setButtonColor(ui.buttonBlock, currentColor);
        currentColor = settings->currentValue("gui/hdl/class").value<QColor>();
        setButtonColor(ui.buttonClass, currentColor);
        currentColor = settings->currentValue("gui/hdl/data").value<QColor>();
        setButtonColor(ui.buttonData, currentColor);
        currentColor = settings->currentValue("gui/hdl/comment").value<QColor>();
        setButtonColor(ui.buttonComment, currentColor);
        currentColor = settings->currentValue("gui/hdl/system").value<QColor>();
        setButtonColor(ui.buttonSystem, currentColor);
    }

} // namespace Caneda
