/***************************************************************************
                             digisettingsdialog.h
                            ----------------------
    begin                : Sat Apr 01 2006
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

#ifndef DIGISETTINGSDIALOG_H
#define DIGISETTINGSDIALOG_H

#include <qdialog.h>
#include <qregexp.h>

class TextDoc;
class QLineEdit;
class QPushButton;
class QRegExpValidator;


class DigiSettingsDialog : public QDialog  {
Q_OBJECT
public:
  DigiSettingsDialog(TextDoc*);
 ~DigiSettingsDialog();

  QString SimTime;
  QLineEdit *TimeEdit;

private slots:
  void slotOk();

private:
  TextDoc *Doc;
  QRegExp Expr;
  QRegExpValidator *Validator;
};

#endif
