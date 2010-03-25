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
#include <QtGui/QScrollArea>
#include <QtCore/QMapIterator>

#include <QtCore/QTextStream>

Value::Value(double value, Units::UnitType ut, int inUnit)
{
  m_value = value;
  m_unitType = ut;
  m_currentUnit = inUnit;
}

Value::Value(const Value& val)
{
  m_value = val.m_value;
  m_unitType = val.m_unitType;
  m_currentUnit = val.m_currentUnit;
}

Value& Value::operator=(const Value& val)
{
  m_value = val.m_value;
  m_unitType = val.m_unitType;
  m_currentUnit = val.m_currentUnit;
  return *this;
}

double Value::value() const
{
  return m_value;
}

void Value::setValue(double val)
{
  m_value = val;
}
Value Value::convertedTo(int unit) const
{
  double cnv = Units::convert(m_value,m_unitType,m_currentUnit,unit);
  return Value(cnv,m_unitType,unit);
}

void Value::convertTo(int unit)
{
  m_value = Units::convert(m_value,m_unitType,m_currentUnit,unit);
  m_currentUnit = unit;
}

void Value::setUnit(int unit)
{
  m_currentUnit = unit;
}

QString Value::toString() const
{
  return QString::number(m_value);
}

int Value::currentUnit() const
{
  return m_currentUnit;
}

Units::UnitType Value::unitType() const
{
  return m_unitType;
}


PropertyBox::PropertyRow::PropertyRow() : val(-1.0)
{
  l = 0l;
  le = 0l;
  cb = 0l;
  rb = 0l;
  ocb = 0l;
}

PropertyBox::PropertyRow::~PropertyRow()
{
  delete l;
  delete le;
  delete cb;
  delete rb;
  delete ocb;
}

PropertyBox::PropertyBox(const QString& title,QWidget *parent) : QGroupBox(title,parent)
{
  layout = new QGridLayout(this);
  lastRow = 0;
}

QDoubleValidator* PropertyBox::doubleValidator()
{
  static QDoubleValidator* d = new QDoubleValidator(0l);
  return d;
}

QIntValidator* PropertyBox::intValidator()
{
  static QIntValidator* d = new QIntValidator(0l);
  return d;
}

void PropertyBox::addDoubleProperty(const QString& name,const QString &tip,double val,
                                    Units::UnitType ut,int curUnit,bool isSelectable)
{
   if(paramMap.contains(name))
      return;
   PropertyRow *row = new PropertyRow();
   row->l = new QLabel(name);
   row->l->setToolTip(tip);
   row->le = new QLineEdit();
   row->le->setValidator(doubleValidator());
   connect(row->le, SIGNAL(textEdited(const QString&)), this, SLOT(storeLineEditValues()));
   if(ut != Units::None) {
      row->cb = new QComboBox();
      connect(row->cb, SIGNAL(activated(int)), this, SLOT(storeComboValues()));
      switch(ut)
      {
         case Units::Frequency:
            row->cb->addItems(Units::freqList);
            break;
         case Units::Resistance:
            row->cb->addItems(Units::resList);
            break;
         case Units::Length:
            row->cb->addItems(Units::lenList);
            break;
         case Units::Angle:
            row->cb->addItems(Units::angleList);
            break;
         default:break;
      };
   }
   if(isSelectable)
   {
      row->rb = new QRadioButton();
   }
   row->val = Value(val,ut,curUnit);
   if(ut != Units::None)
      row->cb->setCurrentIndex(curUnit);
   row->le->setText(row->val.toString());

   layout->addWidget(row->l,lastRow,0);
   layout->addWidget(row->le,lastRow,1);
   if(ut != Units::None)
      layout->addWidget(row->cb,lastRow,2);
   if(isSelectable)
      layout->addWidget(row->rb,lastRow,3);
   paramMap[name] = row;
   ++lastRow;
}

void PropertyBox::addIntProperty(const QString& name,const QString &tip,int value)
{
  PropertyRow *row = new PropertyRow();
  row->l = new QLabel(name);
  row->l->setToolTip(tip);
  row->le = new QLineEdit();
  row->le->setText(QString::number(value));
  row->le->setValidator(intValidator());
  layout->addWidget(row->l,lastRow,0);
  layout->addWidget(row->le,lastRow,1);
  paramMap[name] = row;
  ++lastRow;
}

void PropertyBox::addComboProperty(const QString& name, const QString& tip,const QStringList& values)
{
  PropertyRow *row = new PropertyRow();
  row->l = new QLabel(name);
  row->l->setToolTip(tip);
  row->ocb = new QComboBox();
  row->ocb->addItems(values);
  layout->addWidget(row->l,lastRow,0);
  layout->addWidget(row->ocb,lastRow,1);
  paramMap[name] = row;
  ++lastRow;
}

double PropertyBox::doubleValue(const QString& name) const
{
  Q_ASSERT(paramMap.contains(name));
  return paramMap[name]->val.value();
}

double PropertyBox::doubleValue(const QString& name,int inUnit) const
{
  Q_ASSERT(paramMap.contains(name));
  return paramMap[name]->val.convertedTo(inUnit).value();
}

void PropertyBox::setDoubleValue(const QString& name,double val)
{
  Q_ASSERT(paramMap.contains(name));
  PropertyRow *r = paramMap[name];
  r->val.setValue(val);
  r->le->setText(r->val.toString());
}

void PropertyBox::setDoubleValue(const QString& name,double val,int toUnit)
{
  Q_ASSERT(paramMap.contains(name));
  PropertyRow *r = paramMap[name];
  r->val.setValue(val);
  r->val.setUnit(toUnit);
  r->le->setText(r->val.toString());
  r->cb->setCurrentIndex(toUnit);
}

