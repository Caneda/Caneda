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
#include "qucs-tools/global.h"

//****************************************************
// General configuration pages
//****************************************************
/**
        Constructor
        @param QWidget *parent The parent of the dialog.
*/
SettingsPage::SettingsPage(QucsMainWindow *parent) : QWidget(parent)
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

/// Destructor
SettingsPage::~SettingsPage()
{
}

//****************************************************
// General configuration pages
//****************************************************
/**
        Constructor
        @param QWidget *parent The parent of the dialog.
*/
GeneralConfigurationPage::GeneralConfigurationPage(QucsMainWindow *parent) : SettingsPage(parent)
{
        App = parent;
	
        //First we set the appereance settings group of options
        QGroupBox *appereance = new QGroupBox(tr("Appereance"), this);
        QGridLayout *appereanceLayout = new QGridLayout(appereance);

        QLabel *labelFonts = new QLabel(tr("Fonts (set after reload):"));
        buttonFont = new QPushButton(appereance);
        font = Qucs::font();
        buttonFont->setText(font.toString());
        QLabel *labelBackgroud = new QLabel(tr("Document Background Color:"));
        buttonBackground = new QPushButton(appereance);
        setBackgroundColor(buttonBackground, parent->BGColor);
        QLabel *labelIcons = new QLabel(tr("Icons Size:"));
        spinIcons = new QSpinBox(appereance);
        spinIcons->setValue(parent->iconSize().height());
        spinIcons->setMinimum(10);
        spinIcons->setMaximum(48);

        connect(buttonFont, SIGNAL(clicked()), SLOT(slotFontDialog()));
        connect(buttonBackground, SIGNAL(clicked()), SLOT(slotBGColorDialog()));

        appereanceLayout->addWidget(labelFonts, 0, 0, Qt::AlignLeft);
        appereanceLayout->addWidget(buttonFont, 0, 1, Qt::AlignRight);
        appereanceLayout->addWidget(labelBackgroud, 1,0, Qt::AlignLeft);
        appereanceLayout->addWidget(buttonBackground, 1, 1, Qt::AlignRight);
        appereanceLayout->addWidget(labelIcons, 2, 0, Qt::AlignLeft);
        appereanceLayout->addWidget(spinIcons, 2, 1, Qt::AlignRight);


        //Now we set the misc group of options
        QGroupBox *misc = new QGroupBox(tr("Misc"), this);
        QGridLayout *miscLayout = new QGridLayout(misc);

        QLabel *labelLanguage = new QLabel(tr("Language (set after reload):"));
        comboLanguage = new QComboBox(misc);
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
        for(int z=comboLanguage->count()-1; z>=0; z--)
            if(comboLanguage->itemText(z).section('(',1,1).remove(')') == Qucs::language())
                comboLanguage->setCurrentIndex(z);
        QLabel *labelUndoOps = new QLabel(tr("Maximum undo operations:"));
        spinUndoNum = new QSpinBox(misc);
        spinUndoNum->setValue(parent->maxUndo);
        spinUndoNum->setMinimum(0);
        spinUndoNum->setMaximum(200);
        QLabel *labelTextEditor = new QLabel(tr("Text Editor:"));
        editEditor = new QLineEdit(misc);
        editEditor->setText(parent->Editor);

        miscLayout->addWidget(labelLanguage, 3, 0, Qt::AlignLeft);
        miscLayout->addWidget(comboLanguage, 3, 1, Qt::AlignRight);
        miscLayout->addWidget(labelUndoOps, 4, 0, Qt::AlignLeft);
        miscLayout->addWidget(spinUndoNum, 4, 1, Qt::AlignRight);
        miscLayout->addWidget(labelTextEditor, 5, 0, Qt::AlignLeft);
        miscLayout->addWidget(editEditor, 5, 1, Qt::AlignRight);


        //Now we set the file extensions group of options
        QGroupBox *fileExtensions = new QGroupBox(tr("File Types"), this);
        QGridLayout *fileExtensionsLayout = new QGridLayout(fileExtensions);

        QLabel *fileExtensionsWarning = new QLabel(tr("Register filename extensions here in order to\n"
                                                      "open files with an appropriate program."));

        listSuffix = new QTableWidget(fileExtensions);
        listSuffix->setColumnCount(2);
        listSuffix->setHorizontalHeaderLabels(QStringList() << tr("Suffix") << tr("Program"));
        listSuffix->verticalHeader()->hide();
        listSuffix->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listSuffix->horizontalHeader()->setClickable(false);
        listSuffix->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        // fill tableview with already registered file extensions
        QStringList::Iterator it = parent->FileTypes.begin();
        int r = 0;
        while(it != parent->FileTypes.end()) {
            QTableWidgetItem *Item0, *Item1;
            Item0 = new QTableWidgetItem((*it).section('/',0,0));
            Item1 = new QTableWidgetItem((*it).section('/',1));
            listSuffix->insertRow(r);
            listSuffix->setItem(r,0,Item0);
            listSuffix->setItem(r,1,Item1);
            it++;
            r++;
        }
        listSuffix->resizeColumnsToContents();
        listSuffix->resizeRowsToContents();

        QLabel *labelSuffix = new QLabel(tr("Suffix:"));
        inputSuffix = new QLineEdit(fileExtensions);
        QRegExpValidator *validator  = new QRegExpValidator(QRegExp("[\\w_]+"), this);
        inputSuffix->setValidator(validator);

        QLabel *labelProgram = new QLabel(tr("Program:"));
        inputProgram = new QLineEdit(fileExtensions);

        QPushButton *buttonSet = new QPushButton(tr("Set"), fileExtensions);

        QPushButton *buttonRemove = new QPushButton(tr("Remove"), fileExtensions);
        connect(listSuffix, SIGNAL(itemPressed(QTableWidgetItem*)), SLOT(slotEditSuffix(QTableWidgetItem*)));
        connect(buttonSet, SIGNAL(clicked()), SLOT(slotAdd()));
        connect(buttonRemove, SIGNAL(clicked()), SLOT(slotRemove()));

        fileExtensionsLayout->addWidget(fileExtensionsWarning, 0, 0, 1, 2, Qt::AlignLeft);
        fileExtensionsLayout->addWidget(listSuffix, 1, 0, 6, 1);
        fileExtensionsLayout->addWidget(labelSuffix, 1, 1, Qt::AlignLeft);
        fileExtensionsLayout->addWidget(inputSuffix, 2, 1);
        fileExtensionsLayout->addWidget(labelProgram , 3, 1, Qt::AlignLeft);
        fileExtensionsLayout->addWidget(inputProgram, 4, 1);
        fileExtensionsLayout->addWidget(buttonSet, 5, 1);
        fileExtensionsLayout->addWidget(buttonRemove, 6, 1);


        //Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        QLabel *title_label_ = new QLabel(title());
        vlayout1 -> addWidget(title_label_);

        QFrame *horiz_line_ = new QFrame();
        horiz_line_ -> setFrameShape(QFrame::HLine);
        vlayout1 -> addWidget(horiz_line_);

        vlayout1 -> addWidget(appereance);
        vlayout1 -> addWidget(misc);
        vlayout1 -> addWidget(fileExtensions);

        vlayout1 -> addStretch();

        setLayout(vlayout1);
}

