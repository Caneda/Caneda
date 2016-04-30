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

#include "graphicsscene.h"
#include "graphictext.h"
#include "port.h"
#include "wire.h"

namespace Caneda
{
    /*************************************************************************
     *                                MoveCmd                                *
     *************************************************************************/
    /*!
     * \brief Constructs a QUndoCommand object with parent parent.
     *
     * In the constructor of the QUndoCommand classes, the basic information
     * necessary to perform the undo/redo actions must be stored. In particular
     * that information is stored in private members and only available to this
     * class. All undo/redo actions must be performed by calling the
     * undo() or redo() methods.
     *
     * sa undo(), redo()
     */
    MoveCmd::MoveCmd(GraphicsItem *item, const QPointF &init, const QPointF &end,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_item(item), m_initialPos(init), m_finalPos(end)
    {
    }

    /*!
     * \brief Reverts a change to a document.
     *
     * After undo() is called, the state of the document should be the same as
     * before redo() was called. This function must be implemented in each
     * QUndoCommand derived class. Calling QUndoStack::undo() or
     * QUndoStack::redo() from the QUndoStack in a scene will invoke the
     * methods defined here which must define the sequence of steps to
     * redo/undo the corresponding actions.
     *
     * \sa redo()
     */
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

    /*!
     * \brief Applies a change to a document.
     *
     * After redo() is called, the state of the document should reflect the
     * invoked action. This function must be implemented in each
     * QUndoCommand derived class. Calling QUndoStack::undo() or
     * QUndoStack::redo() from the QUndoStack in a scene will invoke the
     * methods defined here which must define the sequence of steps to
     * redo/undo the corresponding actions.
     *
     * \sa undo()
     */
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
    //! \copydoc MoveCmd::MoveCmd()
    DisconnectCmd::DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_port1(p1), m_port2(p2)
    {
    }

    //! \copydoc MoveCmd::undo()
    void DisconnectCmd::undo()
    {
        m_port1->connectTo(m_port2);
    }

    //! \copydoc MoveCmd::redo()
    void DisconnectCmd::redo()
    {
        m_port1->disconnect();
    }


    /*************************************************************************
     *                                AddWireCmd                             *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    AddWireCmd::AddWireCmd(Wire *wire, GraphicsScene *scene, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_wire(wire),
        m_scene(scene)
    {
    }

    //! \copydoc MoveCmd::undo()
    void AddWireCmd::undo()
    {
        m_scene->removeItem(m_wire);
    }

    //! \copydoc MoveCmd::redo()
    void AddWireCmd::redo()
    {
        m_scene->addItem(m_wire);
    }


    /*************************************************************************
     *                           InsertItemCmd                               *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    InsertItemCmd::InsertItemCmd(GraphicsItem *const item, GraphicsScene *scene,
            QPointF pos, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_item(item),
        m_scene(scene),
        m_pos(pos)
    {
    }

    //! \copydoc MoveCmd::undo()
    void InsertItemCmd::undo()
    {
        m_scene->disconnectItems(m_item);
        m_scene->removeItem(m_item);
    }

    //! \copydoc MoveCmd::redo()
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
    //! \copydoc MoveCmd::MoveCmd()
    RemoveItemsCmd::RemoveItemsCmd(const QList<GraphicsItem*> &items, GraphicsScene *scene,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_scene(scene)
    {
        foreach(GraphicsItem *item, items) {
            m_itemPointPairs << ItemPointPair(item, item->pos());
        }
    }

    //! \copydoc MoveCmd::undo()
    void RemoveItemsCmd::undo()
    {
        foreach(ItemPointPair p, m_itemPointPairs) {
            m_scene->addItem(p.first);
            p.first->setPos(p.second);
            m_scene->connectItems(p.first);
        }
    }

    //! \copydoc MoveCmd::redo()
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
    //! \copydoc MoveCmd::MoveCmd()
    RotateItemsCmd::RotateItemsCmd(const QList<GraphicsItem*> &items, const Caneda::AngleDirection dir,
                                   GraphicsScene *scene, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_items(items),
        m_dir(dir),
        m_scene(scene)
    {
    }

    //! \copydoc MoveCmd::undo()
    void RotateItemsCmd::undo()
    {
        // Disconnect
        m_scene->disconnectItems(m_items);

        // Rotate
        QPointF rotationCenter = m_scene->centerOfItems(m_items);

        foreach(GraphicsItem *item, m_items) {
            item->rotate(m_dir == Caneda::Clockwise ? Caneda::AntiClockwise : Caneda::Clockwise, rotationCenter);
        }

        // Reconnect
        m_scene->connectItems(m_items);
    }

    //! \copydoc MoveCmd::redo()
    void RotateItemsCmd::redo()
    {
        // Disconnect
        m_scene->disconnectItems(m_items);

        // Rotate
        QPointF rotationCenter = m_scene->centerOfItems(m_items);

        foreach(GraphicsItem *item, m_items) {
            item->rotate(m_dir, rotationCenter);
        }

        // Reconnect
        m_scene->connectItems(m_items);
    }


    /*************************************************************************
     *                          MirrorItemsCmd                               *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    MirrorItemsCmd::MirrorItemsCmd(QList<GraphicsItem*> items, const Qt::Axis axis,
                                   GraphicsScene *scene, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_items(items),
        m_axis(axis),
        m_scene(scene)
    {
    }

    //! \copydoc MoveCmd::undo()
    void MirrorItemsCmd::undo()
    {
        redo();
    }

    //! \copydoc MoveCmd::redo()
    void MirrorItemsCmd::redo()
    {
        // Disconnect item before mirroring
        m_scene->disconnectItems(m_items);

        // Mirror
        QPointF mirrorCenter = m_scene->centerOfItems(m_items);

        foreach(GraphicsItem *item, m_items) {
            item->mirror(m_axis, mirrorCenter);
        }

        // Reconnect
        m_scene->connectItems(m_items);
    }


    /*************************************************************************
     *                        PaintingRectChangeCmd                          *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    PaintingRectChangeCmd::PaintingRectChangeCmd(Painting *painting, QRectF oldRect,
            QRectF newRect,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_painting(painting),
        m_oldRect(oldRect),
        m_newRect(newRect)
    {
    }

    //! \copydoc MoveCmd::undo()
    void PaintingRectChangeCmd::undo()
    {
        m_painting->setPaintingRect(m_oldRect);
    }

    //! \copydoc MoveCmd::redo()
    void PaintingRectChangeCmd::redo()
    {
        m_painting->setPaintingRect(m_newRect);
    }


    /*************************************************************************
     *                       PaintingPropertyChangeCmd                       *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    PaintingPropertyChangeCmd::PaintingPropertyChangeCmd(Painting *painting, QString oldText,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_painting(painting),
        m_oldPropertyText(oldText),
        m_newPropertyText(m_painting->saveDataText())
    {
        qDebug() << m_newPropertyText.toLatin1().constData();
    }

    //! \copydoc MoveCmd::undo()
    void PaintingPropertyChangeCmd::undo()
    {
        m_painting->loadDataFromText(m_oldPropertyText);
    }

    //! \copydoc MoveCmd::redo()
    void PaintingPropertyChangeCmd::redo()
    {
        m_painting->loadDataFromText(m_newPropertyText);
    }


    /*************************************************************************
     *                         GraphicTextChangeCmd                          *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    GraphicTextChangeCmd::GraphicTextChangeCmd(GraphicText *text, QString oldText,
            QString newText,
            QUndoCommand *parent) :
        QUndoCommand(parent),
        m_graphicText(text),
        m_oldText(oldText),
        m_newText(newText)
    {
    }

    //! \copydoc MoveCmd::undo()
    void GraphicTextChangeCmd::undo()
    {
        m_graphicText->setRichText(m_oldText);
    }

    //! \copydoc MoveCmd::redo()
    void GraphicTextChangeCmd::redo()
    {
        m_graphicText->setRichText(m_newText);
    }


    /*************************************************************************
     *                            PropertyMapCmd                             *
     *************************************************************************/
    //! \copydoc MoveCmd::MoveCmd()
    PropertyMapCmd::PropertyMapCmd(PropertyGroup *propGroup, const PropertyMap& old,
            const PropertyMap& newMap, QUndoCommand *parent) :
        QUndoCommand(parent),
        m_propertyGroup(propGroup),
        m_oldMap(old),
        m_newMap(newMap)
    {
    }

    //! \copydoc MoveCmd::undo()
    void PropertyMapCmd::undo()
    {
        m_propertyGroup->setPropertyMap(m_oldMap);
    }

    //! \copydoc MoveCmd::redo()
    void PropertyMapCmd::redo()
    {
        m_propertyGroup->setPropertyMap(m_newMap);
    }

} // namespace Caneda
