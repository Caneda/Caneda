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

#ifndef UNDO_COMMANDS_H
#define UNDO_COMMANDS_H

#include "component.h"
#include "wire.h"

#include <QPair>
#include <QUndoCommand>
#include <QVariant>

namespace Caneda
{
    // Forward declarations.
    class Component;
    class Painting;
    class Port;
    class CGraphicsItem;

    static const QPointF InvalidPoint(-30000, -30000);

    class PropertyChangeCmd : public QUndoCommand
    {
    public:
        PropertyChangeCmd(const QString& propertyName,
                const QVariant& newValue,
                const QVariant& oldValue,
                Component *const component,
                QUndoCommand *parent = 0);

        virtual void undo();
        virtual void redo();

    private:
        const QString m_property;
        const QVariant m_newValue;
        const QVariant m_oldValue;
        Component *const m_component;
    };

    class MoveCmd : public QUndoCommand
    {
    public:
        MoveCmd(QGraphicsItem *i,const QPointF& init, const QPointF& final,
                QUndoCommand *parent = 0);
        void undo();
        void redo();

    private:
        QGraphicsItem * const m_item;
        QPointF m_initialPos;
        QPointF m_finalPos;
    };

    class ConnectCmd : public QUndoCommand
    {
    public:
        ConnectCmd(Port *p1, Port *p2,
                CGraphicsScene *scene, QUndoCommand *parent = 0);
        void undo();
        void redo();

    private:
        Port * const m_port1;
        Port * const m_port2;

        CGraphicsScene *const m_scene;
    };

    class DisconnectCmd : public QUndoCommand
    {
    public:
        DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent = 0);
        void undo();
        void redo();

    private:
        Port * const m_port1;
        Port * const m_port2;
    };

    class AddWireCmd : public QUndoCommand
    {
    public:
        AddWireCmd(Wire *wire, CGraphicsScene *scene, QUndoCommand *parent = 0);
        ~AddWireCmd();

        void undo();
        void redo();

    private:
        Wire *m_wire;
        CGraphicsScene *m_scene;
        QPointF m_pos;
    };

    class WireStateChangeCmd : public QUndoCommand
    {
    public:
        typedef Wire::Data WireData;
        WireStateChangeCmd(Wire *wire,WireData initState, WireData finalState,
                QUndoCommand *parent = 0);

        void undo();
        void redo();

    private:
        Wire *m_wire;
        Wire::Data m_initState, m_finalState;
    };

    class InsertItemCmd : public QUndoCommand
    {
    public:
        InsertItemCmd(QGraphicsItem *const item, CGraphicsScene *scene,
                QPointF pos = InvalidPoint, QUndoCommand *parent = 0);
        ~InsertItemCmd();

        void undo();
        void redo();

    protected:
        QGraphicsItem *const m_item;
        CGraphicsScene *const m_scene;
        QPointF m_pos;
    };

    class RemoveItemsCmd : public QUndoCommand
    {
    public:
        typedef QPair<CGraphicsItem*, QPointF> ItemPointPair;

        RemoveItemsCmd(const QList<CGraphicsItem*> &items, CGraphicsScene *scene,
                QUndoCommand *parent = 0);
        ~RemoveItemsCmd();

        void undo();
        void redo();

    protected:
        QList<ItemPointPair> m_itemPointPairs;
        CGraphicsScene *const m_scene;
    };

    class RotateItemsCmd : public QUndoCommand
    {
    public:
        RotateItemsCmd(QList<CGraphicsItem*> items, const  Caneda::AngleDirection=Caneda::Clockwise,
                QUndoCommand *parent = 0);
        RotateItemsCmd(CGraphicsItem *item, const  Caneda::AngleDirection=Caneda::Clockwise,
                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        QList<CGraphicsItem*> m_items;
        Caneda::AngleDirection m_angleDirection;
    };

    class MirrorItemsCmd : public QUndoCommand
    {
    public:
        MirrorItemsCmd(QList<CGraphicsItem*> items, const Qt::Axis axis, QUndoCommand *parent = 0);
        MirrorItemsCmd(CGraphicsItem *item, const Qt::Axis axis, QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        QList<CGraphicsItem*> m_items;
        Qt::Axis m_axis;
    };

    class ToggleActiveStatusCmd : public QUndoCommand
    {
    public:
        ToggleActiveStatusCmd(const QList<Component*> &components, QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        typedef QPair<Component*, Caneda::ActiveStatus> ComponentStatusPair;
        QList<ComponentStatusPair> m_componentStatusPairs;
    };

    class PaintingRectChangeCmd : public QUndoCommand
    {
    public:
        PaintingRectChangeCmd(Painting *paintng, QRectF oldRect, QRectF newRect,
                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        Painting *const m_painting;
        QRectF m_oldRect;
        QRectF m_newRect;
    };

    class PaintingPropertyChangeCmd : public QUndoCommand
    {
    public:
        PaintingPropertyChangeCmd(Painting *painting, QString oldText,
                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        Painting *const m_painting;
        QString m_oldPropertyText;
        QString m_newPropertyText;
    };

    class GraphicText;  // Forward declaration

    class GraphicTextChangeCmd : public QUndoCommand
    {
    public:
        GraphicTextChangeCmd(GraphicText *text, QString oldText, QString newText,
                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        GraphicText *const m_graphicText;
        QString m_oldText;
        QString m_newText;
    };

    class PropertyMapCmd : public QUndoCommand
    {
    public:
        PropertyMapCmd(Component *comp, const PropertyMap& old, const PropertyMap& newMap,
                QUndoCommand *parent = 0);

        void undo();
        void redo();

    private:
        Component *m_component;
        PropertyMap m_oldMap;
        PropertyMap m_newMap;
    };

} // namespace Caneda

#endif //UNDO_COMMANDS_H
