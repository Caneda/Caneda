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
#include "schematicview.h"
#include "wire.h"
#include "port.h"
#include "paintings/paintings.h"
#include "diagrams/diagram.h"
#include "undocommands.h"
#include "xmlutilities/xmlutilities.h"
#include "qucs-tools/global.h"
#include "component.h"
#include "library.h"
#include "qucsmainwindow.h"
#include "propertygroup.h"

#include <QtCore/QMimeData>
#include <QtCore/QtDebug>
#include <QtCore/QFileInfo>

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QUndoStack>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QScrollBar>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QMessageBox>
#include <QtGui/QCursor>
#include <QtGui/QShortcutEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QColor>
#include <QtGlobal>

#include <cmath>
#include <memory>


/*!\file Schematicscene 
  
  This file implement the schematic scene ie the place where you draw components
*/


/*!\todo document */
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

/*!\brief Default Constructor */
SchematicScene::SchematicScene(QObject *parent) : QGraphicsScene(parent)
{
  this->init();
}

/*!\brief Constructs a SchematicScene object, 
  using the rectangle specified by (x, y), 
  and the given width and height for its scene rectangle. 
  The parent parameter is passed to QObject's constructor.
  \param x: first coordinate of rectangle
  \param y: second coordinate of rectangle
  \param width: schematic width
  \param height: schematic height
  \param parent: passed to  QObject's constructor.
*/
SchematicScene::SchematicScene ( qreal x, qreal y, qreal width, qreal height, QObject * parent ) :
  QGraphicsScene(x,y,width,height,parent)
{
  this->init();
}


/*!\brief Default grid spacing
  \todo Must be configurable 
*/
static const uint DEFAULT_GRID_SPACE = 10;

/*!\brief Default grid color 
  \todo Must be configurable 
*/
#define DEFAULT_GRID_COLOR Qt::darkGray;

/*!\brief Initialize a schematic scene  */
void SchematicScene::init()
{
  /* setup undo stack */
  this->m_undoStack = new QUndoStack(this);

  /* setup grid */
  this->m_gridWidth = this->m_gridHeight = DEFAULT_GRID_SPACE;
  this->m_gridcolor = DEFAULT_GRID_COLOR;
  this->m_snapToGrid = true;
  this->m_gridVisible = true;
  this->m_OriginDrawn = true;
  
  this->m_currentMode = Qucs::SchematicMode;
  this->m_frameVisible = false;
  this->m_modified = false;
  
  this->m_opensDataDisplay = true;
  this->m_frameTexts = QStringList() << tr("Title") << tr("Drawn By:") << tr("Date:") << tr("Revision:");
  this->m_macroProgress = false;
  this->m_areItemsMoving = false;
  this->m_shortcutsBlocked = false;

  /* wire state machine */
  m_wiringState = NO_WIRE;
  this->m_currentWiringWire = NULL;

  this->m_paintingDrawItem = 0;
  this->m_paintingDrawClicks = 0;
  this->m_zoomBand = 0;

  this->setCurrentMouseAction(Normal);
}

/*!\brief Default Destructor */
SchematicScene::~SchematicScene()
{
  delete this->m_undoStack;
}

/*!\todo Chck usefulness */
void SchematicScene::test()
{
}

/*!\brief Data set file suffix 
  \todo use something more explicit 
*/
static const char dataSetSuffix[] = ".dat"; 
/*!\brief Data display file suffix */
static const char dataDisplaySuffix[] = ".dpl";

/*!\brief Set schematic and datafile name 
  \param name: name to set
  \todo document
  \todo Why do we need this. A new theme will be each project is a subdirectory. 
  Schematic name is only a prefix
*/
void SchematicScene::setFileName(const QString& name)
{
  if(name == this->m_fileName || name.isEmpty())
    return;
  this->m_fileName = name;
  QFileInfo info(this->m_fileName);
  this->m_dataSet = info.baseName() + dataSetSuffix;
  this->m_dataDisplay = info.baseName() + dataDisplaySuffix;
  emit fileNameChanged(this->m_fileName);
  emit titleToBeUpdated();
}

/*! Get nearest point on grid 
  \param pos: current position to be rounded
  \return rounded position
  \todo explain algorithm
*/
QPointF SchematicScene::nearingGridPoint(const QPointF &pos) const
{
  QPoint point = pos.toPoint();
  int x = point.x();
  int y = point.y();
  
  if(x<0) 
    x -= (this->m_gridWidth / 2) - 1;
  else 
    x += this->m_gridWidth / 2; 
  x -= x % this->m_gridWidth;
  
  if(y<0) 
    y -= (this->m_gridHeight / 2) - 1;
  else 
    y += this->m_gridHeight / 2;
  y -= y % this->m_gridHeight;
  
  return QPointF(x,y);
}

/*! Set grid size 
  \param width: grid width in pixel
  \param height: grid height in pixel
*/
void SchematicScene::setGridSize(const uint width, const uint height)
{
  /* avoid redrawing */
  if(this->m_gridWidth == width && this->m_gridHeight == height)
    return;
  
  this->m_gridWidth = width;
  this->m_gridHeight = height;
  
  if(isGridVisible())
    this->update();
}

/*! Set grid visibility 
  \param visibility: Grid visibility 
*/
void SchematicScene::setGridVisible(const bool visibility)
{
  /* avoid updating */
  if(this->m_gridVisible == visibility) 
    return;
  
  this->m_gridVisible = visibility;
  this->update();
}

/*! Set grid visibility 
  \param visibility: Grid visibility 
*/
void SchematicScene::setGridColor(const QColor &color)
{
  /* avoid updating */
  if(this->m_gridcolor == color) 
    return;

  this->m_gridcolor = color;
  this->update();
}

/*! Set origin visibility
  \param visibility: origin visibility 
*/
void SchematicScene::setOriginDrawn(const bool visibility)
{
  /* avoid updating */
  if(this->m_OriginDrawn == visibility) 
    return;

  this->m_OriginDrawn = visibility;
  this->update();
}

/*!\todo Document */
void SchematicScene::setDataSet(const QString& _dataSet)
{
  this->m_dataSet = _dataSet;
}

/*!\todo Documenent */
void SchematicScene::setDataDisplay(const QString& display)
{
  this->m_dataDisplay = display;
}

/*!\todo Documenent */
void SchematicScene::setOpensDataDisplay(const bool state)
{
  this->m_opensDataDisplay = state;
}

