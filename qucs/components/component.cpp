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
#include <QtGui/QUndoStack>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtCore/QDebug>
#include <QtGui/QPainter>

QMap<int,QPen> Component::pens;

ComponentPort::ComponentPort(Component* owner,const QPointF& pos) : m_owner(owner), m_centrePos(pos)
{
   SchematicScene *s = owner->schematicScene();
   if(s)
   {
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
   foreach(ComponentPort *port, m_ports)
   {
      if( z == 0) continue;
      s += "R:" + name + "." + QString::number(z++) + ' ' +
         Node1 + ' ' + port->node()->name() + " R=\"0\"\n";
   }
   return s;
}


QString Component::saveString() const
{
   QString s = "<" + model;

   if(name.isEmpty()) s += " * ";
   else s += " " + name + " ";

   int i=0;
   if(!showName)
      i = 4;
   i |= activeStatus;
   s += QString::number(i);
   s += " " + QString::number(pos().x()) + " " + QString::number(pos().y());
//    s += " "+QString::number(tx)+" "+QString::number(ty);
//    if(mirroredX) s += " 1";
//    else s += " 0";
//    s += " "+QString::number(rotated);

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

void Component::addProperty(QString _name,QString _initVal,QString _des,bool isVisible,const QStringList& options)
{
   ComponentProperty *prop = new ComponentProperty(this,_name,_initVal,_des,isVisible,options);
   if(m_propertyGroup)
      m_propertyGroup->addChild(prop);
   QPointF p = pos();
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
   for(; it != end; ++it)
   {
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
//   int gs = 10;//schematicScene()->gridSize();
//   int x = hpos.x()), y = hpos.y();

//      //TODO: Implent grid based movement for components
//    if(0 && x%gs)
//    {
//       if(x%gs > gs/2)
//          x -= (x%gs) + gs;
//       else
//          x -= (x%gs);
//    }
//    if(0 && y%gs)
//    {
//       if(y%gs > gs/2)
//          y -= (y%gs) + gs;
//       else
//          y -= (y%gs);
//    }

   qreal dx = hpos.x() - oldPos.x();
   qreal dy = hpos.y() - oldPos.y();

//    QList<ComponentProperty*>::iterator it = m_properties.begin();
//    const QList<ComponentProperty*>::iterator end = m_properties.end();
//    for(; it != end; ++it)
//       (*it)->moveBy(dx,dy);

   if(m_propertyGroup)
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
   return QVariant(hpos);//QPointF(x,y));
}

QVariant Component::itemChange(GraphicsItemChange change,const QVariant& value)
{
   Q_ASSERT(scene());

   if (change == ItemPositionChange)
      return handlePositionChange(value.toPointF());

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
   return QGraphicsItem::itemChange(change,value);
}

void Component::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
   if(event->buttons() & Qt::RightButton)
   {
      QucsItem::rotate(-45);
      foreach(QGraphicsItem *item, scene()->selectedItems())
      {
         if(item != this)
            item->setSelected(false);
         else
            setSelected(true);
      }
   }
   else
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
   else if(comp == "ResistorUS")
   {
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