/// Destructor
GeneralConfigurationPage::~GeneralConfigurationPage() {
}

void GeneralConfigurationPage::slotEditSuffix(QTableWidgetItem *Item)
{
  int row = listSuffix->currentRow();
  QTableWidgetItem *Item0 = listSuffix->item(row,0);
  QTableWidgetItem *Item1 = listSuffix->item(row,1);
  if(Item) {
    inputSuffix->setText(Item0->text());
    inputProgram->setText(Item1->text());
  }
  else {
    inputSuffix->setFocus();
    inputSuffix->setText("");
    inputProgram->setText("");
  }
}

void GeneralConfigurationPage::slotAdd()
{
  int row = listSuffix->currentRow();
  QTableWidgetItem *Item0 = listSuffix->item(row,0);
  QTableWidgetItem *Item1 = listSuffix->item(row,1);
  if(Item0) {
    Item0->setText(inputSuffix->text());
    Item1->setText(inputProgram->text());
    listSuffix->resizeColumnsToContents();
    listSuffix->resizeRowsToContents();
    return;
  }

  for(int r = 0; r < listSuffix->rowCount(); r++) {
    Item0 = listSuffix->item(r,0);
    if(Item0->text() == inputSuffix->text()) {
      QMessageBox::critical(this, tr("Error"),
                            tr("This suffix is already registered!"));
      return;
    }
  }

  Item0 = new QTableWidgetItem(inputSuffix->text());
  Item1 = new QTableWidgetItem(inputProgram->text());
  row = listSuffix->rowCount();
  listSuffix->insertRow(row);
  listSuffix->setItem(row,0,Item0);
  listSuffix->setItem(row,1,Item1);
  listSuffix->resizeColumnsToContents();
  listSuffix->resizeRowsToContents();
  inputSuffix->setFocus();
  inputSuffix->setText("");
  inputProgram->setText("");
}

