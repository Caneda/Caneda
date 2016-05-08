/***************************************************************************
 * Copyright (C) 2009-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "settingspages.h"

#include "global.h"
#include "mainwindow.h"
#include "settings.h"

#include <QColorDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

namespace Caneda
{
    /*************************************************************************
     *                      Abstract configuration page                      *
     *************************************************************************/
    /*!
     * \brief Constructor.
     *
     * \param parent The parent of the dialog.
     */
    SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
    {
    }

    void SettingsPage::setForegroundColor(QPushButton *b, QColor col)
    {
        QPalette palette(b->palette());
        palette.setColor(b->foregroundRole(), col);
        b->setPalette(palette);
    }

    QColor SettingsPage::getForegroundColor(QPushButton *b)
    {
        QPalette palette(b->palette());
        return palette.color(b->foregroundRole());
    }

    void SettingsPage::setBackgroundColor(QPushButton *b, QColor col)
    {
        QPalette palette(b->palette());
        palette.setColor(b->backgroundRole(), col);
        b->setPalette(palette);
    }

    QColor SettingsPage::getBackgroundColor(QPushButton *b)
    {
        QPalette palette(b->palette());
        return palette.color(b->backgroundRole());
    }


    /*************************************************************************
     *                      General configuration pages                      *
     *************************************************************************/
    /*!
     * \brief Constructor.
     *
     * \param parent The parent of the dialog.
     */
    GeneralConfigurationPage::GeneralConfigurationPage(QWidget *parent) :
        SettingsPage(parent)
    {
        Settings *settings = Settings::instance();

        // First we set the appereance settings group of options
        checkShowGrid = new QCheckBox();
        checkShowGrid->setChecked(settings->currentValue("gui/gridVisible").value<bool>());

        QColor currentColor;

        buttonBackground = new QPushButton;
        currentColor = settings->currentValue("gui/backgroundColor").value<QColor>();
        setBackgroundColor(buttonBackground, currentColor);

        buttonSimulationBackground = new QPushButton;
        currentColor = settings->currentValue("gui/simulationBackgroundColor").value<QColor>();
        setBackgroundColor(buttonSimulationBackground, currentColor);

        buttonForeground = new QPushButton;
        currentColor = settings->currentValue("gui/foregroundColor").value<QColor>();
        setBackgroundColor(buttonForeground, currentColor);

        buttonLine = new QPushButton;
        currentColor = settings->currentValue("gui/lineColor").value<QColor>();
        setBackgroundColor(buttonLine, currentColor);

        buttonSelection = new QPushButton;
        currentColor = settings->currentValue("gui/selectionColor").value<QColor>();
        setBackgroundColor(buttonSelection, currentColor);

        spinWidth = new QSpinBox;
        spinWidth->setValue(settings->currentValue("gui/lineWidth").toInt());
        spinWidth->setMinimum(1);
        spinWidth->setMaximum(10);

        connect(buttonBackground, SIGNAL(clicked()), SLOT(slotBackgroundColorDialog()));
        connect(buttonSimulationBackground, SIGNAL(clicked()), SLOT(slotBackgroundSimulationColorDialog()));
        connect(buttonForeground, SIGNAL(clicked()), SLOT(slotForegroundColorDialog()));
        connect(buttonLine, SIGNAL(clicked()), SLOT(slotLineColorDialog()));
        connect(buttonSelection, SIGNAL(clicked()), SLOT(slotSelectionColorDialog()));

        QGroupBox *appereance = new QGroupBox(tr("Appereance"), this);
        QFormLayout *appereanceLayout = new QFormLayout(appereance);
        appereanceLayout->addRow(tr("Show grid:"), checkShowGrid);
        appereanceLayout->addRow(tr("Background color:"), buttonBackground);
        appereanceLayout->addRow(tr("Simulation color:"), buttonSimulationBackground);
        appereanceLayout->addRow(tr("Foreground color:"), buttonForeground);
        appereanceLayout->addRow(tr("Line color:"), buttonLine);
        appereanceLayout->addRow(tr("Selection color:"), buttonSelection);
        appereanceLayout->addRow(tr("Line width:"), spinWidth);


        // Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        title_label_ = new QLabel(title());
        vlayout1->addWidget(title_label_);

        horiz_line_ = new QFrame();
        horiz_line_->setFrameShape(QFrame::HLine);
        vlayout1->addWidget(horiz_line_);

        vlayout1->addWidget(appereance);

        vlayout1->addStretch();

        setLayout(vlayout1);
    }

    void GeneralConfigurationPage::slotColorButtonDialog(QPushButton *button)
    {
        QColor color = QColorDialog::getColor(getBackgroundColor(button), this);
        if(color.isValid()) {
            setBackgroundColor(button, color);
        }
    }

    void GeneralConfigurationPage::slotBackgroundColorDialog()
    {
        slotColorButtonDialog(buttonBackground);
    }

    void GeneralConfigurationPage::slotBackgroundSimulationColorDialog()
    {
        slotColorButtonDialog(buttonSimulationBackground);
    }

    void GeneralConfigurationPage::slotForegroundColorDialog()
    {
        slotColorButtonDialog(buttonForeground);
    }

    void GeneralConfigurationPage::slotLineColorDialog()
    {
        slotColorButtonDialog(buttonLine);
    }

    void GeneralConfigurationPage::slotSelectionColorDialog()
    {
        slotColorButtonDialog(buttonSelection);
    }

    //! \brief Applies the configuration of this page.
    void GeneralConfigurationPage::applyConf()
    {
        Settings *settings = Settings::instance();

        settings->setCurrentValue("gui/gridVisible", checkShowGrid->isChecked());

        settings->setCurrentValue("gui/backgroundColor", getBackgroundColor(buttonBackground));
        settings->setCurrentValue("gui/simulationBackgroundColor", getBackgroundColor(buttonSimulationBackground));
        settings->setCurrentValue("gui/foregroundColor", getBackgroundColor(buttonForeground));
        settings->setCurrentValue("gui/lineColor", getBackgroundColor(buttonLine));
        settings->setCurrentValue("gui/selectionColor", getBackgroundColor(buttonSelection));

        settings->setCurrentValue("gui/lineWidth", spinWidth->value());

        settings->save();
    }

    //! \return Icon of this page.
    QIcon GeneralConfigurationPage::icon() const
    {
        return(Caneda::icon("preferences-other"));
    }

    //! \return Title of this page.
    QString GeneralConfigurationPage::title() const
    {
        return(tr("General", "configuration page title"));
    }


    /*************************************************************************
     *                    Libraries configuration pages                      *
     *************************************************************************/
    /*!
     * \brief Constructor.
     *
     * \param parent The parent of the dialog.
     */
    LibrariesConfigurationPage::LibrariesConfigurationPage(QWidget *parent) :
        SettingsPage(parent)
    {
        Settings *settings = Settings::instance();

        // Set the schematic library options
        libraryList = new QListWidget(this);
        QStringList libraries;
        libraries << settings->currentValue("libraries/schematic").toStringList();
        foreach (const QString &str, libraries) {
            libraryList->addItem(str);
        }
        libraryList->sortItems(Qt::AscendingOrder);

        addLibrary = new QPushButton(tr("Add library..."));
        removeLibrary = new QPushButton(tr("Remove library"));
        connect(addLibrary, SIGNAL(clicked()), SLOT(slotAddLibrary()));
        connect(removeLibrary, SIGNAL(clicked()), SLOT(slotRemoveLibrary()));

        QGroupBox *currentLibraries = new QGroupBox(tr("Schematic Libraries"), this);
        QHBoxLayout *libraryButtons = new QHBoxLayout();
        libraryButtons->addWidget(addLibrary);
        libraryButtons->addWidget(removeLibrary);
        QVBoxLayout *currentLibrariesLayout = new QVBoxLayout(currentLibraries);
        currentLibrariesLayout->addWidget(libraryList);
        currentLibrariesLayout->addLayout(libraryButtons);


        // Set the schematic library options
        hdlLibraryList = new QListWidget(this);
        QStringList hdlLibraries;
        hdlLibraries << settings->currentValue("libraries/hdl").toStringList();
        foreach (const QString &str, hdlLibraries) {
            hdlLibraryList->addItem(str);
        }
        hdlLibraryList->sortItems(Qt::AscendingOrder);

        addHdlLibrary = new QPushButton(tr("Add library..."));
        removeHdlLibrary = new QPushButton(tr("Remove library"));
        connect(addHdlLibrary, SIGNAL(clicked()), SLOT(slotAddHdlLibrary()));
        connect(removeHdlLibrary, SIGNAL(clicked()), SLOT(slotRemoveHdlLibrary()));

        QGroupBox *currentHdlLibraries = new QGroupBox(tr("Hdl Libraries"), this);
        QHBoxLayout *hdlLibraryButtons = new QHBoxLayout();
        hdlLibraryButtons->addWidget(addHdlLibrary);
        hdlLibraryButtons->addWidget(removeHdlLibrary);
        QVBoxLayout *currentHdlLibrariesLayout = new QVBoxLayout(currentHdlLibraries);
        currentHdlLibrariesLayout->addWidget(hdlLibraryList);
        currentHdlLibrariesLayout->addLayout(hdlLibraryButtons);

        // Finally we set the general layout of all groups
        getNewLibraries = new QPushButton(tr("Get new libraries..."));
        connect(getNewLibraries, SIGNAL(clicked()), SLOT(slotGetNewLibraries()));
        QLabel *warningLabel = new QLabel(tr("Warning: libraries will be set upon program restart"));

        QVBoxLayout *vlayout1 = new QVBoxLayout();
        title_label_ = new QLabel(title());
        vlayout1->addWidget(title_label_);

        horiz_line_ = new QFrame();
        horiz_line_->setFrameShape(QFrame::HLine);
        vlayout1->addWidget(horiz_line_);

        vlayout1->addWidget(currentLibraries);
        vlayout1->addWidget(currentHdlLibraries);
        vlayout1->addWidget(getNewLibraries);
        vlayout1->addWidget(warningLabel);

        vlayout1->addStretch();

        setLayout(vlayout1);
    }

    void LibrariesConfigurationPage::slotAddLibrary()
    {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Library"),
                                                        "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(!dir.isEmpty()) {
            new QListWidgetItem(dir, libraryList);
        }

        libraryList->sortItems(Qt::AscendingOrder);
    }

    void LibrariesConfigurationPage::slotRemoveLibrary()
    {
        qDeleteAll(libraryList->selectedItems());
    }

    void LibrariesConfigurationPage::slotAddHdlLibrary()
    {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Library"),
                                                        "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(!dir.isEmpty()) {
            new QListWidgetItem(dir, hdlLibraryList);
        }

        hdlLibraryList->sortItems(Qt::AscendingOrder);
    }

    void LibrariesConfigurationPage::slotRemoveHdlLibrary()
    {
        qDeleteAll(hdlLibraryList->selectedItems());
    }

    void LibrariesConfigurationPage::slotGetNewLibraries()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/Caneda/Libraries"));
    }

    //! \brief Applies the configuration of this page.
    void LibrariesConfigurationPage::applyConf()
    {
        Settings *settings = Settings::instance();

        QStringList newLibraries;
        for(int i=0; i<libraryList->count(); i++) {
            newLibraries << libraryList->item(i)->text();
        }
        settings->setCurrentValue("libraries/schematic", newLibraries);

        QStringList newHdlLibraries;
        for(int i=0; i<hdlLibraryList->count(); i++) {
            newHdlLibraries << hdlLibraryList->item(i)->text();
        }
        settings->setCurrentValue("libraries/hdl", newHdlLibraries);

        settings->save();
    }

    //! \return Icon of this page.
    QIcon LibrariesConfigurationPage::icon() const
    {
        return(Caneda::icon("library"));
    }

    //! \return Title of this page.
    QString LibrariesConfigurationPage::title() const
    {
        return(tr("Libraries", "libraries page title"));
    }


    /*************************************************************************
     *                   Simulation configuration pages                      *
     *************************************************************************/
    /*!
     * \brief Constructor.
     *
     * \param parent The parent of the dialog.
     */
    SimulationConfigurationPage::SimulationConfigurationPage(QWidget *parent) :
        SettingsPage(parent)
    {
        Settings *settings = Settings::instance();

        // First we set the simulation group of options
        QGroupBox *groupSimulator = new QGroupBox(tr("Simulation Engine"), this);
        QGroupBox *groupOutput = new QGroupBox(tr("Output Data"), this);

        lineSimulationCommand = new QLineEdit;
        lineSimulationCommand->setText(settings->currentValue("sim/simulationCommand").toString());

        ngspiceMode = new QRadioButton(tr("Ngspice"), groupSimulator);
        customMode = new QRadioButton(tr("Custom"), groupSimulator);
        binaryMode = new QRadioButton(tr("Binary"), groupOutput);
        asciiMode = new QRadioButton(tr("Ascii"), groupOutput);

        if(settings->currentValue("sim/simulationEngine").toString() == "ngspice") {
            ngspiceMode->setChecked(true);
            lineSimulationCommand->setEnabled(false);
        }
        else if(settings->currentValue("sim/simulationEngine").toString() == "custom") {
            customMode->setChecked(true);
        }

        if(settings->currentValue("sim/outputFormat").toString() == "binary") {
            binaryMode->setChecked(true);
        }
        else if(settings->currentValue("sim/outputFormat").toString() == "ascii") {
            asciiMode->setChecked(true);
        }

        QFormLayout *simulatorLayout = new QFormLayout(groupSimulator);
        simulatorLayout->addRow(tr("Engine:"), ngspiceMode);
        simulatorLayout->addRow("", customMode);
        simulatorLayout->addRow(tr("Command:"), lineSimulationCommand);

        QFormLayout *outputLayout = new QFormLayout(groupOutput);
        outputLayout->addRow(tr("Output format:"), binaryMode);
        outputLayout->addRow("", asciiMode);

        connect(ngspiceMode, SIGNAL(clicked()), SLOT(slotSimulationEngineSelected()));
        connect(customMode, SIGNAL(clicked()), SLOT(slotSimulationEngineSelected()));

        // Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        title_label_ = new QLabel(title());
        vlayout1->addWidget(title_label_);

        horiz_line_ = new QFrame();
        horiz_line_->setFrameShape(QFrame::HLine);
        vlayout1->addWidget(horiz_line_);

        vlayout1->addWidget(groupSimulator);
        vlayout1->addWidget(groupOutput);

        vlayout1->addStretch();

        setLayout(vlayout1);
    }

    //! \brief Updates the lineSimulationCommand enabled status.
    void SimulationConfigurationPage::slotSimulationEngineSelected()
    {
        if(customMode->isChecked()) {
            lineSimulationCommand->setEnabled(true);
        }
        else {
            lineSimulationCommand->setEnabled(false);
        }
    }

    //! \brief Applies the configuration of this page.
    void SimulationConfigurationPage::applyConf()
    {
        Settings *settings = Settings::instance();

        // Simulation engine
        if(ngspiceMode->isChecked()) {
            // If using ngspice simulator, the command to simulate is:
            // ngspice -b -r output.raw input.net
            settings->setCurrentValue("sim/simulationEngine", QString("ngspice"));
            settings->setCurrentValue("sim/simulationCommand", QString("ngspice -b -r %filename.raw %filename.net"));
        }
        else if(customMode->isChecked()) {
            // If using the custom simulator, take the user provided command
            settings->setCurrentValue("sim/simulationEngine", QString("custom"));
            settings->setCurrentValue("sim/simulationCommand", lineSimulationCommand->text());
        }

        // Output format
        if(binaryMode->isChecked()) {
            settings->setCurrentValue("sim/outputFormat", QString("binary"));
        }
        else if(asciiMode->isChecked()) {
            settings->setCurrentValue("sim/outputFormat", QString("ascii"));
        }

        // Save previous settings
        settings->save();
    }

    //! \return Icon of this page.
    QIcon SimulationConfigurationPage::icon() const
    {
        return(Caneda::icon("media-playback-start"));
    }

    //! \return Title of this page.
    QString SimulationConfigurationPage::title() const
    {
        return(tr("Simulation", "simulation page title"));
    }


    /*************************************************************************
     *                        HDL configuration pages                        *
     *************************************************************************/
    /*!
     * \brief Constructor.
     *
     * \param parent The parent of the dialog.
     */
    HdlConfigurationPage::HdlConfigurationPage(QWidget *parent) : SettingsPage(parent)
    {
        // First we set the color settings group of options
        QGroupBox *colorsHighlighting = new QGroupBox(tr("Colors for Syntax Highlighting"),
                this);
        QGridLayout *generalLayout = new QGridLayout(colorsHighlighting);

        keywordButton = new QPushButton(tr("Keyword"), colorsHighlighting);
        typeButton = new QPushButton(tr("Type"), colorsHighlighting);
        attributeButton = new QPushButton(tr("Attribute"), colorsHighlighting);
        blockButton = new QPushButton(tr("Block"), colorsHighlighting);
        classButton = new QPushButton(tr("Class"), colorsHighlighting);
        dataButton = new QPushButton(tr("Data"), colorsHighlighting);
        commentButton = new QPushButton(tr("Comment"), colorsHighlighting);
        systemButton = new QPushButton(tr("System"), colorsHighlighting);

        Settings *settings = Settings::instance();

        const QColor currentKeywordColor =
            settings->currentValue("gui/hdl/keyword").value<QColor>();
        const QColor currentTypeColor =
            settings->currentValue("gui/hdl/type").value<QColor>();
        const QColor currentAttributeColor =
            settings->currentValue("gui/hdl/attribute").value<QColor>();
        const QColor currentBlockColor =
            settings->currentValue("gui/hdl/block").value<QColor>();
        const QColor currentClassColor =
            settings->currentValue("gui/hdl/class").value<QColor>();
        const QColor currentDataColor =
            settings->currentValue("gui/hdl/data").value<QColor>();
        const QColor currentCommentColor =
            settings->currentValue("gui/hdl/comment").value<QColor>();
        const QColor currentSystemColor =
            settings->currentValue("gui/hdl/system").value<QColor>();

        const QColor currentBackgroundColor =
            settings->currentValue("gui/backgroundColor").value<QColor>();

        setBackgroundColor(keywordButton, currentBackgroundColor);
        setForegroundColor(keywordButton, currentKeywordColor);

        setBackgroundColor(typeButton, currentBackgroundColor);
        setForegroundColor(typeButton, currentTypeColor);

        setBackgroundColor(attributeButton, currentBackgroundColor);
        setForegroundColor(attributeButton, currentAttributeColor);

        setBackgroundColor(blockButton, currentBackgroundColor);
        setForegroundColor(blockButton, currentBlockColor);

        setBackgroundColor(classButton, currentBackgroundColor);
        setForegroundColor(classButton, currentClassColor);

        setBackgroundColor(dataButton, currentBackgroundColor);
        setForegroundColor(dataButton, currentDataColor);

        setBackgroundColor(commentButton, currentBackgroundColor);
        setForegroundColor(commentButton, currentCommentColor);

        setBackgroundColor(systemButton, currentBackgroundColor);
        setForegroundColor(systemButton, currentSystemColor);

        connect(keywordButton, SIGNAL(clicked()), SLOT(slotColorKeyword()));
        connect(typeButton, SIGNAL(clicked()), SLOT(slotColorType()));
        connect(attributeButton, SIGNAL(clicked()), SLOT(slotColorAttribute()));
        connect(blockButton, SIGNAL(clicked()), SLOT(slotColorBlock()));
        connect(classButton, SIGNAL(clicked()), SLOT(slotColorClass()));
        connect(dataButton, SIGNAL(clicked()), SLOT(slotColorData()));
        connect(commentButton, SIGNAL(clicked()), SLOT(slotColorComment()));
        connect(systemButton, SIGNAL(clicked()), SLOT(slotColorSystem()));

        generalLayout->addWidget(keywordButton, 0, 0);
        generalLayout->addWidget(typeButton, 0, 1);
        generalLayout->addWidget(attributeButton, 1, 0);
        generalLayout->addWidget(blockButton, 1, 1);
        generalLayout->addWidget(classButton, 2, 0);
        generalLayout->addWidget(dataButton, 2, 1);
        generalLayout->addWidget(commentButton, 3, 0);
        generalLayout->addWidget(systemButton, 3, 1);


        // Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        title_label_ = new QLabel(title());
        vlayout1->addWidget(title_label_);

        horiz_line_ = new QFrame();
        horiz_line_->setFrameShape(QFrame::HLine);
        vlayout1->addWidget(horiz_line_);

        vlayout1->addWidget(colorsHighlighting);

        vlayout1->addStretch();

        setLayout(vlayout1);
    }

    //! \brief Applies the configuration of this page.
    void HdlConfigurationPage::applyConf()
    {
        Settings *settings = Settings::instance();

        settings->setCurrentValue("gui/hdl/keyword", getForegroundColor(keywordButton));
        settings->setCurrentValue("gui/hdl/type", getForegroundColor(typeButton));
        settings->setCurrentValue("gui/hdl/attribute", getForegroundColor(attributeButton));
        settings->setCurrentValue("gui/hdl/block", getForegroundColor(blockButton));
        settings->setCurrentValue("gui/hdl/class", getForegroundColor(classButton));
        settings->setCurrentValue("gui/hdl/data", getForegroundColor(dataButton));
        settings->setCurrentValue("gui/hdl/comment", getForegroundColor(commentButton));
        settings->setCurrentValue("gui/hdl/system", getForegroundColor(systemButton));

        settings->save();
    }

    //! \return Icon of this page.
    QIcon HdlConfigurationPage::icon() const
    {
        return(Caneda::icon("code-context"));
    }

    //! \return Title of this page.
    QString HdlConfigurationPage::title() const
    {
        return(tr("HDL", "hdl page title"));
    }

    void HdlConfigurationPage::slotColorKeyword()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(keywordButton), this);
        if(c.isValid()) {
            setForegroundColor(keywordButton, c);
        }
    }

    void HdlConfigurationPage::slotColorType()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(typeButton), this);
        if(c.isValid()) {
            setForegroundColor(typeButton, c);
        }
    }

    void HdlConfigurationPage::slotColorAttribute()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(attributeButton), this);
        if(c.isValid()) {
            setForegroundColor(attributeButton, c);
        }
    }

    void HdlConfigurationPage::slotColorBlock()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(blockButton), this);
        if(c.isValid()) {
            setForegroundColor(blockButton, c);
        }
    }

    void HdlConfigurationPage::slotColorClass()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(classButton), this);
        if(c.isValid()) {
            setForegroundColor(classButton, c);
        }
    }

    void HdlConfigurationPage::slotColorData()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(dataButton), this);
        if(c.isValid()) {
            setForegroundColor(dataButton, c);
        }
    }

    void HdlConfigurationPage::slotColorComment()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(commentButton), this);
        if(c.isValid()) {
            setForegroundColor(commentButton, c);
        }
    }

    void HdlConfigurationPage::slotColorSystem()
    {
        QColor c = QColorDialog::getColor(
                getForegroundColor(systemButton), this);
        if(c.isValid()) {
            setForegroundColor(systemButton, c);
        }
    }


    /*************************************************************************
     *                      Layout configuration pages                       *
     *************************************************************************/
    /*!
     * \brief Constructor.
     *
     * \param parent The parent of the dialog.
     */
    LayoutConfigurationPage::LayoutConfigurationPage(QWidget *parent) :
        SettingsPage(parent)
    {
        // First we set the color settings group of options
        QGroupBox *colorsLayers = new QGroupBox(tr("Layer Colors"),
                this);
        QGridLayout *generalLayout = new QGridLayout(colorsLayers);

        metal1Button = new QPushButton(tr("Metal 1"), colorsLayers);
        metal2Button = new QPushButton(tr("Metal 2"), colorsLayers);
        poly1Button = new QPushButton(tr("Poly 1"), colorsLayers);
        poly2Button = new QPushButton(tr("Poly 2"), colorsLayers);
        activeButton = new QPushButton(tr("Active"), colorsLayers);
        contactButton = new QPushButton(tr("Contact"), colorsLayers);
        nwellButton = new QPushButton(tr("N Well"), colorsLayers);
        pwellButton = new QPushButton(tr("P Well"), colorsLayers);

        Settings *settings = Settings::instance();

        const QColor currentMetal1Color =
            settings->currentValue("gui/layout/metal1").value<QColor>();
        const QColor currentMeta2Color =
            settings->currentValue("gui/layout/metal2").value<QColor>();
        const QColor currentPoly1Color =
            settings->currentValue("gui/layout/poly1").value<QColor>();
        const QColor currentPoly2Color =
            settings->currentValue("gui/layout/poly2").value<QColor>();
        const QColor currentActiveColor =
            settings->currentValue("gui/layout/active").value<QColor>();
        const QColor currentContactColor =
            settings->currentValue("gui/layout/contact").value<QColor>();
        const QColor currentNwellColor =
            settings->currentValue("gui/layout/nwell").value<QColor>();
        const QColor currentPwellColor =
            settings->currentValue("gui/layout/pwell").value<QColor>();

        const QColor currentForegroundColor =
            settings->currentValue("gui/foregroundColor").value<QColor>();

        setBackgroundColor(metal1Button, currentMetal1Color);
        setForegroundColor(metal1Button, currentForegroundColor);

        setBackgroundColor(metal2Button, currentMeta2Color);
        setForegroundColor(metal2Button, currentForegroundColor);

        setBackgroundColor(poly1Button, currentPoly1Color);
        setForegroundColor(poly1Button, currentForegroundColor);

        setBackgroundColor(poly2Button, currentPoly2Color);
        setForegroundColor(poly2Button, currentForegroundColor);

        setBackgroundColor(activeButton, currentActiveColor);
        setForegroundColor(activeButton, currentForegroundColor);

        setBackgroundColor(contactButton, currentContactColor);
        setForegroundColor(contactButton, currentForegroundColor);

        setBackgroundColor(nwellButton, currentNwellColor);
        setForegroundColor(nwellButton, currentForegroundColor);

        setBackgroundColor(pwellButton, currentPwellColor);
        setForegroundColor(pwellButton, currentForegroundColor);

        connect(metal1Button, SIGNAL(clicked()), SLOT(slotColorMetal1()));
        connect(metal2Button, SIGNAL(clicked()), SLOT(slotColorMetal2()));
        connect(poly1Button, SIGNAL(clicked()), SLOT(slotColorPoly1()));
        connect(poly2Button, SIGNAL(clicked()), SLOT(slotColorPoly2()));
        connect(activeButton, SIGNAL(clicked()), SLOT(slotColorActive()));
        connect(contactButton, SIGNAL(clicked()), SLOT(slotColorContact()));
        connect(nwellButton, SIGNAL(clicked()), SLOT(slotColorNwell()));
        connect(pwellButton, SIGNAL(clicked()), SLOT(slotColorPwell()));

        generalLayout->addWidget(metal1Button, 0, 0);
        generalLayout->addWidget(metal2Button, 0, 1);
        generalLayout->addWidget(poly1Button, 1, 0);
        generalLayout->addWidget(poly2Button, 1, 1);
        generalLayout->addWidget(activeButton, 2, 0);
        generalLayout->addWidget(contactButton, 2, 1);
        generalLayout->addWidget(nwellButton, 3, 0);
        generalLayout->addWidget(pwellButton, 3, 1);


        // Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        title_label_ = new QLabel(title());
        vlayout1->addWidget(title_label_);

        horiz_line_ = new QFrame();
        horiz_line_->setFrameShape(QFrame::HLine);
        vlayout1->addWidget(horiz_line_);

        vlayout1->addWidget(colorsLayers);

        vlayout1->addStretch();

        setLayout(vlayout1);
    }

    //! \brief Applies the configuration of this page.
    void LayoutConfigurationPage::applyConf()
    {
        Settings *settings = Settings::instance();

        settings->setCurrentValue("gui/layout/metal1", getBackgroundColor(metal1Button));
        settings->setCurrentValue("gui/layout/metal2", getBackgroundColor(metal2Button));
        settings->setCurrentValue("gui/layout/poly1", getBackgroundColor(poly1Button));
        settings->setCurrentValue("gui/layout/poly2", getBackgroundColor(poly2Button));
        settings->setCurrentValue("gui/layout/active", getBackgroundColor(activeButton));
        settings->setCurrentValue("gui/layout/contact", getBackgroundColor(contactButton));
        settings->setCurrentValue("gui/layout/nwell", getBackgroundColor(nwellButton));
        settings->setCurrentValue("gui/layout/pwell", getBackgroundColor(pwellButton));

        settings->save();
    }

    //! \return Icon of this page.
    QIcon LayoutConfigurationPage::icon() const
    {
        return(Caneda::icon("view-grid"));
    }

    //! \return Title of this page.
    QString LayoutConfigurationPage::title() const
    {
        return(tr("Layout", "layout page title"));
    }

    void LayoutConfigurationPage::slotColorMetal1()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(metal1Button), this);
        if(c.isValid()) {
            setBackgroundColor(metal1Button, c);
        }
    }

    void LayoutConfigurationPage::slotColorMetal2()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(metal2Button), this);
        if(c.isValid()) {
            setBackgroundColor(metal2Button, c);
        }
    }

    void LayoutConfigurationPage::slotColorPoly1()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(poly1Button), this);
        if(c.isValid()) {
            setBackgroundColor(poly1Button, c);
        }
    }

    void LayoutConfigurationPage::slotColorPoly2()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(poly2Button), this);
        if(c.isValid()) {
            setBackgroundColor(poly2Button, c);
        }
    }

    void LayoutConfigurationPage::slotColorActive()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(activeButton), this);
        if(c.isValid()) {
            setBackgroundColor(activeButton, c);
        }
    }

    void LayoutConfigurationPage::slotColorContact()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(contactButton), this);
        if(c.isValid()) {
            setBackgroundColor(contactButton, c);
        }
    }

    void LayoutConfigurationPage::slotColorNwell()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(nwellButton), this);
        if(c.isValid()) {
            setBackgroundColor(nwellButton, c);
        }
    }

    void LayoutConfigurationPage::slotColorPwell()
    {
        QColor c = QColorDialog::getColor(
                getBackgroundColor(pwellButton), this);
        if(c.isValid()) {
            setBackgroundColor(pwellButton, c);
        }
    }

} // namespace Caneda
