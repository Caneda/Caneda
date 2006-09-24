#include <QtGui/QWidget>
#include <QtCore/QMap>

class QVariant;
class QGridLayout;
class QLabel;
class QComboBox;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;
class QRadioButton;

class UnitManager
{
public:
  enum LengthUnitsIndices {
    mil=0,cm,mm,m,um,in,ft
  };

  enum ResistanceUnitsIndices {
    Ohm=0,kOhm
  };

  enum FreqUnitsIndices {
    GHz=0,Hz,KHz,MHz
  };

};



struct Property
{
  enum Type {
    String,
    Int,
    Double
  };
  enum UnitType {
    Frequency=0,
    Length,
    Resistance,
    Angle,
    NoUnit
  };

  QLabel *label;
  QLineEdit *le;
  QComboBox *units;
  QRadioButton *radio;
  Type type;
  UnitType unitType;
  
  Property();
  ~Property();
};

class PropertyGridWidget : public QWidget
{
public:
  
 public:
  PropertyGridWidget(QWidget *parent = 0l);
  ~PropertyGridWidget() {};
  void addProperty(const QString& propName,const QString &tip,Property::Type t,
		   Property::UnitType unit,bool selectable=false);
  void setValue(const QString& name,const QVariant &val);
  QVariant value(const QString &property);
  QVariant value(const QString &p,int unit);
  void setUnit(const QString &property,int unit);
  QString unit(const QString &property);
  

 private:
  QGridLayout *layout;
  //static QIntValidator *p_intValidator;
  //static QDoubleValidator *p_doubleValidator;
  static QIntValidator* intValidator();
  static QDoubleValidator* doubleValidator();

 private:
  int currentRow;
  QMap<QString,Property*> propMap;
};
  
