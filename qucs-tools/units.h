#ifndef __UNITS_H
#define __UNITS_H
#include <QtCore/QStringList>
#include <math.h>

#ifdef __MINGW32__
#define atanh(x) (0.5 * log((1.0 + (x)) / (1.0 - (x))))
#define asinh(x) log((x) + sqrt((x) * (x) + 1.0))
#define acosh(x) log((x) + sqrt((x) * (x) - 1.0))
#endif

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef M_E
#define M_E            2.7182818284590452354   /* e */
#endif

#define MU0  12.566370614e-7          /* magnetic constant         */
#define C0   299792458.0              /* speed of light in vacuum  */
#define ZF0  376.73031346958504364963 /* wave resistance in vacuum */

namespace Units
{
  enum UnitType {
    Frequency=0,
    Length,
    Resistance,
    Angle,
    None = -1
  };

  enum FrequencyUnits {
    GHz=0,Hz,KHz,MHz
  };

  enum LengthUnits {
    mil=0,cm,mm,m,um,in,ft
  };

  enum ResistanceUnits {
    Ohm=0,kOhm
  };

  enum AngleUnits {
    Deg=0,Rad
  };
  const QStringList freqList(QStringList() << "GHz" << "Hz" << "kHz" << "MHz");
  const QStringList resList(QStringList() << "Ohm" << "kOhm");
  const QStringList lenList(QStringList() << "mil" << "cm" << "mm" << "m" << "um" << "in" << "ft");
  const QStringList angleList(QStringList() << "deg" << "rad");

  QString toString(FrequencyUnits f);
  QString toString(LengthUnits l);
  QString toString(ResistanceUnits r);
  QString toString(AngleUnits a);
  QString toString(int un,UnitType t);
  int toInt(const QString& unit);
  double convert(double value, Units::UnitType ut, int fromUnit, int toUnit);
};

#endif
