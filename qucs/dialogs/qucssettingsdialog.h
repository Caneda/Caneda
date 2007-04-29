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

#ifndef QUCSSETTINGSDIALOG_H
#define QUCSSETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QFont>
#include <QtCore/QRegExp>

class QListView;
class QListViewItem;
class QLineEdit;
class QVBoxLayout;
class QPushButton;
class QComboBox;
class QIntValidator;
class QRegExpValidator;

class QucsMainWindow;

class QucsSettingsDialog : public QDialog  {
   Q_OBJECT
public:
  QucsSettingsDialog(QucsMainWindow *parent=0, const char *name=0);
 ~QucsSettingsDialog();

private slots:
  void slotOK();
  void slotApply();
  void slotFontDialog();
  void slotBGColorDialog();
  void slotDefaultValues();
  void slotAdd();
  void slotRemove();
  void slotEditSuffix(QListViewItem*);
  void slotColorComment();
  void slotColorString();
  void slotColorInteger();
  void slotColorReal();
  void slotColorCharacter();
  void slotColorDataType();
  void slotColorAttributes();

public:
  QucsMainWindow *App;

  QFont Font;
  QComboBox *LanguageCombo;
  QPushButton *FontButton, *BGColorButton;
  QLineEdit *undoNumEdit, *editorEdit, *Input_Suffix, *Input_Program;
  QListView *List_Suffix;
  QPushButton *ColorComment, *ColorString, *ColorInteger,
       *ColorReal, *ColorCharacter, *ColorDataType, *ColorAttributes;

  QVBoxLayout *all;
  QIntValidator *val200;
  QRegExp Expr;
  QRegExpValidator *Validator;
};

#endif
