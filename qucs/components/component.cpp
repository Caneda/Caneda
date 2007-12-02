/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "multisymbolcomponent.h"
#include "node.h"
#include "wire.h"
#include "xmlutilities.h"
#include "components.h"
#include "schematicscene.h"
#include "propertytext.h"
#include "undocommands.h"
#include "shapes.h"
#include "componentproperty.h"
#include "qucs-tools/global.h"

#include <QtCore/QDebug>

#include <QtGui/QUndoStack>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>

#include <QtXml/QXmlStreamWriter>
#include <QtXml/QXmlStreamReader>

ComponentPort::ComponentPort(Component* owner,const QPointF& pos) : m_owner(owner), m_centrePos(pos)
{
   SchematicScene *s = owner->schematicScene();
   if(s) {
      QPointF spos = m_owner->mapToScene(pos);
      m_node = s->createNode(spos);
   }
   else
      m_node = new Node(); // To avoid crashes
   m_node->addComponent(m_owner);
}

Component::Component(SchematicScene* scene) : QucsItem(0,scene),
                                              showName(true),
                                              activeStatus(Active),
                                              m_propertyGroup(new PropertyGroup(this,scene))
{
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
}

Component::~Component()
{
   foreach(ComponentPort *port, m_ports)
      port->node()->removeComponent(this);
   if(schematicScene())
      schematicScene()->removeComponent(this);
   delete m_propertyGroup;
}

Component* Component::newOne() const
{
   return Component::componentFromModel(model, schematicScene());
}

void Component::copyTo(Component *component) const
{
   //QucsItem::copyTo(component);

   m_propertyGroup->copyTo(component->m_propertyGroup);

   component->showName = showName;
   component->activeStatus = activeStatus;

   component->update();
}

QString Component::saveString() const
{
   using namespace Qucs;
   QString s = "<" + model;

   if(name.isEmpty()) s += " * ";
   else s += " " + name + " ";

   int i=0;
   if(!showName)
      i = 4;
   i |= activeStatus;
   s += QString::number(i);
   s += " " + realToString(pos().x()) + " " + realToString(pos().y());
   QPointF textPos = m_propertyGroup ? m_propertyGroup->pos() : QPointF(0.,0.);
   s += " " + realToString(textPos.x()) + " " + realToString(textPos.y());
   //FIXME:
   bool m_mirroredX = true;
   uint m_rotated = 0;
   s += m_mirroredX ? " 1" : " 0";
   s += " " + QString::number(m_rotated);

   // write all properties
   foreach(ComponentProperty *p1, properties()) {
      if(p1->description().isEmpty())
         s += " \""+p1->name() + "=" + p1->value() + "\"";   // e.g. for equations
      else s += " \"" + p1->value() + "\"";
      if(p1->isVisible()) s += " 1";
      else s += " 0";
   }

   s += QChar('>');
   return s;
}

bool Component::loadFromString(QString s)
{
   bool ok;
   int tmp;

   if(s.at(0) != '<') return false;
   if(s.at(s.length()-1) != '>') return false;
   s = s.mid(1, s.length()-2);   // cut off start and end character

   QString n;
   name = s.section(' ',1,1);    // Name
   if(name == "*") name = "";

   n  = s.section(' ',2,2);      // isActive
   tmp = n.toInt(&ok);
   if(!ok) return false;
   activeStatus = ActiveStatus(tmp & 3);

   showName = tmp & 4 ? true : false;
   QPointF p;
   n  = s.section(' ',3,3);    // cx
   p.setX(n.toDouble(&ok));
   if(!ok) return false;

   n  = s.section(' ',4,4);    // cy
   p.setY(n.toDouble(&ok));
   if(!ok) return false;
   setPos(p);

   n  = s.section(' ',5,5);    // tx
   p.setX(n.toDouble(&ok));
   if(!ok) return false;

   n  = s.section(' ',6,6);    // ty
   p.setY(n.toDouble(&ok));
   if(!ok) return false;

   m_propertyGroup->setPos(p);

   if(model.at(0) != '.') {  // is simulation component (dc, ac, ...) ?

      n  = s.section(' ',7,7);    // mirroredX
      if(n.toInt(&ok) == 1) mirrorAlong(Qt::XAxis);

      if(!ok) return false;


      n  = s.section(' ',8,8);    // rotated
      uint m_rotated = n.toInt(&ok);
      if(!ok) return false;
      QGraphicsItem::rotate(m_rotated * -90.0);

   }

   unsigned int z=0;
   ComponentProperty *p1;
   foreach(p1,properties()) {
      z++;
      n = s.section('"',z,z);    // property value
      z++;
      p1->setValue(n);
      n  = s.section('"',z,z);    // display
      bool shdShow = n.at(1) == '1';
      if(shdShow)
         p1->show();
      else
         p1->hide();
   }

   return true;
}

