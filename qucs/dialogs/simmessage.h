/***************************************************************************
                               simmessage.h
                              --------------
    begin                : Sat Sep 6 2003
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

#ifndef SIMMESSAGE_H
#define SIMMESSAGE_H

#include <qdialog.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qfile.h>

class QTextEdit;
class QVBoxLayout;
class QPushButton;
class QProgressBar;
class QucsDoc;

// #define SPEEDUP_PROGRESSBAR


class SimMessage : public QDialog  {
   Q_OBJECT
public:
  SimMessage(QucsDoc*, QWidget *parent=0);
 ~SimMessage();

  bool startProcess();

signals:
  void SimulationEnded(int, SimMessage*);
  void displayDataPage(QString&, QString&);

public slots:
  void slotClose();

private slots:
  void slotDisplayMsg();
  void slotDisplayErr();
  void slotSimEnded();
  void slotDisplayButton();

  void slotReadSpiceNetlist();
  void slotFinishSpiceNetlist();

/* #ifdef SPEEDUP_PROGRESSBAR
  void slotUpdateProgressBar();
private:
  int  iProgress;
  bool waitForUpdate;
#endif
*/

private:
  void FinishSimulation(int);
  void nextSPICE();
  void startSimulator();

public:
  QucsDoc *Doc;
  int showBias;
  bool SimOpenDpl;
  QString DocName, DataSet, DataDisplay;

  QProcess      SimProcess;
  QTextEdit    *ProgText, *ErrText;
  bool          wasLF;   // linefeed for "ProgText"
  QPushButton  *Display, *Abort;
  QProgressBar *SimProgress;
  QString       ProgressText;

  int           SimPorts;
  bool          makeSubcircuit, insertSim;
  QStringList   Collect;
  QFile         NetlistFile;
  QTextStream   Stream;

  QVBoxLayout  *all;
};

#endif
