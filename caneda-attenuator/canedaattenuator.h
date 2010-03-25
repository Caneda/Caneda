/***************************************************************************
                       Caneda Attenuator Synthesis
                            canedaattenuator.h
                               ------------
    begin                : Jun 14 2006

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CANEDAATTENUATOR_H
#define CANEDAATTENUATOR_H

#include "attenuatorfunc.h"

#include <QtGui/QMainWindow>

class QComboBox;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;
class QLabel;
class QPushButton;
class QStatusBar;

class CanedaAttenuator : public QWidget
{
 Q_OBJECT
 public:
  CanedaAttenuator();
  ~CanedaAttenuator();

 private slots:
  void slotHelpIntro();
  void slotHelpAbout();
  void slotHelpAboutQt();
  void slotTopologyChanged();
  void slotCalculate();
  void slotSetText_Zin(const QString &);
  void slotSetText_Zout(const QString &);

 protected:
  void closeEvent(QCloseEvent *e);

 private:
  void readSettings();
  void writeSettings();
      
  QComboBox *ComboTopology;
  QLabel *LabelTopology, *LabelAtten, *LabelImp1, *LabelImp2;
  QLabel *LabelR1, *LabelR2, *LabelR3, *pixTopology, *LabelResult;
  QLabel *LabelR3_Ohm;
  QLineEdit *lineEdit_Attvalue, *lineEdit_Zin, *lineEdit_Zout;
  QLineEdit *lineEdit_R1, *lineEdit_R2, *lineEdit_R3, *lineEdit_Results;
  QPushButton *Calculate;
  QIntValidator *IntVal;
  QDoubleValidator *DoubleVal;
  QStatusBar *statusBar;

};

#endif