/*!\to-sdo Documenent */
void SchematicScene::setFrameVisible(const bool visibility)
{
  /* avoid updating */
  if(this->m_frameVisible == visibility) 
    return;
  
  this->m_frameVisible = visibility;
  this->update();
}

/*!\brief Todo document */
void SchematicScene::setFrameTexts(const QStringList& texts)
{
  this->m_frameTexts = texts;
  if(this->isFrameVisible())
    update();
}

/*!\brief Todo document */
void SchematicScene::setMode(const Qucs::Mode mode)
{
  if(this->m_currentMode == mode) 
    return;
  this->m_currentMode = mode;
  this->update();
  //TODO:
}

/*!\brief Set mouse action
  \param MouseAction: mouse action to set
  \todo document
*/
void SchematicScene::setCurrentMouseAction(const MouseAction action)
{
  if(this->m_currentMouseAction == action) 
    return;

  if(this->m_currentMouseAction == InsertingItems)
    this->blockShortcuts(false);

  if(action == InsertingItems)
    this->blockShortcuts(true);

  this->m_areItemsMoving = false;
  this->m_currentMouseAction = action;

  QGraphicsView::DragMode dragMode = (action == Normal) ?
    QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag;
  foreach(QGraphicsView *view, views())
    view->setDragMode(dragMode);
  this->resetState();
  //TODO: Implemement this appropriately for all mouse actions
}


/***********************************************************************
 *
 *       RESET STATE
 *
 ***********************************************************************/


/*!\brief Reset state helper wire part */
void SchematicScene::resetStateWiring()
{
  switch(this->m_wiringState) {
  case NO_WIRE:
    /* do nothing */
    this->m_wiringState = NO_WIRE;
    return;
  case SINGLETON_WIRE:
    /* wire is singleton do nothing except delete last attempt */
    Q_ASSERT(this->m_currentWiringWire != NULL);
    delete this->m_currentWiringWire;
    this->m_wiringState = NO_WIRE;
    return;
  case COMPLEX_WIRE:
    /* last inserted point is end point */
    Q_ASSERT(this->m_currentWiringWire != NULL);
    this->m_currentWiringWire->show();
    this->m_currentWiringWire->setState(this->m_currentWiringWire->storedState());
    this->m_currentWiringWire->movePort1(this->m_currentWiringWire->port1()->pos());
    delete this->m_currentWiringWire;
    this->m_wiringState = NO_WIRE;
    return;
  }
}


/*!\brief Reset the state 

   This callback is called when for instance you press esc key
   \todo document each step
*/
void SchematicScene::resetState()
{
  this->setFocusItem(0);
  this->clearSelection();

  qDeleteAll(this->m_insertibles);
  this->m_insertibles.clear();

  /* reset wiring */
  this->resetStateWiring();

  /* reset drawing item */
  delete this->m_paintingDrawItem;
  this->m_paintingDrawItem = NULL;
  this->m_paintingDrawClicks = 0;
  
  delete this->m_zoomBand;
  this->m_zoomBand = NULL;
}

/*! Cut items */
void SchematicScene::cutItems(QList<QucsItem*> &_items, const Qucs::UndoOption opt)
{
  this->copyItems(_items);
  this->deleteItems(_items, opt);
}

/*!\brief Copy item
  \todo Document format 
  \todo Use own mime type
*/
void SchematicScene::copyItems(QList<QucsItem*> &_items) const
{
  if(_items.isEmpty())
    return;

  QString clipText;
  Qucs::XmlWriter *writer = new Qucs::XmlWriter(&clipText);
  writer->setAutoFormatting(true);
  writer->writeStartDocument();
  writer->writeDTD(QString("<!DOCTYPE qucs>"));
  writer->writeStartElement("qucs");
  writer->writeAttribute("version", Qucs::version);

  foreach(QucsItem *_item, _items)
    _item->saveData(writer);

  writer->writeEndDocument();

  QClipboard *clipboard =  QApplication::clipboard();
  clipboard->setText(clipText);
}

/*!\brief Paste item 
  \todo Use own mime type
*/
void SchematicScene::paste()
{
  const QString text = qApp->clipboard()->text();

  Qucs::XmlReader reader(text.toUtf8());

  while(!reader.atEnd()) {
    reader.readNext();

    if(reader.isStartElement() && reader.name() == "qucs")
      break;
  }

  if(reader.hasError() || !(reader.isStartElement() && reader.name() == "qucs"))
    return;

  if(!Qucs::checkVersion(reader.attributes().value("version").toString()))
    return;

  QList<QucsItem*> _items;
  while(!reader.atEnd()) {
    reader.readNext();

    if(reader.isEndElement())
      break;

    if(reader.isStartElement()) {
      QucsItem *readItem = 0;
      if(reader.name() == "component") {
	readItem = Component::loadComponentData(&reader, this);
      }
      else if(reader.name() == "wire") {
	readItem = Wire::loadWireData(&reader, this);
      }
      else if(reader.name() == "painting")  {
	readItem = Painting::loadPainting(&reader, this);
      }

      if(readItem)
	_items << readItem;
    }
  }

  this->beginInsertingItems(_items);
}

/*!\todo document */
SchematicView* SchematicScene::activeView() const
{
  if(views().isEmpty())
    return 0;
  return qobject_cast<SchematicView*>(views().first());
}

/*!\todo document 
   \todo create a insert qucscomponents property in order to avoid ugly cast 
   \todo gpk: why two loop??
*/
void SchematicScene::beginInsertingItems(const QList<QucsItem*> &items)
{
  SchematicView *active = this->activeView();
  if(!active) 
    return;
  
  /* why ??? */
  this->resetState();
  
  /* add to insert list */
  this->m_insertibles = items;

  QPoint pos = active->viewport()->mapFromGlobal(QCursor::pos());
  bool cursorOnScene = active->viewport()->rect().contains(pos);

  this->m_insertActionMousePos = this->smartNearingGridPoint(active->mapToScene(pos));

  /* add items */
  foreach(QucsItem *item, this->m_insertibles) {
    item->setSelected(true);
    item->setVisible(cursorOnScene);
    /* replace by item->insertonscene() */
    if(item->isComponent()) {
      Component *comp = qucsitem_cast<Component*>(item);
      if(comp->propertyGroup())
	comp->propertyGroup()->hide();
    }
  }

  /* why two loop */ 
  QPointF delta = this->smartNearingGridPoint(active->mapToScene(pos) 
					      - centerOfItems(this->m_insertibles));
  foreach(QucsItem *item, this->m_insertibles) {
    item->moveBy(delta.x(), delta.y());
  }
}



