 /***************************************************************************
                          qucstrans.h  -  description
                             -------------------
    begin                : Sun Feb 27 2005
    copyright            : (C) 2005 by Stefan Jahn
    email                : stefan@lkcc.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUCSTRANS_H
#define QUCSTRANS_H

#include <QtGui/QWidget>

class QSplitter;
class QComboBox;
class QLabel;
class QStackedWidget;
class QStatusBar;
class transline;
class PropertyBox;
class ResultBox;
class QTextStream;

// Application settings.
struct tQucsSettings {
  int x, y, dx, dy;    // position and size of main window
  QFont font;          // font
  QString BitmapDir;   // pixmap directory
  QString LangDir;     // translation directory
  QString Language;
  int length_unit;     // default length unit
  int freq_unit;       // default frequency unit
  int res_unit;        // default resistance unit
  int ang_unit;        // default angle unit
  QString Mode;        // current mode
};

extern tQucsSettings QucsSettings;

struct TransWidgets
{
  PropertyBox *subParams;
  PropertyBox *phyParams;
  PropertyBox *comParams;
  PropertyBox *eleParams;
  ResultBox *result;
  transline *line;
  TransWidgets();
  PropertyBox* boxWithProperty(const QString& name);
};

class QucsTranscalc : public QWidget
{
Q_OBJECT

public:
  QucsTranscalc();
  ~QucsTranscalc();
  int currentModeIndex() const;
  bool loadFile(QString fname);
  bool saveFile(QString fname);
  void saveToStream(QTextStream &str);
  QString currentModeString();
  void setCurrentMode(const QString& m);

private:
  void setupFrames();
  void setupMenu();
  void setupMicrostrip();
  void setupRectWaveGuide();
  void setupCoaxialLine();
  void setupCoupledMicrostrip();
  
private slots:
  void slotAbout();
  void slotSelectType(int type);
  void slotAnalyze();
  void slotSynthesize();
  void slotFileLoad();
  void slotFileSave();
  void slotHelpIntro();
  void slotOptions();
  void slotCopyToClipboard();

private:
  TransWidgets *transWidgets[4];
  QSplitter *tranContainers[4];
  QComboBox *tranType;
  QLabel *tranPix;
  QStackedWidget *widStack;
  QStatusBar *statBar;
};
#endif /* QUCSTRANS_H */
