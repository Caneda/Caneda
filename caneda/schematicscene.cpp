/***************************************************************************
 * Copyright (C) 2006 Gopala Krishna A <krishna.ggk@gmail.com>             *
 * Copyright (C) 2008 Bastien ROUCARIES <roucaries.bastien+qucs@gmail.com> *
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

#include "schematicscene.h"

#include "component.h"
#include "library.h"
#include "port.h"
#include "propertygroup.h"
#include "qucsmainwindow.h"
#include "schematicview.h"
#include "settings.h"
#include "undocommands.h"
#include "wire.h"

#include "diagrams/diagram.h"

#include "paintings/paintings.h"

#include "qucs-tools/global.h"

#include "xmlutilities/xmlutilities.h"

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QCursor>
#include <QFileInfo>
#include <QGraphicsSceneEvent>
#include <QGraphicsView>
#include <QKeySequence>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QShortcutEvent>
#include <QStyleOptionGraphicsItem>
#include <QtDebug>
#include <QtGlobal>
#include <QUndoStack>

#include <cmath>
#include <memory>


/*!
 * \brief This template method calculates the center of the items on the scene
 *
 * It actually unites the boundingRect of the items sent as param and then
 * returns the center of the united rectangle. This center is used as
 * reference point while copy/paste/inserting items on the scene.
 *
 * \param items The items with respect to which the geometrical center has to
 * be calculated.
 *
 * \return Returns the geometrical center of the items sent through parameter.
 */
template <typename T>
QPointF centerOfItems(const QList<T*> &items)
{
    QRectF rect = items.isEmpty() ? QRectF() :
        items.first()->sceneBoundingRect();


    foreach(T *item, items) {
        rect |= item->sceneBoundingRect();
    }

    return rect.center();
}

//! \brief Default Constructor
SchematicScene::SchematicScene(QObject *parent) : QGraphicsScene(parent)
{
    init();
}

/*!
 * \brief Default grid spacing
 * \todo Must be configurable
 */
static const uint DEFAULT_GRID_SPACE = 10;

/*!
 * \brief Default grid color
 * \todo Must be configurable
 */
#define DEFAULT_GRID_COLOR Qt::darkGray;

//! \brief Initialize a schematic scene
void SchematicScene::init()
{
    /* setup undo stack */
    m_undoStack = new QUndoStack(this);

    /* setup grid */
    m_gridWidth = m_gridHeight = DEFAULT_GRID_SPACE;
    m_gridcolor = DEFAULT_GRID_COLOR;
    m_snapToGrid = true;
    m_gridVisible = true;
    m_OriginDrawn = true;

    m_currentMode = Qucs::SchematicMode;
    m_backgroundVisible = true;
    m_frameVisible = false;
    m_modified = false;

    m_opensDataDisplay = true;
    m_frameTexts = QStringList() << tr("Title: ") << tr("Drawn By: ") << tr("Date: ")+QDate::currentDate().toString() << tr("Revision: ");
    m_frameRows = 11;
    m_frameColumns = 16;
    m_macroProgress = false;
    m_areItemsMoving = false;
    m_shortcutsBlocked = false;

    /* wire state machine */
    m_wiringState = NO_WIRE;
    m_currentWiringWire = NULL;

    m_paintingDrawItem = 0;
    m_paintingDrawClicks = 0;
    m_zoomBand = 0;

    setCurrentMouseAction(Normal);

    connect(undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(setModified(bool)));
}

//! \brief Default Destructor
SchematicScene::~SchematicScene()
{
    delete m_undoStack;
}

/*!
 * \brief Data set file suffix
 * \todo use something more explicit
 */
static const char dataSetSuffix[] = ".dat";
//! \brief Data display file suffix
static const char dataDisplaySuffix[] = ".dpl";

/*!
 * \brief Set schematic and datafile name
 *
 * \param name: name to set
 * \todo Why do we need this. A new theme will be each project is a subdirectory.
 * Schematic name is only a prefix
 */
void SchematicScene::setFileName(const QString& name)
{
    if(name == m_fileName) {
        return;
    }
    else if(name.isEmpty()) {
        m_fileName.clear();
        m_dataSet.clear();
        m_dataDisplay.clear();
    }
    else {
        m_fileName = name;
        QFileInfo info(m_fileName);
        m_dataSet = info.baseName() + dataSetSuffix;
        m_dataDisplay = info.baseName() + dataDisplaySuffix;
    }

    emit fileNameChanged(m_fileName);
    emit titleToBeUpdated();
}

//! \brief A helper method to return sign of given integer.
inline int sign(int value)
{
    return value >= 0 ? +1 : -1;
}

/*!
 * \brief Get nearest point on grid
 *
 * \param pos: current position to be rounded
 * \return rounded position
 * \todo Algorithm to be explained.
 */
QPointF SchematicScene::nearingGridPoint(const QPointF &pos) const
{
    const QPoint point = pos.toPoint();

    int x = qAbs(point.x());
    x += (m_gridWidth >> 1);
    x -= x % m_gridWidth;
    x *= sign(point.x());

    int y = qAbs(point.y());
    y += (m_gridHeight >> 1);
    y -= y % m_gridHeight;
    y *= sign(point.y());

    return QPointF(x, y);
}

/*!
 * \brief Set grid size
 *
 * \param width: grid width in pixel
 * \param height: grid height in pixel
 */
void SchematicScene::setGridSize(const uint width, const uint height)
{
    /* avoid redrawing */
    if(m_gridWidth == width && m_gridHeight == height) {
        return;
    }

    m_gridWidth = width;
    m_gridHeight = height;

    if(isGridVisible()) {
        update();
    }
}

/*!
 * \brief Set grid visibility
 *
 * \param visibility: Grid visibility
 */
void SchematicScene::setGridVisible(const bool visibility)
{
    /* avoid updating */
    if(m_gridVisible == visibility)  {
        return;
    }

    m_gridVisible = visibility;
    update();
}

/*!
 * \brief Set grid visibility
 *
 * \param visibility: Grid visibility
 */
void SchematicScene::setGridColor(const QColor &color)
{
    /* avoid updating */
    if(m_gridcolor == color) {
        return;
    }

    m_gridcolor = color;
    update();
}

/*!
 * \brief Method that unifies various property setters.
 *
 * \param propName The property which is to be set.
 * \param value The new value to be set.
 * \return Returns true if successful, else returns false.
 */
bool SchematicScene::setProperty(const QString& propName, const QVariant& value)
{
    if(propName == "grid visibility"){
        setGridVisible(value.toBool());
        return true;
    }
    else if(propName == "grid width"){
        setGridWidth(value.toInt());
        return true;
    }
    else if(propName == "grid height"){
        setGridHeight(value.toInt());
        return true;
    }
    else if(propName == "frame visibility"){
        setFrameVisible(value.toBool());
        return true;
    }
    else if(propName == "document properties"){
        setFrameTexts(value.toStringList());
        return true;
    }
    else if(propName == "frame rows"){
        setFrameSize(value.toInt(), frameColumns());
        return true;
    }
    else if(propName == "frame columns"){
        setFrameSize(frameRows(), value.toInt());
        return true;
    }
    else if(propName == "schematic width"){
        setSceneRect(0, 0, value.toDouble(), height());
        return true;
    }
    else if(propName == "schematic height"){
        setSceneRect(0, 0, width(), value.toDouble());
        return true;
    }

    return false;
}

/*!
 * \brief Set origin visibility
 *
 * \param visibility: origin visibility
 */
void SchematicScene::setOriginDrawn(const bool visibility)
{
    /* avoid updating */
    if(m_OriginDrawn == visibility)  {
        return;
    }

    m_OriginDrawn = visibility;
    update();
}

//! \brief Set the dataset filename(file which holds the plot data)
void SchematicScene::setDataSet(const QString& _dataSet)
{
    m_dataSet = _dataSet;
}

//! \todo Documenent
void SchematicScene::setDataDisplay(const QString& display)
{
    m_dataDisplay = display;
}

//! \todo Documenent
void SchematicScene::setOpensDataDisplay(const bool state)
{
    m_opensDataDisplay = state;
}

