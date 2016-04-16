/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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
#include "graphictext.h"
#include "port.h"
#include "wire.h"

namespace Caneda
{
    /*************************************************************************
     *                                MoveCmd                                *
     *************************************************************************/
    //! \brief Constructor.
    MoveCmd::MoveCmd(CGraphicsItem *item, const QPointF &init, const QPointF &end,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_item(item), m_initialPos(init), m_finalPos(end)
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

    void AddWireCmd::undo()
    {
        m_scene->removeItem(m_wire);
    }

    void AddWireCmd::redo()
    {
        m_scene->addItem(m_wire);
    }

    /*************************************************************************
     *                           InsertItemCmd                               *
     *************************************************************************/
    //! \brief Constructor.
    InsertItemCmd::InsertItemCmd(CGraphicsItem *const item, CGraphicsScene *scene,
            QPointF pos, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_item(item), m_scene(scene), m_pos(pos)
    {
    }

    void InsertItemCmd::undo()
    {
        m_scene->disconnectItems(m_item);
        m_scene->removeItem(m_item);
    }

    void InsertItemCmd::redo()
    {
        m_scene->addItem(m_item);
        m_item->setPos(m_pos);
        m_scene->connectItems(m_item);
        m_scene->splitAndCreateNodes(m_item);
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

    void RemoveItemsCmd::undo()
    {
        foreach(ItemPointPair p, m_itemPointPairs) {
            m_scene->addItem(p.first);
            p.first->setPos(p.second);
            m_scene->connectItems(p.first);
        }
    }

    void RemoveItemsCmd::redo()
    {
        foreach(ItemPointPair p, m_itemPointPairs) {
            m_scene->disconnectItems(p.first);
            m_scene->removeItem(p.first);
        }
    }


    /*************************************************************************
     *                          RotateItemsCmd                               *
     *************************************************************************/
    //! \brief Constructor.
    RotateItemsCmd::RotateItemsCmd(const QList<CGraphicsItem*> &items, const Caneda::AngleDirection dir,
                                   CGraphicsScene *scene, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_items(items),
        m_dir(dir),
        m_scene(scene)
    {
    }

    void RotateItemsCmd::undo()
    {
        // Disconnect
        m_scene->disconnectItems(m_items);

        // Rotate
        QPointF rotationCenter = m_scene->centerOfItems(m_items);

        foreach(CGraphicsItem *item, m_items) {
            item->rotate(m_dir == Caneda::Clockwise ? Caneda::AntiClockwise : Caneda::Clockwise, rotationCenter);
        }

        // Reconnect
        m_scene->connectItems(m_items);
    }

    void RotateItemsCmd::redo()
    {
        // Disconnect
        m_scene->disconnectItems(m_items);

        // Rotate
        QPointF rotationCenter = m_scene->centerOfItems(m_items);

        foreach(CGraphicsItem *item, m_items) {
            item->rotate(m_dir, rotationCenter);
        }

        // Reconnect
        m_scene->connectItems(m_items);
    }

    /*************************************************************************
     *                          MirrorItemsCmd                               *
     *************************************************************************/
    //! \brief Constructor.
    MirrorItemsCmd::MirrorItemsCmd(QList<CGraphicsItem*> items, const Qt::Axis axis,
                                   CGraphicsScene *scene, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_items(items),
        m_axis(axis),
        m_scene(scene)
    {
    }

    void MirrorItemsCmd::undo()
    {
        redo();
    }

    void MirrorItemsCmd::redo()
    {
        // Disconnect item before mirroring
        m_scene->disconnectItems(m_items);

        // Mirror
        QPointF mirrorCenter = m_scene->centerOfItems(m_items);

        foreach(CGraphicsItem *item, m_items) {
            item->mirror(m_axis, mirrorCenter);
        }

        // Reconnect
        m_scene->connectItems(m_items);
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
        qDebug() << m_newPropertyText.toLatin1().constData();
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
