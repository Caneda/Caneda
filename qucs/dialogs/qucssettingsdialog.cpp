/***************************************************************************
 * Copyright (C) 2007 by Stefan Jahn <stefan@lkcc.org>                     *
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
#include <QtGui/QListView>
#include <QtGui/QComboBox>
#include <QtGui/QMessageBox>

#include "global.h"
#include "qucsmainwindow.h"
#include "qucssettingsdialog.h"

QucsSettingsDialog::QucsSettingsDialog(QucsMainWindow *parent,
				       const char *name) : QDialog(parent)
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
  //  QGridLayout *gp = new QGridLayout(Tab1,6,2,5,5);
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
  //  QGridLayout *gp3 = new QGridLayout(Tab3,5,2,5,5);
  QGridLayout *gp3 = new QGridLayout(Tab3);

  //  gp3->addMultiCellWidget(new QLabel(tr("Colors for Syntax Highlighting:"), Tab3), 0,0,0,1);

  QPalette palette;
  ColorComment = new QPushButton(tr("Comment"), Tab3);
  //palette.setColor(ColorComment->foregroundRole(), QucsSettings.VHDL_Comment);
  //ColorComment->setPaletteForegroundColor(QucsSettings.VHDL_Comment);
  //ColorComment->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorComment, SIGNAL(clicked()), SLOT(slotColorComment()));
  gp3->addWidget(ColorComment,1,0);

  ColorString = new QPushButton(tr("String"), Tab3);
  //ColorString->setPaletteForegroundColor(QucsSettings.VHDL_String);
  //ColorString->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorString, SIGNAL(clicked()), SLOT(slotColorString()));
  gp3->addWidget(ColorString,1,1);

  ColorInteger = new QPushButton(tr("Integer Number"), Tab3);
  //ColorInteger->setPaletteForegroundColor(QucsSettings.VHDL_Integer);
  //ColorInteger->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorInteger, SIGNAL(clicked()), SLOT(slotColorInteger()));
  gp3->addWidget(ColorInteger,2,0);

  ColorReal = new QPushButton(tr("Real Number"), Tab3);
  //ColorReal->setPaletteForegroundColor(QucsSettings.VHDL_Real);
  //ColorReal->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorReal, SIGNAL(clicked()), SLOT(slotColorReal()));
  gp3->addWidget(ColorReal,2,1);

  ColorCharacter = new QPushButton(tr("Character"), Tab3);
  //ColorCharacter->setPaletteForegroundColor(QucsSettings.VHDL_Character);
  //ColorCharacter->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorCharacter, SIGNAL(clicked()), SLOT(slotColorCharacter()));
  gp3->addWidget(ColorCharacter,3,0);

  ColorDataType = new QPushButton(tr("Data Type"), Tab3);
  //ColorDataType->setPaletteForegroundColor(QucsSettings.VHDL_Types);
  //ColorDataType->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorDataType, SIGNAL(clicked()), SLOT(slotColorDataType()));
  gp3->addWidget(ColorDataType,3,1);

  ColorAttributes = new QPushButton(tr("Attribute"), Tab3);
  //ColorAttributes->setPaletteForegroundColor(QucsSettings.VHDL_Attributes);
  //ColorAttributes->setPaletteBackgroundColor(QucsSettings.BGColor);
  connect(ColorAttributes, SIGNAL(clicked()), SLOT(slotColorAttributes()));
  gp3->addWidget(ColorAttributes,4,0);


  t->addTab(Tab3, tr("VHDL Editor"));

  // ...........................................................
  QWidget *Tab2 = new QWidget(t);
  //  QGridLayout *gp2 = new QGridLayout(Tab2,5,3,3,3);
  QGridLayout *gp2 = new QGridLayout(Tab2);

  QLabel *l7 = new QLabel(
     tr("Register filename extensions here in order to\nopen files with an appropriate program.")
     , Tab2);
  //  gp2->addMultiCellWidget(l7,0,0,0,2);

  List_Suffix = new QListView(Tab2);
  //  List_Suffix->addColumn(tr("Suffix"));
  //  List_Suffix->addColumn(tr("Program"));
  //  gp2->addMultiCellWidget(List_Suffix,1,4,0,0);
  connect(List_Suffix, SIGNAL(clicked(QListViewItem*)),
		SLOT(slotEditSuffix(QListViewItem*)));

  // fill listview with already registered file extensions
  /*QStringList::Iterator it = QucsSettings.FileTypes.begin();
  while(it != QucsSettings.FileTypes.end()) {
    new QListViewItem(List_Suffix,
        (*it).section('/',0,0), (*it).section('/',1,1));
    it++;
    }*/

  QLabel *l5 = new QLabel(tr("Suffix:"), Tab2);
  gp2->addWidget(l5,1,1);
  Input_Suffix = new QLineEdit(Tab2);
  Input_Suffix->setValidator(Validator);
  gp2->addWidget(Input_Suffix,1,2);