/*!
 * \brief Makes the background color visible.
 *
 * \param visibility Set true of false to show or hide the background color.
 */
void SchematicScene::setBackgroundVisible(const bool visibility)
{
    /* avoid updating */
    if(m_backgroundVisible == visibility) {
        return;
    }

    m_backgroundVisible = visibility;
    update();
}

/*!
 * \brief Makes the outer frame visible.
 *
 * The frame is the outer rectangles with printed fields to enter name and other
 * properties of the schematic diagram.
 *
 * \param visibility Set true of false to show or hide the frame.
 */
void SchematicScene::setFrameVisible(const bool visibility)
{
    /* avoid updating */
    if(m_frameVisible == visibility) {
        return;
    }

    m_frameVisible = visibility;
    update();
}

/*!
 * \brief Sets current schematic's frame texts.
 *
 * \param texts The texts to be set
 */
void SchematicScene::setFrameTexts(const QStringList& texts)
{
    m_frameTexts = texts;
    if(isFrameVisible()) {
        update();
    }
}

/*!
 * \brief Sets current schematic's frame size.
 *
 * \param rows Number of rows to be set
 * \param columns Number of columns to be set
 */
void SchematicScene::setFrameSize(int rows, int columns)
{
    m_frameRows = rows;
    m_frameColumns = columns;
    if(isFrameVisible()) {
        update();
    }
}

//!  \brief Set the current mode (one of symbol mode and schematic mode)
void SchematicScene::setMode(const Qucs::Mode mode)
{
    if(m_currentMode == mode) {
        return;
    }
    m_currentMode = mode;
    update();
}

/*!
 * \brief Set mouse action
 * This method takes care to disable the shortcuts while items are being added
 * to the schematic thus preventing sideeffects. It also sets the appropriate
 * drag mode for all the views associated with this scene.
 * Finally the state variables are reset.
 *
 * \param MouseAction: mouse action to set
 */
void SchematicScene::setCurrentMouseAction(const MouseAction action)
{
    if(m_currentMouseAction == action) {
        return;
    }

    // Remove the shortcut blocking if the current action uptil now was InsertItems
    if(m_currentMouseAction == InsertingItems) {
        blockShortcuts(false);
    }

    // Blocks shortcut if the new action to be set is InsertingItems
    if(action == InsertingItems) {
        blockShortcuts(true);
    }

    m_areItemsMoving = false;
    m_currentMouseAction = action;

    // Set the appropriate drag mode for all views associated with this scene.
    QGraphicsView::DragMode dragMode = (action == Normal) ?
        QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag;
    foreach(QGraphicsView *view, views()) {
        view->setDragMode(dragMode);
    }

    resetState();
    //TODO: Implemement this appropriately for all mouse actions
}


/***********************************************************************
 *
 *       RESET STATE
 *
 ***********************************************************************/


//! \brief Reset state helper wire part
void SchematicScene::resetStateWiring()
{
    switch(m_wiringState) {
        case NO_WIRE:
            /* do nothing */
            m_wiringState = NO_WIRE;
            return;

        case SINGLETON_WIRE:
            /* wire is singleton do nothing except delete last attempt */
            Q_ASSERT(m_currentWiringWire != NULL);
            delete m_currentWiringWire;
            m_wiringState = NO_WIRE;
            return;

        case COMPLEX_WIRE:
            /* last inserted point is end point */
            Q_ASSERT(m_currentWiringWire != NULL);
            m_currentWiringWire->show();
            m_currentWiringWire->setState(m_currentWiringWire->storedState());
            m_currentWiringWire->movePort1(m_currentWiringWire->port1()->pos());
            delete m_currentWiringWire;
            m_wiringState = NO_WIRE;
            return;
    }
}


/*!
 * \brief Reset the state
 *
 * This callback is called when for instance you press esc key
 * \todo document each step
 */
void SchematicScene::resetState()
{
    // Clear focus on any item on this scene.
    setFocusItem(0);
    // Clear selection.
    clearSelection();

    // Clear the list holding items to be pasted/placed on schematic scene.
    qDeleteAll(m_insertibles);
    m_insertibles.clear();

    /* reset wiring */
    resetStateWiring();

    /* reset drawing item */
    delete m_paintingDrawItem;
    m_paintingDrawItem = NULL;
    m_paintingDrawClicks = 0;

    delete m_zoomBand;
    m_zoomBand = 0;
}

//! \brief Cut items
void SchematicScene::cutItems(QList<QucsItem*> &_items, const Qucs::UndoOption opt)
{
    copyItems(_items);
    deleteItems(_items, opt);
}

/*!
 * \brief Copy item
 * \todo Document format
 * \todo Use own mime type
 */
void SchematicScene::copyItems(QList<QucsItem*> &_items) const
{
    if(_items.isEmpty()) {
        return;
    }

    QString clipText;
    Qucs::XmlWriter *writer = new Qucs::XmlWriter(&clipText);
    writer->setAutoFormatting(true);
    writer->writeStartDocument();
    writer->writeDTD(QString("<!DOCTYPE qucs>"));
    writer->writeStartElement("qucs");
    writer->writeAttribute("version", Qucs::version);

    foreach(QucsItem *_item, _items) {
        _item->saveData(writer);
    }

    writer->writeEndDocument();

    QClipboard *clipboard =  QApplication::clipboard();
    clipboard->setText(clipText);
}

