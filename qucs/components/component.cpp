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
#include "components.h"
#include "schematicscene.h"
#include "propertytext.h"
#include "undocommands.h"
#include "shapes.h"
#include "componentproperty.h"
#include "qucs-tools/global.h"

#include <QtGui/QUndoStack>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>

QMap<int,QPen> Component::pens;

ComponentPort::ComponentPort(Component* owner,const QPointF& pos) : m_owner(owner), m_centrePos(pos)
{
   SchematicScene *s = owner->schematicScene();
   if(s) {
      QPointF spos = m_owner->mapToScene(pos);

      m_node = s->nodeAt(spos);
      if(!m_node)
         m_node = s->createNode(spos);
   }
   else
      m_node = new Node(); // To avoid crashes
   m_node->addComponent(m_owner);
}

Component::Component(SchematicScene* scene) : QucsItem(0,scene),showName(true),
                                              activeStatus(Active), m_pen(getPen(Qt::darkBlue,0))
{
   setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
   m_rotated = 0;
   m_mirroredX = false;
   m_mirroredY = false;
   if(scene) {
      scene->insertComponent(this);
      m_propertyGroup = new PropertyGroup(this,scene);
   }
   else
      m_propertyGroup = 0;
}

Component::~Component()
{
   foreach(ComponentPort *port, m_ports)
      port->node()->removeComponent(this);
   if(schematicScene())
      schematicScene()->removeComponent(this);
   delete m_propertyGroup;
}

Component* Component::copy() const
{
   Component *c = Component::componentFromName(model,schematicScene());
   qDeleteAll(m_properties);
   c->m_properties = m_properties;
   c->setMatrix(matrix());
   c->name = name;
   c->model = model;
   c->description = description;
   c->showName = showName;
   c->activeStatus = activeStatus;
   return c;
}

QString Component::netlist() const
{
   QString s = model + ":" + name;

   // output all node names
   foreach(ComponentPort *port, m_ports)
      s += ' ' + port->node()->name(); // node names

   // output all properties
   foreach(ComponentProperty *prop, m_properties)
      s += ' ' + prop->name() + "'=\"" + prop->value() + "\"";
   return s;
}

QString Component::shortNetlist() const
{
   int z=0;
   QString s;
   QString Node1 = m_ports.first()->node()->name();
   foreach(ComponentPort *port, m_ports) {
      if( z == 0) continue;
      s += "R:" + name + "." + QString::number(z++) + ' ' +
         Node1 + ' ' + port->node()->name() + " R=\"0\"\n";
   }
   return s;
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
   s += m_mirroredX ? " 1" : " 0";
   s += " " + QString::number(m_rotated);

   // write all properties
   foreach(ComponentProperty *p1, m_properties) {
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
      if(n.toInt(&ok) == 1) mirrorX();

      if(!ok) return false;


      n  = s.section(' ',8,8);    // rotated
      m_rotated = n.toInt(&ok);
      if(!ok) return false;
      QGraphicsItem::rotate(m_rotated*-90.0);

   }

   unsigned int z=0;
   ComponentProperty *p1;
   foreach(p1,m_properties) {
      z++;
      n = s.section('"',z,z);    // property value
      z++;
      *p1 = n;
      n  = s.section('"',z,z);    // display
      bool shdShow = n.at(1) == '1';
      if(shdShow)
         p1->show();
      else
         p1->hide();
   }

   return true;
}

void Component::addProperty(QString _name,QString _initVal,QString _des,bool isVisible,const QStringList& options)
{
   ComponentProperty *prop = new ComponentProperty(this,_name,_initVal,_des,isVisible,options);
   if(m_propertyGroup)
      m_propertyGroup->addChild(prop);
   m_properties.append(prop);
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

QVariant Component::handlePositionChange(const QPointF& hpos)
{
   QPointF oldPos = pos();

   qreal dx = hpos.x() - oldPos.x();
   qreal dy = hpos.y() - oldPos.y();

   if(m_propertyGroup && !m_propertyGroup->isSelected())
      m_propertyGroup->moveBy(dx,dy);

   if(schematicScene()->areItemsMoving() == false)
   {
      QList<ComponentPort*>::iterator _it = m_ports.begin();
      const QList<ComponentPort*>::iterator _end = m_ports.end();
      for(; _it != _end; ++_it)
      {
         ComponentPort *port = *_it;
         if(port->node()->isControllerSet() && port->node()->controller() != this)
            continue;
         port->node()->setController(this);
         port->node()->moveBy(dx,dy);
         port->node()->resetController();
      }
   }
   return QVariant(hpos);
}

QVariant Component::itemChange(GraphicsItemChange change,const QVariant& value)
{
   if (change == ItemPositionChange && scene())
      return handlePositionChange(value.toPointF());

#if QT_VERSION >= 0x040300
   else if( change == ItemTransformChange && scene())
   {
      QTransform newTransform = qVariantValue<QTransform>(value);
      QList<ComponentPort*>::iterator it = m_ports.begin();
      const QList<ComponentPort*>::iterator end = m_ports.end();

      for(; it != end; ++it)
      {
         ComponentPort *port = *it;
         QTransform newSceneTransform = newTransform * QTransform().translate(pos().x(),pos().y());
         QPointF newP = newSceneTransform.map(port->centrePos());

         port->node()->setController(this);
         port->node()->setPos(newP);
         port->node()->resetController();
      }
      return QVariant(newTransform);
   }
#else
   else if(change == ItemMatrixChange && scene())
   {
      QMatrix newMatrix = qVariantValue<QMatrix>(value);
      QList<ComponentPort*>::iterator it = m_ports.begin();
      const QList<ComponentPort*>::iterator end = m_ports.end();

      for(; it != end; ++it)
      {
         ComponentPort *port = *it;
         QMatrix newSceneMatrix = newMatrix * QMatrix().translate(pos().x(),pos().y());
         QPointF newP = newSceneMatrix.map(port->centrePos());

         port->node()->setController(this);
         port->node()->setPos(newP);
         port->node()->resetController();
      }
      return QVariant(newMatrix);
   }
#endif
   return QGraphicsItem::itemChange(change,value);
}

void Component::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
   QucsItem::mousePressEvent(event);
}