//  connect(Input_Suffix, SIGNAL(returnPressed()), SLOT(slotGotoProgEdit()));

  QLabel *l6 = new QLabel(tr("Program:"), Tab2);
  gp2->addWidget(l6,2,1);
  Input_Program = new QLineEdit(Tab2);
  gp2->addWidget(Input_Program,2,2);

  QHBoxLayout *h = new QHBoxLayout();
  h->setSpacing(3);
  //  gp2->addMultiCellWidget(h,3,3,1,2);

  QPushButton *AddButt = new QPushButton(tr("Set"));
  h->addWidget(AddButt);
  connect(AddButt, SIGNAL(clicked()), SLOT(slotAdd()));
  QPushButton *RemoveButt = new QPushButton(tr("Remove"));
  h->addWidget(RemoveButt);
  connect(RemoveButt, SIGNAL(clicked()), SLOT(slotRemove()));

  gp2->setRowStretch(4,5);
  t->addTab(Tab2, tr("File Types"));

  // ...........................................................
  // buttons on the bottom of the dialog (independent of the TabWidget)
  QHBoxLayout *Butts = new QHBoxLayout(this);
  Butts->setSpacing(3);
  Butts->setMargin(3);
  all->addLayout(Butts);

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

  OkButt->setDefault(true);

  // ...........................................................
  // fill the fields with the Qucs-Properties

  Font  = Qucs::font();
  FontButton->setText(Font.toString());
  //  BGColorButton->setPaletteBackgroundColor(QucsSettings.BGColor);
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

