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

#ifndef SCHEMATICSCENE_H
#define SCHEMATICSCENE_H

#include "undocommands.h"

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QPair>
#include <QPointF>
#include <QVarLengthArray>

class Diagram;
class Painting;
class QucsItem;
class QRubberBand;
class QUndoCommand;
class QUndoStack;
class SvgPainter;

namespace Qucs
{
    enum Mode {
        SchematicMode,
        SymbolMode
    };
}

class Component;
class Wire;
class SchematicView;

typedef QGraphicsSceneMouseEvent MouseActionEvent;

/*!
 * SchematicScene
 * This class provides a surface for managing a large number of schematic element
 */
class SchematicScene : public QGraphicsScene
{
    Q_OBJECT
public:
    //! \brief The different mouse action possible
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
        //! Set on grid
        SettingOnGrid,
        //! Zoom at point
        ZoomingAtPoint,
        //! Zoom out at point
        ZoomingOutAtPoint,
        //! Painting item's drawing (like Ellipse, Rectangle)
        PaintingDrawEvent,
        //! insert an item
        InsertingItems,
        //! insert a wire label
        InsertingWireLabel,
        //! Normal (ie select)
        Normal
    };

    /* constructor/destructor */
    SchematicScene(QObject *parent =0);
    ~SchematicScene();

    bool areItemsMoving() const { return m_areItemsMoving; }

    /* geometry change */

    /* mirror */
    void mirrorItems(QList<QucsItem*> &itemsenum,
            const Qucs::UndoOption opt,
            const Qt::Axis axis);
    void mirrorXItems(QList<QucsItem*> &items, const Qucs::UndoOption opt) {
        mirrorItems(items, opt, Qt::XAxis);
    }

    void mirrorYItems(QList<QucsItem*> &items, const Qucs::UndoOption opt) {
        mirrorItems(items, opt, Qt::YAxis);
    }

    void rotateItems(QList<QucsItem*> &items, const Qucs::AngleDirection dir,
            const Qucs::UndoOption);
    void rotateItems(QList<QucsItem*> &items, const Qucs::UndoOption undo) {
        rotateItems(items, Qucs::Clockwise, undo);
    }
    void setItemsOnGrid(QList<QucsItem*> &items, const Qucs::UndoOption);

    void deleteItems(QList<QucsItem*> &items, const Qucs::UndoOption);
    void toggleActiveStatus(QList<QucsItem*> &components, const Qucs::UndoOption);

    // these aren't toggle actions.
    void cutItems(QList<QucsItem*> &items, const Qucs::UndoOption = Qucs::PushUndoCmd);
    void copyItems(QList<QucsItem*> &items) const;

    QString fileName() const { return m_fileName; }
    void setFileName(const QString& fn);

    bool isModified() const { return m_modified; }

    //! round to nearest grid point according to grid snapping setting
    QPointF smartNearingGridPoint(const QPointF &pos) const {
        return m_snapToGrid == true ? nearingGridPoint(pos) : pos;
    }

    //! return current undo stack
    QUndoStack* undoStack() { return m_undoStack; }

    //! return grid width
    uint gridWidth() const { return m_gridWidth; }
    //! set grid witdh
    void setGridWidth(uint width) { setGridSize(width, gridHeight()); }

    //!  return grid  height
    uint gridHeight() const { return m_gridHeight; }
    //! set grid height
    void setGridHeight(uint height) { setGridSize(gridWidth(), height); }

    void setGridSize(uint width, uint height);

    //! return grid visibility
    bool isGridVisible() const { return m_gridVisible; }
    void setGridVisible(const bool visibility);

    //! get origin drawing status
    bool isOriginDrawn() const { return m_OriginDrawn; }
    void setOriginDrawn(const bool visibility);

    //! get grid color
    QColor GridColor() const { return m_gridcolor; }
    void setGridColor(const QColor & color);

    //! set grid witdh
    bool setProperty(const QString& propName, const QVariant& value);

    QString dataSet() const { return m_dataSet; }
    void setDataSet(const QString& str);

    QString dataDisplay() const { return m_dataDisplay; }
    void setDataDisplay(const QString& disp);

    bool opensDataDisplay() const { return m_opensDataDisplay; }
    void setOpensDataDisplay(bool b);

    bool isBackgroundVisible() const { return m_backgroundVisible; }
    void setBackgroundVisible(bool vis);

    bool isFrameVisible() const { return m_frameVisible; }
    void setFrameVisible(bool vis);

    QStringList frameTexts() const { return m_frameTexts; }
    void setFrameTexts(const QStringList& texts);

    int frameRows() const { return m_frameRows; }
    int frameColumns() const { return m_frameColumns; }
    void setFrameSize(int rows, int columns);

    Qucs::Mode currentMode() const { return m_currentMode; }
    void setMode(const Qucs::Mode mode);

    MouseAction currentMouseAction() const { return m_currentMouseAction; }
    void setCurrentMouseAction(const MouseAction ma);

    void resetState();
    void beginPaintingDraw(Painting *item);
    void beginInsertingItems(const QList<QucsItem*> &items);

    bool alignElements(const Qt::Alignment alignment);
    bool distributeElements(const Qt::Orientation orientation);

    bool eventFilter(QObject *object, QEvent *event);

    bool shortcutsBlocked() const { return m_shortcutsBlocked; }
    void blockShortcuts(bool block);

    bool toPaintDevice(QPaintDevice &, qreal = -1, qreal = -1,
            Qt::AspectRatioMode = Qt::KeepAspectRatio);
    QRectF imageBoundingRect() const;