void Component::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
   QucsItem::mouseMoveEvent(event);
}

void Component::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
   QucsItem::mouseReleaseEvent(event);
}

Component* Component::componentFromName(const QString& comp,SchematicScene *scene)
{
   Component *c = 0;
   if(comp == "Resistor")
      c =  new Resistor(scene);
   else if(comp == "ResistorUS") {
      c =  new Resistor(scene);
      ((MultiSymbolComponent*)c)->setSymbol("US");
   }
   else if(comp == "AM_Mod")
      c = new AM_Modulator(scene);
   else if(comp == "Iac")
      c = new Ampere_ac(scene);
   else if(comp == "Idc")
      c = new Ampere_dc(scene);
   else if(comp == "Inoise")
      c = new Ampere_noise(scene);
   else if(comp == "Amp")
      c = new Amplifier(scene);
   else if(comp == "Attenuator")
      c = new Attenuator(scene);
   else if(comp == "BiasT")
      c = new BiasT(scene);
   else if(comp == "BOND")
      c = new BondWire(scene);
   else if(comp == "CCCS")
      c = new CCCS(scene);
   else if(comp == "CCVS")
      c = new CCVS(scene);
   else if(comp == "Circulator")
      c = new Circulator(scene);
   else if(comp == "COAX")
      c = new CoaxialLine(scene);
   else if(comp == "CLIN")
      c = new Coplanar(scene);
   else if(comp == "Coupler")
      c = new Coupler(scene);
   else if(comp == "CGAP")
      c = new CPWgap(scene);
   else if(comp == "COPEN")
      c = new CPWopen(scene);
   else if(comp == "CSHORT")
      c = new CPWshort(scene);
   else if(comp == "CSTEP")
      c = new CPWstep(scene);
   else if(comp == "DCBlock")
      c = new dcBlock(scene);
   else if(comp == "DCFeed")
      c = new dcFeed(scene);
   else if(comp == "DFF")
      c = new D_FlipFlop(scene);
   else if(comp == "DigiSource")
      c = new Digi_Source(scene);
   else if(comp == "GND")
      c = new Ground(scene);
   else if(comp == "Gyrator")
      c = new Gyrator(scene);
   else if(comp == "L")
      c = new Inductor(scene);
   else if(comp == "IProbe")
      c = new iProbe(scene);
   else if(comp == "Ipulse")
      c = new iPulse(scene);
   else if(comp == "Irect")
      c = new iRect(scene);
   else if(comp == "Isolator")
      c = new Isolator(scene);
   else if(comp == "JKFF")
      c = new JK_FlipFlop(scene);
   else if(comp == "MCORN")
      c = new MScorner(scene);
   else if(comp == "MCOUPLED")
      c = new MScoupled(scene);
   else if(comp == "MGAP")
      c = new MSgap(scene);
   else if(comp == "MLIN")
      c = new MSline(scene);
   else if(comp == "MMBEND")
      c = new MSmbend(scene);
   else if(comp == "MOPEN")
      c = new MSopen(scene);
   else if(comp == "MSTEP")
      c = new MSstep(scene);
   else if(comp == "MVIA")
      c = new MSvia(scene);
   else if(comp == "MUT2")
      c = new Mutual2(scene);
   else if(comp == "MUT")
      c = new Mutual(scene);
   else if(comp == "IInoise")
      c = new Noise_ii(scene);
   else if(comp == "IVnoise")
      c = new Noise_iv(scene);
   else if(comp == "VVnoise")
      c = new Noise_vv(scene);
   else if(comp == "OpAmp")
      c = new OpAmp(scene);
   else if(comp == "PShift")
      c = new Phaseshifter(scene);
   else if(comp == "PM_Mod")
      c = new PM_Modulator(scene);
   else if(comp == "Relais")
      c = new Relais(scene);
   else if(comp == "RSFF")
      c = new RS_FlipFlop(scene);
   else if(comp == "Pac")
      c = new Source_ac(scene);
   else if(comp == "SUBST")
      c = new Substrate(scene);
   else if(comp == "sTr")
      c = new symTrafo(scene);
   else if(comp == "TLIN4P")
      c = new TLine_4Port(scene);
   else if(comp == "TLIN")
      c = new TLine(scene);
   else if(comp == "Tr")
      c = new Transformer(scene);
   else if(comp == "TWIST")
      c = new TwistedPair(scene);
   else if(comp == "VCCS")
      c = new VCCS(scene);
   else if(comp == "VCVS")
      c = new VCVS(scene);
   else if(comp == "Vac")
      c = new Volt_ac(scene);
   else if(comp == "Vdc")
      c = new Volt_dc(scene);
   else if(comp == "Vnoise")
      c = new Volt_noise(scene);
   else if(comp == "VProbe")
      c = new vProbe(scene);
   else if(comp == "Vpulse")
      c = new vPulse(scene);
   else if(comp == "Vrect")
      c = new vRect(scene);
   return c;
}