void SchematicScene::beginPaintingDraw(Painting *item)
{
    Q_ASSERT(m_currentMouseAction == SchematicScene::PaintingDrawEvent);
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
 * \todo create a insert qucscomponents property in order to avoid ugly cast
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
 * to QucsItem which does nothing. Then classes like component can specialize
 * this method to do necessary operation like hiding properties.
 * Then in the loop, there is no need for cast. Just call that prepare method
 * on all items.
 */
void SchematicScene::beginInsertingItems(const QList<QucsItem*> &items)
{
    Q_ASSERT(m_currentMouseAction == SchematicScene::InsertingItems);

    // Delete all previous insertibles
    qDeleteAll(m_insertibles);
    /* add to insert list */
    m_insertibles = items;

    /* add items */
    foreach(QucsItem *item, m_insertibles) {
        item->setSelected(true);
        // Hide all items here, they are made visible in ::insertingItemsEvent
        item->hide();
        /* replace by item->insertonscene() */
        if(item->isComponent()) {
            Component *comp = qucsitem_cast<Component*>(item);
            if(comp->propertyGroup()) {
                comp->propertyGroup()->hide();
            }
        }
    }
}

/*!
 * \brief Event filter filter's out some events on the watched object.
 *
 * This filter is used to install on QApplication object to filter our
 * shortcut events.
 * This filter is installed by \a setCurrentMouseAction method if the new action
 * is InsertingItems and removed if the new action is different, thus blocking
 * shortcuts on InsertItems and unblocking for other mouse actions
 * \sa SchematicScene::setCurrentMouseAction, SchematicScene::blockShortcuts
 * \sa QObject::eventFilter
 *
 * \todo Take care if multiple scenes install event filters.
 */
bool SchematicScene::eventFilter(QObject *watched, QEvent *event)
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
 * \sa SchematicScene::eventFilter
 */
void SchematicScene::blockShortcuts(const bool block)
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
 * \brief Exports the schematic to an image
 *
 * @return bool True on success, false otherwise
 */
bool SchematicScene::toPaintDevice(QPaintDevice &pix, qreal width, qreal height,
        Qt::AspectRatioMode aspectRatioMode)
{
    QRectF source_area = imageBoundingRect();

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
 * \brief Used to know the dimensions of the image
 *
 * @return The bounding rect of the image
 */
QRectF SchematicScene::imageBoundingRect() const
{
    if(!isFrameVisible()) {
        return itemsBoundingRect();
    }
    else {
        return QRectF(0, 0, width(), height());
    }
}

/*!
 * \brief Set whether this schematic is modified or not
 *
 * This method emits the signal modificationChanged(bool) as well
 * as the signal titleToBeUpdated()
 *
 * \param m True/false to set it to unmodified/modified.
 */
void SchematicScene::setModified(const bool m)
{
    if(m_modified != !m) {
        m_modified = !m;
        emit modificationChanged(m_modified);
        emit titleToBeUpdated();
    }
}

/*!
 * \brief Draw background of schematic including grid
 *
 * \param painter: Where to draw
 * \param rect: Visible area
 * \todo Finish visual representation
 * \todo draw origin should be configurable
 */
void SchematicScene::drawBackground(QPainter *painter, const QRectF& rect)
{
    QPen savedpen = painter->pen();

    /* disable anti aliasing */
    painter->setRenderHint(QPainter::Antialiasing, false);

    if(isBackgroundVisible()) {
        const QColor backgroundColor =
            Settings::instance()->currentValue("gui/backgroundColor").value<QColor>();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(backgroundColor));
        painter->drawRect(rect);
    }

    /* configure pen */
    painter->setPen(QPen(GridColor(), 0));
    painter->setBrush(Qt::NoBrush);

    /* draw frame */
    if(isFrameVisible()) {
        // First we draw the users content
        foreach(QString frame_text, m_frameTexts){
            if(frame_text.contains("Title: ")) {
                painter->drawText(width()/3, height()-30, frame_text);
            }
            else if(frame_text.contains("Drawn By: ")) {
                painter->drawText(10, height()-30, frame_text);
            }
            else if(frame_text.contains("Date: ")) {
                painter->drawText(10, height()-10, frame_text);
            }
            else if(frame_text.contains("Revision: ")) {
                painter->drawText(width()*4/5, height()-30, frame_text);
            }
        }

        // Next we draw the footer lines
        painter->drawRect(sceneRect()); //Bounding rect
        painter->drawLine(QLineF(0, height()-50, width(),
                    height()-50)); //Upper footer line
        painter->drawLine(QLineF(width()/3-20, height()-50, width()/3-20,
                    height())); //Name division
        painter->drawLine(QLineF(width()*4/5-20, height()-50,
                    width()*4/5-20, height())); //Title division
        painter->drawLine(QLineF(20, 0, 20, height()-50)); //Left line
        painter->drawLine(QLineF(0, 20, width(), 20));  //Upper line

        // Finally we draw the rows and columns
        int step = (height()-20-50) / frameRows();
        for(int i=1; i<frameRows()+1; i++) { //Row numbering
            painter->drawLine(QLineF(0, i*step+20, 20, i*step+20));
            painter->drawText(6, (i*step)-(step/2)+20, QString(QChar('A'+i-1)));
        }
        step = (width()-20) / frameColumns();
        for(int i=1; i<frameColumns()+1; i++) { //Column numbering
            painter->drawLine(QLineF(i*step+20, 0, i*step+20, 20));
            painter->drawText((i*step)-(step/2)+20, 16, QString::number(i));
        }
    }

    /* no grid */
    if(!isGridVisible()) {
        return;
    }

    /* draw origin */
    const QPointF origin(0, 0);
    if(isOriginDrawn() && rect.contains(origin)) {
        //qreal width = width();
        //qreal height = height();
        painter->drawLine(QLineF(origin.x() - 3.0, origin.y(),
                    origin.x() + 3.0, origin.y()));
        painter->drawLine(QLineF(origin.x(), origin.y() - 3.0,
                    origin.x(), origin.y() + 3.0));
    }

    // Adjust  visual representation of grid to be multiple, if
    // grid sizes are very small
#if 0
    while(gridWidth < 20) {
        gridWidth *= 2;
    }
    while(gridHeight < 20) {
        gridHeight *= 2;
    }
    while(gridWidth > 60) {
        gridWidth /= 2;
    }
    while(gridHeight > 60) {
        gridHeight /= 2;
    }
#endif

    // While drawing, choose spaing to be twice the actual grid size.
    const int drawingGridWidth = gridWidth() * 2;
    const int drawingGridHeight = gridHeight() * 2;

    /* extrema grid points */
    qreal left = int(rect.left()) + drawingGridWidth - (int(rect.left()) % drawingGridWidth);
    qreal top = int(rect.top()) + drawingGridHeight - (int(rect.top()) % drawingGridHeight);
    qreal right = int(rect.right()) - (int(rect.right()) % drawingGridWidth);
    qreal bottom = int(rect.bottom()) - (int(rect.bottom()) % drawingGridHeight);
    qreal x, y;

    /* draw grid */
    painter->setBrush(Qt::NoBrush);
    for(x = left; x <= right; x += drawingGridWidth) {
        for(y = top; y <=bottom; y += drawingGridHeight) {
            painter->drawPoint(QPointF(x, y));
        }
    }

    /* restore painter */
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
bool SchematicScene::event(QEvent *event)
{
    static int ii = 0;
    if(m_currentMouseAction == InsertingItems) {
        if(event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
            bool visible = (event->type() == QEvent::Enter);
            foreach(QucsItem *item, m_insertibles) {
                item->setVisible(visible);
            }
        }
    }

    return QGraphicsScene::event(event);
}

/*!
 * \brief Context menu
 * \todo Implement
 */
void SchematicScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    switch(selectedItems().size()) {
        case 0:
            //launch a general menu
            break;

        case 1:
            //launch context menu of item.
            QGraphicsScene::contextMenuEvent(event);
            break;

        default: ;
                 //launch a common menu
    }
}

/*!
 * \brief Event handler, for event drag enter event
 *
 * Drag enter events are generated as the cursor enters the item's area.
 * Accept event from sidebar
 * \param event event to be accepted
 */
void SchematicScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    if(event->mimeData()->formats().contains("application/qucs.sidebarItem")) {
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
void SchematicScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    if(event->mimeData()->formats().contains("application/qucs.sidebarItem")) {
        event->acceptProposedAction();
    }
    else {
        event->ignore();
    }
}

/*!
 * \brief Event handler, for event drop event
 *
 * Receive drop events for SchematicScene
 * Items can only receive drop events if the last drag move event was accepted
 * Accept event only from sidebar
 *
 * \param event event to be accepted
 * \todo factorize
 */
void SchematicScene::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    if(event->mimeData()->formats().contains("application/qucs.sidebarItem")) {
        event->accept();
        QWidget *parentWidget = event->widget() ? event->widget()->parentWidget() : 0;
        SchematicView *view = qobject_cast<SchematicView*>(parentWidget);
        if (!view) {
            event->ignore();
            return;
        }
        view->saveScrollState();

        QByteArray encodedData = event->mimeData()->data("application/qucs.sidebarItem");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QString item, category;
        stream >> item >> category;
        QucsItem *qItem = itemForName(item, category);
        /* XXX: extract and factorize */
        if(qItem->type() == GraphicText::Type) {
            GraphicTextDialog dialog(0, Qucs::DontPushUndoCmd);
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

            placeItem(qItem, dest, Qucs::PushUndoCmd);
            view->restoreScrollState();
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
 * \todo Remove debug
 * \todo finish grid snap mode
 */
void SchematicScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    /* grid snap mode */
    if(m_snapToGrid) {
        lastPos = nearingGridPoint(e->scenePos());
    }
    sendMouseActionEvent(e);
}

/*!
 * \brief mouse move event
 * \todo document
 */
void SchematicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    if(m_snapToGrid) {
        //HACK: Fool the event receivers by changing event parameters with new grid position.
        QPointF point = nearingGridPoint(e->scenePos());
        if(point == lastPos) {
            e->accept();
            return;
        }
        e->setScenePos(point);
        e->setPos(point);
        e->setLastScenePos(lastPos);
        e->setLastPos(lastPos);
        //Now cache this point for next move
        lastPos = point;
    }
    sendMouseActionEvent(e);
}

/*!
 * \brief release mouse
 * \todo Document
 */
void SchematicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    sendMouseActionEvent(e);
}

/*!
 * \brief Mouse double click
 *
 * Encapsulates the mouseDoubleClickEvent as one of MouseAction and calls
 * corresponding callback.
 */
void SchematicScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    sendMouseActionEvent(e);
}

