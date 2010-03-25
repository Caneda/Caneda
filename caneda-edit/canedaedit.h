/***************************************************************************
                                canedaedit.h
                               ------------
    begin                : Mon Nov 17 2003
    copyright            : (C) 2003 by Michael Margraf
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

#ifndef CANEDAEDIT_H
#define CANEDAEDIT_H

#include <QtGui/QDialog>

class QLabel;
class QTextEdit;
class QTextCharFormat;


struct tCanedaSettings {
  int x, y, dx, dy;    // position and size of main window
  QFont font;
  QString BitmapDir;
  QString LangDir;
  QString Language;
};

extern tCanedaSettings CanedaSettings;


class CanedaEdit : public QDialog  {
   Q_OBJECT
public:
  CanedaEdit(const QString&, bool readOnly=false);
 ~CanedaEdit();

private slots:
  void slotAbout();
  void slotLoad();
  void slotSave();
  void slotQuit();
  void slotPrintCursorPosition();

private:
  void closeEvent(QCloseEvent*);
  bool loadFile(const QString&);
  bool closeFile();

  QString FileName;
  QTextEdit *text;
  QLabel *PosText;
};

#endif
