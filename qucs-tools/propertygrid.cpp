#include <cmath>
//using std::M_PI;
#include "propertygrid.h"

#include <QtCore/QVariant>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QIntValidator>
#include <QtGui/QDoubleValidator>
#include <QtGui/QRadioButton>

const QStringList freqList(QStringList() << "GHz" << "Hz" << "kHz" << "MHz");
const QStringList resList(QStringList() << "Ohm" << "kOhm");
const QStringList lenList(QStringList() << "mil" << "cm" << "mm" << "m" << "um" << "in" << "ft");
const QStringList angleList(QStringList() << "deg" << "rad");

// Unit conversion array for length.
static double convLength[7][7] = {
  { 1.0,       2.54e-3,   2.54e-2,   2.54e-5,   25.4,     1.e-3,   1./12000},
  {1./2.54e-3,    1.0,     10.0,      1.e-2,    1.e4,   1./2.54,   1./30.48},
  {1./2.54e-2,  1./10.,     1.0,       1.e-3,   1.e3,   1./25.4,   1./304.8},
  {1./2.54e-5,    1.e2,    1.e3,         1.0,   1.e6,  1./2.54e-2, 1./0.3048},
  {1./25.4,      1.e-4,   1.e-3,       1.e-6,    1.0,  1./2.54e4,  1./3.048e5},
  {1.e3,          2.54,    25.4,     2.54e-2, 2.54e4,        1.0,    1./12.},
  {1.2e4,        30.48,   304.8,      0.3048, 3.048e5,      12.0,       1.0}
};

// Unit conversion array for frequencies.
static double convFrequency[4][4] = {
  { 1.0,     1.e9,     1.e6,      1.e3},
  { 1.e-9,   1.0,      1.e-3,     1.e-6},
  { 1.e-6,   1.e3,     1.0,       1.e-3},
  { 1.e-3,   1.e6,     1.e3,      1.0}
};

// Unit conversion array for resistances.
static double convResistance[2][2] = {
  {1.0,    1.e-3},
  {1.e3,   1.0}
};

// Unit conversion array for angles.
static double convAngle[2][2] = {
  {1.0,         M_PI/180.0},
  {180.0/M_PI,         1.0}
};


QIntValidator* PropertyGridWidget::intValidator()
{
  static QIntValidator *p_intValidator = new QIntValidator(0l);
  return p_intValidator;
}

QDoubleValidator* PropertyGridWidget::doubleValidator()
{
  static QDoubleValidator* p_doubleValidator = new QDoubleValidator(0l);
  return p_doubleValidator;
}

Property::Property()
{
  label = 0l;
  le = 0l;
  units = 0l;
  radio = 0l;
}

Property::~Property()
{
  delete label;
  delete le;
  delete units;
  delete radio;
}

PropertyGridWidget::PropertyGridWidget(QWidget *parent) : QWidget(parent)
{
  layout = new QGridLayout(this);
  layout->setSpacing(3);
  layout->setMargin(3);
  currentRow = 0;    
}

void PropertyGridWidget::addProperty(const QString& propName,const QString& tip,Property::Type t,
				     Property::UnitType unit,bool selectable)
{
  if(propMap.contains(propName))
  {
    qDebug("PropertyGridWidget::addProperty():  Key already exists");
    return;
  }
  Property *p = new Property();
  p->label = new QLabel(propName);
  p->label->setToolTip(tip);
  p->le = new QLineEdit();
  p->type = t;
  p->unitType = unit;
  if(t == Property::Int)
      p->le->setValidator(intValidator());
  else if(t ==  Property::Double)
      p->le->setValidator(doubleValidator());   
  
  if(unit != Property::NoUnit)
  {
    p->units = new QComboBox();
    switch(unit)
    {
    case Property::Frequency:
      p->units->addItems(freqList);
      break;
    case Property::Resistance:
      p->units->addItems(resList);
      break;
    case Property::Length:
      p->units->addItems(lenList);
    default:break;
    };
  }
  if(selectable)
  {
    p->radio = new QRadioButton();
  }
  propMap.insert(propName,p);
  layout->addWidget(p->label,currentRow,0);
  layout->addWidget(p->le,currentRow,1);
  if(p->units)
    layout->addWidget(p->units,currentRow,2);
  if(p->radio)
    layout->addWidget(p->radio,currentRow,3);
  ++currentRow;
}