void PropertyBox::convertTo(const QString& name, int unit)
{
  Q_ASSERT(paramMap.contains(name));
  PropertyRow *r = paramMap[name];
  r->val.convertTo(unit);
  r->le->setText(r->val.toString());
  r->cb->setCurrentIndex(unit);
}

void PropertyBox::setIntValue(const QString& name,int val)
{
  Q_ASSERT(paramMap.contains(name) );
  paramMap[name]->le->setText(QString::number(val));
}

int PropertyBox::intValue(const QString& name) const
{
  Q_ASSERT(paramMap.contains(name));
  return paramMap[name]->le->text().toInt();
}

void PropertyBox::setCurrentComboIndex(const QString& name,int index)
{
  Q_ASSERT(paramMap.contains(name) && paramMap[name]->ocb != 0l);
  paramMap[name]->ocb->setCurrentIndex(index);
}

int PropertyBox::currentComboIndex(const QString &name) const
{
  Q_ASSERT(paramMap.contains(name) && paramMap[name]->ocb != 0l);
  return paramMap[name]->ocb->currentIndex();
}

void PropertyBox::setEnabled(const QString& name,bool state)
{
  Q_ASSERT(paramMap.contains(name));
  PropertyRow *r = paramMap[name];
  r->l->setEnabled(state);
  if(r->le)
    r->le->setEnabled(state);
  if(r->cb)
    r->cb->setEnabled(state);
  if(r->rb)
    r->rb->setEnabled(state);
  if(r->ocb)
    r->ocb->setEnabled(state);
}

bool PropertyBox::exists(const QString& name) const
{
  return paramMap.contains(name);
}

bool PropertyBox::isSelected(const QString& name) const
{
  Q_ASSERT(paramMap.contains(name) && paramMap[name]->rb != 0l);
  return paramMap[name]->rb->isChecked();
}

void PropertyBox::setSelected(const QString& name, bool state)
{
  Q_ASSERT(paramMap.contains(name) && paramMap[name]->rb != 0l);
  paramMap[name]->rb->setChecked(state);
}

int PropertyBox::unit(const QString& name) const
{
  Q_ASSERT(paramMap.contains(name));
  return paramMap[name]->val.currentUnit();
}

Units::UnitType PropertyBox::unitType(const QString& name)
{
  Q_ASSERT(paramMap.contains(name));
  return paramMap[name]->val.unitType();
}

Value PropertyBox::value(const QString& name)
{
  Q_ASSERT(paramMap.contains(name));
  return paramMap[name]->val;
}

void PropertyBox::storeComboValues()
{
  QMapIterator<QString,PropertyBox::PropertyRow *> i(paramMap);
  while( i.hasNext()) {
    i.next();
    PropertyBox::PropertyRow *r = i.value();
    if(r->val.unitType() != Units::None)
      r->val.setUnit(r->cb->currentIndex());
  }
}

void PropertyBox::storeLineEditValues()
{
  QMapIterator<QString,PropertyBox::PropertyRow *> i(paramMap);
  while( i.hasNext()) {
    i.next();
    PropertyBox::PropertyRow *r = i.value();
    if(r->le != 0l)
      r->val.setValue(r->le->text().toDouble());
  }
}

QTextStream& operator<<(QTextStream &str, const PropertyBox& box)
{
  QMapIterator<QString,PropertyBox::PropertyRow *> i(box.paramMap);
  while( i.hasNext()) {
    i.next();
    PropertyBox::PropertyRow *r = i.value();
    if(r->val.unitType() != Units::None)
      str << "  " << i.key() << " " << r->val.value() << " "
	  << Units::toString(r->val.currentUnit(), r->val.unitType()) << "\n";
    else
      str << "  " << i.key() << " " << r->val.value() << " " << "NA\n";
  }
  return str;
}


ResultBox::ResultBox(QWidget *par = 0l) : QGroupBox(par)
{
  lastRow = 0;
  QVBoxLayout *ll = new QVBoxLayout(this);
  QScrollArea *area = new QScrollArea();
  ll->addWidget(area);
  wid = new QWidget(area);
  area->setWidget(wid);
  setTitle("       " + tr("Calculated Results") + "     ");
  layout = new QGridLayout(wid);
  layout->setColumnStretch(0,1);
  layout->setColumnStretch(1,1);
  wid->resize(300,200);
}

void ResultBox::addResultItem(const QString& name,const QString& val)
{
  if(resultMap.contains(name))
    return;
  QLabel *l1 = new QLabel(name + QString(":"));
  l1->setAlignment(Qt::AlignRight);
  QLabel *l2 = new QLabel(val);
  l2->setAlignment(Qt::AlignLeft);
  layout->addWidget(l1,lastRow,0);
  layout->addWidget(l2,lastRow,1);
  resultMap[name] = l2;
  ++lastRow;
}

void ResultBox::setValue(const QString& name, int val, const QString& us)
{
  Q_ASSERT(resultMap.contains(name));
  resultMap[name]->setText(QString::number(val).append(us));
}

void ResultBox::setValue(const QString& name, double val, const QString& us)
{
  Q_ASSERT(resultMap.contains(name));
  resultMap[name]->setText(QString::number(val).append(us));
}

void ResultBox::setValue(const QString& name, const QString& val)
{
  Q_ASSERT(resultMap.contains(name));
  resultMap[name]->setText(val);
}

void ResultBox::adjustSize()
{
  //wid->resize(wid->sizeHint());
  //wid->setHeight(wid->
}