void GeneralConfigurationPage::slotRemove()
{
  int row = listSuffix->currentRow();
  QTableWidgetItem *Item0 = listSuffix->item(row,0);
  if(Item0 == 0) return;

  listSuffix->removeRow(row);   // remove from TableWidget

  inputSuffix->setText("");
  inputProgram->setText("");
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
  if(c.isValid())
    setBackgroundColor(buttonBackground,c);
}

void GeneralConfigurationPage::slotDefaultValues()
{
  font = QFont("Helvetica", 12);
  buttonFont->setText(font.toString());
  comboLanguage->setCurrentIndex(0);
  setBackgroundColor(buttonBackground,QColor(255,250,225));
  spinUndoNum->setValue(20);
  editEditor->setText(Qucs::binaryDir + "qucsedit");
}

/**
        Applies the configuration of this page
*/
void GeneralConfigurationPage::applyConf() {

    bool changed = false;

    if(App->BGColor != getBackgroundColor(buttonBackground)) {
        App->BGColor = getBackgroundColor(buttonBackground);
        changed = true;
    }

    if(App->savingFont != font.toString()) {
        App->savingFont = font.toString();
        changed = true;
    }

    if(App->iconSize().height() != spinIcons->value()) {
        App->setIconSize(QSize(spinIcons->value(), spinIcons->value()));
        App->iconsPixelSize = spinIcons->value();
    }

    App->Language = comboLanguage->currentText().section('(',1,1).remove(')');

    if(App->maxUndo != spinUndoNum->value())
        App->maxUndo = spinUndoNum->value();

    if(App->Editor != editEditor->text())
        App->Editor = editEditor->text();

    QTableWidgetItem *Item0, *Item1;
    App->FileTypes.clear();
    for(int r = 0; r < listSuffix->rowCount(); r++) {
        Item0 = listSuffix->item(r,0);
        Item1 = listSuffix->item(r,1);
        App->FileTypes.append(Item0->text()+"/"+Item1->text());
    }

    App->saveSettings();
    if(changed)
        App->repaint();
}

/// @return Icon of this page
QIcon GeneralConfigurationPage::icon() const {
        return(QIcon(Qucs::bitmapDirectory() + "configure.png"));
}

/// @return Title of this page
QString GeneralConfigurationPage::title() const {
        return(tr("General", "configuration page title"));
}


