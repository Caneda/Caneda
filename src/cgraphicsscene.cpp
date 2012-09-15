/***************************************************************************
 * Copyright (C) 2006 Gopala Krishna A <krishna.ggk@gmail.com>             *
 * Copyright (C) 2008 Bastien Roucaries <roucaries.bastien@gmail.com>      *
 * Copyright (C) 2009-2012 by Pablo Daniel Pareja Obregon                  *
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

#include "cgraphicsscene.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "documentviewmanager.h"
#include "iview.h"
#include "library.h"
#include "propertygroup.h"
#include "settings.h"
#include "xmlutilities.h"

#include "paintings/ellipsearc.h"
#include "paintings/graphictextdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QGraphicsSceneEvent>
#include <QKeySequence>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QShortcutEvent>

#include <cmath>

namespace Caneda
{
    //! \brief Default Constructor
    CGraphicsScene::CGraphicsScene(QObject *parent) :
        QGraphicsScene(QRectF(-2500, -2500, 5000, 5000), parent)
    {
        // Setup undo stack
        m_undoStack = new QUndoStack(this);

        // Setup grid
        m_backgroundVisible = true;
        m_modified = false;

        m_areItemsMoving = false;
        m_shortcutsBlocked = false;

        // Wire state machine
        m_wiringState = NO_WIRE;
        m_currentWiringWire = NULL;

        m_paintingDrawItem = 0;
        m_paintingDrawClicks = 0;

        Settings *settings = Settings::instance();
        QColor zoomBandColor =
            settings->currentValue("gui/foregroundColor").value<QColor>();
        m_zoomBand = new QGraphicsRectItem();
        m_zoomBand->setPen(QPen(zoomBandColor));
        zoomBandColor.setAlpha(25);
        m_zoomBand->setBrush(QBrush(zoomBandColor));
        m_zoomBand->hide();
        addItem(m_zoomBand);

        m_zoomBandClicks = 0;

        setMouseAction(Normal);

        connect(undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(setModified(bool)));
    }

    //! \brief Default Destructor
    CGraphicsScene::~CGraphicsScene()
    {
        delete m_undoStack;
    }

    //! \brief Cut items
    void CGraphicsScene::cutItems(QList<CGraphicsItem*> &_items, const Caneda::UndoOption opt)
    {
        copyItems(_items);
        deleteItems(_items, opt);
    }

    /*!
     * \brief Copy item
     * \todo Document format
     * \todo Use own mime type
     */
    void CGraphicsScene::copyItems(QList<CGraphicsItem*> &_items) const
    {
        if(_items.isEmpty()) {
            return;
        }

        QString clipText;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&clipText);
        writer->setAutoFormatting(true);
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        foreach(CGraphicsItem *_item, _items) {
            _item->saveData(writer);
        }

        writer->writeEndDocument();

        QClipboard *clipboard =  QApplication::clipboard();
        clipboard->setText(clipText);
    }

    /*!
     * \brief Delete an item list
     *
     * \param items: item list
     * \param opt: undo option
     */
    void CGraphicsScene::deleteItems(QList<CGraphicsItem*> &items,
            const Caneda::UndoOption opt)
    {
        if(opt == Caneda::DontPushUndoCmd) {
            foreach(CGraphicsItem* item, items) {
                delete item;
            }
        }
        else {
            // Configure undo
            m_undoStack->beginMacro(QString("Delete items"));

            // Diconnect then remove
            disconnectItems(items, opt);
            m_undoStack->push(new RemoveItemsCmd(items, this));

            m_undoStack->endMacro();
        }
    }

    /*!
     * \brief Mirror an item list
     *
     * \param items: item to mirror
     * \param opt: undo option
     * \param axis: mirror axis
     * \todo Create a custom undo class for avoiding if
     * \note assert X or Y axis
     */
    void CGraphicsScene::mirrorItems(QList<CGraphicsItem*> &items,
            const Caneda::UndoOption opt,
            const Qt::Axis axis)
    {
        Q_ASSERT(axis == Qt::XAxis || axis == Qt::YAxis);

        // Prepare undo stack
        if(opt == Caneda::PushUndoCmd) {
            if(axis == Qt::XAxis) {
                m_undoStack->beginMacro(QString("Mirror X"));
            }
            else {
                m_undoStack->beginMacro(QString("Mirror Y"));
            }
        }

        // Disconnect item before mirroring
        disconnectItems(items, opt);

        // Mirror
        MirrorItemsCmd *cmd = new MirrorItemsCmd(items, axis);
        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->push(cmd);
        }
        else {
            cmd->redo();
            delete cmd;
        }

        // Try to reconnect
        connectItems(items, opt);

        // End undo
        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->endMacro();
        }
    }

    /*!
     * \brief Align elements
     *
     * \param alignment: alignement used
     * \todo use smart alignment ie: port alignement
     * \todo string of undo
     * \todo filter wires ???
     */
    bool CGraphicsScene::alignElements(const Qt::Alignment alignment)
    {
        QList<QGraphicsItem*> gItems = selectedItems();
        QList<CGraphicsItem*> items = filterItems<CGraphicsItem>(gItems, DontRemoveItems);

        // Could not align less than two elements
        if(items.size() < 2) {
            return false;
        }

        // Setup undo
        m_undoStack->beginMacro(Alignment2QString(alignment));

        // Disconnect
        disconnectItems(items, Caneda::PushUndoCmd);

        // Compute bounding rectangle
        QRectF rect = items.first()->sceneBoundingRect();
        QList<CGraphicsItem*>::iterator it = items.begin()+1;
        while(it != items.end()) {
            rect |= (*it)->sceneBoundingRect();
            ++it;
        }

        it = items.begin();
        while(it != items.end()) {
            if((*it)->isWire()) {
                ++it;
                continue;
            }

            QRectF itemRect = (*it)->sceneBoundingRect();
            QPointF delta;

            switch(alignment) {
                case Qt::AlignLeft :
                    delta.rx() =  rect.left() - itemRect.left();
                    break;
                case Qt::AlignRight :
                    delta.rx() = rect.right() - itemRect.right();
                    break;
                case Qt::AlignTop :
                    delta.ry() = rect.top() - itemRect.top();
                    break;
                case Qt::AlignBottom :
                    delta.ry() = rect.bottom() - itemRect.bottom();
                    break;
                case Qt::AlignHCenter :
                    delta.rx() = rect.center().x() - itemRect.center().x();
                    break;
                case Qt::AlignVCenter :
                    delta.ry() = rect.center().y() - itemRect.center().y();
                    break;
                case Qt::AlignCenter:
                    delta.rx() = rect.center().x() - itemRect.center().x();
                    delta.ry() = rect.center().y() - itemRect.center().y();
                    break;
                default:
                    break;
            }

            // Move item
            QPointF itemPos = (*it)->pos();
            m_undoStack->push(new MoveCmd(*it, itemPos, itemPos + delta));;
            ++it;
        }

        // Reconnect items
        connectItems(items, Caneda::PushUndoCmd);

        // Finish undo
        m_undoStack->endMacro();
        return true;
    }

    /*!
     * \brief Distribute elements
     *
     * Distribute elements ie each element is equally spaced
     *
     * \param orientation: distribute according to orientation
     * \todo filter wire ??? Do not distribute wire ??
     */
    bool CGraphicsScene::distributeElements(const Qt::Orientation orientation)
    {
        QList<QGraphicsItem*> gItems = selectedItems();
        QList<CGraphicsItem*> items = filterItems<CGraphicsItem>(gItems);

        /* could not distribute single items */
        if(items.size() < 2) {
            return false;
        }

        if(orientation == Qt::Horizontal) {
            distributeElementsHorizontally(items);
        }
        else {
            distributeElementsVertically(items);
        }
        return true;
    }

    /*!
     * \brief Makes the background color visible.
     *
     * \param visibility Set true of false to show or hide the background color.
     */
    void CGraphicsScene::setBackgroundVisible(const bool visibility)
    {
        /* avoid updating */
        if(m_backgroundVisible == visibility) {
            return;
        }

        m_backgroundVisible = visibility;
        update();
    }

    /*!
     * \brief Exports the scene to an image
     *
     * @return bool True on success, false otherwise
     */
    bool CGraphicsScene::toPaintDevice(QPaintDevice &pix, qreal width, qreal height,
            Qt::AspectRatioMode aspectRatioMode)
    {
        QRectF source_area = itemsBoundingRect();

        // we move the origin to fit in grid
        QPointF newOrigin = smartNearingGridPoint(source_area.topLeft());

        QPointF delta = source_area.topLeft();
        delta.setX(source_area.left() - newOrigin.x());
        delta.setY(source_area.top() - newOrigin.y());

        source_area.setLeft(newOrigin.x());
        source_area.setTop(newOrigin.y());

        // if the dimensions are not specified, the image is exported at 1:1 scale
        QRectF dest_area;
        if(width == -1 && height == -1) {
            dest_area = source_area;
        }
        else {
            dest_area = QRectF(0, 0, width+delta.x(), height+delta.y()); // we add the delta added to fit in grid
        }

        // hack: we make the source_area a little bit bigger that dest_area to avoid expanding the image
        source_area.setBottom(source_area.bottom()+1);
        source_area.setRight(source_area.right()+1);

        // prepare the device
        QPainter p;
        if(!p.begin(&pix)) {
            return(false);
        }

        // deselect the elements
        QList<QGraphicsItem *> selected_elmts = selectedItems();
        foreach(QGraphicsItem *qgi, selected_elmts) {
            qgi->setSelected(false);
        }

        // performs rendering itself
        setBackgroundVisible(false);
        render(&p, dest_area, source_area, aspectRatioMode);
        setBackgroundVisible(true);
        p.end();

        // restores the selected items
        foreach(QGraphicsItem *qgi, selected_elmts) {
            qgi->setSelected(true);
        }

        return(true);
    }

    /*!
     * \brief Get nearest grid point (grid snapping)
     *
     * \param pos: current position to be rounded
     * \return rounded position
     */
    QPointF CGraphicsScene::smartNearingGridPoint(const QPointF &pos) const
    {
        const QPoint point = pos.toPoint();

        int x = qAbs(point.x());
        x += (Caneda::GRID_SPACE >> 1);
        x -= x % Caneda::GRID_SPACE;
        x *= sign(point.x());

        int y = qAbs(point.y());
        y += (Caneda::GRID_SPACE >> 1);
        y -= y % Caneda::GRID_SPACE;
        y *= sign(point.y());

        return QPointF(x, y);
    }

    /*!
     * \brief Set mouse action
     * This method takes care to disable the shortcuts while items are being added
     * to the scene thus preventing side effects. It also sets the appropriate
     * drag mode for all the views associated with this scene.
     * Finally the state variables are reset.
     *
     * \param MouseAction: mouse action to set
     */
    void CGraphicsScene::setMouseAction(const MouseAction action)
    {
        if(m_mouseAction == action) {
            return;
        }

        // Remove the shortcut blocking if the current action uptil now was InsertItems
        if(m_mouseAction == InsertingItems) {
            blockShortcuts(false);
        }

        // Blocks shortcut if the new action to be set is InsertingItems
        if(action == InsertingItems) {
            blockShortcuts(true);
        }

        m_areItemsMoving = false;
        m_mouseAction = action;

        emit mouseActionChanged();

        resetState();
        //TODO: Implemement this appropriately for all mouse actions
    }

    /*!
     * \brief Event filter filter's out some events on the watched object.
     *
     * This filter is used to install on QApplication object to filter our
     * shortcut events.
     * This filter is installed by \a setMouseAction method if the new action
     * is InsertingItems and removed if the new action is different, thus blocking
     * shortcuts on InsertItems and unblocking for other mouse actions
     * \sa CGraphicsScene::setMouseAction, CGraphicsScene::blockShortcuts
     * \sa QObject::eventFilter
     *
     * \todo Take care if multiple scenes install event filters.
     */
    bool CGraphicsScene::eventFilter(QObject *watched, QEvent *event)
    {
        if(event->type() != QEvent::Shortcut && event->type() != QEvent::ShortcutOverride) {
            return QGraphicsScene::eventFilter(watched, event);
        }

        QKeySequence key;

        if(event->type() == QEvent::Shortcut) {
            key = static_cast<QShortcutEvent*>(event)->key();
        }
        else {
            key = static_cast<QKeyEvent*>(event)->key();
        }

        if(key == QKeySequence(Qt::Key_Escape)) {
            return false;
        }
        else {
            return true;
        }
    }

    /*!
     * \brief Blocks/unblocks the shortcuts on the QApplication.
     *
     * \param block True blocks while false unblocks the shortcuts.
     * \sa CGraphicsScene::eventFilter
     */
    void CGraphicsScene::blockShortcuts(const bool block)
    {
        if(block) {
            if(!m_shortcutsBlocked) {
                qApp->installEventFilter(this);
                m_shortcutsBlocked = true;
            }
        }
        else {
            if(m_shortcutsBlocked) {
                qApp->removeEventFilter(this);
                m_shortcutsBlocked = false;
            }
        }
    }

    /*!
     * \brief Starts beginPaintingDraw mode.
     *
     * This is the mode which is used while inserting painting items.
     */
    void CGraphicsScene::beginPaintingDraw(Painting *item)
    {
        Q_ASSERT(m_mouseAction == CGraphicsScene::PaintingDrawEvent);
        m_paintingDrawClicks = 0;
        delete m_paintingDrawItem;
        m_paintingDrawItem = item->copy();
    }

    /*!
     * \brief Starts insertItem mode.
     *
     * This is the mode which is used while pasting components or inserting
     * components after selecting it from the sidebar. This initiates the process
     * by filling the internal m_insertibles list whose contents will be moved on
     * mouse events.
     * Meanwhile it also prepares for this process by hiding component's properties
     * which should not be shown while responding to mouse events in this mode.
     *
     * \todo create a insert canedacomponents property in order to avoid ugly cast
     * \todo gpk: why two loop??
     *
     * \note Follow up for the above question:
     * Actually there are 3 loops involved here one encapsulated in centerOfItems
     * method.
     * The first loop prepares the items for insertion by either hiding/showing
     * based on cursor position.
     * Then we have to calculate center of these items with respect to which the
     * items have to be moved. (encapsulated in centerOfItems method)
     * Finally, the third loop actually moves the items.
     * Now the second implicit loop is very much required to run completely as
     * we have to parse each item's bounding rect to calcuate final center.
     * So best approach would be to call centerOfItems first to find delta.
     * Then combine the first and the third loop.
     * Bastein can you look into that ?
     *
     * \note Regarding ugly cast:
     * I think a virtual member function - prepareForInsertion() should be added
     * to CGraphicsItem which does nothing. Then classes like component can specialize
     * this method to do necessary operation like hiding properties.
     * Then in the loop, there is no need for cast. Just call that prepare method
     * on all items.
     */
    void CGraphicsScene::beginInsertingItems(const QList<CGraphicsItem*> &items)
    {
        Q_ASSERT(m_mouseAction == CGraphicsScene::InsertingItems);

        // Delete all previous insertibles
        qDeleteAll(m_insertibles);
        // Add to insert list
        m_insertibles = items;

        // Add items
        foreach(CGraphicsItem *item, m_insertibles) {
            item->setSelected(true);
            // Hide all items here, they are made visible in ::insertingItemsEvent
            item->hide();
            // Replace by item->prepareForInsertion()
            if(item->isComponent()) {
                Component *comp = canedaitem_cast<Component*>(item);
                comp->properties()->hide();
            }
        }
    }

    /*!
     * \brief Set whether this scene is modified or not
     *
     * This method emits the signal modificationChanged(bool) as well
     * as the signal titleToBeUpdated()
     *
     * \param m True/false to set it to unmodified/modified.
     */
    void CGraphicsScene::setModified(const bool m)
    {
        if(m_modified != !m) {
            m_modified = !m;
            emit changed();
        }
    }

    /*!
     * \brief Draw background of scene including grid
     *
     * \param painter: Where to draw
     * \param rect: Visible area
     * \todo Finish visual representation
     */
    void CGraphicsScene::drawBackground(QPainter *painter, const QRectF& rect)
    {
        QPen savedpen = painter->pen();

        // Disable anti aliasing
        painter->setRenderHint(QPainter::Antialiasing, false);

        if(isBackgroundVisible()) {
            const QColor backgroundColor =
                Settings::instance()->currentValue("gui/backgroundColor").value<QColor>();
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(backgroundColor));
            painter->drawRect(rect);
        }

        // Configure pen
        const QColor foregroundColor =
            Settings::instance()->currentValue("gui/foregroundColor").value<QColor>();
        painter->setPen(QPen(foregroundColor, 0));
        painter->setBrush(Qt::NoBrush);


        const QPointF origin(0, 0);

        // Draw origin
        if(rect.contains(origin)) {
            painter->drawLine(QLineF(origin.x() - 3.0, origin.y(),
                        origin.x() + 3.0, origin.y()));
            painter->drawLine(QLineF(origin.x(), origin.y() - 3.0,
                        origin.x(), origin.y() + 3.0));
        }

        // Draw grid
        if(Settings::instance()->currentValue("gui/gridVisible").value<bool>()) {

            int drawingGridWidth = Caneda::GRID_SPACE;
            int drawingGridHeight = Caneda::GRID_SPACE;

            //Make grid size display dinamic, depending on zoom level
            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *v = manager->currentView();
            if(!v) {
                return;
            }

            if(v->currentZoom() < 1) {
                // While drawing, choose spacing to be multiple times the actual grid size.
                if(v->currentZoom() > 0.5) {
                    drawingGridWidth *= 4;
                    drawingGridHeight *= 4;
                }
                else {
                    drawingGridWidth *= 16;
                    drawingGridHeight *= 16;
                }
            }

            // Extrema grid points
            qreal left = int(rect.left()) - (int(rect.left()) % drawingGridWidth);
            qreal top = int(rect.top()) - (int(rect.top()) % drawingGridHeight);
            qreal right = int(rect.right()) - (int(rect.right()) % drawingGridWidth);
            qreal bottom = int(rect.bottom()) - (int(rect.bottom()) % drawingGridHeight);
            qreal x, y;

            // Draw grid
            painter->setBrush(Qt::NoBrush);
            for(x = left; x <= right; x += drawingGridWidth) {
                for(y = top; y <=bottom; y += drawingGridHeight) {
                    painter->drawPoint(QPointF(x, y));
                }
            }
        }

        // Restore painter
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(savedpen);
    }

    /*!
     * Handle some events at lower level. This callback is called before the
     * specialized event handler methods (like mousePressEvent) are called.
     *
     * Here this callback is mainly reimplemented to handle the QEvent::Enter and
     * QEvent::Leave event while the current mouse actions is InsertingItems.
     * When the mouse cursor goes out of the scene, this hides the items to be inserted
     * and the items are shown back once the cursor enters the scene.
     * This actually is used to optimize by not causing much changes on scene when
     * cursor is moved outside the scene.
     * Hint: Hidden items don't result in any changes to the scene's states.
     */
    bool CGraphicsScene::event(QEvent *event)
    {
        static int ii = 0;
        if(m_mouseAction == InsertingItems) {
            if(event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
                bool visible = (event->type() == QEvent::Enter);
                foreach(CGraphicsItem *item, m_insertibles) {
                    item->setVisible(visible);
                }
            }
        }

        return QGraphicsScene::event(event);
    }

    /*!
     * \brief Constructs and returns a menu with default actions inserted.
     */
    void CGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        if(m_mouseAction == Normal) {
            ActionManager* am = ActionManager::instance();
            QMenu *_menu = new QMenu();

            switch(selectedItems().size()) {
            case 0:
                //launch a general menu
                _menu->addAction(am->actionForName("select"));
                _menu->addAction(am->actionForName("insWire"));
                _menu->addAction(am->actionForName("insLabel"));
                _menu->addAction(am->actionForName("insGround"));
                _menu->addAction(am->actionForName("editDelete"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("editPaste"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("symEdit"));
                _menu->addAction(am->actionForName("intoH"));
                _menu->addAction(am->actionForName("popH"));

                _menu->exec(event->screenPos());
                break;

            case 1:
                //launch context menu of item.
                QGraphicsScene::contextMenuEvent(event);
                break;

            default:
                //launch a common menu
                _menu->addAction(am->actionForName("editCut"));
                _menu->addAction(am->actionForName("editCopy"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("editRotate"));
                _menu->addAction(am->actionForName("editMirror"));
                _menu->addAction(am->actionForName("editMirrorY"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("editDelete"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("centerHor"));
                _menu->addAction(am->actionForName("centerVert"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("alignTop"));
                _menu->addAction(am->actionForName("alignBottom"));
                _menu->addAction(am->actionForName("alignLeft"));
                _menu->addAction(am->actionForName("alignRight"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("distrHor"));
                _menu->addAction(am->actionForName("distrVert"));

                _menu->exec(event->screenPos());
            }
        }
    }

    /*!
     * \brief Event handler, for event drag enter event
     *
     * Drag enter events are generated as the cursor enters the item's area.
     * Accept event from sidebar
     * \param event event to be accepted
     */
    void CGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
    {
        if(event->mimeData()->formats().contains("application/caneda.sidebarItem")) {
            event->acceptProposedAction();
            blockShortcuts(true);
        }
        else {
            event->ignore();
        }
    }

    /*!
     * \brief Event handler, for event drag move event
     *
     * Drag move events are generated as the cursor moves around inside the item's area.
     * It is used to indicate that only parts of the item can accept drops.
     * Accept event from sidebar
     * \param event event to be accepted
     */
    void CGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
    {
        if(event->mimeData()->formats().contains("application/caneda.sidebarItem")) {
            event->acceptProposedAction();
        }
        else {
            event->ignore();
        }
    }

    /*!
     * \brief Event handler, for event drop event
     *
     * Receive drop events for CGraphicsScene
     * Items can only receive drop events if the last drag move event was accepted
     * Accept event only from sidebar
     *
     * \param event event to be accepted
     * \todo factorize
     */
    void CGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent * event)
    {
        if(event->mimeData()->formats().contains("application/caneda.sidebarItem")) {
            event->accept();

            QByteArray encodedData = event->mimeData()->data("application/caneda.sidebarItem");
            QDataStream stream(&encodedData, QIODevice::ReadOnly);
            QString item, category;
            stream >> item >> category;
            CGraphicsItem *qItem = itemForName(item, category);
            /* XXX: extract and factorize */
            if(qItem->type() == GraphicText::Type) {
                GraphicTextDialog dialog(0, Caneda::DontPushUndoCmd);
                if(dialog.exec() == QDialog::Accepted) {
                    GraphicText *textItem = static_cast<GraphicText*>(qItem);
                    textItem->setRichText(dialog.richText());
                }
                else {
                    delete qItem;
                    return;
                }
            }
            if(qItem) {
                QPointF dest = smartNearingGridPoint(event->scenePos());

                placeItem(qItem, dest, Caneda::PushUndoCmd);
                event->acceptProposedAction();
            }
        }
        else {
            event->ignore();
        }

        blockShortcuts(false);
    }

    /*!
     * \brief Event called when mouse is pressed
     */
    void CGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
    {
        lastPos = smartNearingGridPoint(e->scenePos());

        // This is not to lose grid snaping when moving objects
        e->setScenePos(lastPos);
        e->setPos(lastPos);

        sendMouseActionEvent(e);
    }

    /*!
     * \brief Mouse move event
     */
    void CGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
    {
        QPointF point = smartNearingGridPoint(e->scenePos());
        if(point == lastPos) {
            e->accept();
            return;
        }

        // Implement grid snap by changing event parameters with new grid position
        e->setScenePos(point);
        e->setPos(point);
        e->setLastScenePos(lastPos);
        e->setLastPos(lastPos);

        // Now cache this point for next move
        lastPos = point;

        sendMouseActionEvent(e);
    }

    /*!
     * \brief Release mouse event
     */
    void CGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
    {
        sendMouseActionEvent(e);
    }

    /*!
     * \brief Mouse double click event
     *
     * Encapsulates the mouseDoubleClickEvent as one of MouseAction and calls
     * corresponding callback.
     */
    void CGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
    {
        sendMouseActionEvent(e);
    }

    void CGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *e)
    {
        QGraphicsView *v = static_cast<QGraphicsView *>(e->widget()->parent());
        CGraphicsView *sv = qobject_cast<CGraphicsView*>(v);
        if(!sv) {
            return;
        }

        if(e->modifiers() & Qt::ControlModifier){
            if(e->delta() > 0) {
                QPoint viewPoint = sv->mapFromScene(e->scenePos());

                sv->zoomIn();

                QPointF afterScalePoint(sv->mapFromScene(e->scenePos()));
                int dx = (afterScalePoint - viewPoint).toPoint().x();
                int dy = (afterScalePoint - viewPoint).toPoint().y();

                sv->translate(-dx,-dy);
            }
            else {
                sv->zoomOut();
            }
        }
        else if(e->modifiers() & Qt::ShiftModifier){
            if(e->delta() > 0) {
                sv->translate(-50,0);
            }
            else {
                sv->translate(50,0);
            }
        }
        else{
            if(e->delta() > 0) {
                sv->translate(0,50);
            }
            else {
                sv->translate(0,-50);
            }
        }

        e->accept();
    }

    /*!
     * \brief Call the appropriate mouseAction event based on the current mouse action
     */
    void CGraphicsScene::sendMouseActionEvent(MouseActionEvent *e)
    {
        switch(m_mouseAction) {
            case Wiring:
                wiringEvent(e);
                break;

            case Deleting:
                deletingEvent(e);
                break;

            case Marking:
                markingEvent(e);
                break;

            case Rotating:
                rotatingEvent(e);
                break;

            case MirroringX:
                mirroringXEvent(e);
                break;

            case MirroringY:
                mirroringYEvent(e);
                break;

            case ZoomingAreaEvent:
                zoomingAreaEvent(e);
                break;

            case PaintingDrawEvent:
                paintingDrawEvent(e);
                break;

            case InsertingItems:
                insertingItemsEvent(e);
                break;

            case InsertingWireLabel:
                insertingWireLabelEvent(e);
                break;

            case Normal:
                normalEvent(e);
                break;

            default:;
        };
    }

    /*!
     * \brief Reset the state
     *
     * This callback is called when for instance you press esc key
     */
    void CGraphicsScene::resetState()
    {
        // Clear focus on any item on this scene.
        setFocusItem(0);
        // Clear selection.
        clearSelection();

        // Clear the list holding items to be pasted/placed on graphics scene.
        qDeleteAll(m_insertibles);
        m_insertibles.clear();

        // If current state is wiring, delete last attempt
        if(m_wiringState == SINGLETON_WIRE){
            Q_ASSERT(m_currentWiringWire != NULL);
            delete m_currentWiringWire;
            m_wiringState = NO_WIRE;
        }

        // Reset drawing item
        delete m_paintingDrawItem;
        m_paintingDrawItem = 0;
        m_paintingDrawClicks = 0;

        // Clear zoom
        m_zoomRect = QRectF();
        m_zoomBand->hide();
        m_zoomBandClicks = 0;
    }

    /*********************************************************************
     *
     *            WIRING
     *
     *********************************************************************/
    //! \brief Wiring event
    void CGraphicsScene::wiringEvent(MouseActionEvent *event)
    {
        QPointF pos = smartNearingGridPoint(event->scenePos());

        // Press mouse event
        if(event->type() == QEvent::GraphicsSceneMousePress)  {
            return wiringEventMouseClick(event, pos);
        }
        // Move mouse event
        else if(event->type() == QEvent::GraphicsSceneMouseMove)  {
            return wiringEventMouseMove(pos);
        }
    }

    /*!
     * \brief Mouse click wire event
     *
     * \param Event: mouse event
     * \param pos: coordinate of mouse action point
     */
    void CGraphicsScene::wiringEventMouseClick(const MouseActionEvent *event, const QPointF &pos)
    {
        // Left click - Add control point
        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton)  {
            return wiringEventLeftMouseClick(pos);
        }
        // Right click - Finish wire
        if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            return wiringEventRightMouseClick();
        }
    }

    /*!
     * \brief Left mouse click wire event
     *
     * \param pos: coordinate of mouse action point
     */
    void CGraphicsScene::wiringEventLeftMouseClick(const QPointF &pos)
    {
        if(m_wiringState == NO_WIRE) {
            // Create a new wire
            m_currentWiringWire = new Wire(pos, pos, this);
            m_wiringState = SINGLETON_WIRE;
            return;
        }

        if(m_wiringState == SINGLETON_WIRE) {
            // Check if port 1 and 2 overlap
            if(m_currentWiringWire->isNull())  {
                return;
            }

            // Connect ports to any coinciding port in the scene
            m_currentWiringWire->checkAndConnect(Caneda::PushUndoCmd);

            if(m_currentWiringWire->port2()->hasConnection()) {
                // If a connection was made, detach current wire and finalize
                m_currentWiringWire = NULL;
                m_wiringState = NO_WIRE;
            }
            else  {
                // Add a wire segment
                QPointF refPos = m_currentWiringWire->port2()->pos() + m_currentWiringWire->pos();
                m_currentWiringWire = new Wire(refPos, refPos, this);
            }

            return;
        }

    }

    //! \brief Right mouse click wire event, ie finish wire event
    void CGraphicsScene::wiringEventRightMouseClick()
    {
        if(m_wiringState ==  SINGLETON_WIRE) {
            // Check if port 1 and 2 overlap
            if(m_currentWiringWire->isNull()) {
                return;
            }

            m_currentWiringWire->checkAndConnect(Caneda::PushUndoCmd);

            // Detach current wire and finalize
            m_currentWiringWire = NULL;
            m_wiringState = NO_WIRE;

            return;
        }
    }

    /*!
     * \brief Mouse move wire event
     *
     * \param pos: coordinate of mouse action point
     */
    void CGraphicsScene::wiringEventMouseMove(const QPointF &pos)
    {
        if(m_wiringState != NO_WIRE) {
            QPointF newPos = m_currentWiringWire->mapFromScene(pos);
            QPointF refPos = m_currentWiringWire->port1()->pos();

            if( abs(refPos.x()-newPos.x()) > abs(refPos.y()-newPos.y()) ) {
                m_currentWiringWire->movePort2(QPointF(newPos.x(), refPos.y()));
            }
            else {
                m_currentWiringWire->movePort2(QPointF(refPos.x(), newPos.y()));
            }

        }
    }

    /*************************************************************
     *
     *          DELETING
     *
     *************************************************************/
    /*!
     * \brief Delete action
     *
     * Delete action: left click delete, right click disconnect item
     */
    void CGraphicsScene::deletingEvent(const MouseActionEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        QPointF pos = event->scenePos();
        /* left click */
        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) {
            return deletingEventLeftMouseClick(pos);
        }
        /* right click */
        if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            return deletingEventRightMouseClick(pos);
        }
        return;
    }

    /*!
     * \brief Left button deleting event: delete items
     *
     * \param pos: pos clicked
     */
    void CGraphicsScene::deletingEventLeftMouseClick(const QPointF &pos)
    {
        /* create a list of items */
        QList<QGraphicsItem*> _list = items(pos);
        if(!_list.isEmpty()) {
            QList<CGraphicsItem*> _items = filterItems<CGraphicsItem>(_list);

            if(!_items.isEmpty()) {
                deleteItems(QList<CGraphicsItem*>() << _items.first(), Caneda::PushUndoCmd);
            }
        }
    }

    /*!
     * \brief Left button deleting event: delete items
     *
     * \param pos: pos clicked
     */
    void CGraphicsScene::deletingEventRightMouseClick(const QPointF &pos)
    {
        /* create a list of items */
        QList<QGraphicsItem*> _list = items(pos);
        if(!_list.isEmpty()) {
            QList<CGraphicsItem*> _items = filterItems<CGraphicsItem>(_list);

            if(!_items.isEmpty()) {
                disconnectItems(QList<CGraphicsItem*>() << _items.first(), Caneda::PushUndoCmd);
            }
        }
    }

    /******************************************************************
     *
     *                   Rotate Event
     *
     *****************************************************************/
    /*!
     * \brief Rotate item
     * \note right anticlockwise
     */
    void CGraphicsScene::rotatingEvent(MouseActionEvent *event)
    {
        Caneda::AngleDirection angle;

        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        // left == clock wise
        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) {
            angle = Caneda::Clockwise;
        }
        // right == anticlock wise
        else if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            angle = Caneda::AntiClockwise;
        }
        // Avoid angle unitialized
        else {
            return;
        }

        // Get items
        QList<QGraphicsItem*> _list = items(event->scenePos());
        // Filter item
        QList<CGraphicsItem*> qItems = filterItems<CGraphicsItem>(_list, DontRemoveItems);
        if(!qItems.isEmpty()) {
            rotateItems(QList<CGraphicsItem*>() << qItems.first(), angle, Caneda::PushUndoCmd);
        }
    }

    /*!
     * \brief Rotate an item list
     *
     * \param items: item list
     * \param opt: undo option
     * \param diect: is rotation in trigonometric sense
     * \todo Create a custom undo class for avoiding if
     */
    void CGraphicsScene::rotateItems(QList<CGraphicsItem*> &items,
            const Caneda::AngleDirection dir,
            const Caneda::UndoOption opt)
    {
        // Setup undo
        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->beginMacro(dir == Caneda::Clockwise ?
                    QString("Rotate Clockwise") :
                    QString("Rotate Anti-Clockwise"));
        }

        // Disconnect
        disconnectItems(items, opt);

        // Rotate
        RotateItemsCmd *cmd = new RotateItemsCmd(items, dir);
        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->push(cmd);
        }
        else {
            cmd->redo();
            delete cmd;
        }

        // Reconnect
        connectItems(items, opt);

        // Finish undo
        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->endMacro();
        }
    }

    /******************************************************************
     *
     *  Zooming Area Event
     *
     *****************************************************************/
    /*!
     * \brief Zoom in event handles zooming of the view based on mouse signals.
     *
     * If just a point is clicked(mouse press + release) then, an ordinary zoomIn
     * is done (similar to selecting from menu)
     *
     * On the otherhand if mouse is pressed and dragged and then release,
     * corresponding feedback (zoom band) is shown which indiates area that will
     * be zoomed. On mouse release, the area (rect) selected is zoomed.
     */
    void CGraphicsScene::zoomingAreaEvent(MouseActionEvent *event)
    {
        QGraphicsView *view = static_cast<QGraphicsView *>(event->widget()->parent());
        CGraphicsView *cView = qobject_cast<CGraphicsView*>(view);
        if(!cView) {
            return;
        }

        QPointF dest = smartNearingGridPoint(event->scenePos());

        if(event->type() == QEvent::GraphicsSceneMousePress) {
            clearSelection();
            ++m_zoomBandClicks;

            // This is the generic case
            if(m_zoomBandClicks == 1) {
                m_zoomRect.setRect(dest.x(), dest.y(), 0, 0);
                m_zoomBand->setRect(m_zoomRect.normalized());
                m_zoomBand->show();
            }
            else {
                m_zoomBandClicks = 0;
                m_zoomBand->hide();
                cView->zoomFitRect(m_zoomRect.normalized());
                m_zoomRect.setRect(0, 0, 0, 0);
            }
        }

        else if(event->type() == QEvent::GraphicsSceneMouseMove) {
            if(m_zoomBandClicks == 1) {
                m_zoomRect.setBottomRight(dest);
                m_zoomBand->setRect(m_zoomRect.normalized());
            }
        }
    }

    /******************************************************************
     *
     *                   Other Events
     *
     *****************************************************************/
    //! \todo document
    void CGraphicsScene::markingEvent(MouseActionEvent *event)
    {
        Q_UNUSED(event);
        //TODO:
    }

    void CGraphicsScene::paintingDrawEvent(MouseActionEvent *event)
    {
        if(!m_paintingDrawItem) {
            return;
        }

        EllipseArc *arc = 0;
        GraphicText *text = 0;
        QPointF dest = event->scenePos();
        dest += m_paintingDrawItem->paintingRect().topLeft();
        dest = smartNearingGridPoint(dest);

        if(m_paintingDrawItem->type() == EllipseArc::Type) {
            arc = static_cast<EllipseArc*>(m_paintingDrawItem);
        }

        if(m_paintingDrawItem->type() == GraphicText::Type) {
            text = static_cast<GraphicText*>(m_paintingDrawItem);
        }


        if(event->type() == QEvent::GraphicsSceneMousePress) {
            clearSelection();
            ++m_paintingDrawClicks;

            // First handle special painting items
            if(arc && m_paintingDrawClicks < 4) {
                if(m_paintingDrawClicks == 1) {
                    arc->setStartAngle(0);
                    arc->setSpanAngle(360);
                    arc->setPos(dest);
                    addItem(arc);
                }
                else if(m_paintingDrawClicks == 2) {
                    arc->setSpanAngle(180);
                }
                return;
            }
            else if(text) {
                Q_ASSERT(m_paintingDrawClicks == 1);
                text->setPos(dest);
                int result = text->launchPropertyDialog(Caneda::DontPushUndoCmd);
                if(result == QDialog::Accepted) {
                    placeAndDuplicatePainting();
                }

                // this means the text is set through the dialog.
                m_paintingDrawClicks = 0;
                return;
            }

            // This is generic case
            if(m_paintingDrawClicks == 1) {
                m_paintingDrawItem->setPos(dest);
                addItem(m_paintingDrawItem);
            }
            else {
                m_paintingDrawClicks = 0;
                placeAndDuplicatePainting();
            }
        }

        else if(event->type() == QEvent::GraphicsSceneMouseMove) {
            if(arc && m_paintingDrawClicks > 1) {
                QPointF delta = event->scenePos() - arc->scenePos();
                int angle = int(180/M_PI * std::atan2(-delta.y(), delta.x()));

                if(m_paintingDrawClicks == 2) {
                    while(angle < 0) {
                        angle += 360;
                    }
                    arc->setStartAngle(int(angle));
                }

                else if(m_paintingDrawClicks == 3) {
                    int span = angle - arc->startAngle();
                    while(span < 0) {
                        span += 360;
                    }
                    arc->setSpanAngle(span);
                }
            }

            else if(m_paintingDrawClicks == 1) {
                QRectF rect = m_paintingDrawItem->paintingRect();
                const QPointF gridifiedPos = smartNearingGridPoint(event->scenePos());
                rect.setBottomRight(m_paintingDrawItem->mapFromScene(gridifiedPos));
                m_paintingDrawItem->setPaintingRect(rect);
            }
        }
    }

    /*!
     * \brief This event corresponds to placing/pasting items on scene.
     *
     * When the mouse is moved without pressing, then feed back of all
     * m_insertibles items moving is done here.
     * On mouse press, these items are placed on the scene and a duplicate is
     * retained to support further placing/insertion/paste.
     */
    void CGraphicsScene::insertingItemsEvent(MouseActionEvent *event)
    {
        if(event->type() == QEvent::GraphicsSceneMousePress) {

            if (event->button() == Qt::LeftButton) {

                clearSelection();
                foreach(CGraphicsItem *item, m_insertibles) {
                    removeItem(item);
                }

                m_undoStack->beginMacro(QString("Insert items"));
                foreach(CGraphicsItem *item, m_insertibles) {
                    CGraphicsItem *copied = item->copy(0);
                    placeItem(copied, smartNearingGridPoint(item->pos()), Caneda::PushUndoCmd);
                }
                m_undoStack->endMacro();

                foreach(CGraphicsItem *item, m_insertibles) {
                    addItem(item);
                    item->setSelected(true);
                }

            } else if (event->button() == Qt::RightButton) {

                emit rotateInvokedWhileInserting();
                // HACK: Assuming the above signal is connected to StateHandler
                // through Qt::DirectConnection, all m_insertibles would have been
                // updated with rotated items.  However, beginInsertingItems would have
                // hidden all items, so show them back.
                // I see no point why we would be not using Qt::DirectConenction though!
                QPointF delta = event->scenePos() - centerOfItems(m_insertibles);
                foreach(CGraphicsItem *item, m_insertibles) {
                    item->show();
                    item->setPos(smartNearingGridPoint(item->pos() + delta));
                }

            } else if (event->button() == Qt::MidButton) {

                emit mirrorInvokedWhileInserting();
                // HACK: Same as above!
                QPointF delta = event->scenePos() - centerOfItems(m_insertibles);
                foreach(CGraphicsItem *item, m_insertibles) {
                    item->show();
                    item->setPos(smartNearingGridPoint(item->pos() + delta));
                }

            }

        }
        else if(event->type() == QEvent::GraphicsSceneMouseMove) {

            QPointF delta = event->scenePos() - centerOfItems(m_insertibles);

            foreach(CGraphicsItem *item, m_insertibles) {
                item->show();
                item->setPos(smartNearingGridPoint(item->pos() + delta));
            }

        }
    }

    /*!
     * \brief Here the wireLabel placing is handled. WireLabel should be
     * placed only if the clicked point is wire or node.
     * \todo Implement
     */
    void CGraphicsScene::insertingWireLabelEvent(MouseActionEvent *event)
    {
        Q_UNUSED(event);
        //TODO:
    }

    void CGraphicsScene::placeAndDuplicatePainting()
    {
        if(!m_paintingDrawItem) {
            return;
        }

        QPointF dest = m_paintingDrawItem->pos();
        placeItem(m_paintingDrawItem, dest, Caneda::PushUndoCmd);

        m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
        m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
        if(m_paintingDrawItem->type() == GraphicText::Type) {
            static_cast<GraphicText*>(m_paintingDrawItem)->setText("");
        }
    }

    /******************************************************************
     *
     *                   Moving Events
     *
     *****************************************************************/
    /*!
     * \brief Handle events other than the specilized mouse actions.
     *
     * This involves moving items in a special way so that wires
     * are created if a connected component is moved away from
     * an unselected component.
     */
    void CGraphicsScene::normalEvent(MouseActionEvent *e)
    {
        switch(e->type()) {
            case QEvent::GraphicsSceneMousePress:
                {
                    QGraphicsScene::mousePressEvent(e);
                    processForSpecialMove(selectedItems());
                }
                break;

            case QEvent::GraphicsSceneMouseMove:
                {
                    if(!m_areItemsMoving) {
                        if(e->buttons() & Qt::LeftButton && !selectedItems().isEmpty()) {
                            m_areItemsMoving = true;
                            m_undoStack->beginMacro(QString("Move items"));
                        }
                        else {
                            return;
                        }
                    }

                    disconnectDisconnectibles();
                    QGraphicsScene::mouseMoveEvent(e);
                    specialMove();
                }
                break;

            case QEvent::GraphicsSceneMouseRelease:
                {
                    if(m_areItemsMoving) {
                        m_areItemsMoving = false;
                        endSpecialMove();
                        m_undoStack->endMacro();
                    }
                    QGraphicsScene::mouseReleaseEvent(e);
                }
                break;

            case QEvent::GraphicsSceneMouseDoubleClick:
                QGraphicsScene::mouseDoubleClickEvent(e);
                break;

            default:
                qDebug("CGraphicsScene::normalEvent() :  Unknown event type");
        };
    }

    /*!
     * \brief Check which items should be moved in a special way
     *        and where there are possible wirable nodes.
     */
    void CGraphicsScene::processForSpecialMove(QList<QGraphicsItem*> _items)
    {
        disconnectibles.clear();
        movingWires.clear();

        foreach(QGraphicsItem *item, _items) {
            // Save item's position for later use
            storePos(item, smartNearingGridPoint(item->scenePos()));

            CGraphicsItem *_item = canedaitem_cast<CGraphicsItem*>(item);
            if(_item) {
                // Check for disconnections and wire resizing
                foreach(Port *port, _item->ports()) {

                    QList<Port*> *connections = port->connections();
                    if(connections) {

                        foreach(Port *other, *connections) {

                            // The item connected is a component
                            Component *otherComponent = other->owner()->component();
                            // Determine whether the ports "other" and "port" should
                            // be disconnected.
                            if(otherComponent && !otherComponent->isSelected()) {
                                disconnectibles << _item;
                            }

                            // The item connected is a wire
                            Wire *wire = other->owner()->wire();
                            // Determine whether this wire should be resized or
                            // moved.
                            if(wire && !wire->isSelected()) {
                                movingWires << wire;
                            }

                        }

                    }
                }
            }

        }
    }

    /*!
     * \brief Disconnect the ports in the disconnectibles list. This happens when two
     * (or more) components are connected and one of them is clicked and dragged.
     */
    void CGraphicsScene::disconnectDisconnectibles()
    {
        QSet<CGraphicsItem*> remove;

        foreach(CGraphicsItem *_item, disconnectibles) {

            int disconnections = 0;
            foreach(Port *port, _item->ports()) {

                if(!port->connections()) {
                    continue;
                }

                foreach(Port *other, *port->connections()) {
                    if(other->owner()->component() && other->owner()->component() != _item &&
                            !other->owner()->component()->isSelected()) {

                        m_undoStack->push(new DisconnectCmd(port, other));
                        ++disconnections;

                        break;
                    }
                }
            }

            if(disconnections) {
                remove << _item;
            }
        }

        foreach(CGraphicsItem *_item, remove)
            disconnectibles.removeAll(_item);
    }

    /*!
     * \brief Move the selected items in a special way to allow proper wire movements
     * and as well as checking for possible disconnections.
     */
    void CGraphicsScene::specialMove()
    {
        foreach(Wire *wire, movingWires) {
            
            wire->storeState();

            if(wire->port1()->connections()) {
                foreach(Port *other, *(wire->port1()->connections())) {
                    if(other != wire->port1()) {
                        wire->movePort(wire->port1()->connections(), other->scenePos());
                        break;
                    }
                }
            }

            if(wire->port2()->connections()) {
                foreach(Port *other, *(wire->port2()->connections())) {
                    if(other != wire->port2()) {
                        wire->movePort(wire->port2()->connections(), other->scenePos());
                        break;
                    }
                }
            }

        }
    }

    /*!
     * \brief End the special move by pushing the UndoCommands for position change
     * of items on scene. Also wire's segements are finalized here.
     */
    void CGraphicsScene::endSpecialMove()
    {
        foreach(QGraphicsItem *item, selectedItems()) {

            m_undoStack->push(new MoveCmd(item, storedPos(item),
                        smartNearingGridPoint(item->scenePos())));


            CGraphicsItem * m_item = canedaitem_cast<CGraphicsItem*>(item);
            if(m_item) {
                m_item->checkAndConnect(Caneda::PushUndoCmd);
            }

        }

        foreach(Wire *wire, movingWires) {
            m_undoStack->push(new WireStateChangeCmd(wire, wire->storedState(),
                        wire->currentState()));
        }

        movingWires.clear();
        disconnectibles.clear();
    }

    /******************************************************************************
     *
     *          Sidebar
     *
     *****************************************************************************/
    /*!
     * \brief This function is called when a side bar item is clicked
     *
     * \param itemName: name of item
     * \param category: categoy name
     * \todo Add tracing
     */
    bool CGraphicsScene::sidebarItemClicked(const QString& itemName, const QString& category)
    {
        if(itemName.isEmpty()) {
            return false;
        }

        if(category == "Paint Tools" || category == "Layout Tools") {
            return sidebarItemClickedPaintingsItems(itemName);
        }
        else {
            return sidebarItemClickedNormalItems(itemName, category);
        }
    }

    /*!
     * \brief Action when a painting item is selected
     *
     * \param itemName: name of item
     */
    bool CGraphicsScene::sidebarItemClickedPaintingsItems(const QString& itemName)
    {
        setMouseAction(PaintingDrawEvent);
        m_paintingDrawItem = Painting::fromName(itemName);
        if(!m_paintingDrawItem) {
            setMouseAction(Normal);
            return false;
        }
        m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
        return true;
    }

    bool CGraphicsScene::sidebarItemClickedNormalItems(const QString& itemName, const QString& category)
    {
        CGraphicsItem *item = itemForName(itemName, category);
        if(!item) {
            return false;
        }

        addItem(item);
        setMouseAction(InsertingItems);
        beginInsertingItems(QList<CGraphicsItem*>() << item);

        return true;
    }

    /**********************************************************************
     *
     *                           Place item
     *
     **********************************************************************/
    /*!
     * \brief Place an item on the scene
     *
     * \param item: item to place
     * \param: pos position of the item
     * \param opt: undo option
     * \warning: pos is not rounded (grid snapping)
     */
    void CGraphicsScene::placeItem(CGraphicsItem *item, const QPointF &pos, const Caneda::UndoOption opt)
    {
        if(item->scene() == this) {
            removeItem(item);
        }

        if(item->isComponent()) {
            Component *component = canedaitem_cast<Component*>(item);

            int labelSuffix = componentLabelSuffix(component->labelPrefix());
            QString label = QString("%1%2").
                arg(component->labelPrefix()).
                arg(labelSuffix);

            component->setLabel(label);
        }

        if(opt == Caneda::DontPushUndoCmd) {
            addItem(item);
            item->setPos(pos);
            item->checkAndConnect(opt);
        }
        else {
            m_undoStack->beginMacro(QString("Place item"));
            m_undoStack->push(new InsertItemCmd(item, this, pos));

            item->checkAndConnect(opt);

            m_undoStack->endMacro();
        }
    }

    /*!
     * \brief Return a component or painting based on \a name and \a category.
     *
     * The painting is processed in a special hard coded way(no library)
     * On the other hand components are loaded from the library.
     */
    CGraphicsItem* CGraphicsScene::itemForName(const QString& name, const QString& category)
    {
        if(category == QObject::tr("Paint Tools") || category == QObject::tr("Layout Tools")) {
            return Painting::fromName(name);
        }

        return LibraryManager::instance()->newComponent(name, 0, category);
    }

    /*!
     * \brief Returns an appropriate label suffix as 1 and 2 in R1, R2
     *
     * This method walks through all the items on the scene matching the
     * labelprefix and uses the highest of these suffixes + 1 as the new
     * suffix candidate.
     */
    int CGraphicsScene::componentLabelSuffix(const QString& prefix) const
    {
        int _max = 1;

        foreach(QGraphicsItem *item, items()) {
            Component *comp = canedaitem_cast<Component*>(item);
            if(comp && comp->labelPrefix() == prefix) {
                bool ok;
                int suffix = comp->labelSuffix().toInt(&ok);
                if(ok) {
                    _max = qMax(_max, suffix+1);
                }
            }
        }

        return _max;
    }

    /*!
     * \brief Calculates the center of the items given as a parameter.
     *
     * It actually unites the boundingRect of the items sent as parameters
     * and then returns the center of the united rectangle. This center is
     * used as reference point while copy/paste/inserting items on the scene.
     *
     * \param items The items which geometrical center has to be calculated.
     * \return Returns the geometrical center of the items.
     */
    QPointF CGraphicsScene::centerOfItems(const QList<CGraphicsItem*> &items)
    {
        QRectF rect = items.isEmpty() ? QRectF() :
            items.first()->sceneBoundingRect();

        foreach(CGraphicsItem *item, items) {
            rect |= item->sceneBoundingRect();
        }

        return rect.center();
    }

    /**********************************************************************
     *
     *                           Mirror
     *
     **********************************************************************/
    /*!
     * \brief Mirror event
     *
     * \param event: event
     * \param axis: mirror axis
     */
    void CGraphicsScene::mirroringEvent(const MouseActionEvent *event,
            const Qt::Axis axis)
    {
        /* select item */
        QList<QGraphicsItem*> _list = items(event->scenePos());
        /* filters item */
        QList<CGraphicsItem*> qItems = filterItems<CGraphicsItem>(_list, DontRemoveItems);
        if(!qItems.isEmpty()) {
            /* mirror */
            mirrorItems(QList<CGraphicsItem*>() << qItems.first(), Caneda::PushUndoCmd, axis);
        }
    }

    /*!
     * \brief Mirror X event
     * \note right button mirror Y
     */
    void CGraphicsScene::mirroringXEvent(const MouseActionEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) {
            return mirroringEvent(event, Qt::XAxis);
        }
        if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            return mirroringEvent(event, Qt::YAxis);
        }

        return;
    }

    /*!
     * \brief Mirror Y event
     * \note right button mirror X
     */
    void CGraphicsScene::mirroringYEvent(const MouseActionEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) {
            return mirroringEvent(event, Qt::YAxis);
        }
        if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            return mirroringEvent(event, Qt::XAxis);
        }

        return;
    }

    /**********************************************************************
     *
     *                           Distribute elements
     *
     **********************************************************************/
    //! \brief Short function for qsort sort by abscissa
    static inline bool pointCmpFunction_X(const CGraphicsItem *lhs, const CGraphicsItem  *rhs)
    {
        return lhs->pos().x() < rhs->pos().x();
    }

    //!Short function for qsort sort by abscissa
    static inline bool pointCmpFunction_Y(const CGraphicsItem *lhs, const CGraphicsItem  *rhs)
    {
        return lhs->pos().y() < rhs->pos().y();
    }

    /*!
     * \brief Distribute horizontally
     *
     * \param items: items to distribute
     * \todo Why not filter wire ??
     * +     * Ans: Because wires need special treatment. Wire's don't have single
     * +     * x and y coord (think of several segments of wires which form single
     * +     * Wire object)
     * +     * Therefore distribution needs separate check for segments which make it
     * +     * hard now. We should come out with some good solution for this.
     * +     * Bastein: Do you have any solution ?
     */
    void CGraphicsScene::distributeElementsHorizontally(QList<CGraphicsItem*> items)
    {
        qreal x1, x2, x, dx;
        QPointF newPos;

        /* undo */
        m_undoStack->beginMacro("Distribute horizontally");

        /* disconnect */
        disconnectItems(items, Caneda::PushUndoCmd);

        /*sort item */
        qSort(items.begin(), items.end(), pointCmpFunction_X);
        x1 = items.first()->pos().x();
        x2 = items.last()->pos().x();

        /* compute step */
        dx = (x2 - x1) / (items.size() - 1);
        x = x1;

        foreach(CGraphicsItem *item, items) {
            /* why not filter wire ??? */
            if(item->isWire()) {
                continue;
            }

            /* compute new position */
            newPos = item->pos();
            newPos.setX(x);
            x += dx;

            /* move to new pos */
            m_undoStack->push(new MoveCmd(item, item->pos(), newPos));
        }

        /* try to reconnect */
        connectItems(items, Caneda::PushUndoCmd);

        /* end command */
        m_undoStack->endMacro();

    }

    /*!
     * \brief Distribute vertically
     *
     * \param items: items to distribute
     * \todo Why not filter wire ??
     */
    void CGraphicsScene::distributeElementsVertically(QList<CGraphicsItem*> items)
    {
        qreal y1, y2, y, dy;
        QPointF newPos;

        /* undo */
        m_undoStack->beginMacro("Distribute vertically");

        /* disconnect */
        disconnectItems(items, Caneda::PushUndoCmd);

        /*sort item */
        qSort(items.begin(), items.end(), pointCmpFunction_Y);
        y1 = items.first()->pos().y();
        y2 = items.last()->pos().y();

        /* compute step */
        dy = (y2 - y1) / (items.size() - 1);
        y = y1;

        foreach(CGraphicsItem *item, items) {
            /* why not filter wire ??? */
            if(item->isWire()) {
                continue;
            }

            /* compute new position */
            newPos = item->pos();
            newPos.setY(y);
            y += dy;

            /* move to new pos */
            m_undoStack->push(new MoveCmd(item, item->pos(), newPos));
        }

        /* try to reconnect */
        connectItems(items, Caneda::PushUndoCmd);

        /* end command */
        m_undoStack->endMacro();

    }

    /**********************************************************************
     *
     *                           Misc
     *
     **********************************************************************/
    //! @return A string corresponding to alignement
    const QString CGraphicsScene::Alignment2QString(const Qt::Alignment alignment)
    {
        switch(alignment) {
            case Qt::AlignLeft :
                return tr("Align left");
            case Qt::AlignRight :
                return tr("Align right");
            case Qt::AlignTop :
                return tr("Align top");
            case Qt::AlignBottom :
                return tr("Align bottom");
            case Qt::AlignHCenter :
                return tr("Centers horizontally");
            case Qt::AlignVCenter :
                return tr("Centers vertically");
            case Qt::AlignCenter:
                return tr("Center both vertically and horizontally");
            default:
                return "";
        }
    }

    /*!
     * \brief Automatically connect items if port or wire overlap
     *
     * \param qItems: item to connect
     * \param opt: undo option
     */
    void CGraphicsScene::connectItems(const QList<CGraphicsItem*> &qItems,
            const Caneda::UndoOption opt)
    {
        foreach(CGraphicsItem *qItem, qItems) {
            qItem->checkAndConnect(opt);
        }
    }

    /*!
     * \brief Disconnect an item from wire or other components
     *
     * \param qItems: item to connect
     * \param opt: undo option
     */
    void CGraphicsScene::disconnectItems(const QList<CGraphicsItem*> &qItems,
            const Caneda::UndoOption opt)
    {
        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->beginMacro(QString("Disconnect items"));
        }

        foreach(CGraphicsItem *item, qItems) {
            QList<Port*> ports = item->ports();

            foreach(Port *p, ports) {
                Port *other = p->getAnyConnectedPort();

                // Do not register new undo if nothing to do
                if(other == NULL) {
                    continue;
                }

                if(opt == Caneda::PushUndoCmd) {
                    m_undoStack->push(new DisconnectCmd(p, other));
                }
                else {
                    p->disconnectFrom(other);
                }
            }
        }

        if(opt == Caneda::PushUndoCmd) {
            m_undoStack->endMacro();
        }
    }

} // namespace Caneda
