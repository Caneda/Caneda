/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2011 by Pablo Daniel Pareja Obregon                  *
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

#ifndef C_GRAPHICS_SCENE_H
#define C_GRAPHICS_SCENE_H

#include "undocommands.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>

// Forward declarations
class QUndoCommand;
class QUndoStack;

namespace Caneda
{
    // Forward declarations
    class Component;
    class CGraphicsItem;
    class Painting;
    class SvgPainter;
    class Wire;

    typedef QGraphicsSceneMouseEvent MouseActionEvent;

    /*!
     * CGraphicsScene
     * This class provides a canvas for managing schematic elements
     */
    class CGraphicsScene : public QGraphicsScene
    {
        Q_OBJECT
    public:
        //! \brief Possible mouse actions
        enum MouseAction {
            //! Wire action
            Wiring,
            //! Delete
            Deleting,
            //! Placing a mark on the graph.
            Marking,
            //! Rotate
            Rotating,
            //! Mirror X
            MirroringX,
            //! Mirror Y
            MirroringY,
            //! Change status ie short, open
            ChangingActiveStatus,
            //! Zoom an area
            ZoomingAreaEvent,
            //! Painting item's drawing (like Ellipse, Rectangle)
            PaintingDrawEvent,
            //! insert an item
            InsertingItems,
            //! insert a wire label
            InsertingWireLabel,
            //! Normal (ie select)
            Normal
        };

        // Constructor-destructor
        CGraphicsScene(QObject *parent = 0);
        ~CGraphicsScene();

        // Edit actions
        void cutItems(QList<CGraphicsItem*> &items, const Caneda::UndoOption = Caneda::PushUndoCmd);
        void copyItems(QList<CGraphicsItem*> &items) const;
        void deleteItems(QList<CGraphicsItem*> &items, const Caneda::UndoOption);

        void mirrorItems(QList<CGraphicsItem*> &itemsenum,
                const Caneda::UndoOption opt,
                const Qt::Axis axis);
        void mirrorXItems(QList<CGraphicsItem*> &items, const Caneda::UndoOption opt) {
            mirrorItems(items, opt, Qt::XAxis);
        }
        void mirrorYItems(QList<CGraphicsItem*> &items, const Caneda::UndoOption opt) {
            mirrorItems(items, opt, Qt::YAxis);
        }

        void rotateItems(QList<CGraphicsItem*> &items, const Caneda::AngleDirection dir,
                const Caneda::UndoOption);
        void rotateItems(QList<CGraphicsItem*> &items, const Caneda::UndoOption undo) {
            rotateItems(items, Caneda::Clockwise, undo);
        }

        bool alignElements(const Qt::Alignment alignment);
        bool distributeElements(const Qt::Orientation orientation);

        void toggleActiveStatus(QList<CGraphicsItem*> &components, const Caneda::UndoOption);

        // Document properties
        bool isBackgroundVisible() const { return m_backgroundVisible; }
        void setBackgroundVisible(bool vis);

        bool toPaintDevice(QPaintDevice &, qreal = -1, qreal = -1,
                Qt::AspectRatioMode = Qt::KeepAspectRatio);

        bool snapToGrid() const { return m_snapToGrid; }
        void setSnapToGrid(const bool snap) { m_snapToGrid = snap; }
        QPointF smartNearingGridPoint(const QPointF &pos) const;

        // Mouse actions
        MouseAction mouseAction() const { return m_mouseAction; }
        void setMouseAction(const MouseAction ma);

        bool eventFilter(QObject *object, QEvent *event);
        void blockShortcuts(bool block);

        void beginPaintingDraw(Painting *item);
        void beginInsertingItems(const QList<CGraphicsItem*> &items);

        //! Return current undo stack
        QUndoStack* undoStack() { return m_undoStack; }
        bool isModified() const { return m_modified; }

    public Q_SLOTS:
        void setModified(const bool m = true);
        bool sidebarItemClicked(const QString &item, const QString& category);

    Q_SIGNALS:
        void changed();
        void mouseActionChanged();
        void rotateInvokedWhileInserting();
        void mirrorInvokedWhileInserting();

    protected:
        void drawBackground(QPainter *p, const QRectF& r);

        // Events
        bool event(QEvent *event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *e);

        void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        void dropEvent(QGraphicsSceneDragDropEvent * event);

        void mousePressEvent(QGraphicsSceneMouseEvent *e);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
        void wheelEvent(QGraphicsSceneWheelEvent *e);

    private:
        // Custom handlers
        void sendMouseActionEvent(QGraphicsSceneMouseEvent *e);
        void resetState();

        void wiringEvent(MouseActionEvent *e);
        void wiringEventMouseClick(const MouseActionEvent *event, const QPointF &pos);
        void wiringEventLeftMouseClick(const QPointF &pos);
        void wiringEventRightMouseClick();
        void wiringEventMouseMove(const QPointF &pos);

