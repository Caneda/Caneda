/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef GRAPHICS_SCENE_H
#define GRAPHICS_SCENE_H

#include "global.h"
#include "undocommands.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>

#include <QtPrintSupport/QPrinter>

// Forward declarations
class QUndoStack;

namespace Caneda
{
    // Forward declarations
    class Component;
    class GraphicsItem;
    class Painting;
    class Wire;

    /*!
     * \brief This class provides a canvas for managing graphics elements
     * common to all Caneda's graphics scenes (schematics, symbols, layouts,
     * etc). This is one of Caneda's most important classes, along with
     * MainWindow class and \ref DocumentViewFramework.
     *
     * This class implements the scene class of Qt's Graphics View
     * Architecture, representing the actual document interface (scene). Each
     * scene must have at least one associated view (GraphicsView), to display
     * the contents of the scene. Several views can be attached to the same
     * scene, providing different viewports into the same data set (for
     * example, when using split views).
     *
     * The scene serves as a container for item objects and handles their
     * manipulation. In this class common item operations are implemented, for
     * example mirror and rotate items. Nevertheless this class must be
     * subclassed for more specific operations and handling of the different
     * types of documents.
     *
     * \sa GraphicsView, GraphicsItem
     */
    class GraphicsScene : public QGraphicsScene
    {
        Q_OBJECT

    public:
        explicit GraphicsScene(QObject *parent = 0);

        // Edit actions
        void cutItems(QList<GraphicsItem*> &items);
        void copyItems(QList<GraphicsItem*> &items);
        void deleteItems(QList<GraphicsItem*> &items);

        void mirrorItems(QList<GraphicsItem*> &items, const Qt::Axis axis);
        void mirrorXItems(QList<GraphicsItem*> &items) { mirrorItems(items, Qt::XAxis); }
        void mirrorYItems(QList<GraphicsItem*> &items) { mirrorItems(items, Qt::YAxis); }

        void rotateItems(QList<GraphicsItem*> &items, const Caneda::AngleDirection dir);
        void rotateItems(QList<GraphicsItem*> &items) { rotateItems(items, Caneda::Clockwise); }

        bool alignElements(const Qt::Alignment alignment);
        bool distributeElements(const Qt::Orientation orientation);
        void distributeElementsHorizontally(QList<GraphicsItem*> items);
        void distributeElementsVertically(QList<GraphicsItem*> items);

        // Document properties
        bool isBackgroundVisible() const { return m_backgroundVisible; }
        void setBackgroundVisible(bool visible);

        void print(QPrinter *printer, bool fitInView);
        bool exportImage(QPaintDevice &);

        // Mouse actions
        void setMouseAction(const Caneda::MouseAction ma);

        void beginInsertingItems(const QList<GraphicsItem*> &items);
        void beginPaintingDraw(Painting *item);

        // Connect/disconnect methods
        QPointF centerOfItems(const QList<GraphicsItem*> &items);

        void connectItems(GraphicsItem *item);
        void connectItems(QList<GraphicsItem *> &items);
        void disconnectItems(GraphicsItem *item);
        void disconnectItems(QList<GraphicsItem *> &items);

        void splitAndCreateNodes(GraphicsItem *item);
        void splitAndCreateNodes(QList<GraphicsItem *> &items);

        //! \brief Return current undo stack
        QUndoStack* undoStack() { return m_undoStack; }

        // Spice/electric related scene properties
        PropertyGroup* properties() { return m_properties; }
        void addProperty(Property property);

    Q_SIGNALS:
        //! \brief This signal is emitted whenever the undostack enters or leaves the clean state.
        void changed();
        void mouseActionChanged(Caneda::MouseAction);

    protected:
        void drawBackground(QPainter *p, const QRectF& r);

