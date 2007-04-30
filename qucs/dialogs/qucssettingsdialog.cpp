/***************************************************************************
 * Copyright (C) 2007 Stefan Jahn <stefan@lkcc.org>                        *
 * Copyright (C) 2004 Michael Margraf <michael.margraf@alumni.tu-berlin.de>*
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

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QLayout>
#include <QtGui/QColorDialog>
#include <QtGui/QFontDialog>
#include <QtGui/QValidator>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QTableWidget>
#include <QtGui/QComboBox>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>

#include "global.h"
#include "qucsmainwindow.h"
#include "qucssettingsdialog.h"

QucsSettingsDialog::QucsSettingsDialog(QucsMainWindow *parent)
  : QDialog(parent)
{
  setWindowModality (Qt::ApplicationModal);
  setAttribute (Qt::WA_DeleteOnClose);
  App = parent;
  setWindowTitle(tr("Edit Qucs Properties"));

  Expr.setPattern("[\\w_]+");
  Validator  = new QRegExpValidator(Expr, this);

  all = new QVBoxLayout(this); // to provide the neccessary size
  QTabWidget *t = new QTabWidget(this);
  all->addWidget(t);

  // ...........................................................
  QWidget *Tab1 = new QWidget(t);
  QGridLayout *gp = new QGridLayout(Tab1);

  gp->addWidget(new QLabel(tr("Font (set after reload):"), Tab1), 0,0);
  FontButton = new QPushButton(Tab1);
  connect(FontButton, SIGNAL(clicked()), SLOT(slotFontDialog()));
  gp->addWidget(FontButton,0,1);

  gp->addWidget(new QLabel(tr("Document Background Color:"), Tab1) ,1,0);
  BGColorButton = new QPushButton("      ", Tab1);
  connect(BGColorButton, SIGNAL(clicked()), SLOT(slotBGColorDialog()));
  gp->addWidget(BGColorButton,1,1);

  gp->addWidget(new QLabel(tr("Language (set after reload):"), Tab1) ,2,0);
  LanguageCombo = new QComboBox(Tab1);
  LanguageCombo->addItem(tr("system language"));
  LanguageCombo->addItem(tr("English")+" (en)");
  LanguageCombo->addItem(tr("German")+" (de)");
  LanguageCombo->addItem(tr("French")+" (fr)");
  LanguageCombo->addItem(tr("Spanish")+" (es)");
  LanguageCombo->addItem(tr("Italian")+" (it)");
  LanguageCombo->addItem(tr("Polish")+" (pl)");
  LanguageCombo->addItem(tr("Romanian")+" (ro)");
  LanguageCombo->addItem(tr("Japanese")+" (jp)");
  LanguageCombo->addItem(tr("Swedish")+" (sv)");
  LanguageCombo->addItem(tr("Hungarian")+" (hu)");
  LanguageCombo->addItem(tr("Hebrew")+" (he)");
  LanguageCombo->addItem(tr("Portuguese")+" (pt)");
  LanguageCombo->addItem(tr("Turkish")+" (tr)");
  LanguageCombo->addItem(tr("Ukrainian")+" (uk)");
  LanguageCombo->addItem(tr("Russian")+" (ru)");
  LanguageCombo->addItem(tr("Czech")+" (cs)");
  LanguageCombo->addItem(tr("Catalan")+" (ca)");
  gp->addWidget(LanguageCombo,2,1);

  val200 = new QIntValidator(0, 200, this);
  gp->addWidget(new QLabel(tr("maximum undo operations:"), Tab1) ,3,0);
  undoNumEdit = new QLineEdit(Tab1);
  undoNumEdit->setValidator(val200);
  gp->addWidget(undoNumEdit,3,1);

  gp->addWidget(new QLabel(tr("text editor:"), Tab1) ,4,0);
  editorEdit = new QLineEdit(Tab1);
  gp->addWidget(editorEdit,4,1);


  t->addTab(Tab1, tr("Settings"));

  // ...........................................................
  QWidget *Tab3 = new QWidget(t);
  QGridLayout *gp3 = new QGridLayout(Tab3);

  gp3->addWidget(new QLabel(tr("Colors for Syntax Highlighting:"), Tab3), 0,0,1,2);

  ColorComment = new QPushButton(tr("Comment"), Tab3);
  setBackgroundColor(ColorComment, App->BGColor);
  setForegroundColor(ColorComment, App->VHDL_Comment);
  connect(ColorComment, SIGNAL(clicked()), SLOT(slotColorComment()));
  gp3->addWidget(ColorComment,1,0);

  ColorString = new QPushButton(tr("String"), Tab3);
  setBackgroundColor(ColorString, App->BGColor);
  setForegroundColor(ColorString, App->VHDL_String);
  connect(ColorString, SIGNAL(clicked()), SLOT(slotColorString()));
  gp3->addWidget(ColorString,1,1);

  ColorInteger = new QPushButton(tr("Integer Number"), Tab3);
  setBackgroundColor(ColorInteger, App->BGColor);
  setForegroundColor(ColorInteger, App->VHDL_Integer);
  connect(ColorInteger, SIGNAL(clicked()), SLOT(slotColorInteger()));
  gp3->addWidget(ColorInteger,2,0);

  ColorReal = new QPushButton(tr("Real Number"), Tab3);
  setBackgroundColor(ColorReal, App->BGColor);
  setForegroundColor(ColorReal, App->VHDL_Real);
  connect(ColorReal, SIGNAL(clicked()), SLOT(slotColorReal()));
  gp3->addWidget(ColorReal,2,1);

  ColorCharacter = new QPushButton(tr("Character"), Tab3);
  setBackgroundColor(ColorCharacter, App->BGColor);
  setForegroundColor(ColorCharacter, App->VHDL_Character);
  connect(ColorCharacter, SIGNAL(clicked()), SLOT(slotColorCharacter()));
  gp3->addWidget(ColorCharacter,3,0);

  ColorDataType = new QPushButton(tr("Data Type"), Tab3);
  setBackgroundColor(ColorDataType, App->BGColor);
  setForegroundColor(ColorDataType, App->VHDL_Types);
  connect(ColorDataType, SIGNAL(clicked()), SLOT(slotColorDataType()));
  gp3->addWidget(ColorDataType,3,1);

  ColorAttributes = new QPushButton(tr("Attribute"), Tab3);
  setBackgroundColor(ColorAttributes, App->BGColor);
  setForegroundColor(ColorAttributes, App->VHDL_Attributes);
  connect(ColorAttributes, SIGNAL(clicked()), SLOT(slotColorAttributes()));
  gp3->addWidget(ColorAttributes,4,0);


  t->addTab(Tab3, tr("VHDL Editor"));

  // ...........................................................
  QWidget *Tab2 = new QWidget(t);
  QGridLayout *gp2 = new QGridLayout(Tab2);

  QLabel *l7 = new QLabel(
    tr("Register filename extensions here in order to\n"
       "open files with an appropriate program."), Tab2);
  gp2->addWidget(l7,0,0,1,3);

  List_Suffix = new QTableWidget(Tab2);
  List_Suffix->setColumnCount(2);
  List_Suffix->setHorizontalHeaderLabels(
    QStringList() << tr("Suffix") << tr("Program"));
  List_Suffix->verticalHeader()->hide();
  List_Suffix->setEditTriggers(QAbstractItemView::NoEditTriggers);
  List_Suffix->horizontalHeader()->setClickable(false);
  List_Suffix->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  gp2->addWidget(List_Suffix,1,0,4,1);
  connect(List_Suffix, SIGNAL(itemPressed(QTableWidgetItem*)),
		SLOT(slotEditSuffix(QTableWidgetItem*)));

  // fill tableview with already registered file extensions
  QStringList::Iterator it = App->FileTypes.begin();
  int r = 0;
  while(it != App->FileTypes.end()) {
    QTableWidgetItem *Item0, *Item1;
    Item0 = new QTableWidgetItem((*it).section('/',0,0));
    Item1 = new QTableWidgetItem((*it).section('/',1));
    List_Suffix->insertRow(r);
    List_Suffix->setItem(r,0,Item0);
    List_Suffix->setItem(r,1,Item1);
    it++;
    r++;
  }
  List_Suffix->resizeColumnsToContents();
  List_Suffix->resizeRowsToContents();

  QLabel *l5 = new QLabel(tr("Suffix:"), Tab2);
  gp2->addWidget(l5,1,1);
  Input_Suffix = new QLineEdit(Tab2);
  Input_Suffix->setValidator(Validator);
  gp2->addWidget(Input_Suffix,1,2);

  QLabel *l6 = new QLabel(tr("Program:"), Tab2);
  gp2->addWidget(l6,2,1);
  Input_Program = new QLineEdit(Tab2);
  gp2->addWidget(Input_Program,2,2);

  QWidget *b = new QWidget(this);
  QHBoxLayout *h = new QHBoxLayout();
  h->setSpacing(3);
  gp2->addWidget(b,3,1,1,2);

  QPushButton *AddButt = new QPushButton(tr("Set"));
  h->addWidget(AddButt);
  connect(AddButt, SIGNAL(clicked()), SLOT(slotAdd()));
  QPushButton *RemoveButt = new QPushButton(tr("Remove"));
  h->addWidget(RemoveButt);
  connect(RemoveButt, SIGNAL(clicked()), SLOT(slotRemove()));
  b->setLayout(h);

  gp2->setRowStretch(4,5);
  t->addTab(Tab2, tr("File Types"));

  // ...........................................................
  // buttons on the bottom of the dialog (independent of the TabWidget)
  QWidget *b1 = new QWidget(this);
  QHBoxLayout *Butts = new QHBoxLayout();
  Butts->setSpacing(3);
  Butts->setMargin(3);
  all->addWidget(b1);

  QPushButton *OkButt = new QPushButton(tr("OK"));
  Butts->addWidget(OkButt);
  connect(OkButt, SIGNAL(clicked()), SLOT(slotOK()));
  QPushButton *ApplyButt = new QPushButton(tr("Apply"));
  Butts->addWidget(ApplyButt);
  connect(ApplyButt, SIGNAL(clicked()), SLOT(slotApply()));
  QPushButton *CancelButt = new QPushButton(tr("Cancel"));
  Butts->addWidget(CancelButt);
  connect(CancelButt, SIGNAL(clicked()), SLOT(reject()));
  QPushButton *DefaultButt = new QPushButton(tr("Default Values"));
  Butts->addWidget(DefaultButt);
  connect(DefaultButt, SIGNAL(clicked()), SLOT(slotDefaultValues()));
  b1->setLayout(Butts);

  OkButt->setDefault(true);

  // ...........................................................
  // fill the fields with the Qucs-Properties

  Font  = Qucs::font();
  FontButton->setText(Font.toString());
  setBackgroundColor(BGColorButton, App->BGColor);
  undoNumEdit->setText(QString::number(App->maxUndo));
  editorEdit->setText(App->Editor);

  for(int z=LanguageCombo->count()-1; z>=0; z--)
    if(LanguageCombo->itemText(z).section('(',1,1).remove(')') == Qucs::language())
      LanguageCombo->setCurrentIndex(z);

  resize(300, 200);
}

QucsSettingsDialog::~QucsSettingsDialog()
{
  delete all;
  delete val200;
  delete Validator;
}

void QucsSettingsDialog::setForegroundColor(QPushButton *b, QColor col)
{
  QPalette palette(b->palette());
  palette.setColor(b->foregroundRole(), col);
  b->setPalette(palette);
}

QColor QucsSettingsDialog::getForegroundColor(QPushButton *b)
{
  QPalette palette(b->palette());
  return palette.color(b->foregroundRole());
}

void QucsSettingsDialog::setBackgroundColor(QPushButton *b, QColor col)
{
  QPalette palette(b->palette());
  palette.setColor(b->backgroundRole(), col);
  b->setPalette(palette);
}

QColor QucsSettingsDialog::getBackgroundColor(QPushButton *b)
{
  QPalette palette(b->palette());
  return palette.color(b->backgroundRole());
}


// -----------------------------------------------------------
void QucsSettingsDialog::slotEditSuffix(QTableWidgetItem *Item)
{
  int row = List_Suffix->currentRow();
  QTableWidgetItem *Item0 = List_Suffix->item(row,0);
  QTableWidgetItem *Item1 = List_Suffix->item(row,1);
  if(Item) {
    Input_Suffix->setText(Item0->text());
    Input_Program->setText(Item1->text());
  }
  else {
    Input_Suffix->setFocus();
    Input_Suffix->setText("");
    Input_Program->setText("");
  }
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotAdd()
{
  int row = List_Suffix->currentRow();
  QTableWidgetItem *Item0 = List_Suffix->item(row,0);
  QTableWidgetItem *Item1 = List_Suffix->item(row,1);
  if(Item0) {
    Item0->setText(Input_Suffix->text());
    Item1->setText(Input_Program->text());
    List_Suffix->resizeColumnsToContents();
    List_Suffix->resizeRowsToContents();
    return;
  }

  for(int r = 0; r < List_Suffix->rowCount(); r++) {
    Item0 = List_Suffix->item(r,0);
    if(Item0->text() == Input_Suffix->text()) {
      QMessageBox::critical(this, tr("Error"),
			    tr("This suffix is already registered!"));
      return;
    }
  }

  Item0 = new QTableWidgetItem(Input_Suffix->text());
  Item1 = new QTableWidgetItem(Input_Program->text());
  row = List_Suffix->rowCount();
  List_Suffix->insertRow(row);
  List_Suffix->setItem(row,0,Item0);
  List_Suffix->setItem(row,1,Item1);
  List_Suffix->resizeColumnsToContents();
  List_Suffix->resizeRowsToContents();
  Input_Suffix->setFocus();
  Input_Suffix->setText("");
  Input_Program->setText("");
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotRemove()
{
  int row = List_Suffix->currentRow();
  QTableWidgetItem *Item0 = List_Suffix->item(row,0);
  if(Item0 == 0) return;

  List_Suffix->removeRow(row);   // remove from TableWidget

  Input_Suffix->setText("");
  Input_Program->setText("");
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotOK()
{
  slotApply();
  accept();
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotApply()
{
  bool changed = false;

  if(App->BGColor != getBackgroundColor(BGColorButton)) {
    App->BGColor = getBackgroundColor(BGColorButton);

    /*    int No=0;
    QWidget *w;
    while((w=App->DocumentTab->page(No++)) != 0)
      if(w->inherits("QTextEdit"))
        ((TextDoc*)w)->viewport()->setPaletteBackgroundColor(
					QucsSettings.BGColor);
      else
        ((Schematic*)w)->viewport()->setPaletteBackgroundColor(
					QucsSettings.BGColor);
    */
    changed = true;
    }

  if(App->savingFont != Font.toString()) {
    App->savingFont = Font.toString();
    changed = true;
  }

  App->Language =
      LanguageCombo->currentText().section('(',1,1).remove(')');
  
  if(App->VHDL_Comment != getForegroundColor(ColorComment)) {
    App->VHDL_Comment = getForegroundColor(ColorComment);
    changed = true;
  }
  if(App->VHDL_String != getForegroundColor(ColorString)) {
    App->VHDL_String = getForegroundColor(ColorString);
    changed = true;
  }
  if(App->VHDL_Integer != getForegroundColor(ColorInteger)) {
    App->VHDL_Integer = getForegroundColor(ColorInteger);
    changed = true;
  }
  if(App->VHDL_Real != getForegroundColor(ColorReal)) {
    App->VHDL_Real = getForegroundColor(ColorReal);
    changed = true;
  }
  if(App->VHDL_Character != getForegroundColor(ColorCharacter)) {
    App->VHDL_Character = getForegroundColor(ColorCharacter);
    changed = true;
  }
  if(App->VHDL_Types != getForegroundColor(ColorDataType)) {
    App->VHDL_Types = getForegroundColor(ColorDataType);
    changed = true;
  }
  if(App->VHDL_Attributes != getForegroundColor(ColorAttributes)) {
    App->VHDL_Attributes = getForegroundColor(ColorAttributes);
    changed = true;
  }

  bool ok;
  if(App->maxUndo != undoNumEdit->text().toUInt(&ok)) {
    App->maxUndo = undoNumEdit->text().toInt(&ok);
    changed = true;
  }
  if(App->Editor != editorEdit->text()) {
    App->Editor = editorEdit->text();
    changed = true;
  }

  QTableWidgetItem *Item0, *Item1;
  App->FileTypes.clear();
  for(int r = 0; r < List_Suffix->rowCount(); r++) {
    Item0 = List_Suffix->item(r,0);
    Item1 = List_Suffix->item(r,1);
    App->FileTypes.append(Item0->text()+"/"+Item1->text());
  }

  App->saveSettings();  // also sets the small and large font
  if(changed)
    App->repaint();
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotFontDialog()
{
  bool ok;
  QFont tmpFont = QFontDialog::getFont(&ok, Font, this);
  if(ok) {
    Font = tmpFont;
    FontButton->setText(Font.toString());
  }
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotBGColorDialog()
{
  QColor c = QColorDialog::getColor(
		getBackgroundColor(BGColorButton), this);
  if(c.isValid())
    setBackgroundColor(BGColorButton,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotDefaultValues()
{
  Font = QFont("Helvetica", 12);
  FontButton->setText(Font.toString());
  LanguageCombo->setCurrentIndex(0);
  setBackgroundColor(BGColorButton,QColor(255,250,225));
  setForegroundColor(ColorComment,Qt::gray);
  setBackgroundColor(ColorComment,QColor(255,250,225));
  setForegroundColor(ColorString,Qt::red);
  setBackgroundColor(ColorString,QColor(255,250,225));
  setForegroundColor(ColorInteger,Qt::blue);
  setBackgroundColor(ColorInteger,QColor(255,250,225));
  setForegroundColor(ColorReal,Qt::darkMagenta);
  setBackgroundColor(ColorReal,QColor(255,250,225));
  setForegroundColor(ColorCharacter,Qt::magenta);
  setBackgroundColor(ColorCharacter,QColor(255,250,225));
  setForegroundColor(ColorDataType,Qt::darkRed);
  setBackgroundColor(ColorDataType,QColor(255,250,225));
  setForegroundColor(ColorAttributes,Qt::darkCyan);
  setBackgroundColor(ColorAttributes,QColor(255,250,225));
  undoNumEdit->setText("20");
  editorEdit->setText(Qucs::binaryDir + "qucsedit");
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorComment()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorComment), this);
  if(c.isValid())
    setForegroundColor(ColorComment,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorString()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorString), this);
  if(c.isValid())
    setForegroundColor(ColorString,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorInteger()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorInteger), this);
  if(c.isValid())
    setForegroundColor(ColorInteger,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorReal()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorReal), this);
  if(c.isValid())
    setForegroundColor(ColorReal,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorCharacter()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorCharacter), this);
  if(c.isValid())
    setForegroundColor(ColorCharacter,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorDataType()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorDataType), this);
  if(c.isValid())
    setForegroundColor(ColorDataType,c);
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorAttributes()
{
  QColor c = QColorDialog::getColor(
		getForegroundColor(ColorAttributes), this);
  if(c.isValid())
    setForegroundColor(ColorAttributes,c);
}