void Component::writeXml(Qucs::XmlWriter *writer)
{
   writer->writeStartElement("component");
   writer->writeAttribute("model", model);
   writer->writeAttribute("activestatus", QString::number(int(activeStatus)));

   writer->writeStartElement("name");
   writer->writeAttribute("visible", Qucs::boolToString(showName));
   writer->writeCharacters(name.isEmpty() ? "*" : name);
   writer->writeEndElement(); // </name>

   writer->writeStartElement("pos");
   writer->writePoint(pos());
   writer->writeEndElement(); // </pos>

   writer->writeStartElement("textpos");
   writer->writePoint(propertyGroup()->pos());
   writer->writeEndElement(); // </textpos>

   writer->writeTransform(transform());

   writer->writeStartElement("properties");
   foreach(ComponentProperty *p1, properties()) {
      writer->writeStartElement("property");
      writer->writeAttribute("visible", Qucs::boolToString(p1->isVisible()));

      writer->writeElement("name", p1->name());
      writer->writeElement("value", p1->value());

      writer->writeEndElement(); // </property>
   }
   writer->writeEndElement(); // </properties>

   writer->writeEndElement(); // </component>
}

void Component::readXml(Qucs::XmlReader *reader)
{
   if(!reader->isStartElement() || reader->name() != "component") {
      reader->raiseError(QObject::tr("Unidentified tag %1. Expected %2").arg(reader->name().toString()).arg("component"));
   }
   bool ok = false;
   model = reader->attributes().value("model").toString();
   activeStatus = (ActiveStatus)reader->attributes().value("activestatus").toString().toInt(&ok);
   if(!ok) {
      reader->raiseError(QObject::tr("Couldn't parse the active status attribute"));
   }

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement()) {
         Q_ASSERT(reader->name() == "component");
         break;
      }

      if(reader->isStartElement()) {
         if(reader->name() == "name") {
            name = reader->readElementText();
            //TODO: Take care of visibility. May be make name a property.
         }

         else if(reader->name() == "pos") {
            reader->readFurther();
            QPointF p = reader->readPoint();
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "pos");
            setPos(p);
         }

         else if(reader->name() == "textpos") {
            reader->readFurther();
            QPointF p = reader->readPoint();
            reader->readFurther();
            Q_ASSERT(reader->isEndElement() && reader->name() == "textpos");
            m_propertyGroup->setPos(p);
         }

         else if(reader->name() == "transform") {
            setTransform(reader->readTransform());
         }

         else if(reader->name() == "properties") {
            while(!reader->atEnd()) {
               reader->readNext();

               if(reader->isEndElement()) {
                  Q_ASSERT(reader->name() == "properties");
                  break;
               }

               if(reader->isStartElement()) {
                  if(reader->name() == "property") {
                     QString att = reader->attributes().value("visible").toString();
                     att = att.toLower();
                     if(att != "true" && att != "false") {
                        reader->raiseError(QObject::tr("Invalid bool attribute"));
                        break;
                     }

                     bool vis = (att == "true");

                     reader->readFurther();
                     //read name
                     QString propName = reader->readElementText();
                     reader->readFurther();
                     //read value
                     QString propValue = reader->readElementText();
                     reader->readFurther();

                     ComponentProperty *prop = property(propName);
                     if(prop) {
                        prop->setValue(propValue);
                        prop->setVisible(vis);
                     }
                     Q_ASSERT(reader->isEndElement() && reader->name() == "property");
                  }
                  else
                     reader->readUnknownElement();
               }
            }
         } //end of else if properties
      }
   } //end of topmost while
   propertyGroup()->realignItems();
   checkForConnections();
}