//****************************************************
// Simulation configuration pages
//****************************************************
/**
        Constructor
        @param QWidget *parent The parent of the dialog.
*/
SimulationConfigurationPage::SimulationConfigurationPage(QucsMainWindow *parent) : SettingsPage(parent)
{

        //First we set the simulator group of options
        QGroupBox *groupSimulator = new QGroupBox(tr("Simulator Engine"), this);
        QRadioButton *qucsatorMode = new QRadioButton(tr("Use qucs engine"), groupSimulator);
        QRadioButton *ngspiceMode = new QRadioButton(tr("Use ngspice engine"), groupSimulator);

        qucsatorMode->setChecked(true);

        QVBoxLayout *simulatorLayout = new QVBoxLayout();
        simulatorLayout->addWidget(qucsatorMode);
        simulatorLayout->addWidget(ngspiceMode);
        groupSimulator->setLayout(simulatorLayout);


        //Now we set the simulation tab group of options
        QGroupBox *groupSimTabMode = new QGroupBox(tr("Simulation Display Mode"), this);
        QRadioButton *specialTabMode = new QRadioButton(tr("Use a special tab"), groupSimTabMode);
        QRadioButton *sameTabMode = new QRadioButton(tr("Use same tab as schematic"), groupSimTabMode);

        specialTabMode->setChecked(true);

        QVBoxLayout *simTabModeLayout = new QVBoxLayout();
        simTabModeLayout->addWidget(specialTabMode);
        simTabModeLayout->addWidget(sameTabMode);
        groupSimTabMode->setLayout(simTabModeLayout);


        //Now we set the misc group of options
        QGroupBox *misc = new QGroupBox(tr("Misc"), this);
        QGridLayout *miscLayout = new QGridLayout(misc);

        QLabel *labelOpenDataDisplay = new QLabel(tr("Open data display after simulation:"));
        QCheckBox *checkOpenDataDisplay = new QCheckBox(misc);
        checkOpenDataDisplay->setChecked(true);

        QLabel *labelDataset = new QLabel(tr("Dataset:"));
        QLineEdit *editDataset = new QLineEdit(misc);

        miscLayout->addWidget(labelOpenDataDisplay, 0, 0, Qt::AlignLeft);
        miscLayout->addWidget(checkOpenDataDisplay, 0, 1, Qt::AlignRight);
        miscLayout->addWidget(labelDataset, 1, 0, Qt::AlignLeft);
        miscLayout->addWidget(editDataset, 1, 1, Qt::AlignRight);


        //Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        QLabel *title_label_ = new QLabel(title());
        vlayout1 -> addWidget(title_label_);

        QFrame *horiz_line_ = new QFrame();
        horiz_line_ -> setFrameShape(QFrame::HLine);
        vlayout1 -> addWidget(horiz_line_);

        vlayout1 -> addWidget(groupSimulator);
        vlayout1 -> addWidget(groupSimTabMode);
        vlayout1 -> addWidget(misc);

        vlayout1 -> addStretch();

        setLayout(vlayout1);
}

/// Destructor
SimulationConfigurationPage::~SimulationConfigurationPage() {
}

/**
        Applies the configuration of this page
*/
void SimulationConfigurationPage::applyConf() {
// TODO Implement this
}

/// @return Icon of this page
QIcon SimulationConfigurationPage::icon() const {
        return(QIcon(Qucs::bitmapDirectory() + "start.png"));
}

/// @return Title of this page
QString SimulationConfigurationPage::title() const {
        return(tr("Simulation", "simulation page title"));
}


