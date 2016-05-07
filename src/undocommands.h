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

#ifndef UNDO_COMMANDS_H
#define UNDO_COMMANDS_H

#include "global.h"
#include "property.h"

#include <QPair>
#include <QUndoCommand>

namespace Caneda
{
    // Forward declarations.
    class GraphicsItem;
    class GraphicText;
    class Painting;
    class Port;
    class Wire;

    typedef QPair<GraphicsItem*, QPointF> ItemPointPair;

    /*!
     * \brief Move command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * Caneda uses Qt's Undo Framework to implement undo/redo system. Qt's Undo
     * Framework is an implementation of a Command pattern system, for
     * implementing undo/redo functionality in applications.
     *
     * The Command pattern is based on the idea that all editing in an
     * application is done by creating instances of command objects. Command
     * objects apply changes to a document and are stored on a command stack.
     * Furthermore, each command knows how to undo its changes to bring the
     * document back to its previous state. As long as the application only
     * uses command objects to change the state of the document, it is possible
     * to undo a sequence of commands by traversing the stack downwards and
     * calling undo on each command in turn. It is also possible to redo a
     * sequence of commands by traversing the stack upwards and calling redo on
     * each command.
     *
     * Caneda uses two clases from Qt's Undo Framework:
     * \li QUndoCommand is the base class of all commands stored on an undo
     * stack. It can apply (redo) or undo a single change in the document.
     * \li QUndoStack is a list of QUndoCommand objects. It contains all the
     * commands executed on the document and can roll the document's state
     * backwards or forwards by undoing or redoing them. The undo stacks are
     * usually stored in Caneda as a private member of the scene classes.
     *
     * While a QUndoStack maintains a stack of commands that have been applied
     * to a document. A QUndoCommand represents a single editing action on a
     * document; for example, inserting or deleting a component in a schematic.
     * New commands are pushed on the stack using push(). Any QUndoCommand can
     * apply a change to a document with the redo() method and undo the change
     * with the undo() method. The implementations for these functions must be
     * provided in each derived class.
     */
    class MoveCmd : public QUndoCommand
    {
    public:
        explicit MoveCmd(GraphicsItem *item,
                         const QPointF &init,
                         const QPointF &final,
                         QUndoCommand *parent = 0);

        void undo();
        void redo();

    private:
        GraphicsItem *m_item;
        QPointF m_initialPos;
        QPointF m_finalPos;
    };

    /*!
     * \brief Disconnect command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class DisconnectCmd : public QUndoCommand
    {
    public:
        explicit DisconnectCmd(Port *p1, Port *p2, QUndoCommand *parent = 0);

        void undo();
        void redo();

    private:
        Port * const m_port1;
        Port * const m_port2;
    };

    /*!
     * \brief Add wire command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class AddWireCmd : public QUndoCommand
    {
    public:
        explicit AddWireCmd(Wire *wire,
                            GraphicsScene *scene,
                            QUndoCommand *parent = 0);

        void undo();
        void redo();

    private:
        Wire *m_wire;
        GraphicsScene *m_scene;
    };

    /*!
     * \brief Insert item command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class InsertItemCmd : public QUndoCommand
    {
    public:
        explicit InsertItemCmd(GraphicsItem *const item,
                               GraphicsScene *scene,
                               QPointF pos,
                               QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        GraphicsItem *const m_item;
        GraphicsScene *const m_scene;
        QPointF m_pos;
    };

    /*!
     * \brief Remove items command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class RemoveItemsCmd : public QUndoCommand
    {
    public:
        explicit RemoveItemsCmd(const QList<GraphicsItem*> &items,
                                GraphicsScene *scene,
                                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        QList<ItemPointPair> m_itemPointPairs;
        GraphicsScene *const m_scene;
    };

    /*!
     * \brief Rotate command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class RotateItemsCmd : public QUndoCommand
    {
    public:
        explicit RotateItemsCmd(const QList<GraphicsItem*> &items,
                                const Caneda::AngleDirection,
                                GraphicsScene *scene,
                                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        QList<GraphicsItem*> m_items;
        Caneda::AngleDirection m_dir;
        GraphicsScene *const m_scene;
    };

    /*!
     * \brief Mirror command implementation of the QUndoCommand/QUndoStack
     * pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class MirrorItemsCmd : public QUndoCommand
    {
    public:
        explicit MirrorItemsCmd(const QList<GraphicsItem*> items,
                                const Qt::Axis axis,
                                GraphicsScene *scene,
                                QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        QList<GraphicsItem*> m_items;
        Qt::Axis m_axis;
        GraphicsScene *const m_scene;
    };

    /*!
     * \brief Change painting item geometry command implementation of the
     * QUndoCommand/QUndoStack pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class PaintingRectChangeCmd : public QUndoCommand
    {
    public:
        explicit PaintingRectChangeCmd(Painting *paintng,
                                       QRectF oldRect,
                                       QRectF newRect,
                                       QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        Painting *const m_painting;
        QRectF m_oldRect;
        QRectF m_newRect;
    };

    /*!
     * \brief Change painting item properties command implementation of the
     * QUndoCommand/QUndoStack pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class PaintingPropertyChangeCmd : public QUndoCommand
    {
    public:
        explicit PaintingPropertyChangeCmd(Painting *painting,
                                           QString oldText,
                                           QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        Painting *const m_painting;
        QString m_oldPropertyText;
        QString m_newPropertyText;
    };

    /*!
     * \brief Change graphic text properties command implementation of the
     * QUndoCommand/QUndoStack pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class GraphicTextChangeCmd : public QUndoCommand
    {
    public:
        explicit GraphicTextChangeCmd(GraphicText *text,
                                      QString oldText,
                                      QString newText,
                                      QUndoCommand *parent = 0);

        void undo();
        void redo();

    protected:
        GraphicText *const m_graphicText;
        QString m_oldText;
        QString m_newText;
    };

    /*!
     * \brief Change components' properties command implementation of the
     * QUndoCommand/QUndoStack pattern for Qt's Undo Framework.
     *
     * \copydetails MoveCmd
     */
    class PropertyMapCmd : public QUndoCommand
    {
    public:
        explicit PropertyMapCmd(PropertyGroup *propGroup,
                                const PropertyMap& old,
                                const PropertyMap& newMap,
                                QUndoCommand *parent = 0);

        void undo();
        void redo();

    private:
        PropertyGroup *m_propertyGroup;
        PropertyMap m_oldMap;
        PropertyMap m_newMap;
    };

} // namespace Caneda

#endif //UNDO_COMMANDS_H
