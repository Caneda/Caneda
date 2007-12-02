#include "svgtest.h"
#include "schematicscene.h"
#include "component.h"
#include "xmlutilities.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtGui/QMessageBox>
#include <QtSvg/QGraphicsSvgItem>
#include <QtCore/QDir>

static const char* array[] = { "resistor", "inductor", "current" };

SvgItem* SvgTestItem::styleSheetChangingItem = 0;
static bool firsttime = true;
Qucs::Component *c = 0;

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
   c->setPropertyVisible("symbol", true);
   if(0 && styleSheetChangingItem) {
      schematicScene()->svgPainter()->setStyleSheet(styleSheetChangingItem->svgId(),
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

   const QString path(QDir::homePath() + "/.qucs/lib/");
   const QString xmlFile(path + "resistor.xml");

   QFile file(xmlFile);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return;
   QTextStream in(&file);
   in.setCodec("UTF-8");
   QString data = in.readAll();
   Qucs::XmlReader reader(data);
   while(!reader.atEnd()) {
      reader.readNext();

      if(reader.isStartElement() && reader.name() == "component")
         break;
   }
   c = new Qucs::Component(&reader, path, scene);
   c->setPos(scene->nearingGridPoint(QPointF(120, 20)));
   c->setPropertyVisible("Tc1", true);
   c->setPropertyVisible("Tnom", true);
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