void Component::invokePropertiesDialog()
{
   //TODO:
}

QString Component::netlist() const
{
   QString s = model + ":" + name;

   // output all node names
   foreach(ComponentPort *port, m_ports)
      s += ' ' + port->node()->name(); // node names

   // output all properties
   foreach(ComponentProperty *prop, properties())
      s += ' ' + prop->name() + "'=\"" + prop->value() + "\"";
   return s;
}

QString Component::getNetlist() const
{
   switch(activeStatus) {
      case Active:
         return netlist();
      case Open:
         return QString();
      case Shorten: break;
   }

   // Component is shortened.
   int z=0;
   QString s;
   Q_ASSERT(!m_ports.isEmpty());
   QString Node1 = m_ports.first()->node()->name();
   foreach(ComponentPort *port, m_ports)
      s += "R:" + name + "." + QString::number(z++) + " " +
      Node1 + " " + port->node()->name() + " R=\"0\"\n";

   return s;
}

QString Component::vhdlCode(int) const
{
  return QString("");   // no digital model
}

QString Component::getVHDLCode(int numOfPorts) const
{
   switch(activeStatus) {
      case Open:
         return QString("");
      case Active:
         return vhdlCode(numOfPorts);
      case Shorten: break;
   }

   // Component is shortened.
   // This puts the signal of the second port onto the first port.
   // This is locigally correct for the inverter only, but makes
   // some sense for the gates (OR, AND etc.).
   // Has anyone a better idea?
   Q_ASSERT(m_ports.size() >= 2);
   QString Node1 = m_ports.at(0)->node()->name();
   return "  " + Node1 + " <= " + m_ports.at(1)->node()->name() + ";\n";
}

QString Component::verilogCode(int) const
{
  return QString("");   // no digital model
}

QString Component::getVerilogCode(int numOfPorts) const
{
   switch(activeStatus) {
      case Open:
         return QString("");
      case Active:
         return verilogCode(numOfPorts);
      case Shorten: break;
   }

   // Component is shortened.
   Q_ASSERT(!m_ports.isEmpty());
   QString Node1 = m_ports.first()->node()->name();
   QString s = "";
   foreach(ComponentPort *port, m_ports)
      s += "  assign " + port->node()->name() + " = " + Node1 + ";\n";
   return s;
}

void Component::addPort(const QPointF& pos)
{
   m_ports.append(new ComponentPort(this,pos));
}

ComponentPort* Component::portWithNode(Node *n) const
{
   QList<ComponentPort*>::const_iterator it = m_ports.constBegin();
   const QList<ComponentPort*>::const_iterator end = m_ports.constEnd();
   for(; it != end; ++it) {
      if((*it)->node() == n)
         return *it;
   }
   return 0;
}

void Component::replaceNode(Node *_old, Node *_new)
{
   ComponentPort *p = portWithNode(_old);
   Q_ASSERT(p);
   p->setNode(_new);
}

void Component::checkForConnections(bool exactCentreMatch)
{
   if(!scene()) return;

   if(!exactCentreMatch) {
      bool doBreak = false;
      foreach(ComponentPort *p, m_ports) {
         foreach(QGraphicsItem *item, p->node()->collidingItems()) {
            Node *node = qucsitem_cast<Node*>(item);
            if(node) {
               QPointF delta = node->pos() - p->node()->pos();
               moveBy(delta.x(), delta.y());
               doBreak = true;
               break;
            }
         }
         if(doBreak) break;
      }
   }
   foreach(ComponentPort *p, m_ports) {
      // implicitly calls singlifyNodes() and thus updates connections
      schematicScene()->nodeAt(mapToScene(p->centrePos()));
   }
}

