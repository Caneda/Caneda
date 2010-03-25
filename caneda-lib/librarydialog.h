/***************************************************************************
                             librarydialog.h
                            -----------------
    begin                : Sun Jun 04 2006
    copyright            : (C) 2006 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include <QtGui/QDialog>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>

class QVBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QRadioButton;
class QButtonGroup;
class QRegExpValidator;
class QListWidget;

class LibraryDialog : public QDialog {
   Q_OBJECT
public:
  LibraryDialog(QWidget*);
 ~LibraryDialog();

private slots:
  void slotDelete();
  void slotRename();

private:

  QVBoxLayout *all;   // the mother of all widgets
  QVBoxLayout *Dia_Box;
  QLabel *theLabel;
  QLineEdit *NameEdit;
  QListWidget *listUserLib;

  QPushButton *ButtDelete, *ButtRename, *ButtClose;

  QRegExp Expr;
  QRegExpValidator *Validator;

};

#endif
