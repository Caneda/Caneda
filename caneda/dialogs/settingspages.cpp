/***************************************************************************
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
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

#include "qucsmainwindow.h"
#include "settings.h"

#include "qucs-tools/global.h"

#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QFontDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

//*!**************************************************
// General configuration pages
//*!**************************************************

/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
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

//! Destructor
SettingsPage::~SettingsPage()
{
}

//*!**************************************************
// General configuration pages
//*!**************************************************

/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
GeneralConfigurationPage::GeneralConfigurationPage(QWidget *parent) :
    SettingsPage(parent)
{
    Settings *settings = Settings::instance();
    //First we set the appereance settings group of options *******************
    buttonFont = new QPushButton;
    font = Qucs::font();
    buttonFont->setText(font.toString());

    buttonBackground = new QPushButton;
    const QColor currentBackgroundColor =
        settings->currentValue("gui/backgroundColor").value<QColor>();
    setBackgroundColor(buttonBackground, currentBackgroundColor);

    spinIcons = new QSpinBox;
    spinIcons->setValue(settings->currentValue("gui/iconSize").toSize().height());
    spinIcons->setMinimum(10);
    spinIcons->setMaximum(48);

    connect(buttonFont, SIGNAL(clicked()), SLOT(slotFontDialog()));
    connect(buttonBackground, SIGNAL(clicked()), SLOT(slotBGColorDialog()));

    QGroupBox *appereance = new QGroupBox(tr("Appereance"), this);
    QFormLayout *appereanceLayout = new QFormLayout(appereance);
    appereanceLayout->addRow(tr("Fonts (set after reload):"), buttonFont);
    appereanceLayout->addRow(tr("Document Background Color:"), buttonBackground);
    appereanceLayout->addRow(tr("Icons Size:"), spinIcons);


    //Now we set the misc group of options ************************************
    comboLanguage = new QComboBox();
    comboLanguage->addItem(tr("system language"));
    comboLanguage->addItem(tr("English")+" (en)");
    comboLanguage->addItem(tr("German")+" (de)");
    comboLanguage->addItem(tr("French")+" (fr)");
    comboLanguage->addItem(tr("Spanish")+" (es)");
    comboLanguage->addItem(tr("Italian")+" (it)");
    comboLanguage->addItem(tr("Polish")+" (pl)");
    comboLanguage->addItem(tr("Romanian")+" (ro)");
    comboLanguage->addItem(tr("Japanese")+" (jp)");
    comboLanguage->addItem(tr("Swedish")+" (sv)");
    comboLanguage->addItem(tr("Hungarian")+" (hu)");
    comboLanguage->addItem(tr("Hebrew")+" (he)");
    comboLanguage->addItem(tr("Portuguese")+" (pt)");
    comboLanguage->addItem(tr("Turkish")+" (tr)");
    comboLanguage->addItem(tr("Ukrainian")+" (uk)");
    comboLanguage->addItem(tr("Russian")+" (ru)");
    comboLanguage->addItem(tr("Czech")+" (cs)");
    comboLanguage->addItem(tr("Catalan")+" (ca)");
    for(int z=comboLanguage->count()-1; z>=0; z--) {
        if(comboLanguage->itemText(z).section('(',1,1).remove(')') == Qucs::language()) {
            comboLanguage->setCurrentIndex(z);
        }
    }

    spinUndoNum = new QSpinBox;
    spinUndoNum->setValue(settings->currentValue("gui/maxUndo").toInt());
    spinUndoNum->setMinimum(0);
    spinUndoNum->setMaximum(200);

    editEditor = new QLineEdit;
    editEditor->setText(settings->currentValue("gui/textEditor").toString());

    editLibrary = new QLineEdit;
    editLibrary->setText(settings->currentValue("sidebarLibrary").toString());

    QGroupBox *misc = new QGroupBox(tr("Misc"), this);
    QFormLayout *miscLayout = new QFormLayout(misc);
    miscLayout->addRow(tr("Language (set after reload):"), comboLanguage);
    miscLayout->addRow(tr("Maximum undo operations:"), spinUndoNum);
    miscLayout->addRow(tr("Text Editor:"), editEditor);
    miscLayout->addRow(tr("Components Library:"), editLibrary);


    //Finally we set the general layout of all groups *************************
    QVBoxLayout *vlayout1 = new QVBoxLayout();
    QLabel *title_label_ = new QLabel(title());
    vlayout1->addWidget(title_label_);

    QFrame *horiz_line_ = new QFrame();
    horiz_line_->setFrameShape(QFrame::HLine);
    vlayout1->addWidget(horiz_line_);

    vlayout1->addWidget(appereance);
    vlayout1->addWidget(misc);

    vlayout1->addStretch();

    setLayout(vlayout1);
}

//! Destructor
GeneralConfigurationPage::~GeneralConfigurationPage()
{
}

void GeneralConfigurationPage::slotFontDialog()
{
    bool ok;
    QFont tmpFont = QFontDialog::getFont(&ok, font, this);
    if(ok) {
        font = tmpFont;
        buttonFont->setText(font.toString());
    }
}

void GeneralConfigurationPage::slotBGColorDialog()
{
    QColor c = QColorDialog::getColor(
            getBackgroundColor(buttonBackground), this);
    if(c.isValid()) {
        setBackgroundColor(buttonBackground,c);
    }
}

//! Applies the configuration of this page
void GeneralConfigurationPage::applyConf()
{
    bool changed = false;

    Settings *settings = Settings::instance();

    const QColor currentBackgroundColor =
        settings->currentValue("gui/backgroundColor").value<QColor>();
    const QColor newBackgroundColor = getBackgroundColor(buttonBackground);
    if (currentBackgroundColor != newBackgroundColor) {
        settings->setCurrentValue("gui/backgroundColor", newBackgroundColor);
        changed = true;
    }

    const QFont currentFont = settings->currentValue("gui/font").value<QFont>();
    const QFont newFont = font;
    if(currentFont != newFont) {
        settings->setCurrentValue("gui/font", newFont);
        qApp->setFont(newFont);
        changed = true;
    }

    const QSize currentIconSize = settings->currentValue("gui/iconSize").toSize();
    const QSize newIconSize(spinIcons->value(), spinIcons->value());
    if (currentIconSize != newIconSize) {
        settings->setCurrentValue("gui/iconSize", newIconSize);
        QucsMainWindow::instance()->setIconSize(newIconSize);
    }

    const QString currentLanguage = settings->currentValue("gui/language").toString();
    const QString newLanguage = comboLanguage->currentText().section('(',1,1).remove(')');
    if (currentLanguage != newLanguage) {
        settings->setCurrentValue("gui/language", newLanguage);
    }

    const int currentMaxUndo = settings->currentValue("gui/maxUndo").toInt();
    const int newMaxUndo = spinUndoNum->value();
    if (currentMaxUndo != newMaxUndo) {
        settings->setCurrentValue("gui/maxUndo", newMaxUndo);
        //TODO: Also update all undostacks
    }

    const QString currentTextEditor = settings->currentValue("gui/textEditor").toString();
    const QString newTextEditor = editEditor->text();
    if (currentTextEditor != newTextEditor) {
        settings->setCurrentValue("gui/textEditor", newTextEditor);
    }

    const QString currentLibrary = settings->currentValue("sidebarLibrary").toString();
    QString newLibrary = editLibrary->text();
    if (currentLibrary != newLibrary) {
        if (newLibrary.endsWith(QDir::separator()) == false) {
            newLibrary.append(QDir::separator());
        }
        settings->setCurrentValue("sidebarLibrary", newLibrary);
    }

    QSettings qSettings;
    settings->save(qSettings);

    if(changed) {
        QucsMainWindow::instance()->repaint();
        QucsMainWindow::instance()->slotUpdateAllViews();
    }
}

//! @return Icon of this page
QIcon GeneralConfigurationPage::icon() const {
    return(QIcon(Qucs::bitmapDirectory() + "configure.png"));
}

//! @return Title of this page
QString GeneralConfigurationPage::title() const {
    return(tr("General", "configuration page title"));
}


//*!**************************************************
// Simulation configuration pages
//*!**************************************************
/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
SimulationConfigurationPage::SimulationConfigurationPage(QWidget *parent) :
    SettingsPage(parent)
{
    //First we set the simulator group of options ***************************************
    QGroupBox *groupSimulator = new QGroupBox(tr("Simulator Engine"), this);
    QRadioButton *qucsatorMode = new QRadioButton(tr("Use qucs engine"), groupSimulator);
    QRadioButton *ngspiceMode = new QRadioButton(tr("Use ngspice engine"), groupSimulator);

    qucsatorMode->setChecked(true);

    QVBoxLayout *simulatorLayout = new QVBoxLayout();
    simulatorLayout->addWidget(qucsatorMode);
    simulatorLayout->addWidget(ngspiceMode);
    groupSimulator->setLayout(simulatorLayout);


    //Now we set the simulation tab group of options ************************************
    QGroupBox *groupSimTabMode = new QGroupBox(tr("Simulation Display Mode"), this);
    QRadioButton *specialTabMode = new QRadioButton(tr("Use a special tab"),
            groupSimTabMode);
    QRadioButton *sameTabMode = new QRadioButton(tr("Use same tab as schematic"),
            groupSimTabMode);

    specialTabMode->setChecked(true);

    QVBoxLayout *simTabModeLayout = new QVBoxLayout();
    simTabModeLayout->addWidget(specialTabMode);
    simTabModeLayout->addWidget(sameTabMode);
    groupSimTabMode->setLayout(simTabModeLayout);


    //Now we set the misc group of options **********************************************
    QCheckBox *checkOpenDataDisplay = new QCheckBox(tr("Open data display after simulation:"));
    checkOpenDataDisplay->setChecked(true);

    QLineEdit *editDataset = new QLineEdit;

    QGroupBox *misc = new QGroupBox(tr("Misc"), this);
    QFormLayout *miscLayout = new QFormLayout(misc);
    miscLayout->addRow(checkOpenDataDisplay);
    miscLayout->addRow(tr("Dataset:"), editDataset);


    //Finally we set the general layout of all groups ***********************************
    QVBoxLayout *vlayout1 = new QVBoxLayout();
    QLabel *title_label_ = new QLabel(title());
    vlayout1->addWidget(title_label_);

    QFrame *horiz_line_ = new QFrame();
    horiz_line_->setFrameShape(QFrame::HLine);
    vlayout1->addWidget(horiz_line_);

    vlayout1->addWidget(groupSimulator);
    vlayout1->addWidget(groupSimTabMode);
    vlayout1->addWidget(misc);

    vlayout1->addStretch();

    setLayout(vlayout1);
}

//! Destructor
SimulationConfigurationPage::~SimulationConfigurationPage()
{
}

//! Applies the configuration of this page
void SimulationConfigurationPage::applyConf()
{
    // TODO Implement this
}

//! @return Icon of this page
QIcon SimulationConfigurationPage::icon() const
{
    return(QIcon(Qucs::bitmapDirectory() + "start.png"));
}

//! @return Title of this page
QString SimulationConfigurationPage::title() const
{
    return(tr("Simulation", "simulation page title"));
}


//*!**************************************************
// Document configuration pages
//*!**************************************************

/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
DocumentConfigurationPage::DocumentConfigurationPage(SchematicScene *scene,
        QWidget *parent) : SettingsPage(parent)
{

    Scn = scene;

    //First we set the grid group of options **********************************
    checkShowGrid = new QCheckBox;
    checkShowGrid->setChecked(Scn->isGridVisible());

    spinGridX = new QSpinBox;
    spinGridX->setMinimum(2);
    spinGridX->setMaximum(500);
    spinGridX->setValue(Scn->gridWidth());

    spinGridY = new QSpinBox;
    spinGridY->setMinimum(2);
    spinGridY->setMaximum(500);
    spinGridY->setValue(Scn->gridHeight());

    QGroupBox *grid = new QGroupBox(tr("Grid"), this);
    QFormLayout *gridLayout = new QFormLayout(grid);
    gridLayout->addRow(tr("Show grid:"), checkShowGrid);
    gridLayout->addRow(tr("Horizontal Grid:"), spinGridX);
    gridLayout->addRow(tr("Vertical Grid:"), spinGridY);


    //Next we set the frame group of options **********************************
    checkShowFrame = new QCheckBox;
    checkShowFrame->setChecked(Scn->isFrameVisible());

    spinSchemaX = new QSpinBox;
    spinSchemaX->setMinimum(500);
    spinSchemaX->setMaximum(10000);
    spinSchemaX->setValue(Scn->width());

    spinSchemaY = new QSpinBox;
    spinSchemaY->setMinimum(300);
    spinSchemaY->setMaximum(10000);
    spinSchemaY->setValue(Scn->height());

    spinFrameY = new QSpinBox;
    spinFrameY->setMinimum(1);
    spinFrameY->setMaximum(50);
    spinFrameY->setValue(Scn->frameRows());

    spinFrameX = new QSpinBox;
    spinFrameX->setMinimum(1);
    spinFrameX->setMaximum(50);
    spinFrameX->setValue(Scn->frameColumns());

    QGroupBox *frame = new QGroupBox(tr("Frame"), this);
    QFormLayout *frameLayout = new QFormLayout(frame);
    frameLayout->addRow(tr("Show frame:"), checkShowFrame);
    frameLayout->addRow(tr("Schematic Width:"), spinSchemaX);
    frameLayout->addRow(tr("Schematic Height:"), spinSchemaY);
    frameLayout->addRow(tr("Number of Rows:"), spinFrameY);
    frameLayout->addRow(tr("Number of Columns:"), spinFrameX);


    //Next we set the document group of options *******************************
    editTitle = new QLineEdit;
    editName = new QLineEdit;
    editRevision = new QLineEdit;
    editDate = new QDateEdit;
    foreach(QString frame_text, Scn->frameTexts()){
        if(frame_text.contains("Title: ")) {
            editTitle->setText(frame_text.remove("Title: "));
        }
        else if(frame_text.contains("Drawn By: ")) {
            editName->setText(frame_text.remove("Drawn By: "));
        }
        else if(frame_text.contains("Revision: ")) {
            editRevision->setText(frame_text.remove("Revision: "));
        }
        else if(frame_text.contains("Date: ")) {
            editDate->setDate(QDate::fromString(frame_text.remove("Date: ")));
        }
    }

    QGroupBox *document = new QGroupBox(tr("Document"), this);
    QFormLayout *documentLayout = new QFormLayout(document);
    documentLayout->addRow(tr("Title:"), editTitle);
    documentLayout->addRow(tr("Name:"), editName);
    documentLayout->addRow(tr("Revision:"), editRevision);
    documentLayout->addRow(tr("Date:"), editDate);


    //Finally we set the general layout of all groups *************************
    QVBoxLayout *vlayout1 = new QVBoxLayout();
    QLabel *title_label_ = new QLabel(title());
    vlayout1->addWidget(title_label_);

    QFrame *horiz_line_ = new QFrame();
    horiz_line_->setFrameShape(QFrame::HLine);
    vlayout1->addWidget(horiz_line_);

    vlayout1->addWidget(grid);
    vlayout1->addWidget(frame);
    vlayout1->addWidget(document);

    vlayout1->addStretch();

    setLayout(vlayout1);
}

//! Destructor
DocumentConfigurationPage::~DocumentConfigurationPage()
{
}

/*!
  Applies the configuration of this page
  */