void SchematicScene::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    QGraphicsView *v = static_cast<QGraphicsView *>(e->widget()->parent());
    SchematicView *sv = qobject_cast<SchematicView*>(v);
    if(!sv) {
        return;
    }

    if(e->modifiers() & Qt::ControlModifier){
        if(e->delta() > 0) {
            sv->zoomIn();
        }
        else {
            sv->zoomOut();
        }
    }
    else if(e->modifiers() & Qt::ShiftModifier){
        if(e->delta() > 0) {
            sv->horizontalScrollBar()->setValue(sv->horizontalScrollBar()->value()+50);
        }
        else {
            sv->horizontalScrollBar()->setValue(sv->horizontalScrollBar()->value()-50);
        }
    }
    else{
        if(e->delta() > 0) {
            sv->verticalScrollBar()->setValue(sv->verticalScrollBar()->value()-50);
        }
        else {
            sv->verticalScrollBar()->setValue(sv->verticalScrollBar()->value()+50);
        }
    }

    e->accept();
}


/******************************************************************************
 *
 *          Sidebar
 *
 *****************************************************************************/

/*!
 * \brief Action when a painting item is selected
 *
 * \param itemName: name of item
 */
bool SchematicScene::sidebarItemClickedPaintingsItems(const QString& itemName)
{
    setCurrentMouseAction(PaintingDrawEvent);
    m_paintingDrawItem = Painting::fromName(itemName);
    if(!m_paintingDrawItem) {
        setCurrentMouseAction(Normal);
        return false;
    }
    m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
    return true;
}

bool SchematicScene::sidebarItemClickedNormalItems(const QString& itemName, const QString& category)
{
    QucsItem *item = itemForName(itemName, category);
    if(!item) {
        return false;
    }

    addItem(item);
    setCurrentMouseAction(InsertingItems);
    beginInsertingItems(QList<QucsItem*>() << item);

    return true;
}


/*!
 * \brief This function is called when a side bar item is clicked
 *
 * \param itemName: name of item
 * \param category: categoy name
 * \todo Add tracing
 */
bool SchematicScene::sidebarItemClicked(const QString& itemName, const QString& category)
{
    if(itemName.isEmpty()) {
        return false;
    }

    if(category == "Paint Tools") {
        return sidebarItemClickedPaintingsItems(itemName);
    }
    else {
        return sidebarItemClickedNormalItems(itemName, category);
    }
}


/*********************************************************************
 *
 *            WIRING ACTION
 *
 *********************************************************************/

/*!
 * \brief Finalize wire ie last control point == end
 * \todo Why not a wire operation ?
 * \todo Add undo operation for this
 */
void SchematicScene::wiringEventMouseClickFinalize()
{
    m_currentWiringWire->show();
    m_currentWiringWire->movePort1(m_currentWiringWire->port1()->pos());
    m_currentWiringWire->removeNullLines();
    m_currentWiringWire->updateGeometry();


    /* detach current wire */
    m_currentWiringWire = NULL;
}

/*!
 * \brief Add a wire segment
 * \todo Why not a wire operation
 */
void SchematicScene::wiringEventLeftMouseClickAddSegment()
{
    /* add segment */
    m_currentWiringWire->storeState();

    WireLines& wLinesRef = m_currentWiringWire->wireLinesRef();
    WireLine toAppend(wLinesRef.last().p2(), wLinesRef.last().p2());
    wLinesRef << toAppend << toAppend;
}

/*!
 * \brief Common wiring part
 *
 * \param cmd: undo command to run
 */
void SchematicScene::wiringEventLeftMouseClickCommonComplexSingletonWire(QUndoCommand * cmd)
{
    /* configure undo */
    m_undoStack->beginMacro(tr("Add wiring control point"));

    /* clean up line */
    m_currentWiringWire->removeNullLines();

    /* wiring command */
    m_undoStack->push(cmd);

    /* check and connect */
    m_currentWiringWire->checkAndConnect(Qucs::PushUndoCmd);

    m_undoStack->endMacro();
}


/*!
 * \brief Left mouse click wire event
 *
 * \param Event: mouse event
 * \param rounded: coordinate of mouse action point (rounded if needed)
 */
void SchematicScene::wiringEventLeftMouseClick(const QPointF &pos)
{
    if(m_wiringState == NO_WIRE) {
        /* create a new wire */
        m_currentWiringWire = new Wire(pos, pos, false, this);
        m_wiringState = SINGLETON_WIRE;
        return;
    }
    if(m_wiringState == SINGLETON_WIRE) {
        /* check if wire do not overlap */
        if(m_currentWiringWire->overlap())  {
            return;
        }

        QUndoCommand * singleton_wire = new AddWireCmd(m_currentWiringWire, this);
        wiringEventLeftMouseClickCommonComplexSingletonWire(singleton_wire);

        /* if connect finalize */
        if(m_currentWiringWire->port2()->hasConnection()) {
            wiringEventMouseClickFinalize();
            m_wiringState = NO_WIRE;
        }
        else  {
            wiringEventLeftMouseClickAddSegment();
            m_wiringState = COMPLEX_WIRE;
        }
        return;
    }
    if(m_wiringState == COMPLEX_WIRE) {
        if(m_currentWiringWire->overlap())  {
            return;
        }

        QUndoCommand * complex_wire = new WireStateChangeCmd(m_currentWiringWire,
                m_currentWiringWire->storedState(),
                m_currentWiringWire->currentState());

        wiringEventLeftMouseClickCommonComplexSingletonWire(complex_wire);

        if(m_currentWiringWire->port2()->hasConnection()) {
            /* finalize */
            wiringEventMouseClickFinalize();
            m_wiringState = NO_WIRE;
        } else  {
            wiringEventLeftMouseClickAddSegment();
            m_wiringState = COMPLEX_WIRE;
        }
        return;
    }
}

//! \brief Right mouse click wire event. This is finish wire event
void SchematicScene::wiringEventRightMouseClick()
{

    /* state machine */
    if(m_wiringState == NO_WIRE) {
        m_wiringState = NO_WIRE;
        return;
    }
    if(m_wiringState ==  SINGLETON_WIRE) {
        /* check overlap */
        if(m_currentWiringWire->overlap()) {
            return;
        }

        /* do wiring */
        QUndoCommand * singleton_wire = new AddWireCmd(m_currentWiringWire, this);
        wiringEventLeftMouseClickCommonComplexSingletonWire(singleton_wire);

        /* finalize */
        wiringEventMouseClickFinalize();
        m_wiringState = NO_WIRE;
        return;
    }
    if(m_wiringState == COMPLEX_WIRE) {
        if(m_currentWiringWire->overlap()) {
            return;
        }

        /* do wiring */
        QUndoCommand * complex_wire = new WireStateChangeCmd(m_currentWiringWire,
                m_currentWiringWire->storedState(),
                m_currentWiringWire->currentState());
        wiringEventLeftMouseClickCommonComplexSingletonWire(complex_wire);

        /* finalize */
        wiringEventMouseClickFinalize();
        m_wiringState = NO_WIRE;
        return;
    }
}

/*!
 * \brief Mouse click wire event
 *
 * \param Event: mouse event
 * \param pos: coordinate of mouse action point (rounded if needed)
 */
void SchematicScene::wiringEventMouseClick(const MouseActionEvent *event, const QPointF &pos)
{
    /* left click */
    if((event->buttons() & Qt::LeftButton) == Qt::LeftButton)  {
        return wiringEventLeftMouseClick(pos);
    }
    /* right click */
    if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
        return wiringEventRightMouseClick();
    }
    return;
}


/*!
 * \brief Mouse move wire event
 *
 * \param pos: coordinate of mouse action point (rounded if needed)
 */
void SchematicScene::wiringEventMouseMove(const QPointF &pos)
{
    if(m_wiringState != NO_WIRE) {
        QPointF newPos = m_currentWiringWire->mapFromScene(pos);
        m_currentWiringWire->movePort2(newPos);
    }
}

//! \brief Wiring event
void SchematicScene::wiringEvent(MouseActionEvent *event)
{
    /* round */
    QPointF pos;
    pos = smartNearingGridPoint(event->scenePos());

    /* press mouse */
    if(event->type() == QEvent::GraphicsSceneMousePress)  {
        return wiringEventMouseClick(event, pos);
    }
    /* move mouse */
    else if(event->type() == QEvent::GraphicsSceneMouseMove)  {
        return wiringEventMouseMove(pos);
    }
}



