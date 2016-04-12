/***************************************************************************
 * Copyright (C) 2006 Gopala Krishna A <krishna.ggk@gmail.com>             *
 * Copyright (C) 2008 Bastien Roucaries <roucaries.bastien@gmail.com>      *
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

#include "cgraphicsscene.h"

#include "actionmanager.h"
#include "cgraphicsview.h"
#include "documentviewmanager.h"
#include "ellipsearc.h"
#include "graphictextdialog.h"
#include "idocument.h"
#include "iview.h"
#include "library.h"
#include "portsymbol.h"
#include "property.h"
#include "propertydialog.h"
#include "settings.h"
#include "xmlutilities.h"

#include <QClipboard>
#include <QGraphicsSceneEvent>
#include <QKeySequence>
#include <QMenu>
#include <QMimeData>
#include <QMimeType>
#include <QPainter>
#include <QShortcutEvent>

#include <cmath>

namespace Caneda
{
    /*!
     * \brief Constructs a new graphics scene.
     *
     * \param parent Parent of the scene.
     */
    CGraphicsScene::CGraphicsScene(QObject *parent) :
        QGraphicsScene(QRectF(-2500, -2500, 5000, 5000), parent)
    {
        // Initialize m_mouseAction before anything else to avoid event
        // comparisions with an uninitialized variable.
        m_mouseAction = Normal;

        // Setup spice/electric related scene properties
        m_properties = new PropertyGroup(this);
        m_properties->setUserPropertiesEnabled(true);

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

        connect(undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(setModified(bool)));
    }

    //! \brief Cut items
    void CGraphicsScene::cutItems(QList<CGraphicsItem*> &items)
    {
        copyItems(items);
        deleteItems(items);
    }

    /*!
     * \brief Copy item
     *
     * \todo Document format
     * \todo Use own mime type
     */
    void CGraphicsScene::copyItems(QList<CGraphicsItem*> &_items)
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
     */
    void CGraphicsScene::deleteItems(QList<CGraphicsItem*> &items)
    {
        m_undoStack->beginMacro(QString("Delete items"));
        m_undoStack->push(new RemoveItemsCmd(items, this));
        m_undoStack->endMacro();
    }

    /*!
     * \brief Mirror an item list
     *
     * \param items: item to mirror
     * \param axis: mirror axis
     */
    void CGraphicsScene::mirrorItems(QList<CGraphicsItem*> &items, const Qt::Axis axis)
    {
        m_undoStack->beginMacro(QString("Mirror items"));
        m_undoStack->push(new MirrorItemsCmd(items, axis, this));
        m_undoStack->endMacro();
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
        QList<CGraphicsItem*> items = filterItems<CGraphicsItem>(gItems);

        // Could not align less than two elements
        if(items.size() < 2) {
            return false;
        }

        // Setup undo
        m_undoStack->beginMacro(Alignment2QString(alignment));

        // Disconnect
        disconnectItems(items);

        // Compute bounding rectangle
        QRectF rect = items.first()->sceneBoundingRect();
        QList<CGraphicsItem*>::iterator it = items.begin()+1;
        while(it != items.end()) {
            rect |= (*it)->sceneBoundingRect();
            ++it;
        }

        it = items.begin();
        while(it != items.end()) {
            if((*it)->type() == CGraphicsItem::WireType) {
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
        connectItems(items);
        splitAndCreateNodes(items);

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
     * \brief Prints the current scene to device
     *
     * The device to print the scene on can be a physical printer,
     * a postscript (ps) file or a portable document format (pdf)
     * file.
     */
    void CGraphicsScene::print(QPrinter *printer, bool fitInView)
    {
        QPainter p(printer);
        p.setRenderHints(Caneda::DefaulRenderHints);

        const bool fullPage = printer->fullPage();

        const bool viewGridStatus = Settings::instance()->currentValue("gui/gridVisible").value<bool>();
        Settings::instance()->setCurrentValue("gui/gridVisible", false);

        const QRectF diagramRect = itemsBoundingRect();

        if(fitInView) {
            render(&p, QRectF(), diagramRect, Qt::KeepAspectRatio);
        }
        else {
            //Printing on one or more pages
            QRectF printedArea = fullPage ? printer->paperRect() : printer->pageRect();

            const int horizontalPages =
                int(std::ceil(diagramRect.width() / printedArea.width()));
            const int verticalPages =
                int(std::ceil(diagramRect.height() / printedArea.height()));

            QList<QRectF> pagesToPrint;

            //The schematic is printed on a grid of sheets running from top-bottom, left-right.
            qreal yOffset = 0;
            for(int y = 0; y < verticalPages; ++y) {
                //Runs through the sheets of the line
                qreal xOffset = 0;
                for(int x = 0; x < horizontalPages; ++x) {
                    const qreal width = qMin(printedArea.width(), diagramRect.width() - xOffset);
                    const qreal height = qMin(printedArea.height(), diagramRect.height() - yOffset);
                    pagesToPrint << QRectF(xOffset, yOffset, width, height);
                    xOffset += printedArea.width();
                }

                yOffset += printedArea.height();
            }

            for (int i = 0; i < pagesToPrint.size(); ++i) {
                const QRectF rect = pagesToPrint.at(i);
                render(&p,
                       rect.translated(-rect.topLeft()), // dest - topleft at (0, 0)
                       rect.translated(diagramRect.topLeft()), // src
                       Qt::KeepAspectRatio);

                if(i != (pagesToPrint.size() - 1)) {
                    printer->newPage();
                }
            }
        }

        Settings::instance()->setCurrentValue("gui/gridVisible", viewGridStatus);
    }

    /*!
     * \brief Export the scene to an image.
     *
     * This method exports the scene to a user selected image. This method is
     * used in the ExportDialog class, to generate the image itself into a
     * QPaintDevice. The image will be later saved by the ExportImage class.
     *
     * The image itself can be a raster image (bmp, png, etc) or a vector image
     * (svg). The desired size of the destination (final) image must be set in
     * the QPaintDevice where the image is to be rendered. This size can be a
     * 1:1 ratio or any other size.
     *
     * \param pix QPaintDevice where the image is to be rendered
     * \return bool True on success, false otherwise
     * \sa ExportDialog, IDocument::exportImage()
     */
    bool CGraphicsScene::exportImage(QPaintDevice &pix)
    {
        // Calculate the source area
        QRectF source_area = itemsBoundingRect();

        // Make the source_area a little bit bigger that dest_area to avoid
        // expanding the image due to floating point precision (this is useful
        // in svg images to avoid generating a raster, non-expandable image)
        source_area.setBottom(source_area.bottom()+1);
        source_area.setRight(source_area.right()+1);

        // Calculate the destination area, acording to the user settings
        QRectF dest_area = QRectF(0, 0, pix.width(), pix.height());

        // Prepare the device
        QPainter p;
        if(!p.begin(&pix)) {
            return(false);
        }

        // Deselect the elements
        QList<QGraphicsItem *> selected_elmts = selectedItems();
        foreach(QGraphicsItem *qgi, selected_elmts) {
            qgi->setSelected(false);
        }

        // Perform the rendering itself (without background for svg images)
        // As the size is specified, there is no need to keep the aspect ratio
        // (it will be kept if the dimensions of the source and destination areas
        // are proportional.
        setBackgroundVisible(false);
        render(&p, dest_area, source_area, Qt::IgnoreAspectRatio);
        setBackgroundVisible(true);
        p.end();

        // Restore the selected items
        foreach(QGraphicsItem *qgi, selected_elmts) {
            qgi->setSelected(true);
        }

        return(true);
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
    void CGraphicsScene::setMouseAction(const Caneda::MouseAction action)
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
        //! \todo Implemement this appropriately for all mouse actions
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
        Q_ASSERT(m_mouseAction == Caneda::PaintingDrawEvent);
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
        Q_ASSERT(m_mouseAction == Caneda::InsertingItems);

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
            if(item->type() == CGraphicsItem::ComponentType) {
                Component *comp = canedaitem_cast<Component*>(item);
                comp->properties()->hide();
            }
        }
    }

    /*!
     * \brief Adds a new property to the scene.
     *
     * This method adds a new property to the scene, by calling the
     * PropertyGroup::addProperty() method of the m_properties stored in this
     * scene.
     *
     * \param property New property to add to the PropertyGroup
     * \sa PropertyGroup::addProperty()
     */
    void CGraphicsScene::addProperty(Property property)
    {
        m_properties->addProperty(property.name(), property);
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

        // Draw origin (if visible in the view)
        if(rect.contains(QPointF(0, 0))) {
            painter->drawLine(QLineF(-3.0, 0.0, 3.0, 0.0));
            painter->drawLine(QLineF(0.0, -3.0, 0.0, 3.0));
        }

        // Draw grid
        if(Settings::instance()->currentValue("gui/gridVisible").value<bool>()) {

            int drawingGridWidth = Caneda::DefaultGridSpace;
            int drawingGridHeight = Caneda::DefaultGridSpace;

            //Make grid size display dinamic, depending on zoom level
            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *v = manager->currentView();
            CGraphicsView *sv = qobject_cast<CGraphicsView*>(v->toWidget());

            if(sv) {
                if(sv->currentZoom() < 1) {
                    // While drawing, choose spacing to be multiple times the actual grid size.
                    if(sv->currentZoom() > 0.5) {
                        drawingGridWidth *= 4;
                        drawingGridHeight *= 4;
                    }
                    else {
                        drawingGridWidth *= 16;
                        drawingGridHeight *= 16;
                    }
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
     * \brief Constructs and returns a context menu with the actions
     * corresponding to the selected object.
     */
    void CGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        if(m_mouseAction == Normal) {
            ActionManager* am = ActionManager::instance();
            QMenu *_menu = new QMenu();
            IDocument *document = DocumentViewManager::instance()->currentDocument();

            switch(selectedItems().size()) {
            case 0:
                // Launch the context of the current document
                if (document) {
                    document->contextMenuEvent(event);
                }
                break;

            case 1:
                // Launch the context menu of an item.
                QGraphicsScene::contextMenuEvent(event);
                break;

            default:
                // Launch the context menu of multiple items selected.
                _menu->addAction(am->actionForName("editCut"));
                _menu->addAction(am->actionForName("editCopy"));
                _menu->addAction(am->actionForName("editDelete"));

                _menu->addSeparator();

                _menu->addAction(am->actionForName("editRotate"));
                _menu->addAction(am->actionForName("editMirror"));
                _menu->addAction(am->actionForName("editMirrorY"));

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

                _menu->addSeparator();

                _menu->addAction(am->actionForName("propertiesDialog"));

                _menu->exec(event->screenPos());
            }
        }
    }

    /*!
     * \brief Drag enter event handler.
     *
     * Receive drag enter events in CGraphicsScene. Drag enter events are
     * generated as the cursor enters the scene's area. Items can only perform
     * drop events if the last drag move event was accepted. Accepted events
     * are filtered and only from the sidebar (application/caneda.sidebarItem
     * mime data).
     *
     * This method in conjunction with dragMoveEvent() and dropEvent(), are
     * used to allow drag and dropping items from the sidebar, without the need
     * to enter the insert mode.
     *
     * \param event event to be accepted
     * \sa dropEvent, dragMoveEvent
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
     * \brief Drag move event handler.
     *
     * Receive drag move events in CGraphicsScene. Drag move events are
     * generated as the cursor moves around inside the scene's area. Items can
     * only perform drop events if the last drag move event was accepted. This
     * is acomplished in with this method. Accepted events are filtered and
     * only from the sidebar (application/caneda.sidebarItem mime data).
     *
     * This method in conjunction with dragEnterEvent() and dropEvent(), are
     * used to allow drag and dropping items from the sidebar, without the need
     * to enter the insert mode.
     *
     * \param event event to be accepted
     * \sa dropEvent, dragEnterEvent
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
     * \brief Drop event handler.
     *
     * Receive drop events in CGraphicsScene. Items can only perform drop
     * events if the last drag move event was accepted. Accepted events are
     * filtered and only from the sidebar (application/caneda.sidebarItem mime
     * data).
     *
     * This method in conjunction with dragEnterEvent() and dragMoveEvent(),
     * are used to allow drag and dropping items from the sidebar, without the
     * need to enter the insert mode.
     *
     * \param event event to be accepted
     * \sa dragMoveEvent, dragEnterEvent, StateHandler::slotSidebarItemClicked()
     */
    void CGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent * event)
    {
        if(event->mimeData()->formats().contains("application/caneda.sidebarItem")) {
            event->accept();

            QByteArray encodedData = event->mimeData()->data("application/caneda.sidebarItem");
            QDataStream stream(&encodedData, QIODevice::ReadOnly);
            QString itemName, itemCategory;
            stream >> itemName >> itemCategory;

            // Get a component or painting based on the name and category.
            // The painting is processed in a special hardcoded way (with
            // no libraries involved). Some other "miscellaneous" items
            // are hardcoded too. On the other hand, components are
            // loaded from existing libraries.
            CGraphicsItem *qItem = 0;
            if(itemCategory == QObject::tr("Paint Tools") || itemCategory == QObject::tr("Layout Tools")) {
                qItem = Painting::fromName(itemName);
            }
            else if(itemCategory == QObject::tr("Miscellaneous")) {
                // This must be repeated for each type of miscellaneous item,
                // for example ground, port symbols, etc.
                if(itemName == QObject::tr("Ground")) {
                    qItem = new PortSymbol("Ground", this);
                }
                else if(itemName == QObject::tr("Port Symbol")) {
                    qItem = new PortSymbol(this);
                }
            }

            // If the item was not found in the fixed libraries, search for the
            // item in the dinamic loaded libraries ("Components" category).
            if(!qItem) {
                qItem = LibraryManager::instance()->newComponent(itemName, 0, itemCategory);
            }

            // Check if the item was successfully found and created
            if(qItem) {
                // If the item is a GraphicText item, open a dialog to type the text
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

                // For all item types, place the result in the nearest grid position
                QPointF dest = smartNearingGridPoint(event->scenePos());
                placeItem(qItem, dest);
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
                sv->translate(0,50);
            }
            else {
                sv->translate(0,-50);
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

            sv->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Set transform to zoom into mouse position

            if(e->delta() > 0) {
                sv->zoomIn();
            }
            else {
                sv->zoomOut();
            }

        }

        e->accept();
    }

    /*!
     * \brief Call the appropriate mouseAction event based on the current mouse action
     */
    void CGraphicsScene::sendMouseActionEvent(QGraphicsSceneMouseEvent *e)
    {
        switch(m_mouseAction) {
            case Wiring:
                wiringEvent(e);
                break;

            case Deleting:
                deletingEvent(e);
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
    void CGraphicsScene::wiringEvent(QGraphicsSceneMouseEvent *event)
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
    void CGraphicsScene::wiringEventMouseClick(const QGraphicsSceneMouseEvent *event, const QPointF &pos)
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
            connectItems(m_currentWiringWire);
            splitAndCreateNodes(m_currentWiringWire);

            if(m_currentWiringWire->port2()->hasAnyConnection()) {
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

            connectItems(m_currentWiringWire);
            splitAndCreateNodes(m_currentWiringWire);

            // Detach current wire and finalize
            m_currentWiringWire = NULL;
            m_wiringState = NO_WIRE;

            return;
        }
    }

    /*!
     * \brief Mouse move wire event
     *
     * \param newPos: coordinate of mouse action point
     */
    void CGraphicsScene::wiringEventMouseMove(const QPointF &newPos)
    {
        if(m_wiringState != NO_WIRE) {

            QPointF refPos = m_currentWiringWire->port1()->scenePos();

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
    void CGraphicsScene::deletingEvent(const QGraphicsSceneMouseEvent *event)
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
                deleteItems(QList<CGraphicsItem*>() << _items.first());
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
                disconnectItems(QList<CGraphicsItem*>() << _items.first());
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
    void CGraphicsScene::rotatingEvent(QGraphicsSceneMouseEvent *event)
    {
        Caneda::AngleDirection angle;

        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        // left == clock wise
        if(event->buttons() == Qt::LeftButton) {
            angle = Caneda::Clockwise;
        }
        // right == anticlock wise
        else if(event->buttons() == Qt::RightButton) {
            angle = Caneda::AntiClockwise;
        }
        // Avoid angle unitialized
        else {
            return;
        }

        // Get items
        QList<QGraphicsItem*> _list = items(event->scenePos());
        // Filter item
        QList<CGraphicsItem*> qItems = filterItems<CGraphicsItem>(_list);
        if(!qItems.isEmpty()) {
            rotateItems(QList<CGraphicsItem*>() << qItems.first(), angle);
        }
    }

    /*!
     * \brief Rotate an item list
     *
     * \param items: item list
     * \param dir: rotation direction
     */
    void CGraphicsScene::rotateItems(QList<CGraphicsItem*> &items, const Caneda::AngleDirection dir)
    {
        m_undoStack->beginMacro(QString("Rotate items"));
        m_undoStack->push(new RotateItemsCmd(items, dir, this));
        m_undoStack->endMacro();
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
    void CGraphicsScene::zoomingAreaEvent(QGraphicsSceneMouseEvent *event)
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
    void CGraphicsScene::paintingDrawEvent(QGraphicsSceneMouseEvent *event)
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
                    // Place the text item
                    placeItem(m_paintingDrawItem, dest);

                    // Make an empty copy of the item for the next item insertion
                    m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
                    m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
                    static_cast<GraphicText*>(m_paintingDrawItem)->setText("");
                }

                // This means the text was set through the text dialog
                m_paintingDrawClicks = 0;
                return;
            }

            // This is the generic case
            if(m_paintingDrawClicks == 1) {
                m_paintingDrawItem->setPos(dest);
                addItem(m_paintingDrawItem);
            }
            else {
                m_paintingDrawClicks = 0;

                // Place the painting item
                dest = m_paintingDrawItem->pos();
                placeItem(m_paintingDrawItem, dest);

                // Make an empty copy of the item for the next item insertion
                m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
                m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
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
    void CGraphicsScene::insertingItemsEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->type() == QEvent::GraphicsSceneMousePress) {

            if(event->button() == Qt::LeftButton) {

                // First temporarily remove the item from the scene. This item
                // is the one the user is grabbing with the mouse and about to
                // insert into the scene. If this "moving" item is not removed
                // there is a collision, and a temporal connection between its
                // ports is made (as the ports of the inserting items collides
                // with the ports of the new item created.
                clearSelection();
                foreach(CGraphicsItem *item, m_insertibles) {
                    removeItem(item);
                }

                // Create a new item and copy the properties of the inserting
                // item.
                m_undoStack->beginMacro(QString("Insert items"));
                foreach(CGraphicsItem *item, m_insertibles) {
                    CGraphicsItem *copied = item->copy(0);
                    placeItem(copied, smartNearingGridPoint(item->pos()));
                }
                m_undoStack->endMacro();

                // Re-add the inserting items into the scene, to be able to
                // insert more items of the same kind.
                foreach(CGraphicsItem *item, m_insertibles) {
                    addItem(item);
                    item->setSelected(true);
                }

            }
            else if(event->button() == Qt::RightButton) {

                QPointF delta = event->scenePos() - centerOfItems(m_insertibles);

                foreach(CGraphicsItem *item, m_insertibles) {
                    item->rotate90(Caneda::AntiClockwise);
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

    /******************************************************************
     *
     *                   Moving Events
     *
     *****************************************************************/
    /*!
     * \brief Handle events other than the specilized mouse actions.
     *
     * This involves moving items in a special way so that wires disconnect
     * from unselected components, and unselected wires change their geometry
     * to accomodate item movements.
     *
     * \sa disconnectDisconnectibles(), processForSpecialMove(),
     * specialMove(), endSpecialMove()
     */
    void CGraphicsScene::normalEvent(QGraphicsSceneMouseEvent *e)
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
                {
                    if(selectedItems().size() == 0) {

                        IDocument *document = DocumentViewManager::instance()->currentDocument();
                        if (document) {
                            document->launchPropertiesDialog();
                        }

                    }

                    QGraphicsScene::mouseDoubleClickEvent(e);
                }

                break;

            default:
                qDebug() << "CGraphicsScene::normalEvent() :  Unknown event type";
        };
    }

    /*!
     * \brief Check which items should be moved in a special way, to allow
     * proper wire and component movements.
     *
     * This method decides which tipe of movement each item must perform. In
     * general, moving wires should disconnect from unselected components, and
     * unselected wires should change their geometry to accomodate item
     * movements. This is acomplished by generating two lists:
     *
     * \li A list of items to disconnect
     * \li A list of wires whose geometry must be updated
     *
     * The action of this function is observed, for example, when moving an
     * item (a wire, a component, etc) connected to other components. By
     * processing if the item is a component or a wire and deciding if the
     * items must remain together (in the case of wires or when moving only
     * a component connected to wires) or separated from the connections
     * (when moving a wire away from a component), expected movements are
     * performed.
     *
     * \sa normalEvent(), specialMove()
     */
    void CGraphicsScene::processForSpecialMove(QList<QGraphicsItem*> _items)
    {
        disconnectibles.clear();
        specialMoveItems.clear();

        foreach(QGraphicsItem *item, _items) {
            // Save item's position for later use
            storePos(item, smartNearingGridPoint(item->scenePos()));

            CGraphicsItem *_item = canedaitem_cast<CGraphicsItem*>(item);
            if(_item) {
                // Check for disconnections and wire resizing
                foreach(Port *port, _item->ports()) {

                    foreach(Port *other, *(port->connections())) {
                        // If the item connected is a component, determine whether it should
                        // be disconnected or not.
                        if(other->parentItem()->type() == CGraphicsItem::ComponentType &&
                                !other->parentItem()->isSelected()) {
                            disconnectibles << _item;
                        }
                        // If the item connected is a wire, determine whether it should be
                        // resized or moved.
                        if(other->parentItem()->type() == CGraphicsItem::WireType &&
                                !other->parentItem()->isSelected()) {
                            specialMoveItems << other->parentItem();
                        }
                        // If the item connected is a port, determine whether it should be
                        // moved or not.
                        if(other->parentItem()->type() == CGraphicsItem::PortSymbolType &&
                                !other->parentItem()->isSelected()) {
                            specialMoveItems << other->parentItem();
                        }
                    }

                }
            }
        }
    }

    /*!
     * \brief Disconnect the items in the disconnectibles list.
     *
     * This method disconnects the ports in the disconnectibles list, generated
     * by the processForSpecialMove() method. The disconnection should happen
     * when two (or more) components are connected and one of them is clicked
     * and dragged, or when a wire is moved away from a (unselected) component.
     *
     * \sa normalEvent(), processForSpecialMove()
     */
    void CGraphicsScene::disconnectDisconnectibles()
    {
        QSet<CGraphicsItem*> remove;

        foreach(CGraphicsItem *_item, disconnectibles) {

            int disconnections = 0;
            foreach(Port *port, _item->ports()) {

                foreach(Port *other, *(port->connections())) {
                    if(other->parentItem()->type() == CGraphicsItem::ComponentType &&
                            other->parentItem() != _item &&
                            !other->parentItem()->isSelected()) {

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
     * \brief Move the unselected items in a special way to allow proper wire
     * movements.
     *
     * This method accomodates the geometry of all wires which must be resized
     * due to the current wire movement, but are not selected (and moving)
     * themselves. It also moves some special items which must move along with
     * the selected wires.
     *
     * The action of this function is observed, for example, when moving a wire
     * connected to other wires. Thanks to this function, the connected ports
     * of all wires stay together. If this function was to be removed, after
     * a wire movement action, the connected wires would remain untouched, and
     * a gap would appear between the moved wire and the connected wires (which
     * would remain in their original place).
     *
     * \sa normalEvent(), processForSpecialMove()
     */
    void CGraphicsScene::specialMove()
    {
        foreach(CGraphicsItem *_item, specialMoveItems) {

            // The wires in specialMoveItems are those wires that are not selected
            // but whose geometry must acommodate to the current moving wire.
            if(_item->type() == CGraphicsItem::WireType) {

                Wire *wire = canedaitem_cast<Wire*>(_item);
                wire->storeState();

                // Check both ports (port1 and port2) of the unselected wire for
                // possible ports movement.

                // First check port1
                foreach(Port *other, *(wire->port1()->connections())) {
                    // If some of the connected ports has moved, we have found the
                    // moving wire and this port must copy that port position.
                    if(other->scenePos() != wire->port1()->scenePos()) {
                        wire->movePort1(other->scenePos());
                        break;
                    }
                }

                // Then check port2
                foreach(Port *other, *(wire->port2()->connections())) {
                    // If some of the connected ports has moved, we have found the
                    // moving wire and this port must copy that port position.
                    if(other->scenePos() != wire->port2()->scenePos()) {
                        wire->movePort2(other->scenePos());
                        break;
                    }
                }

            }

            // The ports in specialMoveItems must be moved along the selected
            // (and moving) wires.
            if(_item->type() == CGraphicsItem::PortSymbolType) {

                PortSymbol *portSymbol = canedaitem_cast<PortSymbol*>(_item);

                foreach(Port *other, *(portSymbol->port()->connections())) {
                    // If some of the connected ports has moved, we have found the
                    // moving item and this port must copy that port position.
                    if(other->scenePos() != portSymbol->scenePos()) {
                        portSymbol->setPos(other->scenePos());
                        break;
                    }
                }

            }

        }
    }

    /*!
     * \brief End the special move and finalize wire's segements.
     *
     * This method ends the special move by pushing the necessary UndoCommands
     * relative to position changes of items on a scene. Also finalize wire's
     * segments.
     *
     * \sa normalEvent()
     */
    void CGraphicsScene::endSpecialMove()
    {
        foreach(QGraphicsItem *item, selectedItems()) {

            m_undoStack->push(new MoveCmd(item, storedPos(item),
                        smartNearingGridPoint(item->scenePos())));


            CGraphicsItem * m_item = canedaitem_cast<CGraphicsItem*>(item);
            if(m_item) {
                connectItems(m_item);
                splitAndCreateNodes(m_item);
            }

        }

        foreach(CGraphicsItem *_item, specialMoveItems) {

            if(_item->type() == CGraphicsItem::WireType) {
                Wire *wire = canedaitem_cast<Wire*>(_item);
                m_undoStack->push(new WireStateChangeCmd(wire, wire->storedState(),
                                                         wire->currentState()));
            }

        }

        foreach(QGraphicsItem *item, specialMoveItems) {

            PortSymbol * m_item = canedaitem_cast<PortSymbol*>(item);
            if(m_item) {
                connectItems(m_item);
                splitAndCreateNodes(m_item);
            }

        }

        specialMoveItems.clear();
        disconnectibles.clear();
    }

    /**********************************************************************
     *
     *                           Place item
     *
     **********************************************************************/
    /*!
     * \brief Place an item on the scene
     *
     * \param item item to place
     * \param pos position of the item
     * \warning pos is not rounded (grid snapping)
     */
    void CGraphicsScene::placeItem(CGraphicsItem *item, const QPointF &pos)
    {
        if(item->type() == CGraphicsItem::ComponentType) {
            Component *component = canedaitem_cast<Component*>(item);

            int labelSuffix = componentLabelSuffix(component->labelPrefix());
            QString label = QString("%1%2").
                arg(component->labelPrefix()).
                arg(labelSuffix);

            component->setLabel(label);
        }

        m_undoStack->beginMacro(QString("Place item"));
        m_undoStack->push(new InsertItemCmd(item, this, pos));
        m_undoStack->endMacro();
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
    void CGraphicsScene::mirroringEvent(const QGraphicsSceneMouseEvent *event,
            const Qt::Axis axis)
    {
        // Select item and filter items
        QList<QGraphicsItem*> _list = items(event->scenePos());
        QList<CGraphicsItem*> qItems = filterItems<CGraphicsItem>(_list);

        if(!qItems.isEmpty()) {
            mirrorItems(QList<CGraphicsItem*>() << qItems.first(), axis);
        }
    }

    //! \brief Mirror X event
    void CGraphicsScene::mirroringXEvent(const QGraphicsSceneMouseEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if(event->buttons() == Qt::LeftButton) {
            mirroringEvent(event, Qt::XAxis);
        }
    }

    //! \brief Mirror Y event
    void CGraphicsScene::mirroringYEvent(const QGraphicsSceneMouseEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if(event->buttons() == Qt::LeftButton) {
            mirroringEvent(event, Qt::YAxis);
        }
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
        disconnectItems(items);

        /*sort item */
        qSort(items.begin(), items.end(), pointCmpFunction_X);
        x1 = items.first()->pos().x();
        x2 = items.last()->pos().x();

        /* compute step */
        dx = (x2 - x1) / (items.size() - 1);
        x = x1;

        foreach(CGraphicsItem *item, items) {
            /* why not filter wire ??? */
            if(item->type() == CGraphicsItem::WireType) {
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
        connectItems(items);
        splitAndCreateNodes(items);

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
        disconnectItems(items);

        /*sort item */
        qSort(items.begin(), items.end(), pointCmpFunction_Y);
        y1 = items.first()->pos().y();
        y2 = items.last()->pos().y();

        /* compute step */
        dy = (y2 - y1) / (items.size() - 1);
        y = y1;

        foreach(CGraphicsItem *item, items) {
            /* why not filter wire ??? */
            if(item->type() == CGraphicsItem::WireType) {
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
        connectItems(items);
        splitAndCreateNodes(items);

        /* end command */
        m_undoStack->endMacro();

    }

    /**********************************************************************
     *
     *                           Misc
     *
     **********************************************************************/
    /*!
     * \brief Calculates the center of the items given as a parameter.
     *
     * It actually unites the boundingRect of the items sent as parameters
     * and then returns the center of the united rectangle. This center may be
     * used as a reference point for several actions, for example, rotation,
     * mirroring, and copy/paste/inserting items on the scene.
     *
     * \param items The items which geometric center has to be calculated.
     * \return The geometric center of the items.
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

    //! \copydoc connectItems(CGraphicsItem *item)
    void CGraphicsScene::connectItems(QList<CGraphicsItem*> &items)
    {
        // Check and connect each item
        foreach (CGraphicsItem *item, items) {
            connectItems(item);
        }
    }

    /*!
     * \brief Check for overlapping ports around the scene, and connect the
     * coinciding ports.
     *
     * This method checks for overlapping ports around the scene, and connects
     * the coinciding ports. Although previously this method was included in
     * the CGraphicsItem class, later was moved to CGraphicsScene to give more
     * flexibility and to avoid infinite recursions when calling this method
     * from inside a newly created or deleted item.
     *
     * \param item: items to connect
     *
     * \sa splitAndCreateNodes()
     */
    void CGraphicsScene::connectItems(CGraphicsItem *item)
    {
        // Find existing intersecting ports and connect
        foreach(Port *port, item->ports()) {
            Port *other = port->findCoincidingPort();
            if(other) {
                port->connectTo(other);
            }
        }
    }

    //! \copydoc disconnectItems(CGraphicsItem *item)
    void CGraphicsScene::disconnectItems(QList<CGraphicsItem*> &items)
    {
        foreach(CGraphicsItem *item, items) {
            disconnectItems(item);
        }
    }

    /*!
     * \brief Disconnect an item from any wire or other components
     *
     * \param item: item to disconnect
     */
    void CGraphicsScene::disconnectItems(CGraphicsItem *item)
    {
        QList<Port*> ports = item->ports();
        foreach(Port *p, ports) {
            p->disconnect();
        }
    }

    //! \copydoc splitAndCreateNodes(CGraphicsItem *item)
    void CGraphicsScene::splitAndCreateNodes(QList<CGraphicsItem *> &items)
    {
        foreach (CGraphicsItem *item, items) {
            splitAndCreateNodes(item);
        }
    }

    /*!
     * \brief Search wire collisions and if found split the wire.
     *
     * This method searches for wire collisions, and if a collision is present,
     * splits the wire in two, creating a new node. This is done, for example,
     * when wiring the schematic and a wire ends in the middle of another wire.
     * In that case, a connection must be made, thus the need to split the
     * colliding wire.
     *
     * \return Returns true if new node was created.
     *
     * \sa connectItems()
     */
    void CGraphicsScene::splitAndCreateNodes(CGraphicsItem *item)
    {
        // Check for collisions in each port, otherwise the items intersect
        // but no node should be created.
        foreach(Port *port, item->ports()) {

            // List of wires to delete after collision and creation of new wires
            QList<Wire*> markedForDeletion;

            // Detect all colliding items
            QList<QGraphicsItem*> collisions = port->collidingItems(Qt::IntersectsItemBoundingRect);

            // Filter colliding wires only
            foreach(QGraphicsItem *collidingItem, collisions) {
                Wire* collidingWire = canedaitem_cast<Wire*>(collidingItem);
                if(collidingWire) {

                    // If already connected, the collision is the result of the connection,
                    // otherwise there is a potential new node.
                    bool alreadyConnected = false;
                    foreach(Port *portIterator, item->ports()) {
                        alreadyConnected |=
                                portIterator->isConnectedTo(collidingWire->port1()) ||
                                portIterator->isConnectedTo(collidingWire->port2());
                    }

                    if(!alreadyConnected){
                        // Calculate the start, middle and end points. As the ports are mapped in the parent's
                        // coordinate system, we must calculate the positions (via the mapToScene method) in
                        // the global (scene) coordinate system.
                        QPointF startPoint  = collidingWire->port1()->scenePos();
                        QPointF middlePoint = port->scenePos();
                        QPointF endPoint    = collidingWire->port2()->scenePos();

                        // Mark old wire for deletion. The deletion is performed in a second
                        // stage to avoid referencing null pointers inside the foreach loop.
                        markedForDeletion << collidingWire;

                        // Create two new wires
                        Wire *wire1 = new Wire(startPoint, middlePoint, this);
                        Wire *wire2 = new Wire(middlePoint, endPoint, this);

                        // Create new node (connections to the colliding wire)
                        port->connectTo(wire1->port2());
                        port->connectTo(wire2->port1());

                        wire1->updateGeometry();
                        wire2->updateGeometry();

                        // Restore old wire connections
                        connectItems(wire1);
                        connectItems(wire2);
                    }
                }
            }

            // Delete all wires marked for deletion. The deletion is performed
            // in a second stage to avoid referencing null pointers inside the
            // foreach loop.
            foreach(Wire *w, markedForDeletion) {
                delete w;
            }

            // Clear the list to avoid dereferencing deleted wires
            markedForDeletion.clear();
        }
    }

} // namespace Caneda
