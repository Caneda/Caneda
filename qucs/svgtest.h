#ifndef __SVGTEST_H
#define __SVGTEST_H

#include "qucssvgitem.h"

class SvgItem : public QucsSvgItem
{
   public:
      SvgItem(const QString& id, SchematicScene *scene);
      void writeXml(Qucs::XmlWriter *) {}
      void readXml(Qucs::XmlReader *) {}
      void invokePropertiesDialog() {}

      static void createTestItems(SchematicScene *scene);
};

#endif