        // Custom event handlers
        bool event(QEvent *event);

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        void wheelEvent(QGraphicsSceneWheelEvent *event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    private:
        // Custom event handlers
        void sendMouseActionEvent(QGraphicsSceneMouseEvent *event);
        void normalEvent(QGraphicsSceneMouseEvent *event);

        void insertingItemsEvent(QGraphicsSceneMouseEvent *event);
        void paintingDrawEvent(QGraphicsSceneMouseEvent *event);

        void deletingEvent(const QGraphicsSceneMouseEvent *event);
        void deletingEventLeftMouseClick(const QPointF &pos);
        void deletingEventRightMouseClick(const QPointF &pos);

        void wiringEvent(QGraphicsSceneMouseEvent *event);
        void wiringEventMouseClick(const QGraphicsSceneMouseEvent *event, const QPointF &pos);
        void wiringEventLeftMouseClick(const QPointF &pos);
        void wiringEventRightMouseClick();
        void wiringEventMouseMove(const QPointF &newPos);

        void rotatingEvent(QGraphicsSceneMouseEvent *event);

        void mirroringEvent(const QGraphicsSceneMouseEvent *event, const Qt::Axis axis);
        void mirroringXEvent(const QGraphicsSceneMouseEvent *event);
        void mirroringYEvent(const QGraphicsSceneMouseEvent *event);

        void zoomingAreaEvent(QGraphicsSceneMouseEvent *event);

        // Custom private methods
        void placeItem(GraphicsItem *item, const QPointF &pos);
        int componentLabelSuffix(const QString& labelPrefix) const;

        void processForSpecialMove();
        void specialMove();
        void endSpecialMove();
        void disconnectDisconnectibles();
        void resetState();

        bool eventFilter(QObject *object, QEvent *event);
        void blockShortcuts(bool block);

        // Helper variables or state holders
        //! \brief Last grid position of mouse cursor
        QPointF lastPos;

        /*!
         * \brief Flag to determine whether items are being moved or not
         *        using mouse click + drag (not drag and drop) on scene.
         */
        bool m_areItemsMoving;

        //! \brief List of GraphicsItem which are to be placed/pasted.
        QList<GraphicsItem*> m_insertibles;

        //! \brief The Painting (Ellipse, Rectangle...) being drawn currently
        Painting *m_paintingDrawItem;

        /*!
         * \brief Number of mouse clicks inserting a painting item
         *
         * This is used to determine what feedback to show while painting.
         * For example:
         * \li One click of arc should determine corresponding elliptical point.
         * \li Second click should fix this ellipse and let select the start
         * angle of ellipse.
         * \li Third click should finalize by selecing span of the elliptical arc.
         */
        int m_paintingDrawClicks;

        /*!
         * \brief List of components whose port's needs to be disconencted
         *        due to mouse events
         *
         * When a wire is moved and one of the connected components is
         * unselected, the component must be disconnected from the moving
         * wires' ports. The list of components to disconnect is selected in
         * processForSpecialMove() and the disconnection is performed in the
         * disconnect() method.
         */
        QList<GraphicsItem*> disconnectibles;

        /*!
         * \brief List of items requiring special movements due to mouse event
         *
         * When an item is moved (click + drag) and one of the connected wires
         * is't selected, the latter's geometry needs to be altered to retain
         * its connection. Hence the last wires must be selected in
         * processForSpecialMove() and resized in specialMove().
         *
         * In a similar manner, some while some items must be disconnected (for
         * example normal components) others must be moved along with the wire
         * (for example net labels or port symbols).
         *
         * \sa processForSpecialMove(), specialMove()
         */
        QList<GraphicsItem*> specialMoveItems;

        //! \brief Wiring state machine state enum
        enum wiringStateEnum {
            NO_WIRE,               //! There are no wire segments yet
            SINGLETON_WIRE         //! Already created wire segments
        };

        //! \brief State variable for the wire state machine
        wiringStateEnum m_wiringState;

        //! \brief Current wire
        Wire *m_currentWiringWire;

        //! \brief Current mouse action
        Caneda::MouseAction m_mouseAction;

        /*!
         * \brief State holder to know whether shortcut events are blocked or not
         * \sa GraphicsScene::eventFilter, GraphicsScene::blockShortcuts
         */
        bool m_shortcutsBlocked;

        /*!
         * \brief Flag to hold whether the background color should be drawn or not
         * \sa setBackgroundVisible
         */
        bool m_backgroundVisible;

        /*!
         * \brief Rectangular widget to show feedback of an area being
         * selected for zooming
         */
        QGraphicsRectItem *m_zoomBand;
        QRectF m_zoomRect;
        int m_zoomBandClicks;

        //! \brief GraphicsScene undo stack
        QUndoStack *m_undoStack;

        /*! \brief Spice/electric related scene properties
         *
         * Properties should be always strings. While more specific types
         * could be used, string types allow the use of suffixes and parameters
         * like p for pico, u for micro, and {R} for parameter, for example.
         *
         * \sa PropertyData, Property, PropertyGroup
         */
        PropertyGroup *m_properties;
    };

} // namespace Caneda

#endif //GRAPHICS_SCENE_H