void DocumentConfigurationPage::applyConf()
{
    bool changed = false;

    if(Scn->isGridVisible() != checkShowGrid->isChecked()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("grid visibility",
                    checkShowGrid->isChecked(), Scn->isGridVisible(), Scn));
        Scn->setGridVisible(checkShowGrid->isChecked());
        changed = true;
    }
    if(Scn->isFrameVisible() != checkShowFrame->isChecked()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("frame visibility",
                    checkShowFrame->isChecked(), Scn->isFrameVisible(), Scn));
        Scn->setFrameVisible(checkShowFrame->isChecked());
        changed = true;
    }
    if(Scn->gridWidth() != spinGridX->value()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("grid width",
                    spinGridX->value(), Scn->gridWidth(), Scn));
        Scn->setGridWidth(spinGridX->value());
        changed = true;
    }
    if(Scn->gridHeight() != spinGridY->value()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("grid height",
                    spinGridY->value(), Scn->gridHeight(), Scn));
        Scn->setGridHeight(spinGridY->value());
        changed = true;
    }
    if(Scn->width() != spinSchemaX->value()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("schematic width",
                    spinSchemaX->value(), Scn->width(), Scn));
        Scn->setSceneRect(0, 0, spinSchemaX->value(), Scn->height());
        changed = true;
    }
    if(Scn->height() != spinSchemaY->value()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("schematic height",
                    spinSchemaY->value(), Scn->height(), Scn));
        Scn->setSceneRect(0, 0, Scn->width(), spinSchemaY->value());
        changed = true;
    }
    if(Scn->frameRows() != spinFrameY->value()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("frame rows",
                    spinFrameY->value(), Scn->frameRows(), Scn));
        Scn->setFrameSize(spinFrameY->value(), Scn->frameColumns());
        changed = true;
    }
    if(Scn->frameColumns() != spinFrameX->value()) {
        Scn->undoStack()->push(new ScenePropertyChangeCmd("frame columns",
                    spinFrameX->value(), Scn->frameColumns(), Scn));
        Scn->setFrameSize(Scn->frameRows(), spinFrameX->value());
        changed = true;
    }


    bool modified = false;
    if(!Scn->frameTexts().contains(tr("Title: ")+editTitle->text())) {
        modified = true;
        changed = true;
    }
    else if(!Scn->frameTexts().contains(tr("Drawn By: ")+editName->text())) {
        modified = true;
        changed = true;
    }
    else if(!Scn->frameTexts().contains(tr("Date: ")+editDate->date().toString())) {
        modified = true;
        changed = true;
    }
    else if(!Scn->frameTexts().contains(tr("Revision: ")+editRevision->text())) {
        modified = true;
        changed = true;
    }

    if(modified) {
        QStringList documentProperties = QStringList() <<
            tr("Title: ")+editTitle->text() << tr("Drawn By: ")+editName->text() <<
            tr("Date: ")+editDate->date().toString() <<
            tr("Revision: ")+editRevision->text();
        Scn->undoStack()->push(new ScenePropertyChangeCmd("document properties",
                    documentProperties, Scn->frameTexts(), Scn));
        Scn->setFrameTexts(documentProperties);
    }

    if(changed) {
        QucsMainWindow::instance()->repaint();
        QucsMainWindow::instance()->slotUpdateAllViews();
    }
}