//! \todo document
void SchematicScene::markingEvent(MouseActionEvent *event)
{
    Q_UNUSED(event);
    //TODO:
}




/***************************************************************************
 *
 *             Mirror
 *
 *
 ***************************************************************************/


/*!
 * \brief Mirror an item list
 *
 * \param items: item to mirror
 * \param opt: undo option
 * \param axis: mirror axis
 * \todo Create a custom undo class for avoiding if
 * \note assert X or Y axis
 */
void SchematicScene::mirrorItems(QList<QucsItem*> &items,
        const Qucs::UndoOption opt,
        const Qt::Axis axis)
{
    Q_ASSERT(axis == Qt::XAxis || axis == Qt::YAxis);

    /* prepare undo stack */
    if(opt == Qucs::PushUndoCmd) {
        if(axis == Qt::XAxis) {
            m_undoStack->beginMacro(QString("Mirror X"));
        }
        else {
            m_undoStack->beginMacro(QString("Mirror Y"));
        }
    }

    /* disconnect item before mirroring */
    disconnectItems(items, opt);

    /* mirror */
    MirrorItemsCmd *cmd = new MirrorItemsCmd(items, axis);
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->push(cmd);
    }
    else {
        cmd->redo();
        delete cmd;
    }

    /* try to reconnect */
    connectItems(items, opt);

    /* end undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->endMacro();
    }
}


/*!
 * \brief Mirror event
 *
 * \param event: event
 * \param axis: mirror axis
 */
void SchematicScene::mirroringEvent(const MouseActionEvent *event,
        const Qt::Axis axis)
{
    /* select item */
    QList<QGraphicsItem*> _list = items(event->scenePos());
    /* filters item */
    QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
    if(!qItems.isEmpty()) {
        /* mirror */
        mirrorItems(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd, axis);
    }
}

/*!
 * \brief Mirror X event
 * \note right button mirror Y
 */
void SchematicScene::mirroringXEvent(const MouseActionEvent *event)
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
void SchematicScene::mirroringYEvent(const MouseActionEvent *event)
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


/******************************************************************
 *
 *                   Rotate
 *
 *****************************************************************/


/*!
 * \brief Rotate an item list
 *
 * \param items: item list
 * \param opt: undo option
 * \param diect: is rotation in trigonometric sense
 * \todo Create a custom undo class for avoiding if
 */
void SchematicScene::rotateItems(QList<QucsItem*> &items,
        const Qucs::AngleDirection dir,
        const Qucs::UndoOption opt)
{
    /* setup undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->beginMacro(dir == Qucs::Clockwise ?
                QString("Rotate Clockwise") :
                QString("Rotate Anti-Clockwise"));
    }

    /* disconnect */
    disconnectItems(items, opt);

    /* rotate */
    RotateItemsCmd *cmd = new RotateItemsCmd(items, dir);
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->push(cmd);
    }
    else {
        cmd->redo();
        delete cmd;
    }

    /* reconnect */
    connectItems(items, opt);

    /* finish undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->endMacro();
    }
}


/*!
 * \brief Rotate item
 * \note right anticlockwise
 */
void SchematicScene::rotatingEvent(MouseActionEvent *event)
{
    Qucs::AngleDirection angle;

    if(event->type() != QEvent::GraphicsSceneMousePress) {
        return;
    }

    /* left == clock wise */
    if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) {
        angle = Qucs::Clockwise;
    }
    /* right == anticlock wise */
    else if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
        angle = Qucs::AntiClockwise;
    }
    /* avoid angle unitialized */
    else {
        return;
    }

    /* get items */
    QList<QGraphicsItem*> _list = items(event->scenePos());
    /* filter item */
    QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
    if(!qItems.isEmpty()) {
        rotateItems(QList<QucsItem*>() << qItems.first(), angle, Qucs::PushUndoCmd);
    }
}

/***************************************************************************
 *
 *   Distribute element
 *
 ***************************************************************************/

//! \brief Short function for qsort sort by abscissa
static inline bool pointCmpFunction_X(const QucsItem *lhs, const QucsItem  *rhs)
{
    return lhs->pos().x() < rhs->pos().x();
}

//!Short function for qsort sort by abscissa
static inline bool pointCmpFunction_Y(const QucsItem *lhs, const QucsItem  *rhs)
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
void SchematicScene::distributeElementsHorizontally(QList<QucsItem*> items)
{
    qreal x1, x2, x, dx;
    QPointF newPos;

    /* undo */
    m_undoStack->beginMacro("Distribute horizontally");

    /* disconnect */
    disconnectItems(items, Qucs::PushUndoCmd);

    /*sort item */
    qSort(items.begin(), items.end(), pointCmpFunction_X);
    x1 = items.first()->pos().x();
    x2 = items.last()->pos().x();

    /* compute step */
    dx = (x2 - x1) / (items.size() - 1);
    x = x1;

    foreach(QucsItem *item, items) {
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
    connectItems(items, Qucs::PushUndoCmd);

    /* end command */
    m_undoStack->endMacro();

}

/*!
 * \brief Distribute vertically
 *
 * \param items: items to distribute
 * \todo Why not filter wire ??
 */
void SchematicScene::distributeElementsVertically(QList<QucsItem*> items)
{
    qreal y1, y2, y, dy;
    QPointF newPos;

    /* undo */
    m_undoStack->beginMacro("Distribute vertically");

    /* disconnect */
    disconnectItems(items, Qucs::PushUndoCmd);

    /*sort item */
    qSort(items.begin(), items.end(), pointCmpFunction_Y);
    y1 = items.first()->pos().y();
    y2 = items.last()->pos().y();

    /* compute step */
    dy = (y2 - y1) / (items.size() - 1);
    y = y1;

    foreach(QucsItem *item, items) {
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
    connectItems(items, Qucs::PushUndoCmd);

    /* end command */
    m_undoStack->endMacro();

}

/*!
 * \brief Distribute elements
 *
 * Distribute elements ie each element is equally spaced
 *
 * \param orientation: distribute according to orientation
 * \todo filter wire ??? Do not distribute wire ??
 */
bool SchematicScene::distributeElements(const Qt::Orientation orientation)
{
    QList<QGraphicsItem*> gItems = selectedItems();
    QList<QucsItem*> items = filterItems<QucsItem>(gItems);

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


/***********************************************************************
 *
 * Alignement
 *
 ***********************************************************************/


//! \brief Check if alignement flags are compatible used in assert
static bool checkAlignementFlag(const Qt::Alignment alignment)
{
    switch(alignment) {
        case Qt::AlignLeft :
        case Qt::AlignRight :
        case Qt::AlignTop :
        case Qt::AlignBottom :
        case Qt::AlignHCenter :
        case Qt::AlignVCenter :
        case Qt::AlignCenter:
            return true;
        default:
            return false;
    }
}


//! @return A string corresponding to alignement
const QString SchematicScene::Alignment2QString(const Qt::Alignment alignment)
{
    Q_ASSERT(checkAlignementFlag(alignment));

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
            /* impossible case */
        default:
            return "";
    }
}

/*!
 * \brief Align element
 *
 * \param alignment: alignement used
 * \todo use smart alignment ie: port alignement
 * \todo implement snap on grid
 * \todo string of undo
 * \todo filter wires ???
 */
bool SchematicScene::alignElements(const Qt::Alignment alignment)
{
    Q_ASSERT(checkAlignementFlag(alignment));

    QList<QGraphicsItem*> gItems = selectedItems();
    QList<QucsItem*> items = filterItems<QucsItem>(gItems, DontRemoveItems);


    /* Could not align less than two elements */
    if(items.size() < 2) {
        return false;
    }

    /* setup undo */
    m_undoStack->beginMacro(Alignment2QString(alignment));

    /* disconnect */
    disconnectItems(items, Qucs::PushUndoCmd);

    /* compute bounding rectangle */
    QRectF rect = items.first()->sceneBoundingRect();
    QList<QucsItem*>::iterator it = items.begin()+1;
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
                /* impossible */
                break;
        }

        /* move item */
        QPointF itemPos = (*it)->pos();
        m_undoStack->push(new MoveCmd(*it, itemPos, itemPos + delta));;
        ++it;
    }

    /* reconnect items */
    connectItems(items, Qucs::PushUndoCmd);

    /* finish undo */
    m_undoStack->endMacro();
    return true;
}