public Q_SLOTS:
    void setModified(const bool m = true);
    bool sidebarItemClicked(const QString &item, const QString& category);

Q_SIGNALS:
    void modificationChanged(bool changed);
    void fileNameChanged(const QString& file);
    void titleToBeUpdated();
    void rotateInvokedWhileInserting();
    void mirrorInvokedWhileInserting();

protected:
    void drawBackground(QPainter *p, const QRectF& r);

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

    // Custom handlers
    void wiringEvent(MouseActionEvent *e);
    void deletingEvent(const MouseActionEvent *e);
    void markingEvent(MouseActionEvent *e);
    void rotatingEvent(MouseActionEvent *e);
    void changingActiveStatusEvent(const MouseActionEvent *e);
    void settingOnGridEvent(const MouseActionEvent *e);
    void zoomingAtPointEvent(MouseActionEvent *e);
    void zoomingOutAtPointEvent(MouseActionEvent *e);
    void paintingDrawEvent(MouseActionEvent *e);
    void insertingItemsEvent(MouseActionEvent *e);
    void insertingWireLabelEvent(MouseActionEvent *event);
    void normalEvent(MouseActionEvent *e);

private:
    void init();

    /* private reset state wiring */
    void resetStateWiring();

    void sendMouseActionEvent(QGraphicsSceneMouseEvent *e);

    void processForSpecialMove(QList<QGraphicsItem*> _items);
    void disconnectDisconnectibles();
    void specialMove(qreal dx, qreal dy);
    void endSpecialMove();

    /* private wiring function */
    void wiringEventLeftMouseClickCommonComplexSingletonWire(QUndoCommand * cmd);
    void wiringEventLeftMouseClickAddSegment();
    void wiringEventLeftMouseClick(const QPointF &pos);
    void wiringEventRightMouseClick();
    void wiringEventMouseClickFinalize();
    void wiringEventMouseClick(const MouseActionEvent *event, const QPointF &pos);
    void wiringEventMouseMove(const QPointF &pos);

    /* mirror */
    void mirroringEvent(const MouseActionEvent *event, const Qt::Axis axis);
    void mirroringXEvent(const MouseActionEvent *e);
    void mirroringYEvent(const MouseActionEvent *e);

    /* private delete */
    void deletingEventRightMouseClick(const QPointF &pos);
    void deletingEventLeftMouseClick(const QPointF &pos);

    /* private distribute */
    void distributeElementsHorizontally(QList<QucsItem*> items);
    void distributeElementsVertically(QList<QucsItem*> items);

    /* alignment */
    static const QString Alignment2QString(const Qt::Alignment alignment);

    /* sidebar click */
    bool sidebarItemClickedPaintingsItems(const QString& itemName);
    bool sidebarItemClickedNormalItems(const QString& itemName, const QString& category);

    QucsItem* itemForName(const QString& name, const QString& category);
    void placeItem(QucsItem *item, const QPointF &pos, const Qucs::UndoOption opt);
    int componentLabelSuffix(const QString& labelPrefix) const;

    int unusedPortNumber();
    bool isPortNumberUsed(int num) const;
    void setNumberUnused(int num);

    void disconnectItems(const QList<QucsItem*> &qItems,
            const Qucs::UndoOption opt = Qucs::PushUndoCmd);
    void connectItems(const QList<QucsItem*> &qItems, const Qucs::UndoOption opt);

    void placeAndDuplicatePainting();

    QPointF nearingGridPoint(const QPointF &pos) const;

    //These are helper variables (aka state holders)

    /*!
     * \brief A flag to determine whether items are being moved or not
     *        using mouse click + drag (not drag and drop) on scene.
     * \note Used in normalEvent
     */
    bool m_areItemsMoving;
    /*!
     * \brief A list of components whose port's needs to be disconencted
     *        due to mouse events
     * \sa disconnectDisconnectibles
     */
    QList<Component*> disconnectibles;
    /*!
     * \brief A list of wire's requiring segment changes due to mouse event
     *
     * When a component is moved(click + drag) and one of the connected wire isn't
     * selected its segments needs to be altered to retain connection to the wire
     * and also the wire should be composed of only horizontal and vertical
     * segments only
     * Hence these kinds of wires are predetermined in processForSpecialMove and
     * are resized( + or - wire segments) in specialMove
     */
    QList<Wire*> movingWires;
    /*!
     * \brief A list of wires which needs to be literally moved
     * (no change in segments)
     *
     * These wires are predetermined in processForSpecialMove.
     * A wire is scheduled for grabMove if its both ports have all their owners
     * selected (the wire being scheduled may or may not be selected)
     * In the grabMove method, only a delta is added to the wire.
     */
    QList<Wire*> grabMovingWires;
    //! \brief A helper variable to hold last grid position of mouse cursor
    QPointF lastPos;

    /*!
     * \brief A list of QucsItem which are to be placed/pasted.
     * \sa beginInsertingItems
     */
    QList<QucsItem*> m_insertibles;

    //! Wiring state machine state  enum
    enum wiringStateEnum {
        NO_WIRE,               /*!< They are no wire */
        SINGLETON_WIRE,        /*!< Wire is a singleton, ie only one single point */
        COMPLEX_WIRE           /*!< Wire is composed by more than one point */
    };

    //! State variable for the wire state machine
    wiringStateEnum m_wiringState;
    //! Current wire
    Wire *m_currentWiringWire;

    //! \brief The Painting(Ellipse, Rectangle..) being drawn currently
    Painting *m_paintingDrawItem;
    /*!
     * \brief Helper which holds the number of mouse clicks happened.
     *
     * This is used to determine what feedback to show while painting
     * For example
     * One click of arc should determine corresponding elliptical point
     * Second click should fix this ellipse and let select the start angle
     * of ellipse
     * Third click should finalize by selecing span of the elliptical arc.
     */
    int m_paintingDrawClicks;
    /*!
     * \brief A rectangular dotted line widget to show feedback of
     * an area being selected for zooming
     */
    QRubberBand * m_zoomBand;
    //! \brief An area to be zoomed
    QRectF m_zoomRect;

    //! \todo document
    QList<int> m_usedPortNumbers;
    //! \todo document
    QList<int> m_usablePortNumbers;

    //Document properties
    //! Undo stack state
    QUndoStack *m_undoStack;
    //! Current mouse action
    MouseAction m_currentMouseAction;
    Qucs::Mode m_currentMode;

    //! Grid width in pixel
    uint m_gridWidth;
    //! Grid height in pixel
    uint m_gridHeight;
    //! Grid is visible
    bool m_gridVisible;
    //! Grid color
    QColor m_gridcolor;

    //! Data Set file name
    QString m_dataSet;
    //! Data display file name
    QString m_dataDisplay;
    //! File name
    QString m_fileName;
    //! Frame texts
    QStringList m_frameTexts;
    //! Frame size
    int m_frameRows, m_frameColumns;

    /*!
     * \brief A flag to hold whether a schematic is modified or not
     * i.e to determine whether a file should be saved or not on closing.
     *
     * This flag should be set as and when any modification is done to schematic
     * and usually these are done in event handlers. Usually programmatic changes
     * to the schematic won't set this flag.
     * \sa setModified
     */
    bool m_modified;
    /*!
     * \brief A flag to hold whether a plot diagram should be opened on completion
     * of simulation.
     *
     * This is user configurable and only for convienience of the user.
     */
    bool m_opensDataDisplay;
    /*!
     * \brief Flag to hold whether the background color should be drawn or not
     * \sa setBackgroundVisible
     */
    bool m_backgroundVisible;
    /*!
     * \brief Flag to hold whether a frame should be drawn or not
     * \sa setFrameVisible
     */
    bool m_frameVisible;
    //! Snap component to grid
    bool m_snapToGrid;
    //! Draw origin boolean
    bool m_OriginDrawn;
    //! \brief A state holder whether an UndoStack's macro is started or not
    bool m_macroProgress;
    /*!
     * \brief A state holder to know whether shortcut events are blocked or not
     * \sa SchematicScene::eventFilter, SchematicScene::blockShortcuts
     */
    bool m_shortcutsBlocked;
};

#endif //SCHEMATICSCENE_H