/*!\todo document */
bool SchematicScene::eventFilter(QObject *watched, QEvent *event)
{
  if(event->type() != QEvent::Shortcut && event->type() != QEvent::ShortcutOverride)
    return QGraphicsScene::eventFilter(watched, event);

  QKeySequence key;

  if(event->type() == QEvent::Shortcut)
    key = static_cast<QShortcutEvent*>(event)->key();
  else
    key = static_cast<QKeyEvent*>(event)->key();

  if(key == QKeySequence(Qt::Key_Escape)) {
    return false;
  }
  else {
    return true;
  }
}

/*!\todo document */
void SchematicScene::blockShortcuts(const bool block)
{
  if(!activeView()) 
    return;

  if(block) {
    if(!this->m_shortcutsBlocked) {
      qApp->installEventFilter(this);
      this->m_shortcutsBlocked = true;
    }
  }
  else {
    if(this->m_shortcutsBlocked) {
      qApp->removeEventFilter(this);
      this->m_shortcutsBlocked = false;
    }
  }
}

/*! \todo document */
void SchematicScene::setModified(const bool m)
{
  if(this->m_modified != m) {
    this->m_modified = m;
    emit modificationChanged(this->m_modified);
    emit titleToBeUpdated();
  }
}


/*! Draw background of schematic including grid
  \param painter: Where to draw
  \param rect: Visible area
  \todo Finish visual representation
  \note draw origin 
  \todo draw origin should be configurable 
*/
void SchematicScene::drawBackground(QPainter *painter, const QRectF& rect)
{
  int gridWidth = this->gridWidth();
  int gridHeight = this->gridHeight();
  QPen savedpen;

  /* no grid */
  if(!this->isGridVisible())
    return QGraphicsScene::drawBackground(painter,rect);

  /* configure pen */
  savedpen = painter->pen();
  painter->setPen(QPen(this->GridColor(),0));
  painter->setBrush(Qt::NoBrush);

  /* disable anti aliasing */
  painter->setRenderHint(QPainter::Antialiasing,false);

  /* draw origin */
  if(this->isOriginDrawn() == true &&
     rect.contains(QPointF(0.0,0.0))) {
    painter->drawLine(QLineF(-3.0, 0.0, 3.0, 0.0));
    painter->drawLine(QLineF(0.0, -3.0, 0.0, 3.0));
  }

   
  // Adjust  visual representation of grid to be multiple, if
  // grid sizes are very small
#if 0
  while(gridWidth < 20)
    gridWidth *= 2;
  while(gridHeight < 20)
    gridHeight *= 2;
  while(gridWidth > 60)
    gridWidth /= 2;
  while(gridHeight > 60)
    gridHeight /= 2;
#endif
   
  /* extrema grid points */
  qreal left = int(rect.left()) + gridWidth - (int(rect.left()) % gridWidth);
  qreal top = int(rect.top()) + gridHeight - (int(rect.top()) % gridHeight);
  qreal right = int(rect.right()) - (int(rect.right()) % gridWidth);
  qreal bottom = int(rect.bottom()) - (int(rect.bottom()) % gridHeight);
  qreal x,y;

  /* draw grid */
  for( x = left; x <= right; x += gridWidth)
    for( y = top; y <=bottom; y += gridHeight)
      painter->drawPoint(QPointF(x,y));

  /* restore painter */
  painter->setRenderHint(QPainter::Antialiasing,true);
  painter->setPen(savedpen);
}

/*! \todo Document */
bool SchematicScene::event(QEvent *event)
{
  if(this->m_currentMouseAction == InsertingItems) {
    if(event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
      bool visible = (event->type() == QEvent::Enter);
      foreach(QucsItem *item, m_insertibles)
	item->setVisible(visible);
      if(visible) {
	SchematicView *active = activeView();
	if(!active) 
	  return QGraphicsScene::event(event);

	QPoint pos = active->viewport()->mapFromGlobal(QCursor::pos());
	this->m_insertActionMousePos = this->smartNearingGridPoint(active->mapToScene(pos));

	QPointF delta = this->smartNearingGridPoint(m_insertActionMousePos 
						    - centerOfItems(m_insertibles));

	foreach(QucsItem *item, m_insertibles) {
	  item->moveBy(delta.x(), delta.y());
	}
      }
    }
  }

  return QGraphicsScene::event(event);
}

/*!\brief Context menu
  \todo Implement 
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

/*! \brief Event handler, for event drag enter event 
    
  Drag enter events are generated as the cursor enters the item's area.
  Accept event from sidebar 
  \param event event to be accepted
*/
void SchematicScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
  if(event->mimeData()->formats().contains("application/qucs.sidebarItem")) {
    event->acceptProposedAction();
    this->blockShortcuts(true);
  }
  else
    event->ignore();
}

/*! \brief Event handler, for event drag move event 
    
  Drag move events are generated as the cursor moves around inside the item's area.
  It is used to indicate that only parts of the item can accept drops.
  Accept event from sidebar 
  \param event event to be accepted
*/
void SchematicScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
  if(event->mimeData()->formats().contains("application/qucs.sidebarItem"))
    event->acceptProposedAction();
  else
    event->ignore();
}

/*! \brief Event handler, for event drop event 
     
  Receive drop events for SchematicScene     
  Items can only receive drop events if the last drag move event was accepted
  Accept event only from sidebar
  \param event event to be accepted
  \todo factorize
*/
void SchematicScene::dropEvent(QGraphicsSceneDragDropEvent * event)
{
  if(event->mimeData()->formats().contains("application/qucs.sidebarItem")) {
    event->accept();
    SchematicView *view = activeView();
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
      setModified(true);
    }
  }
  else {
    event->ignore();
  }

  blockShortcuts(false);
}

/*!\brief Event called when mouse is pressed 
  \todo Remove debug 
  \todo finish grid snap mode
*/
void SchematicScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  //Cache the mouse press position
  if((e->buttons() & Qt::MidButton) == Qt::MidButton)
    qDebug() << "pressed" << e->scenePos();
   
  /* grid snap mode */
  if(m_snapToGrid) {
    lastPos = nearingGridPoint(e->scenePos());
  }
  sendMouseActionEvent(e);
}

/*!\brief mouse move event 
  \todo document 
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

/*!\brief release mouse 
  \todo Document 
*/
void SchematicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
  sendMouseActionEvent(e);
}

/*!\brief Mouse double click 
  \todo Document 
*/
void SchematicScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
  sendMouseActionEvent(e);
}