void Component::addProperty(QString _name,QString _initVal,QString _des,bool isVisible,const QStringList& options)
{
   ComponentProperty *prop = new ComponentProperty(this,_name,_initVal,_des,isVisible,options);
   m_propertyGroup->addProperty(prop);
}

ComponentProperty* Component::property(const QString& _name) const
{
   foreach(ComponentProperty* p, properties()) {
      if(p->name() == _name)
         return p;
   }
   return 0;
}

void Component::setInitialPropertyPosition()
{
   m_propertyGroup->forceUpdate();
   QPointF delta = (sceneBoundingRect().bottomLeft() + sceneBoundingRect().bottomRight()) - (m_propertyGroup->sceneBoundingRect().topLeft() + m_propertyGroup->sceneBoundingRect().topRight());
   delta /= 2.;
   m_propertyGroup->moveBy(delta.x(), delta.y());
}

void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *w)
{
   Q_UNUSED(w);

   QList<Shape*>::const_iterator it = m_shapes.constBegin();
   const QList<Shape*>::const_iterator end = m_shapes.constEnd();
   for(; it != end; ++it)
      (*it)->draw(painter, option);

   // For testing purpose
   if( 1 && option->state & QStyle::State_Selected) {
      const QColor fgcolor = option->palette.windowText().color();
      const QColor bgcolor( // ensure good contrast against fgcolor
         fgcolor.red()   > 127 ? 0 : 255,
         fgcolor.green() > 127 ? 0 : 255,
         fgcolor.blue()  > 127 ? 0 : 255);
      const qreal pad = 0.5;

      painter->setPen(QPen(bgcolor, 0, Qt::SolidLine));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(boundingRect().adjusted(pad, pad, -pad, -pad));

      painter->setPen(QPen(fgcolor, 0, Qt::DashLine));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(boundingRect().adjusted(pad, pad, -pad, -pad));
   }
}

void Component::drawNodes(QPainter *p)
{
   QList<ComponentPort*>::const_iterator it = m_ports.constBegin();
   const QList<ComponentPort*>::const_iterator end = m_ports.constEnd();
   p->setPen(QPen(Qt::red));
   for(; it != end; ++it) {
      QRectF rect = (*it)->node()->boundingRect().translated((*it)->centrePos());
      p->drawEllipse(rect);
   }
}

QVariant Component::handlePositionChange(const QPointF& hpos)
{
   QPointF oldPos = pos();

   qreal dx = hpos.x() - oldPos.x();
   qreal dy = hpos.y() - oldPos.y();

   if(m_propertyGroup && !m_propertyGroup->isSelected())
      m_propertyGroup->moveBy(dx,dy);

   if(schematicScene()->areItemsMoving() == false) {
      QList<ComponentPort*>::iterator _it = m_ports.begin();
      const QList<ComponentPort*>::iterator _end = m_ports.end();
      for(; _it != _end; ++_it) {
         ComponentPort *port = *_it;
         if(port->node()->controller() && port->node()->controller() != this)
            continue;
         port->node()->setController(this);
         port->node()->moveBy(dx,dy);
         port->node()->setController(0);
      }
   }
   return QVariant(hpos);
}

QVariant Component::itemChange(GraphicsItemChange change,const QVariant& value)
{
   if (change == ItemPositionChange && scene()) {
      return handlePositionChange(value.toPointF());
   }

   else if( change == ItemTransformChange && scene()) {
      QTransform newTransform = qVariantValue<QTransform>(value);
      QList<ComponentPort*>::iterator it = m_ports.begin();
      const QList<ComponentPort*>::iterator end = m_ports.end();

      for(; it != end; ++it) {
         ComponentPort *port = *it;
         QTransform newSceneTransform = newTransform * QTransform().translate(pos().x(),pos().y());
         QPointF newP = newSceneTransform.map(port->centrePos());

         port->node()->setController(this);
         port->node()->setPos(newP);
         port->node()->setController(0);
      }
      return QVariant(newTransform);
   }
   return QGraphicsItem::itemChange(change,value);
}