        void deletingEvent(const MouseActionEvent *e);
        void deletingEventLeftMouseClick(const QPointF &pos);
        void deletingEventRightMouseClick(const QPointF &pos);

        void rotatingEvent(MouseActionEvent *e);
        void zoomingAreaEvent(MouseActionEvent *e);
        void markingEvent(MouseActionEvent *e);
        void changingActiveStatusEvent(const MouseActionEvent *e);
        void paintingDrawEvent(MouseActionEvent *e);
        void insertingItemsEvent(MouseActionEvent *e);
        void insertingWireLabelEvent(MouseActionEvent *event);

        void placeAndDuplicatePainting();

        void normalEvent(MouseActionEvent *e);
        void processForSpecialMove(QList<QGraphicsItem*> _items);
        void disconnectDisconnectibles();
        void specialMove();
        void endSpecialMove();

        // Sidebar click
        bool sidebarItemClickedPaintingsItems(const QString& itemName);
        bool sidebarItemClickedNormalItems(const QString& itemName, const QString& category);

        // Placing items
        void placeItem(CGraphicsItem *item, const QPointF &pos, const Caneda::UndoOption opt);
        CGraphicsItem* itemForName(const QString& name, const QString& category);
        int componentLabelSuffix(const QString& labelPrefix) const;

        // Private edit events
        void mirroringEvent(const MouseActionEvent *event, const Qt::Axis axis);
        void mirroringXEvent(const MouseActionEvent *e);
        void mirroringYEvent(const MouseActionEvent *e);

        void distributeElementsHorizontally(QList<CGraphicsItem*> items);
        void distributeElementsVertically(QList<CGraphicsItem*> items);

        static const QString Alignment2QString(const Qt::Alignment alignment);

        void connectItems(const QList<CGraphicsItem*> &qItems, const Caneda::UndoOption opt);
        void disconnectItems(const QList<CGraphicsItem*> &qItems,
                const Caneda::UndoOption opt = Caneda::PushUndoCmd);

        // Helper variables (aka state holders)
        //! \brief Last grid position of mouse cursor
        QPointF lastPos;

        /*!
         * \brief Flag to determine whether items are being moved or not
         *        using mouse click + drag (not drag and drop) on scene.
         */
        bool m_areItemsMoving;

        /*!
         * \brief List of components whose port's needs to be disconencted
         *        due to mouse events
         */
        QList<CGraphicsItem*> disconnectibles;

        /*!
         * \brief List of CGraphicsItem which are to be placed/pasted.
         */
        QList<CGraphicsItem*> m_insertibles;

        //! \brief The Painting (Ellipse, Rectangle...) being drawn currently
        Painting *m_paintingDrawItem;

        /*!
         * \brief Number of mouse clicks inserting a painting item
         *
         * This is used to determine what feedback to show while painting.
         * For example:
         * - One click of arc should determine corresponding elliptical point.
         * - Second click should fix this ellipse and let select the start
         * angle of ellipse.
         * - Third click should finalize by selecing span of the elliptical arc.
         */
        int m_paintingDrawClicks;

        /*!
         * \brief List of wire's requiring segment changes due to mouse event
         *
         * When an item is moved (click + drag) and one of the connected wires
         * aren't selected, its geometry needs to be altered to retain connection
         * to the wire. Hence these wires are selected in processForSpecialMove
         * and are resized in specialMove.
         */
        QList<Wire*> movingWires;

        //! Wiring state machine state enum
        enum wiringStateEnum {
            NO_WIRE,               /*!< There are no wires yet */
            SINGLETON_WIRE         /*!< Currently creating wires */
        };

        //! State variable for the wire state machine
        wiringStateEnum m_wiringState;

        //! Current wire
        Wire *m_currentWiringWire;

        //! Current mouse action
        MouseAction m_mouseAction;

        /*!
         * \brief State holder to know whether shortcut events are blocked or not
         * \sa CGraphicsScene::eventFilter, CGraphicsScene::blockShortcuts
         */
        bool m_shortcutsBlocked;

        /*!
         * \brief Flag to hold whether a schematic is modified or not
         * i.e to determine whether a file should be saved or not on closing.
         *
         * This flag should be set as and when any modification is done to schematic
         * and usually these are done in event handlers. Usually programmatic changes
         * to the schematic won't set this flag.
         * \sa setModified
         */
        bool m_modified;

        /*!
         * \brief Flag to hold whether the background color should be drawn or not
         * \sa setBackgroundVisible
         */
        bool m_backgroundVisible;

        /*!
         * \brief Flag to hold whether or not snap to grid is set
         * \sa setSnapToGrid
         */
        bool m_snapToGrid;

        /*!
         * \brief Rectangular widget to show feedback of an area being
         * selected for zooming
         */
        QGraphicsRectItem *m_zoomBand;
        QRectF m_zoomRect;
        int m_zoomBandClicks;

        //! Undo stack state
        QUndoStack *m_undoStack;
    };

} // namespace Caneda

#endif //C_GRAPHICS_SCENE_H
