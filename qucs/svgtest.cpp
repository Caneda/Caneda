#include "svgtest.h"
#include "qucssvgrenderer.h"

ResistorItem::ResistorItem(SchematicScene *scene) :
         QucsSvgItem(QString("/home/gopala/svgs/inductor.svg"), QByteArray(),
                     scene)
{
   QucsSvgRenderer::registerItem(this);
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
}


void ResistorItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option,
             QWidget *)
{
   QucsSvgRenderer::render(painter, this);
}