//****************************************************
// Document configuration pages
//****************************************************
/**
        Constructor
        @param QWidget *parent The parent of the dialog.
*/
DocumentConfigurationPage::DocumentConfigurationPage(SchematicScene *scene, QucsMainWindow *parent) : SettingsPage(parent)
{

        Scn = scene;

        //First we set the grid group of options
        QGroupBox *grid = new QGroupBox(tr("Grid"), this);
        QGridLayout *gridLayout = new QGridLayout(grid);

        QLabel *labelShowGrid = new QLabel(tr("Show grid:"));
        checkShowGrid = new QCheckBox(grid);
        checkShowGrid->setChecked(Scn->isGridVisible());

        QLabel *labelGridX = new QLabel(tr("Horizontal Grid:"));
        spinGridX = new QSpinBox(grid);
        spinGridX->setValue(Scn->gridWidth());
        spinGridX->setMinimum(2);
        spinGridX->setMaximum(99);

        QLabel *labelGridY = new QLabel(tr("Vertical Grid:"));
        spinGridY = new QSpinBox(grid);
        spinGridY->setValue(Scn->gridHeight());
        spinGridY->setMinimum(2);
        spinGridY->setMaximum(99);

        gridLayout->addWidget(labelShowGrid, 0, 0, Qt::AlignLeft);
        gridLayout->addWidget(checkShowGrid, 0, 1, Qt::AlignRight);
        gridLayout->addWidget(labelGridX, 1, 0, Qt::AlignLeft);
        gridLayout->addWidget(spinGridX, 1, 1, Qt::AlignRight);
        gridLayout->addWidget(labelGridY, 2, 0, Qt::AlignLeft);
        gridLayout->addWidget(spinGridY, 2, 1, Qt::AlignRight);


        //Next we set the frame group of options
        QGroupBox *frame = new QGroupBox(tr("Frame"), this);
        QGridLayout *frameLayout = new QGridLayout(frame);

        QLabel *labelShowFrame = new QLabel(tr("Show frame:"));
        checkShowFrame = new QCheckBox(frame);
        checkShowFrame->setChecked(Scn->isFrameVisible());

        frameLayout->addWidget(labelShowFrame, 0, 0, Qt::AlignLeft);
        frameLayout->addWidget(checkShowFrame, 0, 1, Qt::AlignRight);


        //Next we set the document group of options
        QGroupBox *document = new QGroupBox(tr("Document"), this);
        QGridLayout *documentLayout = new QGridLayout(document);

        QLabel *labelTitle = new QLabel(tr("Title:"));
        editTitle = new QLineEdit(document);
        QLabel *labelName = new QLabel(tr("Name:"));
        editName = new QLineEdit(document);
        QLabel *labelDate = new QLabel(tr("Date:"));
        editDate = new QDateEdit(document);
        QLabel *labelRevision = new QLabel(tr("Revision:"));
        editRevision = new QLineEdit(document);
        foreach(QString frame_text, Scn->frameTexts()){
            if(frame_text.contains("Title:"))
                editTitle->setText(frame_text.remove("Title:"));
            else if(frame_text.contains("Drawn By:"))
                editName->setText(frame_text.remove("Drawn By:"));
            else if(frame_text.contains("Date:"))
                editDate->setDate(QDate::fromString(frame_text.remove("Date:")));
            else if(frame_text.contains("Revision:"))
                editRevision->setText(frame_text.remove("Revision:"));
        }

        documentLayout->addWidget(labelTitle, 0, 0, Qt::AlignLeft);
        documentLayout->addWidget(editTitle, 0, 1, Qt::AlignRight);
        documentLayout->addWidget(labelName, 1, 0, Qt::AlignLeft);
        documentLayout->addWidget(editName, 1, 1, Qt::AlignRight);
        documentLayout->addWidget(labelRevision, 2, 0, Qt::AlignLeft);
        documentLayout->addWidget(editRevision, 2, 1, Qt::AlignRight);
        documentLayout->addWidget(labelDate, 3, 0, Qt::AlignLeft);
        documentLayout->addWidget(editDate, 3, 1, Qt::AlignRight);


        //Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        QLabel *title_label_ = new QLabel(title());
        vlayout1 -> addWidget(title_label_);

        QFrame *horiz_line_ = new QFrame();
        horiz_line_ -> setFrameShape(QFrame::HLine);
        vlayout1 -> addWidget(horiz_line_);

        vlayout1 -> addWidget(grid);
        vlayout1 -> addWidget(frame);
        vlayout1 -> addWidget(document);

        vlayout1 -> addStretch();

        setLayout(vlayout1);
}

/// Destructor
DocumentConfigurationPage::~DocumentConfigurationPage() {
}

/**
        Applies the configuration of this page
*/
void DocumentConfigurationPage::applyConf() {

    if(Scn->isGridVisible() != checkShowGrid->isChecked()){
        Scn->undoStack()->push(new ScenePropertyChangeCmd("grid visibility", checkShowGrid->isChecked(), Scn->isGridVisible(), Scn));
        Scn->setGridVisible(checkShowGrid->isChecked());
    }
    if(Scn->isFrameVisible() != checkShowFrame->isChecked()){
        Scn->undoStack()->push(new ScenePropertyChangeCmd("frame visibility", checkShowFrame->isChecked(), Scn->isFrameVisible(), Scn));
        Scn->setFrameVisible(checkShowFrame->isChecked());
    }
    if(Scn->gridWidth() != spinGridX->value()){
        Scn->undoStack()->push(new ScenePropertyChangeCmd("grid width", spinGridX->value(), Scn->gridWidth(), Scn));
        Scn->setGridWidth(spinGridX->value());
    }
    if(Scn->gridHeight() != spinGridY->value()){
        Scn->undoStack()->push(new ScenePropertyChangeCmd("grid height", spinGridY->value(), Scn->gridHeight(), Scn));
        Scn->setGridHeight(spinGridY->value());
    }

    bool modified = false;
    if(!Scn->frameTexts().contains(tr("Title:")+editTitle->text()))
        modified = true;
    else if(!Scn->frameTexts().contains(tr("Drawn By:")+editName->text()))
        modified = true;
    else if(!Scn->frameTexts().contains(tr("Date:")+editDate->date().toString()))
        modified = true;
    else if(!Scn->frameTexts().contains(tr("Revision:")+editRevision->text()))
        modified = true;

    if(modified){
        QStringList documentProperties = QStringList() << tr("Title:")+editTitle->text() << tr("Drawn By:")+editName->text() <<
                           tr("Date:")+editDate->date().toString() << tr("Revision:")+editRevision->text();
        Scn->undoStack()->push(new ScenePropertyChangeCmd("document properties", documentProperties, Scn->frameTexts(), Scn));
        Scn->setFrameTexts(documentProperties);
    }
}