/*************************************************************************
 *
 *         Set on grid
 *
 *************************************************************************/


/*!
 * \brief Set item on grid
 *
 * \param items: item list
 * \param opt: undo option
 * \todo Create a custom undo class for avoiding if
 */
void SchematicScene::setItemsOnGrid(QList<QucsItem*> &items,
        const Qucs::UndoOption opt)
{
    QList<QucsItem*> itemsNotOnGrid;
    /* create a list of item not on grid */
    foreach(QucsItem* item, items) {
        QPointF pos = item->pos();
        QPointF gpos = nearingGridPoint(pos);
        if(pos != gpos) {
            itemsNotOnGrid << item;
        }
    }

    if(itemsNotOnGrid.isEmpty()) {
        return;
    }

    /* setup undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->beginMacro(QString("Set on grid"));
    }

    /* disconnect item */
    disconnectItems(itemsNotOnGrid, opt);

    /* put item on grid */
    foreach(QucsItem *item, itemsNotOnGrid) {
        QPointF pos = item->pos();
        QPointF gridPos = nearingGridPoint(pos);

        if(opt == Qucs::PushUndoCmd) {
            m_undoStack->push(new MoveCmd(item, pos, gridPos));
        }
        else {
            item->setPos(gridPos);
        }
    }

    /* try to create connection */
    connectItems(itemsNotOnGrid, opt);

    /* finalize undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->endMacro();
    }
}


//! \brief Set on grid event
void SchematicScene::settingOnGridEvent(const MouseActionEvent *event)
{
    //  only left click
    if(event->type() != QEvent::GraphicsSceneMousePress) {
        return;
    }
    if((event->buttons() & Qt::LeftButton) != Qt::LeftButton) {
        return;
    }

    //  do action
    QList<QGraphicsItem*> _list = items(event->scenePos());
    if(!_list.isEmpty()) {
        QList<QucsItem*> _items = filterItems<QucsItem>(_list);

        if(!_list.isEmpty()) {
            setItemsOnGrid(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
        }
    }
}

/******************************************************************************
 *
 *     Active status
 *
 *****************************************************************************/

/*!
 * \brief Toggle active status
 *
 * \param items: item list
 * \param opt: undo option
 * \todo Create a custom undo class for avoiding if
 * \todo Change direction of toogle
 */
void SchematicScene::toggleActiveStatus(QList<QucsItem*> &items,
        const Qucs::UndoOption opt)
{
    /* Apply only to components */
    QList<Component*> components = filterItems<Component>(items);
    if(components.isEmpty()) {
        return;
    }

    /* setup undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->beginMacro(QString("Toggle active status"));
    }

    /* toogle */
    ToggleActiveStatusCmd *cmd = new ToggleActiveStatusCmd(components);
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->push(cmd);
    }
    else {
        cmd->redo();
        delete cmd;
    }

    /* finalize undo */
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->endMacro();
    }
}


/*!
 * \brief Activate deactivate
 * \todo implement left right behavior
 */
void SchematicScene::changingActiveStatusEvent(const MouseActionEvent *event)
{
    if(event->type() != QEvent::GraphicsSceneMousePress) {
        return;
    }
    if((event->buttons() & Qt::LeftButton) != Qt::LeftButton) {
        return;
    }

    QList<QGraphicsItem*> _list = items(event->scenePos());
    QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
    if(!qItems.isEmpty()) {
        toggleActiveStatus(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd);
    }
}


/*************************************************************
 *
 *          DELETE
 *
 *************************************************************/



/*!
 * \brief Delete an item list
 *
 * \param items: item list
 * \param opt: undo option
 * \todo Document
 * \todo Create a custom undo class for avoiding if
 * \todo removeitems delete item use asingle name scheme
 */
void SchematicScene::deleteItems(QList<QucsItem*> &items,
        const Qucs::UndoOption opt)
{
    if(opt == Qucs::DontPushUndoCmd) {
        foreach(QucsItem* item, items) {
            delete item;
        }
    }
    else {
        /* configure undo */
        m_undoStack->beginMacro(QString("Delete items"));

        /* diconnect then remove */
        disconnectItems(items, opt);
        m_undoStack->push(new RemoveItemsCmd(items, this));

        m_undoStack->endMacro();
    }
}

/*!
 * \brief Left button deleting event: delete items
 *
 * \param pos: pos clicked
 */
void SchematicScene::deletingEventLeftMouseClick(const QPointF &pos)
{
    /* create a list of items */
    QList<QGraphicsItem*> _list = items(pos);
    if(!_list.isEmpty()) {
        QList<QucsItem*> _items = filterItems<QucsItem>(_list);

        if(!_items.isEmpty()) {
            deleteItems(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
        }
    }
}


/*!
 * \brief Left button deleting event: delete items
 *
 * \param pos: pos clicked
 */
void SchematicScene::deletingEventRightMouseClick(const QPointF &pos)
{
    /* create a list of items */
    QList<QGraphicsItem*> _list = items(pos);
    if(!_list.isEmpty()) {
        QList<QucsItem*> _items = filterItems<QucsItem>(_list);

        if(!_items.isEmpty()) {
            disconnectItems(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
        }
    }
}


/*!
 * \brief Delete action
 *
 * Delete action: left click delete, right click disconnect item
 */
void SchematicScene::deletingEvent(const MouseActionEvent *event)
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

/*********************************************************************
 *
 *  Connect -- disconnect
 *
 ********************************************************************/

/*!
 * \brief Automatically connect items if port or wire overlap
 *
 * \param qItems: item to connect
 * \param opt: undo option
 * \todo remove the cast and create a class connectable item
 */

void SchematicScene::connectItems(const QList<QucsItem*> &qItems,
        const Qucs::UndoOption opt)
{
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->beginMacro(QString("Connect items"));
    }

    /* remove this cast */
    foreach(QucsItem *qItem, qItems) {
        if(qItem->isComponent()) {
            qucsitem_cast<Component*>(qItem)->checkAndConnect(opt);
        }
        else if(qItem->isWire()) {
            qucsitem_cast<Wire*>(qItem)->checkAndConnect(opt);
        }
    }

    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->endMacro();
    }
}


/*!
 * \brief Disconnect an item from wire or other components
 *
 * \param qItems: item to connect
 * \param opt: undo option
 * \todo remove the cast and create a class connectable item
 */
void SchematicScene::disconnectItems(const QList<QucsItem*> &qItems,
        const Qucs::UndoOption opt)
{
    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->beginMacro(QString("Disconnect items"));
    }

    foreach(QucsItem *item, qItems) {
        QList<Port*> ports;

        /* remove this cast */
        if(item->isComponent()) {
            ports = qucsitem_cast<Component*>(item)->ports();
        }
        else if(item->isWire()) {
            ports = qucsitem_cast<Wire*>(item)->ports();
        }

        foreach(Port *p, ports) {
            Port *other = p->getAnyConnectedPort();

            /* do not register new undo if nothing to do */
            if(other == NULL) {
                continue;
            }

            if(opt == Qucs::PushUndoCmd) {
                m_undoStack->push(new DisconnectCmd(p, other));
            }
            else {
                p->disconnectFrom(other);
            }
        }
    }

    if(opt == Qucs::PushUndoCmd) {
        m_undoStack->endMacro();
    }
}

/*********************************************************************
 *
 *  Zoom in -- Zoom out
 *
 ********************************************************************/

/*!
 * \brief Zoom in event handles zooming of the view based on mouse signals.
 *
 * If just a point is clicked(mouse press + release) then, an ordinary zoomIn
 * is done (similar to selecting from menu)
 *
 * On the otherhand if mouse is pressed and dragged and then release,
 * corresponding feedback (zoom band) is shown which indiates area that will
 * be zoomed. On mouse release, the area (rect) selected is zoomed.
 *
 * \todo Should i doucment the code ?
 */
