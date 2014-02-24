/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2012 by Pablo Daniel Pareja Obregon                  *
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
#include "component.h"
#include "port.h"
#include "xmlutilities.h"

#include "paintings/graphictext.h"

#include <QDebug>

namespace Caneda
{
    /*************************************************************************
     *                                MoveCmd                                *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                            ConnectCmd                                 *
     *************************************************************************/
    //! \brief Constructor.
    ConnectCmd::ConnectCmd(Port *p1, Port *p2, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_port1(p1), m_port2(p2)
    {
    }

    void ConnectCmd::undo()
    {
        m_port1->disconnect();
    }

    void ConnectCmd::redo()
    {
        m_port1->connectTo(m_port2);
    }

    /*************************************************************************
     *                           DisconnectCmd                               *
     *************************************************************************/
    //! \brief Constructor.
    DisconnectCmd::DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_port1(p1), m_port2(p2)
    {
    }

    void DisconnectCmd::undo()
    {
        m_port1->connectTo(m_port2);
    }

    void DisconnectCmd::redo()
    {
        m_port1->disconnect();
    }

    /*************************************************************************
     *                                AddWireCmd                             *
     *************************************************************************/
    //! \brief Constructor.
    AddWireCmd::AddWireCmd(Wire *wire, CGraphicsScene *scene, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_wire(wire),
        m_scene(scene),
        m_pos(m_wire->pos())
    {
    }

    //! \brief Destructor.
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

    /*************************************************************************
     *                           WireStateChangeCmd                          *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                           InsertItemCmd                               *
     *************************************************************************/
    //! \brief Constructor.
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

    //! \brief Destructor.
    InsertItemCmd::~InsertItemCmd()
    {
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
    }

    /*************************************************************************
     *                           RemoveItemsCmd                              *
     *************************************************************************/
    //! \brief Constructor.
    RemoveItemsCmd::RemoveItemsCmd(const QList<CGraphicsItem*> &items, CGraphicsScene *scene,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_scene(scene)
    {
        foreach(CGraphicsItem *item, items) {
            m_itemPointPairs << ItemPointPair(item, item->pos());
        }
    }

    //! \brief Destructor.
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


    /*************************************************************************
     *                          RotateItemsCmd                               *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                          MirrorItemsCmd                               *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                        PaintingRectChangeCmd                          *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                       PaintingPropertyChangeCmd                       *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                         GraphicTextChangeCmd                          *
     *************************************************************************/
    //! \brief Constructor.
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

    /*************************************************************************
     *                            PropertyMapCmd                             *
     *************************************************************************/
    //! \brief Constructor.
    PropertyMapCmd::PropertyMapCmd(PropertyGroup *propGroup, const PropertyMap& old,
            const PropertyMap& newMap, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_propertyGroup(propGroup),
        m_oldMap(old),
        m_newMap(newMap)
    {
    }

    void PropertyMapCmd::undo()
    {
        m_propertyGroup->setPropertyMap(m_oldMap);
    }

    void PropertyMapCmd::redo()
    {
        m_propertyGroup->setPropertyMap(m_newMap);
    }

} // namespace Caneda