/******************************************************************************
 *
 *          Sidebar
 *
 *****************************************************************************/

/*!Action when a painting item is selected 
   \param itemName: name of item
*/
bool SchematicScene::sidebarItemClickedPaintingsItems(const QString& itemName)
{
  this->setCurrentMouseAction(PaintingDrawEvent);
  this->m_paintingDrawItem = Painting::fromName(itemName);
  if(!this->m_paintingDrawItem) {
    this->setCurrentMouseAction(Normal);
    return false;
  }
  this->m_paintingDrawItem->setPaintingRect(QRectF(-2, -2, 4, 4));
  return true;
}

bool SchematicScene::sidebarItemClickedNormalItems(const QString& itemName, const QString& category) 
{
  QucsItem *item = itemForName(itemName, category);
  if(!item) 
    return false;
  
  this->addItem(item);
  this->setCurrentMouseAction(InsertingItems);
  this->beginInsertingItems(QList<QucsItem*>() << item);
  
  return true;
}


/*! This function is called when a side bar item is clicked 
  \param itemName: name of item
  \param category: categoy name
  \todo Add tracing
*/
bool SchematicScene::sidebarItemClicked(const QString& itemName, const QString& category)
{
  if(itemName.isEmpty()) 
    return false;

  if(category == "paintings") 
    return this->sidebarItemClickedPaintingsItems(itemName);
  else 
    return this->sidebarItemClickedNormalItems(itemName, category);
}


/*********************************************************************
 *
 *            WIRING ACTION
 *
 *********************************************************************/

/*!\brief Finalize wire ie last control point == end 
   \todo Why not a wire operation ?
*/
void SchematicScene::wiringEventMouseClickFinalize() 
{
  this->m_currentWiringWire->show();
  this->m_currentWiringWire->movePort1(m_currentWiringWire->port1()->pos());
  this->m_currentWiringWire->updateGeometry();
  
  /* detach current wire */
  this->m_currentWiringWire = NULL;
}

/*! Add a wire segment 
    \todo Why not a wire operation
*/
void SchematicScene::wiringEventLeftMouseClickAddSegment()
{
  /* add segment */
  this->m_currentWiringWire->storeState();
  
  WireLines& wLinesRef = m_currentWiringWire->wireLinesRef();
  WireLine toAppend(wLinesRef.last().p2(), wLinesRef.last().p2());
  wLinesRef << toAppend << toAppend;
}

/*! Common wiring part 
    \param cmd: undo command to run
*/
void SchematicScene::wiringEventLeftMouseClickCommonComplexSingletonWire(QUndoCommand * cmd)
{
  /* configure undo */
  this->m_undoStack->beginMacro(tr("Add wiring control point"));
  
  /* clean up line */
  this->m_currentWiringWire->removeNullLines();
  
  /* wiring command */
  this->m_undoStack->push(cmd);
  
  /* check and connect */
  this->m_currentWiringWire->checkAndConnect(Qucs::PushUndoCmd);
  
  this->m_undoStack->endMacro();
}


/*!\brief Left mouse click wire event 
  \param Event: mouse event
  \param rounded: coordinate of mouse action point (rounded if needed)
*/
void SchematicScene::wiringEventLeftMouseClick(const QPointF &pos)
{
  if(this->m_wiringState == NO_WIRE) {
    /* create a new wire */
    this->m_currentWiringWire = new Wire(pos, pos, false, this);
    this->m_wiringState = SINGLETON_WIRE;
    return;
  }
  if(this->m_wiringState == SINGLETON_WIRE) {
    /* check if wire do not overlap */
    if(this->m_currentWiringWire->overlap()) 
      return;
    
    QUndoCommand * singleton_wire = new AddWireCmd(m_currentWiringWire, this);
    this->wiringEventLeftMouseClickCommonComplexSingletonWire(singleton_wire);
    
    /* if connect finalize */
    if(this->m_currentWiringWire->port2()->hasConnection()) {
      this->wiringEventMouseClickFinalize(); 
      this->m_wiringState = NO_WIRE;
    }
    else  {
      wiringEventLeftMouseClickAddSegment();
      this->m_wiringState = COMPLEX_WIRE;
    }
    return;
  }
  if(this->m_wiringState == COMPLEX_WIRE) {
    if(this->m_currentWiringWire->overlap()) 
      return;
    
    QUndoCommand * complex_wire = new WireStateChangeCmd(this->m_currentWiringWire,
							 this->m_currentWiringWire->storedState(),
							 this->m_currentWiringWire->currentState());
    
    this->wiringEventLeftMouseClickCommonComplexSingletonWire(complex_wire);
    
    if(this->m_currentWiringWire->port2()->hasConnection()) {
      /* finalize */
      this->wiringEventMouseClickFinalize(); 
      this->m_wiringState = NO_WIRE;
    } else  {
      wiringEventLeftMouseClickAddSegment();
      this->m_wiringState = COMPLEX_WIRE;
    }
    return;
  }
}

/*!\brief Right mouse click wire event. This is finish wire event
 */
void SchematicScene::wiringEventRightMouseClick()
{
 
  /* state machine */
  if(this->m_wiringState == NO_WIRE) {
    this->m_wiringState = NO_WIRE;
    return;
  }
  if(this->m_wiringState ==  SINGLETON_WIRE) {
    /* check overlap */
    if(this->m_currentWiringWire->overlap())
      return;
    
    /* do wiring */
    QUndoCommand * singleton_wire = new AddWireCmd(m_currentWiringWire, this);
    this->wiringEventLeftMouseClickCommonComplexSingletonWire(singleton_wire);
    
    /* finalize */
    this->wiringEventMouseClickFinalize(); 
    this->m_wiringState = NO_WIRE;
    return;
  }
  if(this->m_wiringState == COMPLEX_WIRE) {
    if(this->m_currentWiringWire->overlap())
      return;
    
    /* do wiring */
    QUndoCommand * complex_wire = new WireStateChangeCmd(this->m_currentWiringWire,
							 this->m_currentWiringWire->storedState(),
							 this->m_currentWiringWire->currentState());
    this->wiringEventLeftMouseClickCommonComplexSingletonWire(complex_wire);
    
    /* finalize */
    this->wiringEventMouseClickFinalize(); 
    this->m_wiringState = NO_WIRE;
    return;
  }
}