Component* Component::componentFromLine(QString Line, SchematicScene *scene)
{
   Component *c = 0;

   Line = Line.trimmed();
   if(Line.at(0) != '<') {
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Format Error:\nWrong line start!"));
      return 0;
   }

   QString cstr = Line.section(' ',0,0); // component type
   char first = Line.at(1).toLatin1();     // first letter of component name
   cstr.remove(0,2);    // remove leading "<" and first letter

   // to speed up the string comparision, they are ordered by the first
   // letter of their name
   switch(first) {
      case 'R' : if(cstr.isEmpty()) c = new Resistor(scene);
      else if(cstr == "us") {
         c = new Resistor(false);  // backward capatible
         ((MultiSymbolComponent*)c)->setSymbol("US");
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
//       case 'O' : if(cstr == "pAmp") c = new OpAmp(scene);
//          else if(cstr == "R") c = new Logical_OR(scene);
//          break;
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
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Format Error:\nUnknown component!"));
      return 0;
   }

   if(!c->loadFromString(Line)) {
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Format Error:\nWrong 'component' line format!"));
      delete c;
      return 0;
   }
   return c;
}

Component* Component::componentFromModel(const QString& _model, SchematicScene *scene)
{
   Component *c = 0;
   char first = _model.at(0).toLatin1();     // first letter of component name
   QString cstr = _model.mid(1);
   // to speed up the string comparision, they are ordered by the first
   // letter of their name
   switch(first) {
      case 'R' : if(cstr.isEmpty()) c = new Resistor(scene);
      else if(cstr == "us") {
         c = new Resistor(false);  // backward capatible
         ((MultiSymbolComponent*)c)->setSymbol("US");
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
//       case 'O' : if(cstr == "pAmp") c = new OpAmp(scene);
//          else if(cstr == "R") c = new Logical_OR(scene);
//          break;
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
      QMessageBox::critical(0, QObject::tr("Error"),
                            QObject::tr("Format Error:\nUnknown component!"));
      return 0;
   }

   return c;
}


void Component::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
   Q_UNUSED(w);

   QList<Shape*>::const_iterator it = m_shapes.constBegin();
   const QList<Shape*>::const_iterator end = m_shapes.constEnd();
   for(; it != end; ++it)
      (*it)->draw(p,o);

   // For testing purpose
   if( 1 && o->state & QStyle::State_Selected)
   {
      p->setPen(getPen(Qt::darkGray,2));
      p->drawRect(boundingRect());
   }
   if(o->state & QStyle::State_Open)
      drawNodes(p);
}

void Component::drawNodes(QPainter *p)
{
   QList<ComponentPort*>::const_iterator it = m_ports.constBegin();
   const QList<ComponentPort*>::const_iterator end = m_ports.constEnd();
   p->setPen(QPen(Qt::red));
   for(; it != end; ++it)
   {
      QRectF rect = (*it)->node()->boundingRect().translated((*it)->centrePos());
      p->drawEllipse(rect);
   }
}

const QPen& Component::getPen(QColor color,int penWidth,Qt::PenStyle style)
{
   int hash = (color.red() * 2) + (color.blue() * 3) + (color.green() * 5) + (penWidth*7) + (int(style)*11);
   if(!(Component::pens.contains(hash)))
   {
      QPen p = QPen(color);
      p.setWidth(penWidth);
      p.setStyle(style);
      Component::pens[hash] = p;
   }
   return Component::pens[hash];
}

void Component::mirrorX()
{
   m_mirroredX = !m_mirroredX;
   QucsItem::mirrorX();
   //TODO : Take care of texts so that only their pos is changed
   //         not symbol!
}

void Component::mirrorY()
{
   m_mirroredY = !m_mirroredY;
   QucsItem::mirrorY();
   //TODO : Take care of texts so that only their pos is changed
   //         not symbol!
}

void Component::rotate()
{
   m_rotated++;
   m_rotated &= 3;
   QucsItem::rotate();
}

ComponentProperty* Component::property(const QString& _name) const
{
   foreach(ComponentProperty* p, m_properties) {
      if(p->name() == _name)
         return p;
   }
   return 0;
}