void PropertyGridWidget::setValue(const QString& name,const QVariant &val)
{
  if(!propMap.contains(name))
  {
    qDebug("PropertyGridWidget::setValue() : Property doesn't exist");
    return;
  }
  Property *pp = propMap[name];
  if(pp->type == Property::Int)
    pp->le->setText(QString::number(val.toInt()));
  else if(pp->type == Property::Double)
    pp->le->setText(QString::number(val.toDouble()));
  else
    pp->le->setText(val.toString());
}

QVariant PropertyGridWidget::value(const QString &name)
{
  if(!propMap.contains(name))
  {
    qDebug("PropertyGridWidget::value(): Property doesn't exist!");
    return QVariant();
  }
  Property *p = propMap[name];
  if(p->type == Property::Int)
    return QVariant(p->le->text().toInt());
  else if(p->type == Property::Double)
    return QVariant(p->le->text().toDouble());
  else
    return QVariant(p->le->text());
}

void PropertyGridWidget::setUnit(const QString &property,int unit)
{
  if(!propMap.contains(property))
  {
    qDebug("PropertyGridWidget::setUnits() : property doesn't exist");
    return;
  }
  Property *p = propMap[property];
  if(unit < 0 || unit >= p->units->count())
    unit = 0;
  p->units->setCurrentIndex(unit);
  /*
  switch(p->unitType)
  {
  case Property::Frequency:
    p->units->setText(freqList[unit]);
    break;
  case Property::Length:
    p->units->setText(lenList[unit]);
    break;
  case Property::Resistance:
    p->units->setText(resList[unit]);
    break;
  case Property::Angle:
    p->units->setText(angleList[unit]);
    break;
  default:
    break;
    };*/
}

QString PropertyGridWidget::unit(const QString &property)
{
  if(!propMap.contains(property))
  {
    qDebug("PropertyGridWidget::unit(): Property doesn't exist ");
    return QString();
  }
  return propMap[property]->units->currentText();
}

QVariant PropertyGridWidget::value(const QString &pr,int unit)
{
  if(!propMap.contains(pr))
  {
    qDebug("PropertyGridWidget::value() : Property %s doesn't exist !!",pr.toLatin1().constData());
    return QVariant();
  }
  Property *p = propMap[pr];
  if(p->type == Property::String)
  {
    qDebug("PropertyGridWidget::value() : No units for string property");
    return QVariant();
  }
  if(p->unitType == Property::NoUnit)
  {
    qDebug("PropertyGridWidget::value() : Tried to fetch unit when unit doesn't exist");
  }
  if(p->type == Property::Int)
  {
    switch(p->unitType)
      {
      case Property::Frequency:
	return QVariant(p->le->text().toInt()*int(convFrequency[p->units->currentIndex()][unit]));
      case Property::Length:
	return QVariant(p->le->text().toInt()*int(convLength[p->units->currentIndex()][unit]));
      case Property::Resistance:
	return QVariant(p->le->text().toInt()*int(convResistance[p->units->currentIndex()][unit]));
      case Property::Angle:
	return QVariant(p->le->text().toInt()*int(convAngle[p->units->currentIndex()][unit]));
      default:
	return QVariant();
      }
  }
  else
  {
    switch(p->unitType)
      {
      case Property::Frequency:
	return QVariant(p->le->text().toDouble()*convFrequency[p->units->currentIndex()][unit]);
      case Property::Length:
	return QVariant(p->le->text().toDouble()*convLength[p->units->currentIndex()][unit]);
      case Property::Resistance:
	return QVariant(p->le->text().toDouble()*convResistance[p->units->currentIndex()][unit]);
      case Property::Angle:
	return QVariant(p->le->text().toDouble()*convAngle[p->units->currentIndex()][unit]);
      default:
	return QVariant();
      }
  }
      
}