/*!\brief Mouse click wire event 
  \param Event: mouse event
  \param pos: coordinate of mouse action point (rounded if needed)
*/
void SchematicScene::wiringEventMouseClick(const MouseActionEvent *event, const QPointF &pos)
{
  /* left click */
  if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) 
    return wiringEventLeftMouseClick(pos);
  /* right click */
  if((event->buttons() & Qt::RightButton) == Qt::RightButton) 
    return wiringEventRightMouseClick();
  return;
}


/*!\brief Mouse move wire event 
  \param pos: coordinate of mouse action point (rounded if needed)
*/
void SchematicScene::wiringEventMouseMove(const QPointF &pos)
{
  if(this->m_wiringState != NO_WIRE) {
    QPointF newPos = this->m_currentWiringWire->mapFromScene(pos);
    this->m_currentWiringWire->movePort2(newPos);
  }
}

/*!\brief Wiring event */
void SchematicScene::wiringEvent(MouseActionEvent *event)
{
  /* round */
  QPointF pos;
  pos = this->smartNearingGridPoint(event->scenePos());

  /* press mouse */
  if(event->type() == QEvent::GraphicsSceneMousePress) 
    return this->wiringEventMouseClick(event,pos);
  /* move mouse */
  else if(event->type() == QEvent::GraphicsSceneMouseMove) 
    return this->wiringEventMouseMove(pos);
}



/*!\todo document */
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


