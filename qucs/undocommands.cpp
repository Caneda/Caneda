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

#include "undocommands.h"

#include "component.h"
#include "port.h"
#include "schematicscene.h"

#include "paintings/paintings.h"
#include "paintings/graphictext.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>

// Remove this from production code soon.
QDebug _debug()
{
    static QString *_str = new QString;
    return QDebug(_str);
}

/*
##########################################################################
#                            PropertyChangeCmd                           #
##########################################################################
*/

static QString debugString;

PropertyChangeCmd::PropertyChangeCmd(const QString& propertyName,
        const QVariant& newValue,
        const QVariant& oldValue,
        Component *const component,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_property(propertyName), m_newValue(newValue), m_oldValue(oldValue),
    m_component(component)
{
}

void PropertyChangeCmd::undo()
{
    m_component->setProperty(m_property, m_oldValue);
    _debug() << "PropertyChangeCmd::undo()\n";
}

void PropertyChangeCmd::redo()
{
    m_component->setProperty(m_property, m_newValue);
    _debug() << "PropertyChangeCmd::redo()\n";
}

/*
##########################################################################
#                         GridPropertyChangeCmd                          #
##########################################################################
*/

ScenePropertyChangeCmd::ScenePropertyChangeCmd(const QString& propertyName,
        const QVariant& newValue,
        const QVariant& oldValue,
        SchematicScene *const schematic,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_property(propertyName), m_newValue(newValue), m_oldValue(oldValue),
    m_schematic(schematic)
{
    setText(QString("Changed ") + propertyName);
}

void ScenePropertyChangeCmd::undo()
{
    m_schematic->setProperty(m_property, m_oldValue);
    _debug() << "ScenePropertyChangeCmd::undo()\n";
}

void ScenePropertyChangeCmd::redo()
{
    m_schematic->setProperty(m_property, m_newValue);
    _debug() << "ScenePropertyChangeCmd::redo()\n";
}

/*
##########################################################################
#                                 MoveCmd                                #
##########################################################################
*/

MoveCmd::MoveCmd(QGraphicsItem *i,const QPointF& init,const QPointF& end,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_item(i), m_initialPos(init), m_finalPos(end)
{
}

void MoveCmd::undo()
{
    if(m_item->parentItem()) {
        QPointF p = m_item->mapFromScene(m_initialPos);
        p = m_item->mapToParent(p);
        m_item->setPos(p);
    }
    else {
        m_item->setPos(m_initialPos);
    }
    _debug() << "MoveCmd::undo()\n";

}

void MoveCmd::redo()
{
    if(m_item->parentItem()) {
        QPointF p = m_item->mapFromScene(m_finalPos);
        p = m_item->mapToParent(p);
        m_item->setPos(p);
    }
    else {
        m_item->setPos(m_finalPos);
    }
    _debug() << "MoveCmd::redo()\n";
}

/*
##########################################################################
#                            ConnectCmd                                  #
##########################################################################
*/

struct WireConnectionInfo
{
    WireConnectionInfo(Wire *w, Port *p1 = 0, Port *p2 = 0) :
        wire(w), port1(p1), port2(p2)
    {
    }

    ~WireConnectionInfo()
    {
        if(!wire->scene()) {
            _debug() << "WireConnectionInfo::destructor Deleted sceneless wire";
            delete wire;
        }
    }

    Wire *const wire;
    Port *port1;
    Port *port2;
    QPointF wirePos;
};

ConnectCmd::ConnectCmd(Port *p1, Port *p2, const QList<Wire*> &wires,
        SchematicScene *scene, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_port1(p1), m_port2(p2), m_scene(scene)
{
    foreach(Wire *w, wires) {
        m_wireConnectionInfos << new WireConnectionInfo(w);
    }
}

ConnectCmd::~ConnectCmd()
{
    qDeleteAll(m_wireConnectionInfos);
}

void ConnectCmd::undo()
{
    QString port1Name = m_port1->owner()->component() ?
        m_port1->owner()->component()->name() : "Wire";
    QString port2Name = m_port2->owner()->component() ?
        m_port2->owner()->component()->name() : "Wire";
    _debug() << "ConnectCmd::undo() : Disconencting "
             << port1Name <<  " from " << port2Name << '\n';

    m_port1->disconnectFrom(m_port2);

    foreach(WireConnectionInfo *info, m_wireConnectionInfos) {
        m_scene->addItem(info->wire);
        info->wire->setPos(info->wirePos);

        info->wire->port1()->connectTo(info->port1);
        info->wire->port2()->connectTo(info->port2);
    }
}

void ConnectCmd::redo()
{
    QString port1Name = m_port1->owner()->component() ?
        m_port1->owner()->component()->name() : "Wire";
    QString port2Name = m_port2->owner()->component() ?
        m_port2->owner()->component()->name() : "Wire";

    _debug() << "ConnectCmd::redo() : Conencting "
             << port1Name <<  " to " << port2Name << '\n';

    foreach(WireConnectionInfo *info, m_wireConnectionInfos) {
        info->port1 = info->wire->port1()->getAnyConnectedPort();
        info->port2 = info->wire->port2()->getAnyConnectedPort();

        Q_ASSERT(info->port1);
        Q_ASSERT(info->port2);

        info->wire->port1()->removeConnections();
        info->wire->port2()->removeConnections();

        info->wirePos = info->wire->pos();
        m_scene->removeItem(info->wire);
    }

    m_port1->connectTo(m_port2);
}

/*
##########################################################################
#                           DisconnectCmd                                #
##########################################################################
*/

DisconnectCmd::DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_port1(p1), m_port2(p2)
{
}

void DisconnectCmd::undo()
{
    QString port1Name = m_port1->owner()->component() ?
        m_port1->owner()->component()->name() : "Wire";
    QString port2Name = m_port2->owner()->component() ?
        m_port2->owner()->component()->name() : "Wire";

    _debug() << "DisconnectCmd::undo() : Conencting "
             << port1Name <<  " to " << port2Name << '\n';
    m_port1->connectTo(m_port2);
}

void DisconnectCmd::redo()
{
    QString port1Name = m_port1->owner()->component() ?
        m_port1->owner()->component()->name() : "Wire";
    QString port2Name = m_port2->owner()->component() ?
        m_port2->owner()->component()->name() : "Wire";

    _debug() << "DisconnectCmd::undo() : Disconencting "
             << port1Name <<  " from " << port2Name << '\n';
    m_port1->disconnectFrom(m_port2);
}

/*
##########################################################################
#                                AddWireCmd                              #
##########################################################################
*/

AddWireCmd::AddWireCmd(Wire *wire, SchematicScene *scene, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_wire(wire),
    m_scene(scene),
    m_pos(m_wire->pos())
{
}

AddWireCmd::~AddWireCmd()
{
    if(!m_wire->scene()) {
        delete m_wire;
    }
}

void AddWireCmd::undo()
{
    m_scene->removeItem(m_wire);
}

void AddWireCmd::redo()
{
    m_scene->addItem(m_wire);
}

/*
##########################################################################
#                         AddWireBetweenPortsCmd                         #
##########################################################################
*/

AddWireBetweenPortsCmd::AddWireBetweenPortsCmd(Port *p1, Port *p2, QUndoCommand *parent) :
    QUndoCommand(parent), m_port1(p1), m_port2(p2)
{
    m_scene = m_port1->schematicScene();
    m_wire = new Wire(m_port1, m_port2, m_scene);
    m_pos = m_wire->scenePos();
}

void AddWireBetweenPortsCmd::undo()
{
    QString port1Name = m_port1->owner()->component() ?
        m_port1->owner()->component()->name() : "Wire";
    QString port2Name = m_port2->owner()->component() ?
        m_port2->owner()->component()->name() : "Wire";

    _debug() << Q_FUNC_INFO << "Removing wire between " << port1Name
             <<  "and"  << port2Name << '\n';

    m_wire->port1()->disconnectFrom(m_port1);
    m_wire->port2()->disconnectFrom(m_port2);
    m_scene->removeItem(m_wire);
}

void AddWireBetweenPortsCmd::redo()
{
    QString port1Name = m_port1->owner()->component() ?
        m_port1->owner()->component()->name() : "Wire";
    QString port2Name = m_port2->owner()->component() ?
        m_port2->owner()->component()->name() : "Wire";

    _debug() << Q_FUNC_INFO << "Adding wire between "
        << port1Name <<  "and"  << port2Name << '\n';

    m_scene->addItem(m_wire);
    m_wire->setPos(m_pos);
    m_wire->port1()->connectTo(m_port1);
    m_wire->port2()->connectTo(m_port2);

    Q_ASSERT(m_wire->port1()->connections() == m_port1->connections());
    Q_ASSERT(m_wire->port2()->connections() == m_port2->connections());
}

/*
##########################################################################
#                           WireStateChangeCmd                           #
##########################################################################
*/

WireStateChangeCmd::WireStateChangeCmd(Wire *wire, WireData initState,
        WireData finalState, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_wire(wire), m_initState(initState), m_finalState(finalState)
{
}

void WireStateChangeCmd::undo()
{
    _debug() << "WireStateChangeCmd::undo() : Setting intial state"
             << "\nWirelines is " << m_initState.wLines
             << "\nPort1 pos = " << m_initState.port1Pos
             << "\nPort2 pos = " << m_initState.port2Pos
             <<"\n\n";
    m_wire->setState(m_initState);
    m_wire->update();
}

void WireStateChangeCmd::redo()
{
    _debug() << "WireStateChangeCmd::redo() : Setting final state"
             << "\nWirelines is " << m_finalState.wLines
             << "\nPort1 pos = " << m_finalState.port1Pos
             << "\nPort2 pos = " << m_finalState.port2Pos
             << "\n\n";
    m_wire->setState(m_finalState);
    m_wire->update();
}

/*
##########################################################################
#                           InsertItemCmd                                #
##########################################################################
*/

InsertItemCmd::InsertItemCmd(QGraphicsItem *const item, SchematicScene *scene,
        QPointF pos, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_item(item), m_scene(scene)
{
    Q_ASSERT(scene);
    if(pos == InvalidPoint) {
        m_pos = m_item->scenePos();
    }
    else {
        m_pos = pos;
    }
}

InsertItemCmd::~InsertItemCmd()
{
    if(!m_item->scene()) {
        _debug() << "InsertItemCmd destructor: Destructing item";
        delete m_item;
    }
}

void InsertItemCmd::undo()
{
    _debug() << "InsertItemCmd::undo()\n";
    m_scene->removeItem(m_item);
}

void InsertItemCmd::redo()
{
    _debug() << "InsertItemCmd::redo()\n";
    if(m_scene != m_item->scene()) {
        m_scene->addItem(m_item);
    }
    m_item->setPos(m_pos);
    Component *comp = qucsitem_cast<Component*>(m_item);
    if(comp) {
        comp->updatePropertyGroup();
    }
}

/*
##########################################################################
#                           RemoveItemsCmd                               #
##########################################################################
*/

RemoveItemsCmd::RemoveItemsCmd(const QList<QucsItem*> &items, SchematicScene *scene,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_scene(scene)
{
    foreach(QucsItem *item, items) {
        m_itemPointPairs << ItemPointPair(item, item->pos());
    }
}

RemoveItemsCmd::~RemoveItemsCmd()
{
    foreach(ItemPointPair p, m_itemPointPairs) {
        if(p.first->scene() != m_scene) {
            delete p.first;
        }
    }
}

void RemoveItemsCmd::undo()
{
    foreach(ItemPointPair p, m_itemPointPairs) {
        m_scene->addItem(p.first);
        p.first->setPos(p.second);
    }
}

void RemoveItemsCmd::redo()
{
    foreach(ItemPointPair p, m_itemPointPairs) {
        m_scene->removeItem(p.first);
    }
}


/*
##########################################################################
#                          RotateItemsCmd                                #
##########################################################################
*/

RotateItemsCmd::RotateItemsCmd(QList<QucsItem*> items, const Qucs::AngleDirection dir, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_items(items)
{
    m_angleDirection = dir;
}

RotateItemsCmd::RotateItemsCmd(QucsItem *item, const Qucs::AngleDirection dir, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    m_items << item;
    m_angleDirection = dir;
}

void RotateItemsCmd::undo()
{
    foreach(QucsItem *item, m_items) {
        item->rotate90(m_angleDirection == Qucs::Clockwise ? Qucs::AntiClockwise : Qucs::Clockwise);
    }
}

void RotateItemsCmd::redo()
{
    foreach(QucsItem *item, m_items) {
        item->rotate90(Qucs::AntiClockwise);
    }
}

/*
##########################################################################
#                          MirrorItemsCmd                                #
##########################################################################
*/

MirrorItemsCmd::MirrorItemsCmd(QList<QucsItem*> items, const Qt::Axis axis, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_items(items),
    m_axis(axis)
{
}

MirrorItemsCmd::MirrorItemsCmd(QucsItem *item, const Qt::Axis axis, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_axis(axis)
{
    m_items << item;
}

void MirrorItemsCmd::undo()
{
    foreach(QucsItem *item, m_items) {
        item->mirrorAlong(m_axis);
    }
}

void MirrorItemsCmd::redo()
{
    foreach(QucsItem *item, m_items) {
        item->mirrorAlong(m_axis);
    }
}


/*
##########################################################################
#                        ToggleActiveStatusCmd                           #
##########################################################################
*/


ToggleActiveStatusCmd::ToggleActiveStatusCmd(const QList<Component*> &components,
        QUndoCommand *parent) :
    QUndoCommand(parent)
{
    foreach(Component *component, components) {
        m_componentStatusPairs << qMakePair(component, component->activeStatus());
    }
}

void ToggleActiveStatusCmd::undo()
{
    foreach(ComponentStatusPair p, m_componentStatusPairs) {
        p.first->setActiveStatus(p.second);
    }
}

void ToggleActiveStatusCmd::redo()
{
    foreach(ComponentStatusPair p, m_componentStatusPairs) {
        p.first->toggleActiveStatus();
    }
}


/*
##########################################################################
#                        PaintingRectChangeCmd                           #
##########################################################################
*/


PaintingRectChangeCmd::PaintingRectChangeCmd(Painting *painting, QRectF oldRect,
        QRectF newRect,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_painting(painting),
    m_oldRect(oldRect),
    m_newRect(newRect)
{
}

void PaintingRectChangeCmd::undo()
{
    m_painting->setPaintingRect(m_oldRect);
}

void PaintingRectChangeCmd::redo()
{
    m_painting->setPaintingRect(m_newRect);
}

/*
##########################################################################
#                       PaintingPropertyChangeCmd                        #
##########################################################################
*/

PaintingPropertyChangeCmd::PaintingPropertyChangeCmd(Painting *painting, QString oldText,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_painting(painting),
    m_oldPropertyText(oldText),
    m_newPropertyText(m_painting->saveDataText())
{
    qDebug(m_newPropertyText.toAscii().constData());
}

void PaintingPropertyChangeCmd::undo()
{
    m_painting->loadDataFromText(m_oldPropertyText);
}

void PaintingPropertyChangeCmd::redo()
{
    m_painting->loadDataFromText(m_newPropertyText);
}

/*
##########################################################################
#                         GraphicTextChangeCmd                           #
##########################################################################
*/

GraphicTextChangeCmd::GraphicTextChangeCmd(GraphicText *text, QString oldText,
        QString newText,
        QUndoCommand *parent) :
    QUndoCommand(parent),
    m_graphicText(text),
    m_oldText(oldText),
    m_newText(newText)
{
}

void GraphicTextChangeCmd::undo()
{
    m_graphicText->setRichText(m_oldText);
}

void GraphicTextChangeCmd::redo()
{
    m_graphicText->setRichText(m_newText);
}

/*
##########################################################################
#                            PropertyMapCmd                              #
##########################################################################
*/

PropertyMapCmd::PropertyMapCmd(Component *comp, const PropertyMap& old,
        const PropertyMap& newMap, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_component(comp),
    m_oldMap(old),
    m_newMap(newMap)
{
}

void PropertyMapCmd::undo()
{
    m_component->setPropertyMap(m_oldMap);
}

void PropertyMapCmd::redo()
{
    m_component->setPropertyMap(m_newMap);
}
