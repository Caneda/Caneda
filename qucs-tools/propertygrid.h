#include "units.h"
#include <QtGui/QGroupBox>
#include <QtCore/QMap>

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
  Value(double value, Units::UnitType ut = Units::None, int inUnit = Units::None);
  Value(const Value& value);
  Value& operator=(const Value& val);
  void setValue(double val);
  double value() const;
  Value convertedTo(int unit) const; // give converted value without midifying this
  void convertTo(int unit); // modify this instance
  void setUnit(int unit);// just set unit
  QString toString() const;
  int currentUnit() const;
  Units::UnitType unitType() const;

 private:
  double m_value;
  Units::UnitType m_unitType;
  int m_currentUnit;
};



class PropertyBox : public QGroupBox
{
Q_OBJECT

public:
  PropertyBox(const QString& title,QWidget *parent=0l );
  ~PropertyBox(){};
  void addDoubleProperty(const QString& name,const QString &tip,double val = 0.0,
		    Units::UnitType ut = Units::None,int curUnit = Units::None,bool isSel = false);
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
  Units::UnitType unitType(const QString& name);
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
    ~PropertyRow();
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