/*!\brief Mirror an item list 
  \param items: item to mirror
  \param opt: undo option
  \param axis: mirror axis
  \todo Create a custom undo class for avoiding if
  \note assert X or Y axis
*/
void SchematicScene::mirrorItems(QList<QucsItem*> &items,
				 const Qucs::UndoOption opt,
				 const Qt::Axis axis)
{
  Q_ASSERT(axis == Qt::XAxis || axis == Qt::YAxis);
  
  /* prepare undo stack */
  if(opt == Qucs::PushUndoCmd)
    if(axis == Qt::XAxis)
      this->m_undoStack->beginMacro(QString("Mirror X"));
    else
      this->m_undoStack->beginMacro(QString("Mirror Y"));
  
  /* disconnect item before mirroring */
  this->disconnectItems(items, opt);
  
  /* mirror */
  MirrorItemsCmd *cmd = new MirrorItemsCmd(items, axis);
  if(opt == Qucs::PushUndoCmd) {
    this->m_undoStack->push(cmd);
  }
  else {
    cmd->redo();
    delete cmd;
  }

  /* try to reconnect */
  this->connectItems(items, opt);

  /* end undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->endMacro();
}


/*! Mirror event 
  \param event: event
  \param axis: mirror axis
*/
void SchematicScene::mirroringEvent(const MouseActionEvent *event, 
				    const Qt::Axis axis)
{
  /* select item */
  QList<QGraphicsItem*> _list = this->items(event->scenePos());
  /* filters item */
  QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
  if(!qItems.isEmpty()) {
    /* mirror */
    this->mirrorItems(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd, axis);
    this->setModified(true);
  } 
}

/*! mirror X event
  \note right button mirror Y
*/
void SchematicScene::mirroringXEvent(const MouseActionEvent *event)
{
  if(event->type() != QEvent::GraphicsSceneMousePress)
    return;
   
  if((event->buttons() & Qt::LeftButton) == Qt::LeftButton)
    return this->mirroringEvent(event, Qt::XAxis);
  if((event->buttons() & Qt::RightButton) == Qt::RightButton)
    return this->mirroringEvent(event, Qt::YAxis);

  return;
}

/*! mirror Y event 
  \note right button mirror X
*/
void SchematicScene::mirroringYEvent(const MouseActionEvent *event)
{
  if(event->type() != QEvent::GraphicsSceneMousePress)
    return;

  if((event->buttons() & Qt::LeftButton) == Qt::LeftButton)
    return this->mirroringEvent(event, Qt::YAxis);
  if((event->buttons() & Qt::RightButton) == Qt::RightButton)
    return this->mirroringEvent(event, Qt::XAxis);
  
  return;
}


/******************************************************************
 *
 *                   Rotate
 *
 *****************************************************************/


/*!\brief Rotate an item list 
  \param items: item list
  \param opt: undo option
  \param diect: is rotation in trigonometric sense
  \todo Create a custom undo class for avoiding if
*/
void SchematicScene::rotateItems(QList<QucsItem*> &items, 
				 const Qucs::AngleDirection dir,
				 const Qucs::UndoOption opt)
{
  /* setup undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack
      ->beginMacro(dir == Qucs::Clockwise ?
		   QString("Rotate Clockwise") :
		   QString("Rotate Anti-Clockwise"));
  
  /* disconnect */
  this->disconnectItems(items, opt);
  
  /* rotate */
  RotateItemsCmd *cmd = new RotateItemsCmd(items, dir);
  if(opt == Qucs::PushUndoCmd) {
    this->m_undoStack->push(cmd);
  }
  else {
    cmd->redo();
    delete cmd;
  }

  /* reconnect */
  this->connectItems(items, opt);
  
  /* finish undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->endMacro();
}


/*!Rotate item 
  \note right anticlockwise 
*/  
void SchematicScene::rotatingEvent(MouseActionEvent *event)
{
  Qucs::AngleDirection angle;
  
  if(event->type() != QEvent::GraphicsSceneMousePress)
    return;
   
  /* left == clock wise */
  if((event->buttons() & Qt::LeftButton) == Qt::LeftButton)
    angle = Qucs::Clockwise;
  /* right == anticlock wise */
  else if((event->buttons() & Qt::RightButton) == Qt::RightButton)
    angle = Qucs::AntiClockwise;
  /* avoid angle unitialized */
  else
    return;

  /* get items */
  QList<QGraphicsItem*> _list = this->items(event->scenePos());
  /* filter item */
  QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
  if(!qItems.isEmpty()) {
    this->rotateItems(QList<QucsItem*>() << qItems.first(), angle, Qucs::PushUndoCmd);
    this->setModified(true);
  }
}
/***************************************************************************
 *
 *   Distribute element 
 *
 ***************************************************************************/


/*!Short function for qsort sort by abscissa */
static inline bool pointCmpFunction_X(const QucsItem *lhs, const QucsItem  *rhs)
{
  return lhs->pos().x() < rhs->pos().x();
}

/*!Short function for qsort sort by abscissa */
static inline bool pointCmpFunction_Y(const QucsItem *lhs, const QucsItem  *rhs)
{
  return lhs->pos().y() < rhs->pos().y();
}

/* Distribute horizontally
   \param items: items to distribute 
   \todo Why not filter wire ??
*/
void SchematicScene::distributeElementsHorizontally(QList<QucsItem*> items)
{
  qreal x1, x2, x, dx;
  QPointF newPos;

  /* undo */
  this->m_undoStack->beginMacro("Distribute horizontally");

  /* disconnect */
  this->disconnectItems(items, Qucs::PushUndoCmd);

  /*sort item */
  qSort(items.begin(), items.end(), pointCmpFunction_X);
  x1 = items.first()->pos().x();
  x2 = items.last()->pos().x();

  /* compute step */
  dx = (x2 - x1) / (items.size() - 1);
  x = x1;
  
  foreach(QucsItem *item, items) {
    /* why not filter wire ??? */
    if(item->isWire()) 
      continue;
    
    /* compute new position */
    newPos = item->pos();
    newPos.setX(x);
    x += dx;

    /* move to new pos */
    this->m_undoStack->push(new MoveCmd(item, item->pos(), newPos));
  }

  /* try to reconnect */
  this->connectItems(items, Qucs::PushUndoCmd);

  /* end command */
  this->m_undoStack->endMacro();

}

/* Distribute vertically
   \param items: items to distribute 
   \todo Why not filter wire ??
*/
void SchematicScene::distributeElementsVertically(QList<QucsItem*> items)
{
  qreal y1, y2, y, dy;
  QPointF newPos;

  /* undo */
  this->m_undoStack->beginMacro("Distribute vertically");

  /* disconnect */
  this->disconnectItems(items, Qucs::PushUndoCmd);

  /*sort item */
  qSort(items.begin(), items.end(), pointCmpFunction_Y);
  y1 = items.first()->pos().y();
  y2 = items.last()->pos().y();

  /* compute step */
  dy = (y2 - y1) / (items.size() - 1);
  y = y1;
  
  foreach(QucsItem *item, items) {
    /* why not filter wire ??? */
    if(item->isWire()) 
      continue;
    
    /* compute new position */
    newPos = item->pos();
    newPos.setY(y);
    y += dy;

    /* move to new pos */
    this->m_undoStack->push(new MoveCmd(item, item->pos(), newPos));
  }

  /* try to reconnect */
  this->connectItems(items, Qucs::PushUndoCmd);

  /* end command */
  this->m_undoStack->endMacro();

}

/*!\brief Distribute elements 
  
  Distribute elements ie each element is equally spaced
  \param orientation: distribute according to orientation
  \todo filter wire ??? Do not distribute wire ??
 */
bool SchematicScene::distributeElements(const Qt::Orientation orientation)
{
  QList<QGraphicsItem*> gItems = selectedItems();
  QList<QucsItem*> items = filterItems<QucsItem>(gItems);

  /* could not distribute single items */
  if(items.size() < 2) 
    return false;

  if(orientation == Qt::Horizontal) 
    this->distributeElementsHorizontally(items);
  else
    this->distributeElementsVertically(items);
  return true;
}


/***********************************************************************
 *
 * Alignement
 *
 ***********************************************************************/


/* Check if alignement flags are compatible used in assert */
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


/* return a string corresponding to alignement */
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

/*!\brief align element 
  \param alignment: alignement used
  \todo use smart alignment ie: port alignement
  \todo implement snap on grid
  \todo string of undo
  \todo filter wires ???
*/
bool SchematicScene::alignElements(const Qt::Alignment alignment)
{
  Q_ASSERT(checkAlignementFlag(alignment));

  QList<QGraphicsItem*> gItems = selectedItems();
  QList<QucsItem*> items = filterItems<QucsItem>(gItems, DontRemoveItems);
  

  /* Could not align less than two elements */
  if(items.size() < 2) 
    return false;

  /* setup undo */
  this->m_undoStack->beginMacro(Alignment2QString(alignment));

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
    this->m_undoStack->push(new MoveCmd(*it, itemPos, itemPos + delta));;
    ++it;
  }

  /* reconnect items */
  connectItems(items, Qucs::PushUndoCmd);

  /* finish undo */
  this->m_undoStack->endMacro();
  return true;
}


/*************************************************************************
 *
 *         Set on grid
 *
 *************************************************************************/


/*!\brief Set item on grid 
  \param items: item list
  \param opt: undo option
  \todo Create a custom undo class for avoiding if
*/
void SchematicScene::setItemsOnGrid(QList<QucsItem*> &items, 
				    const Qucs::UndoOption opt)
{
  QList<QucsItem*> itemsNotOnGrid;
  /* create a list of item not on grid */
  foreach(QucsItem* item, items) {
    QPointF pos = item->pos();
    QPointF gpos = this->nearingGridPoint(pos);
    if(pos != gpos)
      itemsNotOnGrid << item;
  }
  
  if(itemsNotOnGrid.isEmpty())
    return;
  
  /* setup undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->beginMacro(QString("Set on grid"));
  
  /* disconnect item */
  this->disconnectItems(itemsNotOnGrid, opt);
  
  /* put item on grid */
  foreach(QucsItem *item, itemsNotOnGrid) {
    QPointF pos = item->pos();
    QPointF gridPos = this->nearingGridPoint(pos);
    
    if(opt == Qucs::PushUndoCmd)
      this->m_undoStack->push(new MoveCmd(item, pos, gridPos));
    else 
      item->setPos(gridPos);
  }
  
  /* try to create connection */
  this->connectItems(itemsNotOnGrid, opt);
  
  /* finalize undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->endMacro();
}


/*! Set on grid event */
void SchematicScene::settingOnGridEvent(const MouseActionEvent *event)
{
  /*! only left click */
  if(event->type() != QEvent::GraphicsSceneMousePress)
    return;
  if((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
    return;
  
  /* do action */
  QList<QGraphicsItem*> _list = this->items(event->scenePos());
  if(!_list.isEmpty()) {
    QList<QucsItem*> _items = filterItems<QucsItem>(_list);
    
    if(!_list.isEmpty()) {
      setItemsOnGrid(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
      setModified(true);
    }
  }
}

/******************************************************************************
 *
 *     Active status
 *
 *****************************************************************************/

/*!\brief Toggle active status 
  \param items: item list
  \param opt: undo option
  \todo Create a custom undo class for avoiding if
  \todo Change direction of toogle
*/
void SchematicScene::toggleActiveStatus(QList<QucsItem*> &items, 
					const Qucs::UndoOption opt)
{
  /* Apply only to components */
  QList<Component*> components = filterItems<Component>(items);
  if(components.isEmpty())
    return;

  /* setup undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->beginMacro(QString("Toggle active status"));

  /* toogle */
  ToggleActiveStatusCmd *cmd = new ToggleActiveStatusCmd(components);
  if(opt == Qucs::PushUndoCmd) {
    this->m_undoStack->push(cmd);
  }
  else {
    cmd->redo();
    delete cmd;
  }

  /* finalize undo */
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->endMacro();
}


/*! Activate deactivate 
  \todo implement left right behavior
*/
void SchematicScene::changingActiveStatusEvent(const MouseActionEvent *event)
{
  if(event->type() != QEvent::GraphicsSceneMousePress)
    return;
  if((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
    return;
  
  QList<QGraphicsItem*> _list = items(event->scenePos());
  QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
  if(!qItems.isEmpty()) {
    this->toggleActiveStatus(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd);
    this->setModified(true);
  }
}


/*************************************************************
 *
 *          DELETE
 *
 *************************************************************/



/*!\brief delete an item list 
  \param items: item list
  \param opt: undo option
  \todo Document
  \todo Create a custom undo class for avoiding if
  \todo removeitems delete item use asingle name scheme
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
    this->m_undoStack->beginMacro(QString("Delete items"));
    
    /* diconnect then remove */
    this->disconnectItems(items, opt);
    this->m_undoStack->push(new RemoveItemsCmd(items, this));

    this->m_undoStack->endMacro();
  }
}

/*!\brief Left button deleting event: delete items 
  \param pos: pos clicked
*/ 
void SchematicScene::deletingEventLeftMouseClick(const QPointF &pos)
{
  /* create a list of items */
  QList<QGraphicsItem*> _list = items(pos);
  if(!_list.isEmpty()) {
    QList<QucsItem*> _items = filterItems<QucsItem>(_list);
    
    if(!_items.isEmpty()) {
      this->deleteItems(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
      this->setModified(true);
    }
  }
}


/*!\brief Left button deleting event: delete items 
  \param pos: pos clicked
*/ 
void SchematicScene::deletingEventRightMouseClick(const QPointF &pos)
{
  /* create a list of items */
  QList<QGraphicsItem*> _list = items(pos);
  if(!_list.isEmpty()) {
    QList<QucsItem*> _items = filterItems<QucsItem>(_list);
    
    if(!_items.isEmpty()) {
      this->disconnectItems(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
      this->setModified(true);
    }
  }
}


/*!\brief delete action
   
  Delete action: left click delete, right click disconnect item
*/
void SchematicScene::deletingEvent(const MouseActionEvent *event)
{
  if(event->type() != QEvent::GraphicsSceneMousePress)
    return;

  QPointF pos = event->scenePos();
  /* left click */
  if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) 
    return this->deletingEventLeftMouseClick(pos);
  /* right click */
  if((event->buttons() & Qt::RightButton) == Qt::RightButton) 
    return this->deletingEventRightMouseClick(pos);
  return;
}

/*********************************************************************
 *
 *  Connect -- disconnect 
 *
 ********************************************************************/

/*! Automatically connect items if port or wire overlap 
    \param qItems: item to connect
    \param opt: undo option
    \todo remove the cast and create a class connectable item
*/
  
void SchematicScene::connectItems(const QList<QucsItem*> &qItems, 
				  const Qucs::UndoOption opt)
{
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->beginMacro(tr("Connect items"));

  /* remove this cast */
  foreach(QucsItem *qItem, qItems) {
    if(qItem->isComponent())
      qucsitem_cast<Component*>(qItem)->checkAndConnect(opt);
    else if(qItem->isWire())
      qucsitem_cast<Wire*>(qItem)->checkAndConnect(opt);
  }

  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->endMacro();
}


/*! Disconnect an item from wire or other components
    \param qItems: item to connect
    \param opt: undo option
    \todo remove the cast and create a class connectable item
 */
void SchematicScene::disconnectItems(const QList<QucsItem*> &qItems,
				     const Qucs::UndoOption opt)
{
  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->beginMacro(QString());

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
      if( other == NULL)
	continue;

      if(opt == Qucs::PushUndoCmd)
	this->m_undoStack->push(new DisconnectCmd(p, other));
      else
	p->disconnectFrom(other);
    }
  }

  if(opt == Qucs::PushUndoCmd)
    this->m_undoStack->endMacro();
}


/*! Zoom at point event 
    \todo document
*/
void SchematicScene::zoomingAtPointEvent(MouseActionEvent *event)
{
  QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
  SchematicView *sv = qobject_cast<SchematicView*>(v);
  if(!sv) 
    return;
  QPoint viewPoint = sv->mapFromScene(event->scenePos());

  if(event->type() == QEvent::GraphicsSceneMousePress) {
    if(!m_zoomBand) m_zoomBand = new QRubberBand(QRubberBand::Rectangle);
    m_zoomBand->setParent(sv->viewport());
    m_zoomBand->show();
    m_zoomRect.setRect(event->scenePos().x(), event->scenePos().y(), 0, 0);
    QRect rrect = sv->mapFromScene(m_zoomRect).boundingRect().normalized();
    m_zoomBand->setGeometry(rrect);
  }
  else if(event->type() == QEvent::GraphicsSceneMouseMove) {
    if(m_zoomBand && m_zoomBand->isVisible() && m_zoomBand->parent() == sv->viewport()) {
      m_zoomRect.setBottomRight(event->scenePos());
      QRect rrect = sv->mapFromScene(m_zoomRect).boundingRect().normalized();
      m_zoomBand->setGeometry(rrect);
    }
  }
  else {
    if(m_zoomBand->geometry().isNull()) {

      sv->zoomIn();
      QPointF afterScalePoint(sv->mapFromScene(event->scenePos()));
      int dx = (afterScalePoint - viewPoint).toPoint().x();
      int dy = (afterScalePoint - viewPoint).toPoint().y();

      QScrollBar *hb = sv->horizontalScrollBar();
      QScrollBar *vb = sv->verticalScrollBar();

      hb->setValue(hb->value() + dx);
      vb->setValue(vb->value() + dy);
    }
    else {
      sv->fitInView(m_zoomRect, Qt::KeepAspectRatio);
    }
    m_zoomBand->hide();
  }
}

void SchematicScene::placeAndDuplicatePainting()
{
  if(!m_paintingDrawItem) 
    return;

  QPointF dest = m_paintingDrawItem->pos();
  placeItem(m_paintingDrawItem, dest, Qucs::PushUndoCmd);

  m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
  m_paintingDrawItem->setPaintingRect(QRectF(-2, -2, 4, 4));
  if(m_paintingDrawItem->type() == GraphicText::Type)
    static_cast<GraphicText*>(m_paintingDrawItem)->setText("");
}

/*!\todo Document */
void SchematicScene::paintingDrawEvent(MouseActionEvent *event)
{
  if(!m_paintingDrawItem)
    return;
  EllipseArc *arc = 0;
  GraphicText *text = 0;
  QPointF dest = event->scenePos();
  dest += m_paintingDrawItem->paintingRect().topLeft();

  if(m_paintingDrawItem->type() == EllipseArc::Type)
    arc = static_cast<EllipseArc*>(m_paintingDrawItem);
  if(m_paintingDrawItem->type() == GraphicText::Type)
    text = static_cast<GraphicText*>(m_paintingDrawItem);


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
	while(angle < 0)
	  angle += 360;
	arc->setStartAngle(int(angle));
      }

      else if(m_paintingDrawClicks == 3) {
	int span = angle - arc->startAngle();
	while(span < 0) span += 360;
	arc->setSpanAngle(span);
      }
    }

    else if(m_paintingDrawClicks == 1) {
      QRectF rect = m_paintingDrawItem->paintingRect();
      rect.setBottomRight(m_paintingDrawItem->mapFromScene(event->scenePos()));
      m_paintingDrawItem->setPaintingRect(rect);
    }
  }
}

/*!\todo Document */
void SchematicScene::insertingItemsEvent(MouseActionEvent *event)
{
  if(event->type() == QEvent::GraphicsSceneMousePress) {
    clearSelection();
    foreach(QucsItem *item, m_insertibles) {
      removeItem(item);
    }
    m_undoStack->beginMacro(QString());
    foreach(QucsItem *item, m_insertibles) {
      QucsItem *copied = item->copy(0);
      /* round */
      placeItem(copied, this->smartNearingGridPoint(item->pos()), Qucs::PushUndoCmd);
    }
    m_undoStack->endMacro();
    foreach(QucsItem *item, m_insertibles) {
      addItem(item);
      item->setSelected(true);
    }
  }
  else if(event->type() == QEvent::GraphicsSceneMouseMove) {
    QPointF delta = event->scenePos() - m_insertActionMousePos;

    foreach(QucsItem *item, m_insertibles) {
      item->setPos(this->smartNearingGridPoint(item->pos() + delta));
    }
    m_insertActionMousePos = this->smartNearingGridPoint(event->scenePos());
  }
}

/*!\todo Document */
void SchematicScene::insertingWireLabelEvent(MouseActionEvent *event)
{
  Q_UNUSED(event);
  //TODO:
}

/*!\todo Document */
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
	    m_undoStack->beginMacro(QString());
	  }
	}
      }

      if(!m_areItemsMoving)
	return;
      disconnectDisconnectibles();
      QGraphicsScene::mouseMoveEvent(e);
      QPointF delta = this->smartNearingGridPoint(e->scenePos() - e->lastScenePos());
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

