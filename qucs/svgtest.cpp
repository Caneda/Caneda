#include "svgtest.h"
#include "qucssvgrenderer.h"
#include "schematicscene.h"

static const QString svgPath = "/home/gopala/svgs/";

SvgItem::SvgItem(const QString& id, SchematicScene *scene) :
   QucsSvgItem(svgPath + id + ".svg", id, scene)
{
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
//   setStyleSheet("g{stroke:green; stroke-width: 2.5; fill: red}");
}

void SvgItem::createTestItems(SchematicScene *scene)
{
   static char* array[] = { "resistor", "inductor", "current" };
   for(int i=0; i < 10; i++) {
      SvgItem *s = new SvgItem(array[i%3], scene);
      s->setPos(qrand() % int(scene->width()), qrand() % int(scene->height()));
   }
}