//! @return Icon of this page
QIcon DocumentConfigurationPage::icon() const
{
    return(QIcon(Qucs::bitmapDirectory() + "document-edit.png"));
}

//! @return Title of this page
QString DocumentConfigurationPage::title() const
{
    return(tr("General", "document page title"));
}

//*!**************************************************
// VHDL configuration pages
//*!**************************************************
/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
VhdlConfigurationPage::VhdlConfigurationPage(QWidget *parent) : SettingsPage(parent) {

    //First we set the color settings group of options **********************************
    QGroupBox *colorsHighlighting = new QGroupBox(tr("Colors for Syntax Highlighting"),
            this);
    QGridLayout *generalLayout = new QGridLayout(colorsHighlighting);

    commentButton = new QPushButton(tr("Comment"), colorsHighlighting);
    stringButton = new QPushButton(tr("String"), colorsHighlighting);
    integerButton = new QPushButton(tr("Integer Number"), colorsHighlighting);
    realButton = new QPushButton(tr("Real Number"), colorsHighlighting);
    characterButton = new QPushButton(tr("Character"), colorsHighlighting);
    typesButton = new QPushButton(tr("Data Type"), colorsHighlighting);
    attributesButton = new QPushButton(tr("Attribute"), colorsHighlighting);

    Settings *settings = Settings::instance();

    const QColor currentcommentColor =
        settings->currentValue("gui/vhdl/comment").value<QColor>();
    const QColor currentstringColor =
        settings->currentValue("gui/vhdl/string").value<QColor>();
    const QColor currentintegerColor =
        settings->currentValue("gui/vhdl/integer").value<QColor>();
    const QColor currentrealColor =
        settings->currentValue("gui/vhdl/real").value<QColor>();
    const QColor currentcharacterColor =
        settings->currentValue("gui/vhdl/character").value<QColor>();
    const QColor currenttypesColor =
        settings->currentValue("gui/vhdl/types").value<QColor>();
    const QColor currentattributesColor =
        settings->currentValue("gui/vhdl/attributes").value<QColor>();

    const QColor currentBackgroundColor =
        settings->currentValue("gui/backgroundColor").value<QColor>();

    setBackgroundColor(commentButton, currentBackgroundColor);
    setForegroundColor(commentButton, currentcommentColor);

    setBackgroundColor(stringButton, currentBackgroundColor);
    setForegroundColor(stringButton, currentstringColor);

    setBackgroundColor(integerButton, currentBackgroundColor);
    setForegroundColor(integerButton, currentintegerColor);

    setBackgroundColor(realButton, currentBackgroundColor);
    setForegroundColor(realButton, currentrealColor);

    setBackgroundColor(characterButton, currentBackgroundColor);
    setForegroundColor(characterButton, currentcharacterColor);

    setBackgroundColor(typesButton, currentBackgroundColor);
    setForegroundColor(typesButton, currenttypesColor);

    setBackgroundColor(attributesButton, currentBackgroundColor);
    setForegroundColor(attributesButton, currentattributesColor);

    connect(commentButton, SIGNAL(clicked()), SLOT(slotColorComment()));
    connect(stringButton, SIGNAL(clicked()), SLOT(slotColorString()));
    connect(integerButton, SIGNAL(clicked()), SLOT(slotColorInteger()));
    connect(realButton, SIGNAL(clicked()), SLOT(slotColorReal()));
    connect(characterButton, SIGNAL(clicked()), SLOT(slotColorCharacter()));
    connect(typesButton, SIGNAL(clicked()), SLOT(slotColorDataType()));
    connect(attributesButton, SIGNAL(clicked()), SLOT(slotColorAttributes()));

    generalLayout->addWidget(commentButton, 0, 0);
    generalLayout->addWidget(stringButton, 0, 1);
    generalLayout->addWidget(integerButton, 1, 0);
    generalLayout->addWidget(realButton, 1, 1);
    generalLayout->addWidget(characterButton, 2, 0);
    generalLayout->addWidget(typesButton, 2, 1);
    generalLayout->addWidget(attributesButton, 3, 0);


    //Finally we set the general layout of all groups ***********************************
    QVBoxLayout *vlayout1 = new QVBoxLayout();
    QLabel *title_label_ = new QLabel(title());
    vlayout1->addWidget(title_label_);

    QFrame *horiz_line_ = new QFrame();
    horiz_line_->setFrameShape(QFrame::HLine);
    vlayout1->addWidget(horiz_line_);

    vlayout1->addWidget(colorsHighlighting);

    vlayout1->addStretch();

    setLayout(vlayout1);
}