/*!\todo Document */
void SchematicScene::processForSpecialMove(QList<QGraphicsItem*> _items)
{
  disconnectibles.clear();
  movingWires.clear();
  grabMovingWires.clear();

  foreach(QGraphicsItem *item, _items) {
    Component *c = qucsitem_cast<Component*>(item);
    storePos(item, this->smartNearingGridPoint(item->scenePos()));
    if(c) {
      //check for disconnections and wire resizing.
      foreach(Port *port, c->ports()) {
	QList<Port*> *connections = port->connections();
	if(!connections) continue;

	foreach(Port *other, *connections) {
	  if(other == port ) continue;


	  Component *otherComponent = 0;
	  if((otherComponent = other->owner()->component())
	     && !otherComponent->isSelected()) {
	    disconnectibles << c;
	    break;
	  }


	  Wire *wire = other->owner()->wire();
	  if(wire) {
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

  //    qDebug() << "\n############Process special move"
  //             << "\n\tDisconnectibles.size() = " << disconnectibles.size()
  //             << "\n\tMoving wires.size() = " << movingWires.size()
  //             << "\n\tGrab moving wires.size() = " << grabMovingWires.size()
  //             <<"\n#############\n";
}

/*!\todo Document */
void SchematicScene::disconnectDisconnectibles()
{
  QSet<Component*> remove;
  foreach(Component *c, disconnectibles) {
    int disconnections = 0;
    foreach(Port *port, c->ports()) {
      if(!port->connections()) continue;

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

/*!\todo Document */
void SchematicScene::specialMove(qreal dx, qreal dy)
{
  foreach(Wire *wire, movingWires) {
    wire->hide();
    if(wire->port1()->connections()) {
      Port *other = 0;
      foreach(Port *o, *(wire->port1()->connections())) {
	if(o != wire->port1()) {
	  other = o;
	  break;
	}
      }
      if(other) {
	wire->movePort(wire->port1()->connections(), this->smartNearingGridPoint(other->scenePos()));
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
	wire->movePort(wire->port2()->connections(), this->smartNearingGridPoint(other->scenePos()));
      }
    }
  }

  foreach(Wire *wire, grabMovingWires) {
    wire->hide();
    wire->grabMoveBy(dx, dy);
  }
}

/*!\todo document */
void SchematicScene::endSpecialMove()
{
  disconnectibles.clear();
  foreach(QGraphicsItem *item, selectedItems()) {
    m_undoStack->push(new MoveCmd(item, storedPos(item),
				  this->smartNearingGridPoint(item->scenePos())));
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

/*! Place an item on the scene 
  \param item: item to place
  \param: pos position where to place
  \param opt: undo option
  \warning: pos is not rounded
*/
void SchematicScene::placeItem(QucsItem *item, const QPointF &pos, const Qucs::UndoOption opt)
{
  if(item->scene() == this)
    removeItem(item);

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
    m_undoStack->beginMacro(QString());

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

/*!\todo document */
QucsItem* SchematicScene::itemForName(const QString& name, const QString& category)
{
  if(category == QObject::tr("paintings")) {
    return Painting::fromName(name);
  }

  return LibraryLoader::defaultInstance()->newComponent(name, 0, category);
}

/*\todo document */
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

/*\todo document */
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

/*!\todo document */
bool SchematicScene::isPortNumberUsed(int num) const
{
  (void) num;
  return false;
}

/*!\todo document */
void SchematicScene::setNumberUnused(int num)
{
  (void) num;
}



/*!\todo Document */
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