Component* Component::componentFromModel(const QString& _model, SchematicScene *scene)
{
   Component *c = 0;
   Q_ASSERT(!_model.isEmpty());
   char first = _model.at(0).toLatin1();     // first letter of component name
   QString cstr = _model.mid(1);
   // to speed up the string comparision, they are ordered by the first
   // letter of their name
   switch(first) {
      case 'R' : if(cstr.isEmpty()) c = new Resistor(scene);
      else if(cstr == "us") {
         c = new Resistor(scene);  // backward capatible
         static_cast<MultiSymbolComponent*>(c)->setSymbol("US");
      }
      else if(cstr == "SFF") c = new RS_FlipFlop(scene);
      else if(cstr == "elais") c = new Relais(scene);
         break;
      case 'C' : if(cstr.isEmpty()) c = 0;//new Capacitor(scene);
         else if(cstr == "CCS") c = new CCCS(scene);
         else if(cstr == "CVS") c = new CCVS(scene);
         else if(cstr == "irculator") c = new Circulator(scene);
         else if(cstr == "oupler") c = new Coupler(scene);
         else if(cstr == "LIN") c = new Coplanar(scene);
         else if(cstr == "OPEN") c = new CPWopen(scene);
         else if(cstr == "SHORT") c = new CPWshort(scene);
         else if(cstr == "GAP") c = new CPWgap(scene);
         else if(cstr == "STEP") c = new CPWstep(scene);
         else if(cstr == "OAX") c = new CoaxialLine(scene);
         break;
      case 'L' : if(cstr.isEmpty()) c = new Inductor(scene);
         //else if(cstr == "ib") c = new LibComp(scene);
         break;
      case 'G' : if(cstr == "ND") c = new Ground(scene);
         else if(cstr == "yrator") c = new Gyrator(scene);
         break;
      case 'I' : if(cstr == "Probe") c = new iProbe(scene);
         else if(cstr == "dc") c = new Ampere_dc(scene);
         else if(cstr == "ac") c = new Ampere_ac(scene);
         else if(cstr == "noise") c = new Ampere_noise(scene);
         else if(cstr == "solator") c = new Isolator(scene);
         else if(cstr == "pulse") c = new iPulse(scene);
         else if(cstr == "rect") c = new iRect(scene);
         else if(cstr == "Inoise") c = new Noise_ii(scene);
         else if(cstr == "Vnoise") c = new Noise_iv(scene);
         //else if(cstr == "nv") c = new Logical_Inv(scene);
         //else if(cstr == "exp") c = new iExp(scene);
         break;
      case 'J' : //if(cstr == "FET") c = new JFET(scene);
         if(cstr == "KFF") c = new JK_FlipFlop(scene);
         break;
      case 'V' : if(cstr == "dc") c = new Volt_dc(scene);
         else if(cstr == "ac") c = new Volt_ac(scene);
         else if(cstr == "CCS") c = new VCCS(scene);
         else if(cstr == "CVS") c = new VCVS(scene);
         else if(cstr == "Probe") c = new vProbe(scene);
         else if(cstr == "noise") c = new Volt_noise(scene);
         else if(cstr == "pulse") c = new vPulse(scene);
         else if(cstr == "rect") c = new vRect(scene);
         else if(cstr == "Vnoise") c = new Noise_vv(scene);
//         else if(cstr == "HDL") c = new VHDL_File(scene);
//         else if(cstr == "erilog") c = new Verilog_File(scene);
//         else if(cstr == "exp") c = new vExp(scene);
         break;
      case 'T' : if(cstr == "r") c = new Transformer(scene);
         else if(cstr == "LIN") c = new TLine(scene);
         else if(cstr == "LIN4P") c = new TLine_4Port(scene);
         else if(cstr == "WIST") c = new TwistedPair(scene);
         break;
      case 's' : if(cstr == "Tr") c = new symTrafo(scene);
         break;
      case 'P' : if(cstr == "ac") c = new Source_ac(scene);
//         else if(cstr == "ort") c = new SubCirPort(scene);
         else if(cstr == "Shift") c = new Phaseshifter(scene);
         else if(cstr == "M_Mod") c = new PM_Modulator(scene);
         break;
      case 'S' : //if(cstr == "Pfile") c = new SParamFile(scene);
//         else if(cstr.left(5) == "Pfile") {  // backward compatible
//            c = new SParamFile(scene);
//            c->Props.getLast()->Value = cstr.mid(5); }
//         else if(cstr == "ub")   c = new Subcircuit(scene);
         if(cstr == "UBST") c = new Substrate(scene);
//         else if(cstr == "PICE") c = new SpiceFile(scene);
//         else if(cstr == "witch") c = new Switch(scene);
         break;
      case 'D' : if(cstr == "CBlock") c = new dcBlock(scene);
         else if(cstr == "CFeed") c = new dcFeed(scene);
//         else if(cstr == "iode") c = new Diode(scene);
         else if(cstr == "igiSource") c = new Digi_Source(scene);
         else if(cstr == "FF") c = new D_FlipFlop(scene);
         break;
      case 'B' : if(cstr == "iasT") c = new BiasT(scene);
//         else if(cstr == "JT") c = new BJTsub(scene);
         else if(cstr == "OND") c = new BondWire(scene);
         break;
      case 'A' : if(cstr == "ttenuator") c = new Attenuator(scene);
         else if(cstr == "mp") c = new Amplifier(scene);
//         else if(cstr == "ND") c = new Logical_AND(scene);
         else if(cstr == "M_Mod") c = new AM_Modulator(scene);
         break;
      case 'M' : if(cstr == "UT") c = new Mutual(scene);
         else if(cstr == "UT2") c = new Mutual2(scene);
         else if(cstr == "LIN") c = new MSline(scene);
//         else if(cstr == "OSFET") c = new MOSFET_sub(scene);
         else if(cstr == "STEP") c = new MSstep(scene);
         else if(cstr == "CORN") c = new MScorner(scene);
//         else if(cstr == "TEE") c = new MStee(scene);
//         else if(cstr == "CROSS") c = new MScross(scene);
         else if(cstr == "MBEND") c = new MSmbend(scene);
         else if(cstr == "OPEN") c = new MSopen(scene);
         else if(cstr == "GAP") c = new MSgap(scene);
         else if(cstr == "COUPLED") c = new MScoupled(scene);
         else if(cstr == "VIA") c = new MSvia(scene);
         break;
//       case 'E' : if(cstr == "qn") c = new Equation(scene);
//          else if(cstr == "DD") c = new EqnDefined(scene);
//          break;
      case 'O' : if(cstr == "pAmp") c = new OpAmp(scene);
//          else if(cstr == "R") c = new Logical_OR(scene);
         break;
//       case 'N' : if(cstr == "OR") c = new Logical_NOR(scene);
//          else if(cstr == "AND") c = new Logical_NAND(scene);
//          break;
      // case '.' : if(cstr == "DC") c = new DC_Sim(scene);
//          else if(cstr == "AC") c = new AC_Sim(scene);
//          else if(cstr == "TR") c = new TR_Sim(scene);
//          else if(cstr == "SP") c = new SP_Sim(scene);
//          else if(cstr == "HB") c = new HB_Sim(scene);
//          else if(cstr == "SW") c = new Param_Sweep(scene);
//          else if(cstr == "Digi") c = new Digi_Sim(scene);
//          else if(cstr == "Opt") c = new Optimize_Sim(scene);
//          break;
      // case '_' : if(cstr == "BJT") c = new BJT(scene);
//          else if(cstr == "MOSFET") c = new MOSFET(scene);
//          break;
	 //case 'X' : if(cstr == "OR") c = new Logical_XOR(scene);
         //else if(cstr == "NOR") c = new Logical_XNOR(scene);
         //break;
	 //case 'h' : if(cstr == "icumL2p1") c = new hicumL2p1(scene);
         //break;
	 //case 'H' : if(cstr == "BT_X") c = new HBT_X(scene);
         //break;
   }
   if(!c) {
      QString msg = QString("Component::componentFromModel():"
                            "Failed to identify component %1").arg(_model);
      qWarning(qPrintable(msg));
   }

   return c;
}