//! Destructor
VhdlConfigurationPage::~VhdlConfigurationPage() {
}

//! Applies the configuration of this page
void VhdlConfigurationPage::applyConf() {
    bool changed = false;

    Settings *settings = Settings::instance();
    const QColor currentcommentColor =
        settings->currentValue("gui/vhdl/comment").value<QColor>();
    const QColor newcommentColor = getForegroundColor(commentButton);
    if (newcommentColor != currentcommentColor) {
        settings->setCurrentValue("gui/vhdl/comment", newcommentColor);
        changed = true;
    }

    const QColor currentstringColor =
        settings->currentValue("gui/vhdl/string").value<QColor>();
    const QColor newstringColor = getForegroundColor(stringButton);
    if (newstringColor != currentstringColor) {
        settings->setCurrentValue("gui/vhdl/string", newstringColor);
        changed = true;
    }

    const QColor currentintegerColor =
        settings->currentValue("gui/vhdl/integer").value<QColor>();
    const QColor newintegerColor = getForegroundColor(integerButton);
    if (newintegerColor != currentintegerColor) {
        settings->setCurrentValue("gui/vhdl/integer", newintegerColor);
        changed = true;
    }

    const QColor currentrealColor =
        settings->currentValue("gui/vhdl/real").value<QColor>();
    const QColor newrealColor = getForegroundColor(realButton);
    if (newrealColor != currentrealColor) {
        settings->setCurrentValue("gui/vhdl/real", newrealColor);
        changed = true;
    }

    const QColor currentcharacterColor =
        settings->currentValue("gui/vhdl/character").value<QColor>();
    const QColor newcharacterColor = getForegroundColor(characterButton);
    if (newcharacterColor != currentcharacterColor) {
        settings->setCurrentValue("gui/vhdl/character", newcharacterColor);
        changed = true;
    }

    const QColor currenttypesColor =
        settings->currentValue("gui/vhdl/types").value<QColor>();
    const QColor newtypesColor = getForegroundColor(typesButton);
    if (newtypesColor != currenttypesColor) {
        settings->setCurrentValue("gui/vhdl/types", newtypesColor);
        changed = true;
    }

    const QColor currentattributesColor =
        settings->currentValue("gui/vhdl/attributes").value<QColor>();
    const QColor newattributesColor = getForegroundColor(attributesButton);
    if (newattributesColor != currentattributesColor) {
        settings->setCurrentValue("gui/vhdl/attributes", newattributesColor);
        changed = true;
    }

    QSettings qSettings;
    settings->save(qSettings);
    if(changed) {
        QucsMainWindow::instance()->repaint();
        QucsMainWindow::instance()->slotUpdateAllViews();
    }
}

