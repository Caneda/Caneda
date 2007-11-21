#include "svgtest.h"
#include "schematicscene.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>

static const char* array[] = { "resistor", "inductor", "current" };

SvgPainter* SvgTestItem::globalSvgPainter = 0;
SvgItem* SvgTestItem::svgitem = 0;
static bool firsttime = true;
SvgTestItem::SvgTestItem(const QString& id, SchematicScene *scene) :
   SvgItem(scene)
{
   registerConnections(id, globalSvgPainter);
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   if(firsttime) {
      QTimer::singleShot(1500, this, SLOT(changeStyleSheet()));
      firsttime = false;
   }
}

void SvgTestItem::changeStyleSheet()
{
   qDebug("CAlled");
   if(svgitem) {
      globalSvgPainter->setStyleSheet(svgitem->groupId(),
                                      "g{stroke: blue; fill: cyan; stroke-width: .5;}");
      svgitem = 0;
   }
}

void SvgTestItem::createTestItems(SchematicScene *scene)
{
   for(int i=0; i < 10; i++) {
      SvgTestItem *s = new SvgTestItem(array[i%3], scene);
      QPointF randomPos(qrand() % int(scene->width()/2), qrand() % int(scene->height()/2));
      s->setPos(scene->nearingGridPoint(randomPos));
      if(i == 0)
         svgitem = s;
         //s->setStyleSheet("g[id]{stroke: darkgray; fill: darkgreen; stroke-width: 2.5;}");
   }
}

void SvgTestItem::registerSvgs()
{
   globalSvgPainter = new SvgPainter;
   for(int i=0; i < 3; ++i) {
      QString fileName = QString(array[i]) + ".svg";
      QFile file(fileName);
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      QByteArray content = file.readAll();
      globalSvgPainter->registerSvg(array[i], content);
   }
}
