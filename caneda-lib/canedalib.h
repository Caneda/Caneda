/***************************************************************************
                               canedalib.h
                              -----------
    begin                : Sat May 28 2005
    copyright            : (C) 2005 by Michael Margraf
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

#ifndef CANEDALIB_H
#define CANEDALIB_H

#include <QtGui/QDialog>
#include <QtCore/QDir>
#include <QtGui/QFont>
#include <QtCore/QString>
#include <QtCore/QStringList>

class SymbolWidget;
class QTextEdit;
class QComboBox;
class QListWidget;
class QListWidgetItem;
class QGroupBox;
class QVBoxLayout;
class QListViewItem;


// Application settings.
struct tCanedaSettings {
  int x, y, dx, dy;    // position and size of main window
  QFont font;          // font
  QString BitmapDir;   // pixmap directory
  QString LangDir;     // translation directory
  QString LibDir;      // library directory
  QString Language;
};

extern tCanedaSettings CanedaSettings;
extern QDir UserLibDir;

class CanedaLib : public QDialog  {
   Q_OBJECT
public:
  CanedaLib();
 ~CanedaLib();

  QListWidget     *CompList;
  QStringList   LibraryComps;
  QComboBox    *Library;

private slots:
  void slotAbout();
  void slotQuit();
  void slotHelp();
  void slotCopyToClipBoard();
  void slotShowModel();
  void slotSelectLibrary(int);
  void slotSearchComponent();
  void slotShowComponent(QListWidgetItem*,QListWidgetItem*);
  void slotManageLib();

private:
  void closeEvent(QCloseEvent*);
  void putLibrariesIntoCombobox();

  int UserLibCount;
  SymbolWidget *Symbol;
  QTextEdit    *CompDescr;
  QVBoxLayout  *all;
  QString       DefaultSymbol;

  QDir CanedaHomeDir;  // Caneda user directory where all projects are located
};

#endif /* CANEDALIB_H */
