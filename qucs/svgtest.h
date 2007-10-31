#include "qucssvgitem.h"

class ResistorItem : public QucsSvgItem
{
   public:
      ResistorItem(SchematicScene *scene);
      QString uniqueId() const { return "inductor"; }
      void paint (QPainter * painter, const QStyleOptionGraphicsItem * option,
                   QWidget * widget = 0 );
      void writeXml(Qucs::XmlWriter *) {}
      void readXml(Qucs::XmlReader *) {}
      void invokePropertiesDialog() {}
};
