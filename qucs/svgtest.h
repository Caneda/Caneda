#ifndef __SVGTEST_H
#define __SVGTEST_H

#include "svgitem.h"

class SvgTestItem : public SvgItem
{
      Q_OBJECT;
   public:
      SvgTestItem(const QString& id, SchematicScene *scene);
      void writeXml(Qucs::XmlWriter *) {}
      void readXml(Qucs::XmlReader *) {}
      void invokePropertiesDialog() {}

      static void createTestItems(SchematicScene *scene);
      static void registerSvgs(SchematicScene *scene);
      static SvgItem *styleSheetChangingItem;

   private slots:
      void asynchronousChange();
};

#endif
