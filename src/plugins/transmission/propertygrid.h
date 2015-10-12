/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "units.h"

#include <QGroupBox>
#include <QMap>

class QVariant;
class QGridLayout;
class QLabel;
class QComboBox;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;
class QRadioButton;
class QTextStream;

class Value
{
 public:
  Value(double value, Caneda::UnitType ut = Caneda::None, int inUnit = Caneda::None);
  Value(const Value& value);
  Value& operator=(const Value& val);
  void setValue(double val);
  double value() const;
  Value convertedTo(int unit) const; // give converted value without midifying this
  void convertTo(int unit); // modify this instance
  void setUnit(int unit);// just set unit
  QString toString() const;
  int currentUnit() const;
  Caneda::UnitType unitType() const;

 private:
  double m_value;
  Caneda::UnitType m_unitType;
  int m_currentUnit;
};



class PropertyBox : public QGroupBox
{
Q_OBJECT

public:
  PropertyBox(const QString& title,QWidget *parent=0l );

  void addDoubleProperty(const QString& name,const QString &tip,double val = 0.0,
                    Caneda::UnitType ut = Caneda::None,int curUnit = Caneda::None,bool isSel = false);
  void addIntProperty(const QString& name,const QString &tip,int value);
  void addComboProperty(const QString& name, const QString& tip,const QStringList& values);
  double doubleValue(const QString& name) const;
  double doubleValue(const QString& name,int inUnit) const;
  Value value(const QString& prop);
  void setDoubleValue(const QString& name,double val);
  void setDoubleValue(const QString& name,double val,int unit);
  //  void setFoubleValue(const QString& name, double val
  void convertTo(const QString& name, int unit);
  void setIntValue(const QString& name,int val);
  int intValue(const QString& name) const;
  void setCurrentComboIndex(const QString& name,int index);
  int currentComboIndex(const QString& name) const;
  
  void setEnabled(const QString& name,bool state);
  bool exists(const QString& name) const;
  void setSelected(const QString& name, bool state);
  bool isSelected(const QString& name) const;
  int unit(const QString& name) const;
  Caneda::UnitType unitType(const QString& name);
  static QDoubleValidator* doubleValidator();
  static QIntValidator* intValidator();

  friend QTextStream& operator<<(QTextStream &str, const PropertyBox& box);

public:
  struct PropertyRow
  {
    QLabel *l;
    QLineEdit *le;
    QComboBox *cb;
    QRadioButton *rb;
    QComboBox *ocb;//special purpose
    Value val;
    PropertyRow();
  };
private slots:
  void storeComboValues();
  void storeLineEditValues();

 private:  
  QMap<QString,PropertyRow*> paramMap;
  QGridLayout *layout;
  int lastRow;

};

class ResultBox : public QGroupBox
{
Q_OBJECT

 public:
  ResultBox( QWidget *par);
  void addResultItem(const QString& name, const QString& val = QString());
  void setValue(const QString& name, int val, const QString& unitString = QString());
  void setValue(const QString& name, double val, const QString& unitString = QString());
  void setValue(const QString& name, const QString& val);
  void adjustSize();
 private:
  QGridLayout *layout;
  QWidget *wid;
  int lastRow;
  QMap<QString,QLabel*> resultMap;
};