void SchematicScene::zoomingAtPointEvent(MouseActionEvent *event)
{
    QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
    SchematicView *sv = qobject_cast<SchematicView*>(v);
    if(!sv) {
        return;
    }
    QPoint viewPoint = sv->mapFromScene(event->scenePos());

    // Delete the zoom band and return if this event was triggered for non left
    // mouse button.
    if (!(event->buttons().testFlag(Qt::LeftButton)) &&
            event->type() != QEvent::GraphicsSceneMouseRelease) {
        delete m_zoomBand;
        m_zoomBand = 0;
        return;
    }


    if(event->type() == QEvent::GraphicsSceneMousePress) {
        // Another left click when zoom band is active means that a
        // zoom operation was started in another view for this same scene.
        // So delete the old zoom band.
        if (m_zoomBand) {
            delete m_zoomBand;
            m_zoomBand = 0;
        }
    }
    else if(event->type() == QEvent::GraphicsSceneMouseMove) {
        if (!m_zoomBand) {
            m_zoomBand = new QRubberBand(QRubberBand::Rectangle);
            m_zoomBand->setParent(sv->viewport());
            m_zoomBand->show();
            m_zoomRect.setRect(event->scenePos().x(), event->scenePos().y(), 0, 0);
        } else {
            m_zoomRect.setBottomRight(event->scenePos());
        }
        QRect rrect = sv->mapFromScene(m_zoomRect).boundingRect().normalized();
        m_zoomBand->setGeometry(rrect);
    }
    else {
        if (m_zoomBand) {
            sv->fitInView(m_zoomRect, Qt::KeepAspectRatio);

            delete m_zoomBand;
            m_zoomBand = 0;
        }
        else {
            sv->zoomIn();
            QPointF afterScalePoint(sv->mapFromScene(event->scenePos()));
            int dx = (afterScalePoint - viewPoint).toPoint().x();
            int dy = (afterScalePoint - viewPoint).toPoint().y();

            QScrollBar *hb = sv->horizontalScrollBar();
            QScrollBar *vb = sv->verticalScrollBar();

            hb->setValue(hb->value() + dx);
            vb->setValue(vb->value() + dy);
        }
    }
}

/*!
 * \brief Zoom out event handles zooming of the view based on mouse signals.
 *
 * If just a point is clicked(mouse press + release) then, an ordinary zoomOut
 * is done (similar to selecting from menu)
 */
void SchematicScene::zoomingOutAtPointEvent(MouseActionEvent *event)
{
    QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
    SchematicView *sv = qobject_cast<SchematicView*>(v);
    if(!sv) {
        return;
    }

    if(event->type() == QEvent::GraphicsSceneMousePress) {
        sv->zoomOut();
    }
}

void SchematicScene::placeAndDuplicatePainting()
{
    if(!m_paintingDrawItem) {
        return;
    }

    QPointF dest = m_paintingDrawItem->pos();
    placeItem(m_paintingDrawItem, dest, Qucs::PushUndoCmd);

    m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
    m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
    if(m_paintingDrawItem->type() == GraphicText::Type) {
        static_cast<GraphicText*>(m_paintingDrawItem)->setText("");
    }
}

//! \todo Document
void SchematicScene::paintingDrawEvent(MouseActionEvent *event)
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
            int result = text->launchPropertyDialog(Qucs::DontPushUndoCmd);
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
void SchematicScene::insertingItemsEvent(MouseActionEvent *event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        if (event->button() == Qt::LeftButton) {
            clearSelection();
            foreach(QucsItem *item, m_insertibles) {
                removeItem(item);
            }
            m_undoStack->beginMacro(QString("Insert items"));
            foreach(QucsItem *item, m_insertibles) {
                QucsItem *copied = item->copy(0);
                /* round */
                placeItem(copied, smartNearingGridPoint(item->pos()), Qucs::PushUndoCmd);
            }
            m_undoStack->endMacro();
            foreach(QucsItem *item, m_insertibles) {
                addItem(item);
                item->setSelected(true);
            }
        } else if (event->button() == Qt::RightButton) {
            emit rotateInvokedWhileInserting();
            // HACK: Assuming the above signal is connected to SchematicStateHandler
            // through Qt::DirectConnection, all m_insertibles would have been
            // updated with rotated items.  However, beginInsertingItems would have
            // hidden all items, so show them back.
            // I see no point why we would be not using Qt::DirectConenction though!
            QPointF delta = event->scenePos() - centerOfItems(m_insertibles);
            foreach(QucsItem *item, m_insertibles) {
                item->show();
                item->setPos(smartNearingGridPoint(item->pos() + delta));
            }
        } else if (event->button() == Qt::MidButton) {
            emit mirrorInvokedWhileInserting();
            // HACK: Same as above!
            QPointF delta = event->scenePos() - centerOfItems(m_insertibles);
            foreach(QucsItem *item, m_insertibles) {
                item->show();
                item->setPos(smartNearingGridPoint(item->pos() + delta));
            }
        }

    }
    else if(event->type() == QEvent::GraphicsSceneMouseMove) {
        QPointF delta = event->scenePos() - centerOfItems(m_insertibles);

        foreach(QucsItem *item, m_insertibles) {
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
void SchematicScene::insertingWireLabelEvent(MouseActionEvent *event)
{
    Q_UNUSED(event);
    //TODO:
}

/*!
 * \brief Here events other than the specized mouse actions are handled.
 *
 * This involves moving items when selected items are dragged in a special way
 * so that wires are created if a connected component is moved away from
 * unselected component.
 */
void SchematicScene::normalEvent(MouseActionEvent *e)
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
                        if(!m_macroProgress) {
                            m_macroProgress = true;
                            m_undoStack->beginMacro(QString("Move items"));
                        }
                    }
                }

                if(!m_areItemsMoving) {
                    return;
                }
                disconnectDisconnectibles();
                QGraphicsScene::mouseMoveEvent(e);
                QPointF delta = smartNearingGridPoint(e->scenePos() - e->lastScenePos());
                specialMove(delta.x(), delta.y());
            }
            break;


        case QEvent::GraphicsSceneMouseRelease:
            {
                if(m_areItemsMoving) {
                    m_areItemsMoving = false;
                    endSpecialMove();
                }
                if(m_macroProgress) {
                    m_macroProgress = false;
                    m_undoStack->endMacro();
                }
                QGraphicsScene::mouseReleaseEvent(e); // other behaviour by base
            }
            break;

        case QEvent::GraphicsSceneMouseDoubleClick:
            QGraphicsScene::mouseDoubleClickEvent(e);
            break;

        default:
            qDebug("SchematicScene::normalEvent() :  Unknown event type");
    };
}

/*!
 * \brief Check which all selected items should be moved specially
 *        and where there is possible wirable nodes.
 */
void SchematicScene::processForSpecialMove(QList<QGraphicsItem*> _items)
{
    disconnectibles.clear();
    movingWires.clear();
    grabMovingWires.clear();

    foreach(QGraphicsItem *item, _items) {
        Component *c = qucsitem_cast<Component*>(item);
        // save item's position for later use.
        storePos(item, smartNearingGridPoint(item->scenePos()));
        if(c) {
            //check for disconnections and wire resizing.
            foreach(Port *port, c->ports()) {
                QList<Port*> *connections = port->connections();
                if(!connections) {
                    continue;
                }

                foreach(Port *other, *connections) {
                    if(other == port ) {
                        continue;
                    }


                    Component *otherComponent = 0;
                    // Determine whether the "other" and "port" should be disconnected and wired
                    // on mouse move later.
                    if((otherComponent = other->owner()->component())
                            && !otherComponent->isSelected()) {
                        disconnectibles << c;
                        break;
                    }


                    Wire *wire = other->owner()->wire();
                    if(wire) {
                        // Determine whether this wire should be resized or not.
                        // resized means = creating and deleting segments of wire.
                        // on mouse moves later.
                        Port* otherPort = wire->port1() == other ? wire->port2() :
                            wire->port1();
                        if(!otherPort->areAllOwnersSelected()) {
                            movingWires << wire;
                            wire->storeState();
                        }
                    }
                }
            }
        }

        Wire *wire = qucsitem_cast<Wire*>(item);
        // Now determine whether the wire should just be moved rather than resized
        // resized means = creating and deleting segments of wire.
        if(wire && !movingWires.contains(wire)) {
            bool condition = wire->isSelected();
            condition = condition && ((!wire->port1()->areAllOwnersSelected() ||
                        !wire->port2()->areAllOwnersSelected()) ||
                    (wire->port1()->connections() == 0 &&
                     wire->port2()->connections() == 0));
            ;
            if(condition) {
                grabMovingWires << wire;
                wire->storeState();
            }
        }
    }
}