//! @return Icon of this page
QIcon VhdlConfigurationPage::icon() const
{
    return(QIcon(Qucs::bitmapDirectory() + "vhdl-code.png"));
}

//! @return Title of this page
QString VhdlConfigurationPage::title() const
{
    return(tr("VHDL", "vhdl page title"));
}

void VhdlConfigurationPage::slotColorComment()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(commentButton), this);
    if(c.isValid()) {
        setForegroundColor(commentButton,c);
    }
}

void VhdlConfigurationPage::slotColorString()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(stringButton), this);
    if(c.isValid()) {
        setForegroundColor(stringButton,c);
    }
}

void VhdlConfigurationPage::slotColorInteger()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(integerButton), this);
    if(c.isValid()) {
        setForegroundColor(integerButton,c);
    }
}

void VhdlConfigurationPage::slotColorReal()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(realButton), this);
    if(c.isValid()) {
        setForegroundColor(realButton,c);
    }
}

void VhdlConfigurationPage::slotColorCharacter()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(characterButton), this);
    if(c.isValid()) {
        setForegroundColor(characterButton,c);
    }
}

void VhdlConfigurationPage::slotColorDataType()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(typesButton), this);
    if(c.isValid()) {
        setForegroundColor(typesButton,c);
    }
}

void VhdlConfigurationPage::slotColorAttributes()
{
    QColor c = QColorDialog::getColor(
            getForegroundColor(attributesButton), this);
    if(c.isValid()) {
        setForegroundColor(attributesButton,c);
    }
}