/// @return Icon of this page
QIcon DocumentConfigurationPage::icon() const {
        return(QIcon(Qucs::bitmapDirectory() + "document-edit.png"));
}

/// @return Title of this page
QString DocumentConfigurationPage::title() const {
        return(tr("General", "document page title"));
}

//****************************************************
// VHDL configuration pages
//****************************************************
/**
        Constructor
        @param QWidget *parent The parent of the dialog.
*/
VhdlConfigurationPage::VhdlConfigurationPage(QucsMainWindow *parent) : SettingsPage(parent) {

        App = parent;

        //First we set the color settings group of options
        QGroupBox *colorsHighlighting = new QGroupBox(tr("Colors for Syntax Highlighting"), this);
        QGridLayout *generalLayout = new QGridLayout(colorsHighlighting);

        commentButton = new QPushButton(tr("Comment"), colorsHighlighting);
        stringButton = new QPushButton(tr("String"), colorsHighlighting);
        integerButton = new QPushButton(tr("Integer Number"), colorsHighlighting);
        realButton = new QPushButton(tr("Real Number"), colorsHighlighting);
        characterButton = new QPushButton(tr("Character"), colorsHighlighting);
        dataButton = new QPushButton(tr("Data Type"), colorsHighlighting);
        attributeButton = new QPushButton(tr("Attribute"), colorsHighlighting);

        setBackgroundColor(commentButton, parent->BGColor);
        setForegroundColor(commentButton, parent->VHDL_Comment);
        setBackgroundColor(stringButton, parent->BGColor);
        setForegroundColor(stringButton, parent->VHDL_String);
        setBackgroundColor(integerButton, parent->BGColor);
        setForegroundColor(integerButton, parent->VHDL_Integer);
        setBackgroundColor(realButton, parent->BGColor);
        setForegroundColor(realButton, parent->VHDL_Real);
        setBackgroundColor(characterButton, parent->BGColor);
        setForegroundColor(characterButton, parent->VHDL_Character);
        setBackgroundColor(dataButton, parent->BGColor);
        setForegroundColor(dataButton, parent->VHDL_Types);
        setBackgroundColor(attributeButton, parent->BGColor);
        setForegroundColor(attributeButton, parent->VHDL_Attributes);

        connect(commentButton, SIGNAL(clicked()), SLOT(slotColorComment()));
        connect(stringButton, SIGNAL(clicked()), SLOT(slotColorString()));
        connect(integerButton, SIGNAL(clicked()), SLOT(slotColorInteger()));
        connect(realButton, SIGNAL(clicked()), SLOT(slotColorReal()));
        connect(characterButton, SIGNAL(clicked()), SLOT(slotColorCharacter()));
        connect(dataButton, SIGNAL(clicked()), SLOT(slotColorDataType()));
        connect(attributeButton, SIGNAL(clicked()), SLOT(slotColorAttributes()));

        generalLayout->addWidget(commentButton, 0, 0);
        generalLayout->addWidget(stringButton, 0, 1);
        generalLayout->addWidget(integerButton, 1, 0);
        generalLayout->addWidget(realButton, 1, 1);
        generalLayout->addWidget(characterButton, 2, 0);
        generalLayout->addWidget(dataButton, 2, 1);
        generalLayout->addWidget(attributeButton, 3, 0);


        //Finally we set the general layout of all groups
        QVBoxLayout *vlayout1 = new QVBoxLayout();
        QLabel *title_label_ = new QLabel(title());
        vlayout1 -> addWidget(title_label_);

        QFrame *horiz_line_ = new QFrame();
        horiz_line_ -> setFrameShape(QFrame::HLine);
        vlayout1 -> addWidget(horiz_line_);

        vlayout1 -> addWidget(colorsHighlighting);

        vlayout1 -> addStretch();

        setLayout(vlayout1);
}

