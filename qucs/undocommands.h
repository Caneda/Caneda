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

#ifndef UNDOCMDS_H
#define UNDOCMDS_H

#include "component.h"
#include "wire.h"

#include <QPair>
#include <QPointF>
#include <QUndoCommand>
#include <QVariant>

class Component;
class Painting;
class Port;
class QucsItem;

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

class ScenePropertyChangeCmd : public QUndoCommand
{
public:
    ScenePropertyChangeCmd(const QString& propertyName,
            const QVariant& newValue,
            const QVariant& oldValue,
            SchematicScene *const schematic,
            QUndoCommand *parent = 0);

    virtual void undo();
    virtual void redo();

private:
    const QString m_property;
    const QVariant m_newValue;
    const QVariant m_oldValue;
    SchematicScene *const m_schematic;
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

class WireConnectionInfo;
class ConnectCmd : public QUndoCommand
{
public:
    ConnectCmd(Port *p1, Port *p2, const QList<Wire*> &wires,
            SchematicScene *scene, QUndoCommand *parent = 0);
    ~ConnectCmd();

    void undo();
    void redo();

private:
    Port * const m_port1;
    Port * const m_port2;
    QList<WireConnectionInfo*> m_wireConnectionInfos;

    SchematicScene *const m_scene;
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
    AddWireCmd(Wire *wire, SchematicScene *scene, QUndoCommand *parent = 0);
    ~AddWireCmd();

    void undo();
    void redo();

private:
    Wire *m_wire;
    SchematicScene *m_scene;
    QPointF m_pos;
};

class AddWireBetweenPortsCmd : public QUndoCommand
{
public:
    AddWireBetweenPortsCmd(Port *p1, Port* p2, QUndoCommand *parent = 0);
    void undo();
    void redo();

    Wire* wire() const { return m_wire; }

private:
    Port * const m_port1;
    Port * const m_port2;
    SchematicScene *m_scene;
    Wire *m_wire;
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
    InsertItemCmd(QGraphicsItem *const item, SchematicScene *scene,
            QPointF pos = InvalidPoint, QUndoCommand *parent = 0);
    ~InsertItemCmd();

    void undo();
    void redo();

protected:
    QGraphicsItem *const m_item;
    SchematicScene *const m_scene;
    QPointF m_pos;
};

class RemoveItemsCmd : public QUndoCommand
{
public:
    typedef QPair<QucsItem*, QPointF> ItemPointPair;

    RemoveItemsCmd(const QList<QucsItem*> &items, SchematicScene *scene,
            QUndoCommand *parent = 0);
    ~RemoveItemsCmd();

    void undo();
    void redo();

protected:
    QList<ItemPointPair> m_itemPointPairs;
    SchematicScene *const m_scene;
};

class RotateItemsCmd : public QUndoCommand
{
public:
    RotateItemsCmd(QList<QucsItem*> items, const  Qucs::AngleDirection=Qucs::Clockwise,
            QUndoCommand *parent = 0);
    RotateItemsCmd(QucsItem *item, const  Qucs::AngleDirection=Qucs::Clockwise,
            QUndoCommand *parent = 0);

    void undo();
    void redo();

protected:
    QList<QucsItem*> m_items;
    Qucs::AngleDirection m_angleDirection;
};

class MirrorItemsCmd : public QUndoCommand
{
public:
    MirrorItemsCmd(QList<QucsItem*> items, const Qt::Axis axis, QUndoCommand *parent = 0);
    MirrorItemsCmd(QucsItem *item, const Qt::Axis axis, QUndoCommand *parent = 0);

    void undo();
    void redo();

protected:
    QList<QucsItem*> m_items;
    Qt::Axis m_axis;
};

class ToggleActiveStatusCmd : public QUndoCommand
{
public:
    ToggleActiveStatusCmd(const QList<Component*> &components, QUndoCommand *parent = 0);

    void undo();
    void redo();

protected:
    typedef QPair<Component*, Qucs::ActiveStatus> ComponentStatusPair;
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

class GraphicText;

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

#endif //UNDOCMDS_H
