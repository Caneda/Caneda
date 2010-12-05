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

#include "cgraphicsscene.h"
#include "port.h"

#include "paintings/graphictext.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>

namespace Caneda
{
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
    }

    void PropertyChangeCmd::redo()
    {
        m_component->setProperty(m_property, m_newValue);
    }

    /*
    ##########################################################################
    #                         ScenePropertyChangeCmd                         #
    ##########################################################################
    */

    ScenePropertyChangeCmd::ScenePropertyChangeCmd(const QString& propertyName,
            const QVariant& newValue,
            const QVariant& oldValue,
            CGraphicsScene *const scene,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_property(propertyName), m_newValue(newValue), m_oldValue(oldValue),
        m_scene(scene)
    {
        setText(QString("Changed ") + propertyName);
    }

    void ScenePropertyChangeCmd::undo()
    {
        m_scene->setProperty(m_property, m_oldValue);
    }

    void ScenePropertyChangeCmd::redo()
    {
        m_scene->setProperty(m_property, m_newValue);
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
                delete wire;
            }
        }

        Wire *const wire;
        Port *port1;
        Port *port2;
        QPointF wirePos;
    };

    ConnectCmd::ConnectCmd(Port *p1, Port *p2, const QList<Wire*> &wires,
            CGraphicsScene *scene, QUndoCommand *parent) :
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

        m_port1->connectTo(m_port2);
    }

    void DisconnectCmd::redo()
    {
        QString port1Name = m_port1->owner()->component() ?
            m_port1->owner()->component()->name() : "Wire";
        QString port2Name = m_port2->owner()->component() ?
            m_port2->owner()->component()->name() : "Wire";

        m_port1->disconnectFrom(m_port2);
    }

    /*
    ##########################################################################
    #                                AddWireCmd                              #
    ##########################################################################
    */

    AddWireCmd::AddWireCmd(Wire *wire, CGraphicsScene *scene, QUndoCommand *parent) :
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
        m_scene = m_port1->cGraphicsScene();
        m_wire = new Wire(m_port1->pos(), m_port2->pos(), m_scene);
        m_pos = m_wire->scenePos();
    }

    void AddWireBetweenPortsCmd::undo()
    {
        QString port1Name = m_port1->owner()->component() ?
            m_port1->owner()->component()->name() : "Wire";
        QString port2Name = m_port2->owner()->component() ?
            m_port2->owner()->component()->name() : "Wire";


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
        m_wire->setState(m_initState);
        m_wire->update();
    }

    void WireStateChangeCmd::redo()
    {
        m_wire->setState(m_finalState);
        m_wire->update();
    }

    /*
    ##########################################################################
    #                           InsertItemCmd                                #
    ##########################################################################
    */

    InsertItemCmd::InsertItemCmd(QGraphicsItem *const item, CGraphicsScene *scene,
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
            delete m_item;
        }
    }

    void InsertItemCmd::undo()
    {
        m_scene->removeItem(m_item);
    }

    void InsertItemCmd::redo()
    {
        if(m_scene != m_item->scene()) {
            m_scene->addItem(m_item);
        }
        m_item->setPos(m_pos);
        Component *comp = canedaitem_cast<Component*>(m_item);
        if(comp) {
            comp->updatePropertyGroup();
        }
    }

    /*
    ##########################################################################
    #                           RemoveItemsCmd                               #
    ##########################################################################
    */

    RemoveItemsCmd::RemoveItemsCmd(const QList<CGraphicsItem*> &items, CGraphicsScene *scene,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_scene(scene)
    {
        foreach(CGraphicsItem *item, items) {
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

    RotateItemsCmd::RotateItemsCmd(QList<CGraphicsItem*> items, const Caneda::AngleDirection dir, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_items(items)
    {
        m_angleDirection = dir;
    }

    RotateItemsCmd::RotateItemsCmd(CGraphicsItem *item, const Caneda::AngleDirection dir, QUndoCommand *parent) :
        QUndoCommand(parent)
    {
        m_items << item;
        m_angleDirection = dir;
    }

    void RotateItemsCmd::undo()
    {
        foreach(CGraphicsItem *item, m_items) {
            item->rotate90(m_angleDirection == Caneda::Clockwise ? Caneda::AntiClockwise : Caneda::Clockwise);
        }
    }

    void RotateItemsCmd::redo()
    {
        foreach(CGraphicsItem *item, m_items) {
            item->rotate90(Caneda::AntiClockwise);
        }
    }

    /*
    ##########################################################################
    #                          MirrorItemsCmd                                #
    ##########################################################################
    */

    MirrorItemsCmd::MirrorItemsCmd(QList<CGraphicsItem*> items, const Qt::Axis axis, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_items(items),
        m_axis(axis)
    {
    }

    MirrorItemsCmd::MirrorItemsCmd(CGraphicsItem *item, const Qt::Axis axis, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_axis(axis)
    {
        m_items << item;
    }

    void MirrorItemsCmd::undo()
    {
        foreach(CGraphicsItem *item, m_items) {
            item->mirrorAlong(m_axis);
        }
    }

    void MirrorItemsCmd::redo()
    {
        foreach(CGraphicsItem *item, m_items) {
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

} // namespace Caneda