/// Destructor
VhdlConfigurationPage::~VhdlConfigurationPage() {
}

/**
        Applies the configuration of this page
*/
void VhdlConfigurationPage::applyConf() {
      bool changed = false;

      if(App->VHDL_Comment != getForegroundColor(commentButton)) {
          App->VHDL_Comment = getForegroundColor(commentButton);
          changed = true;
      }
      if(App->VHDL_String != getForegroundColor(stringButton)) {
          App->VHDL_String = getForegroundColor(stringButton);
          changed = true;
      }
      if(App->VHDL_Integer != getForegroundColor(integerButton)) {
          App->VHDL_Integer = getForegroundColor(integerButton);
          changed = true;
      }
      if(App->VHDL_Real != getForegroundColor(realButton)) {
          App->VHDL_Real = getForegroundColor(realButton);
          changed = true;
      }
      if(App->VHDL_Character != getForegroundColor(characterButton)) {
          App->VHDL_Character = getForegroundColor(characterButton);
          changed = true;
      }
      if(App->VHDL_Types != getForegroundColor(dataButton)) {
          App->VHDL_Types = getForegroundColor(dataButton);
          changed = true;
      }
      if(App->VHDL_Attributes != getForegroundColor(attributeButton)) {
          App->VHDL_Attributes = getForegroundColor(attributeButton);
          changed = true;
      }

      App->saveSettings();  // also sets the small and large font
      if(changed)
          App->repaint();
}

/// @return Icon of this page
QIcon VhdlConfigurationPage::icon() const {
        return(QIcon(Qucs::bitmapDirectory() + "vhdl-code.png"));
}

/// @return Title of this page
QString VhdlConfigurationPage::title() const {
        return(tr("VHDL", "vhdl page title"));
}

void VhdlConfigurationPage::slotColorComment()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(commentButton), this);
  if(c.isValid())
    setForegroundColor(commentButton,c);
}

void VhdlConfigurationPage::slotColorString()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(stringButton), this);
  if(c.isValid())
    setForegroundColor(stringButton,c);
}

void VhdlConfigurationPage::slotColorInteger()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(integerButton), this);
  if(c.isValid())
    setForegroundColor(integerButton,c);
}

void VhdlConfigurationPage::slotColorReal()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(realButton), this);
  if(c.isValid())
    setForegroundColor(realButton,c);
}

void VhdlConfigurationPage::slotColorCharacter()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(characterButton), this);
  if(c.isValid())
    setForegroundColor(characterButton,c);
}

void VhdlConfigurationPage::slotColorDataType()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(dataButton), this);
  if(c.isValid())
    setForegroundColor(dataButton,c);
}

void VhdlConfigurationPage::slotColorAttributes()
{
  QColor c = QColorDialog::getColor(
                getForegroundColor(attributeButton), this);
  if(c.isValid())
    setForegroundColor(attributeButton,c);
}

void VhdlConfigurationPage::slotDefaultValues()
{
  setForegroundColor(commentButton,Qt::gray);
  setBackgroundColor(commentButton,QColor(255,250,225));
  setForegroundColor(stringButton,Qt::red);
  setBackgroundColor(stringButton,QColor(255,250,225));
  setForegroundColor(integerButton,Qt::blue);
  setBackgroundColor(integerButton,QColor(255,250,225));
  setForegroundColor(realButton,Qt::darkMagenta);
  setBackgroundColor(realButton,QColor(255,250,225));
  setForegroundColor(characterButton,Qt::magenta);
  setBackgroundColor(characterButton,QColor(255,250,225));
  setForegroundColor(dataButton,Qt::darkRed);
  setBackgroundColor(dataButton,QColor(255,250,225));
  setForegroundColor(attributeButton,Qt::darkCyan);
  setBackgroundColor(attributeButton,QColor(255,250,225));
}