/*!
 * \brief Disconnect the ports in the disconnectibles list and add wire's in
 * between them. This happens when two(or more) components are connected and one of
 * them is clicked and dragged.
 */
void SchematicScene::disconnectDisconnectibles()
{
    QSet<Component*> remove;
    foreach(Component *c, disconnectibles) {
        int disconnections = 0;
        foreach(Port *port, c->ports()) {
            if(!port->connections()) {
                continue;
            }

            Port *fromPort = 0;
            foreach(Port *other, *port->connections()) {
                if(other->owner()->component() && other->owner()->component() != c &&
                        !other->owner()->component()->isSelected()) {
                    fromPort = other;
                    break;
                }
            }

            if(fromPort) {
                m_undoStack->push(new DisconnectCmd(port, fromPort));
                ++disconnections;
                AddWireBetweenPortsCmd *wc = new AddWireBetweenPortsCmd(port, fromPort);
                Wire *wire = wc->wire();
                m_undoStack->push(wc);
                movingWires << wire;
            }
        }
        if(disconnections) {
            remove << c;
        }
    }
    foreach(Component *c, remove)
        disconnectibles.removeAll(c);
}

/*!
 * \brief Move the selected items specially to allow proper wire movements
 * and as well as checking for possible disconnections.
 */
void SchematicScene::specialMove(qreal dx, qreal dy)
{
    foreach(Wire *wire, movingWires) {
        if(wire->port1()->connections()) {
            Port *other = 0;
            foreach(Port *o, *(wire->port1()->connections())) {
                if(o != wire->port1()) {
                    other = o;
                    break;
                }
            }
            if(other) {
                wire->movePort(wire->port1()->connections(), smartNearingGridPoint(other->scenePos()));
            }
        }
        if(wire->port2()->connections()) {
            Port *other = 0;
            foreach(Port *o, *(wire->port2()->connections())) {
                if(o != wire->port2()) {
                    other = o;
                    break;
                }
            }
            if(other) {
                wire->movePort(wire->port2()->connections(), smartNearingGridPoint(other->scenePos()));
            }
        }
    }

    foreach(Wire *wire, grabMovingWires) {
        wire->grabMoveBy(dx, dy);
    }
}

/*!
 * \brief End the special move by pushing the UndoCommands for position change
 * of items on scene. Also fwire's segements are finalized here.
 */
void SchematicScene::endSpecialMove()
{
    disconnectibles.clear();
    foreach(QGraphicsItem *item, selectedItems()) {
        m_undoStack->push(new MoveCmd(item, storedPos(item),
                    smartNearingGridPoint(item->scenePos())));
        Component * comp = qucsitem_cast<Component*>(item);
        if(comp) {
            comp->checkAndConnect(Qucs::PushUndoCmd);
        }
        Wire *wire = qucsitem_cast<Wire*>(item);
        if(wire) {
            wire->checkAndConnect(Qucs::PushUndoCmd);
        }

    }

    foreach(Wire *wire, movingWires) {
        wire->removeNullLines();
        wire->show();
        wire->movePort1(wire->port1()->pos());
        m_undoStack->push(new WireStateChangeCmd(wire, wire->storedState(),
                    wire->currentState()));
        wire->checkAndConnect(Qucs::PushUndoCmd);
    }
    foreach(Wire *wire, grabMovingWires) {
        wire->removeNullLines();
        wire->show();
        wire->movePort1(wire->port1()->pos());
        m_undoStack->push(new WireStateChangeCmd(wire, wire->storedState(),
                    wire->currentState()));
    }

    grabMovingWires.clear();
    movingWires.clear();
}


/**********************************************************************
 *
 *                           place item
 *
 **********************************************************************/

/*!
 * \brief Place an item on the scene
 *
 * \param item: item to place
 * \param: pos position where to place
 * \param opt: undo option
 * \warning: pos is not rounded
 */
void SchematicScene::placeItem(QucsItem *item, const QPointF &pos, const Qucs::UndoOption opt)
{
    if(item->scene() == this) {
        removeItem(item);
    }

    if(item->isComponent()) {
        Component *component = qucsitem_cast<Component*>(item);

        int labelSuffix = componentLabelSuffix(component->labelPrefix());
        QString label = QString("%1%2").
            arg(component->labelPrefix()).
            arg(labelSuffix);

        component->setLabel(label);
    }

    if(opt == Qucs::DontPushUndoCmd) {
        addItem(item);
        item->setPos(pos);

        if(item->isComponent()) {
            qucsitem_cast<Component*>(item)->checkAndConnect(opt);
        }
        else if(item->isWire()) {
            qucsitem_cast<Wire*>(item)->checkAndConnect(opt);
        }
    }

    else {
        m_undoStack->beginMacro(QString("Place item"));

        m_undoStack->push(new InsertItemCmd(item, this, pos));
        if(item->isComponent()) {
            qucsitem_cast<Component*>(item)->checkAndConnect(opt);
        }
        else if(item->isWire()) {
            qucsitem_cast<Wire*>(item)->checkAndConnect(opt);
        }

        m_undoStack->endMacro();
    }
}

/*!
 * \brief Return a component or painting based on \a name and \a category.
 *
 * The painting is processed in a special hard coded way(no library)
 * On the other hand components are loaded from the library.
 */
QucsItem* SchematicScene::itemForName(const QString& name, const QString& category)
{
    if(category == QObject::tr("Paint Tools")) {
        return Painting::fromName(name);
    }

    return LibraryLoader::instance()->newComponent(name, 0, category);
}

/*!
 * \brief Returns an appropriate label suffix as 1 and 2 in R1, R2
 *
 * This method walks through all the items present on scene matching the
 * labelprefix "prefix" and uses the
 * highest of these corresponding suffixes + 1
 * as the new suffix candidate.
 */
int SchematicScene::componentLabelSuffix(const QString& prefix) const
{
    int _max = 1;
    foreach(QGraphicsItem *item, items()) {
        Component *comp = qucsitem_cast<Component*>(item);
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
 * \todo document
 * Looks like i am not using this. If so please remove.
 */
int SchematicScene::unusedPortNumber()
{
    int retVal = -1;
    if(m_usablePortNumbers.isEmpty()) {
        retVal = m_usablePortNumbers.takeFirst();
    }
    else {
        retVal = m_usedPortNumbers.last() + 1;

        while(m_usedPortNumbers.contains(retVal)) {
            retVal++;
        }
    }
    return retVal;
}

/*!
 * \todo document
 * Looks like i am not using this. If so please remove.
 */
bool SchematicScene::isPortNumberUsed(int num) const
{
    (void) num;
    return false;
}
/*!
 * \todo document
 * Looks like i am not using this. If so please remove.
 */
void SchematicScene::setNumberUnused(int num)
{
    (void) num;
}

/*!
 * \brief Call the appropriate mouseAction event based on current mouse action.
 */
void SchematicScene::sendMouseActionEvent(MouseActionEvent *e)
{
    switch(m_currentMouseAction) {
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

        case ChangingActiveStatus:
            changingActiveStatusEvent(e);
            break;

        case SettingOnGrid:
            settingOnGridEvent(e);
            break;

        case ZoomingAtPoint:
            zoomingAtPointEvent(e);
            break;

        case ZoomingOutAtPoint:
            zoomingOutAtPointEvent(e);
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
