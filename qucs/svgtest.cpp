#include "svgtest.h"
#include "schematicscene.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <QtGui/QMessageBox>

static const char* array[] = { "resistor", "inductor", "current" };

SvgItem* SvgTestItem::styleSheetChangingItem = 0;
static bool firsttime = true;

SvgTestItem::SvgTestItem(const QString& id, SchematicScene *scene) :
   SvgItem(scene)
{
   registerConnections(id, scene->svgPainter());
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   if(firsttime) {
      QTimer::singleShot(2500, this, SLOT(changeStyleSheet()));
      firsttime = false;
   }
}

void SvgTestItem::changeStyleSheet()
{
   if(styleSheetChangingItem) {
      schematicScene()->svgPainter()->setStyleSheet(styleSheetChangingItem->groupId(),
                                      "g{stroke: blue; fill: cyan; stroke-width: .5;}");
      styleSheetChangingItem = 0;
      QMessageBox::information(0, "Style change",
                               "The stylesheet of resistors changed! It is working :-)");
   }
}

void SvgTestItem::createTestItems(SchematicScene *scene)
{
   for(int i=0; i < 10; i++) {
      SvgTestItem *s = new SvgTestItem(array[i%3], scene);
      QPointF randomPos(qrand() % int(scene->width()/2), qrand() % int(scene->height()/2));
      s->setPos(scene->nearingGridPoint(randomPos));
      if(i == 0)
         styleSheetChangingItem = s;
   }
}

void SvgTestItem::registerSvgs(SchematicScene *scene)
{
   for(int i=0; i < 3; ++i) {
      QString fileName = QString(array[i]) + ".svg";
      QFile file(fileName);
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      QByteArray content = file.readAll();
      scene->svgPainter()->registerSvg(array[i], content);
   }
}