// -----------------------------------------------------------
void QucsSettingsDialog::slotEditSuffix(QListViewItem *Item)
{
  if(Item) {
    //    Input_Suffix->setText(Item->text(0));
    //    Input_Program->setText(Item->text(1));
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
  /*  QListViewItem *Item = List_Suffix->selectedItem();
  if(Item) {
    Item->setText(0, Input_Suffix->text());
    Item->setText(1, Input_Program->text());
    return;
    }


  for(Item = List_Suffix->firstChild(); Item!=0; Item = Item->itemBelow())
    if(Item->text(0) == Input_Suffix->text()) {
      QMessageBox::critical(this, tr("Error"),
			tr("This suffix is already registered!"));
      return;
    }

  List_Suffix->ensureItemVisible(
      new QListViewItem(List_Suffix, List_Suffix->lastItem(),
          Input_Suffix->text(), Input_Program->text()));
  Input_Suffix->setFocus();
  Input_Suffix->setText("");
  Input_Program->setText("");*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotRemove()
{
  /*  QListViewItem *Item = List_Suffix->selectedItem();
  if(Item == 0) return;

  List_Suffix->takeItem(Item);   // remove from ListView
  delete Item;

  Input_Suffix->setText("");
  Input_Program->setText("");*/
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

  /*  if(QucsSettings.BGColor != BGColorButton->paletteBackgroundColor()) {
    QucsSettings.BGColor = BGColorButton->paletteBackgroundColor();

    int No=0;
    QWidget *w;
    while((w=App->DocumentTab->page(No++)) != 0)
      if(w->inherits("QTextEdit"))
        ((TextDoc*)w)->viewport()->setPaletteBackgroundColor(
					QucsSettings.BGColor);
      else
        ((Schematic*)w)->viewport()->setPaletteBackgroundColor(
					QucsSettings.BGColor);
    changed = true;
    }*/

  /*  if(savingFont != Font) {
    savingFont = Font;
    changed = true;
    }*/

  /*  QucsSettings.Language =
      LanguageCombo->currentText().section('(',1,1).remove(')');

  if(QucsSettings.VHDL_Comment != ColorComment->paletteForegroundColor()) {
    QucsSettings.VHDL_Comment = ColorComment->paletteForegroundColor();
    changed = true;
  }
  if(QucsSettings.VHDL_String != ColorString->paletteForegroundColor()) {
    QucsSettings.VHDL_String = ColorString->paletteForegroundColor();
    changed = true;
  }
  if(QucsSettings.VHDL_Integer != ColorInteger->paletteForegroundColor()) {
    QucsSettings.VHDL_Integer = ColorInteger->paletteForegroundColor();
    changed = true;
  }
  if(QucsSettings.VHDL_Real != ColorReal->paletteForegroundColor()) {
    QucsSettings.VHDL_Real = ColorReal->paletteForegroundColor();
    changed = true;
  }
  if(QucsSettings.VHDL_Character != ColorCharacter->paletteForegroundColor()) {
    QucsSettings.VHDL_Character = ColorCharacter->paletteForegroundColor();
    changed = true;
  }
  if(QucsSettings.VHDL_Types != ColorDataType->paletteForegroundColor()) {
    QucsSettings.VHDL_Types = ColorDataType->paletteForegroundColor();
    changed = true;
  }
  if(QucsSettings.VHDL_Attributes != ColorAttributes->paletteForegroundColor()) {
    QucsSettings.VHDL_Attributes = ColorAttributes->paletteForegroundColor();
    changed = true;
  }

  bool ok;
  if(QucsSettings.maxUndo != undoNumEdit->text().toUInt(&ok)) {
    QucsSettings.maxUndo = undoNumEdit->text().toInt(&ok);
    changed = true;
  }
  if(QucsSettings.Editor != editorEdit->text()) {
    QucsSettings.Editor = editorEdit->text();
    changed = true;
  }

  QListViewItem *Item;
  QucsSettings.FileTypes.clear();
  for(Item = List_Suffix->firstChild(); Item!=0; Item = Item->itemBelow())
    QucsSettings.FileTypes.append(Item->text(0)+"/"+Item->text(1));


  saveApplSettings(App);  // also sets the small and large font
  */
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
  /*  QColor c = QColorDialog::getColor(
		BGColorButton->paletteBackgroundColor(), this);
  if(c.isValid())
  BGColorButton->setPaletteBackgroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotDefaultValues()
{
  Font = QFont("Helvetica", 12);
  FontButton->setText(Font.toString());
  LanguageCombo->setCurrentIndex(0);
  /*
  BGColorButton->setPaletteBackgroundColor(QColor(255,250,225));

  ColorComment->setPaletteForegroundColor(Qt::gray);
  ColorString->setPaletteForegroundColor(Qt::red);
  ColorInteger->setPaletteForegroundColor(Qt::blue);
  ColorReal->setPaletteForegroundColor(Qt::darkMagenta);
  ColorCharacter->setPaletteForegroundColor(Qt::magenta);
  ColorDataType->setPaletteForegroundColor(Qt::darkRed);
  ColorAttributes->setPaletteForegroundColor(Qt::darkCyan);
  */
  undoNumEdit->setText("20");
  editorEdit->setText(Qucs::binaryDir + "qucsedit");
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorComment()
{
  /*  QColor c = QColorDialog::getColor(
		ColorComment->paletteForegroundColor(), this);
  if(c.isValid())
  ColorComment->setPaletteForegroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorString()
{
  /*  QColor c = QColorDialog::getColor(
		ColorString->paletteForegroundColor(), this);
  if(c.isValid())
  ColorString->setPaletteForegroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorInteger()
{
  /*  QColor c = QColorDialog::getColor(
		ColorInteger->paletteForegroundColor(), this);
  if(c.isValid())
    ColorInteger->setPaletteForegroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorReal()
{
  /*  QColor c = QColorDialog::getColor(
		ColorReal->paletteForegroundColor(), this);
  if(c.isValid())
    ColorReal->setPaletteForegroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorCharacter()
{
  /*  QColor c = QColorDialog::getColor(
		ColorCharacter->paletteForegroundColor(), this);
  if(c.isValid())
    ColorCharacter->setPaletteForegroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorDataType()
{
  /*  QColor c = QColorDialog::getColor(
		ColorDataType->paletteForegroundColor(), this);
  if(c.isValid())
    ColorDataType->setPaletteForegroundColor(c);*/
}

// -----------------------------------------------------------
void QucsSettingsDialog::slotColorAttributes()
{
  /*  QColor c = QColorDialog::getColor(
		ColorAttributes->paletteForegroundColor(), this);
  if(c.isValid())
    ColorAttributes->setPaletteForegroundColor(c);*/
}
