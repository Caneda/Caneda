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

#include <cmath>
#include <memory>

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
   init();
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
   init();
}


/*!\brief Default grid spacing
  \todo Must be configurable 
*/
static const uint DEFAULT_GRID_SPACE = 10;

/*!\brief Initialize a schematic scene  */
void SchematicScene::init()
{
   m_undoStack = new QUndoStack(this);
   m_gridWidth = m_gridHeight = DEFAULT_GRID_SPACE;
   m_currentMode = Qucs::SchematicMode;
   m_frameVisible = false;
   m_modified = false;
   m_snapToGrid = true;
   m_gridVisible = true;
   m_opensDataDisplay = true;
   m_frameTexts = QStringList() << tr("Title") << tr("Drawn By:") << tr("Date:") << tr("Revision:");
   m_macroProgress = false;
   m_areItemsMoving = false;
   m_shortcutsBlocked = false;
   m_currentWiringWire = 0;
   m_paintingDrawItem = 0;
   m_paintingDrawClicks = 0;
   m_zoomBand = 0;
   m_isWireCmdAdded = false;
   setCurrentMouseAction(Normal);
}

/*!\brief Default Destructor */
SchematicScene::~SchematicScene()
{
   delete m_undoStack;
}

/*!\todo Chck usefulness */
void SchematicScene::test()
{
}

/*!\brief Mirror an item list 
   \param items: item list
   \param opt: undo option
   \todo Document
   \todo Create a custom undo class for avoiding if
   \todo Add string to begin macro
*/
void SchematicScene::mirrorXItems(QList<QucsItem*> &items, 
				  const Qucs::UndoOption opt)
{
   if(opt == Qucs::PushUndoCmd)
      m_undoStack->beginMacro(QString());

   disconnectItems(items, opt);

   MirrorItemsCmd *cmd = new MirrorItemsCmd(items, Qt::XAxis);
   if(opt == Qucs::PushUndoCmd) {
      m_undoStack->push(cmd);
   }
   else {
      cmd->redo();
      delete cmd;
   }

   connectItems(items, opt);

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->endMacro();
}


/*!\brief Mirror an item list 
   \param items: item list
   \param opt: undo option
   \todo Document
   \todo Create a custom undo class for avoiding if
   \todo Add string to begin macro
   \todo Factorize with previous command 
*/
void SchematicScene::mirrorYItems(QList<QucsItem*> &items, 
				  const Qucs::UndoOption opt)
{
   if(opt == Qucs::PushUndoCmd)
      m_undoStack->beginMacro(QString());

   disconnectItems(items, opt);

   MirrorItemsCmd *cmd = new MirrorItemsCmd(items, Qt::YAxis);
   if(opt == Qucs::PushUndoCmd) {
      m_undoStack->push(cmd);
   }
   else {
      cmd->redo();
      delete cmd;
   }

   connectItems(items, opt);

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->endMacro();
}

/*!\brief Rotate an item list 
   \param items: item list
   \param opt: undo option
   \todo Document
   \todo Create a custom undo class for avoiding if
   \todo Add string to begin macro
*/
void SchematicScene::rotateItems(QList<QucsItem*> &items, 
				 const Qucs::UndoOption opt)
{
   if(opt == Qucs::PushUndoCmd)
      m_undoStack->beginMacro(QString());

   disconnectItems(items, opt);

   RotateItemsCmd *cmd = new RotateItemsCmd(items);
   if(opt == Qucs::PushUndoCmd) {
      m_undoStack->push(cmd);
   }
   else {
      cmd->redo();
      delete cmd;
   }

   connectItems(items, opt);

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->endMacro();
}

/*!\brief delete an item list 
   \param items: item list
   \param opt: undo option
   \todo Document
   \todo Create a custom undo class for avoiding if
   \todo Add string to begin macro
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

      m_undoStack->beginMacro(QString());

      disconnectItems(items, opt);
      m_undoStack->push(new RemoveItemsCmd(items, this));

      m_undoStack->endMacro();
   }
}

/*!\brief Set item on list 
   \param items: item list
   \param opt: undo option
   \todo Document
   \todo Create a custom undo class for avoiding if
   \todo Add string to begin macro
*/
void SchematicScene::setItemsOnGrid(QList<QucsItem*> &items, 
				    const Qucs::UndoOption opt)
{
   QList<QucsItem*> itemsNotOnGrid;
   foreach(QucsItem* item, items) {
      QPointF pos = item->pos();
      QPointF gpos = nearingGridPoint(pos);

      if(pos != gpos)
         itemsNotOnGrid << item;
   }

   if(itemsNotOnGrid.isEmpty())
      return;

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->beginMacro(QString());

   disconnectItems(itemsNotOnGrid, opt);

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

   connectItems(itemsNotOnGrid, opt);

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->endMacro();
}

/*!\brief Set item on list 
   \param items: item list
   \param opt: undo option
   \todo Document
   \todo Create a custom undo class for avoiding if
   \todo Add string to begin macro
*/
void SchematicScene::toggleActiveStatus(QList<QucsItem*> &items, 
					const Qucs::UndoOption opt)
{
   QList<Component*> components = filterItems<Component>(items);
   if(components.isEmpty())
      return;

   ToggleActiveStatusCmd *cmd = new ToggleActiveStatusCmd(components);
   if(opt == Qucs::PushUndoCmd) {
      m_undoStack->push(cmd);
   }
   else {
      cmd->redo();
      delete cmd;
   }
}

/*!\brief Data set file suffix 
   \todo use something more explicit 
*/
static const char dataSetSuffix[] = ".dat"; 
/*!\brief Data display file suffix */
static const char dataDisplaySuffix[] = ".dpl";

/*!\brief Set schematic and datafile name 
   \param name: name to set
*/
void SchematicScene::setFileName(const QString& name)
{
   if(name == m_fileName || name.isEmpty())
      return;
   m_fileName = name;
   QFileInfo info(m_fileName);
   m_dataSet = info.baseName() + dataSetSuffix;
   m_dataDisplay = info.baseName() + dataDisplaySuffix;
   emit fileNameChanged(m_fileName);
   emit titleToBeUpdated();
}

/*! Get nearest point on grid 
    \param pos: current position to be rounded
    \return rounded position
*/
QPointF SchematicScene::nearingGridPoint(const QPointF &pos)
{
   QPoint point = pos.toPoint();
   int x = point.x();
   int y = point.y();

   if(x<0) 
     x -= (m_gridWidth >> 1) - 1;
   else 
     x += m_gridWidth >> 1;
   x -= x % m_gridWidth;

   if(y<0) 
     y -= (m_gridHeight >> 1) - 1;
   else 
     y += m_gridHeight >> 1;
   y -= y % m_gridHeight;

   return QPointF(x,y);
}

/*! Set grid size 
    \param width: grid width in pixel
    \param height: grid height in pixel
*/
void SchematicScene::setGridSize(const uint width, const uint height)
{
  /* avoid redrawing */
   if(m_gridWidth == width && m_gridHeight == height)
      return;

   m_gridWidth = width;
   m_gridHeight = height;

   if(isGridVisible())
      update();
}

/*! Set grid visibility 
  \param visibility: Grid visibility 
*/
void SchematicScene::setGridVisible(const bool visibility)
{
   if(m_gridVisible == visibility) 
     return;
   m_gridVisible = visibility;
   update();
}
/*!\todo Document */
void SchematicScene::setDataSet(const QString& _dataSet)
{
   m_dataSet = _dataSet;
}

/*!\todo Documenent */
void SchematicScene::setDataDisplay(const QString& display)
{
   m_dataDisplay = display;
}

/*!\todo Documenent */
void SchematicScene::setOpensDataDisplay(const bool state)
{
   m_opensDataDisplay = state;
}

/*!\todo Documenent */
void SchematicScene::setFrameVisible(const bool visibility)
{
   if(m_frameVisible == visibility) 
     return;
   m_frameVisible = visibility;
   update();
}

/*!\brief Todo document */
void SchematicScene::setFrameTexts(const QStringList& texts)
{
   m_frameTexts = texts;
   if(isFrameVisible())
      update();
}

/*!\brief Todo document */
void SchematicScene::setMode(const Qucs::Mode mode)
{
   if(m_currentMode == mode) 
     return;
   m_currentMode = mode;
   update();
   //TODO:
}

/*!\brief Set mouse action
  \param MouseAction: mouse action to set
  \todo document
*/
void SchematicScene::setCurrentMouseAction(const MouseAction action)
{
   if(m_currentMouseAction == action) 
     return;

   if(m_currentMouseAction == InsertingItems)
      blockShortcuts(false);

   if(action == InsertingItems)
      blockShortcuts(true);

   m_areItemsMoving = false;
   m_currentMouseAction = action;

   QGraphicsView::DragMode dragMode = (action == Normal) ?
      QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag;
   foreach(QGraphicsView *view, views())
      view->setDragMode(dragMode);
   resetState();
   //TODO: Implemement this appropriately for all mouse actions
}

/*!\todo Document */
void SchematicScene::resetState()
{
   setFocusItem(0);
   clearSelection();

   qDeleteAll(m_insertibles);
   m_insertibles.clear();

   if(m_currentWiringWire) {
      if(!m_isWireCmdAdded) {
         delete m_currentWiringWire;
      }
      else {
         m_currentWiringWire->show();
         m_currentWiringWire->setState(m_currentWiringWire->storedState());
         m_currentWiringWire->movePort1(m_currentWiringWire->port1()->pos());
      }
   }

   m_currentWiringWire = 0;

   m_isWireCmdAdded = false;

   delete m_paintingDrawItem;
   m_paintingDrawItem = 0;
   m_paintingDrawClicks = 0;

   delete m_zoomBand;
   m_zoomBand = 0;
}

/*! Cut items */
void SchematicScene::cutItems(QList<QucsItem*> &_items, const Qucs::UndoOption opt)
{
   copyItems(_items);
   deleteItems(_items, opt);
}

/*!\brief Copy item
   \todo Document format 
   \todo Use own mime type
*/
void SchematicScene::copyItems(QList<QucsItem*> &_items)
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

   beginInsertingItems(_items);
}

/*!\todo document */
SchematicView* SchematicScene::activeView() const
{
   if(views().isEmpty())
      return 0;
   return qobject_cast<SchematicView*>(views().first());
}

/*!\todo document */
void SchematicScene::beginInsertingItems(const QList<QucsItem*> &items)
{
   resetState();

   m_insertibles = items;

   SchematicView *active = activeView();
   if(!active) return;

   QPoint pos = active->viewport()->mapFromGlobal(QCursor::pos());
   bool cursorOnScene = active->viewport()->rect().contains(pos);

   m_insertActionMousePos = active->mapToScene(pos);

   foreach(QucsItem *item, m_insertibles) {
      item->setSelected(true);
      item->setVisible(cursorOnScene);
      if(item->isComponent()) {
         Component *comp = qucsitem_cast<Component*>(item);
         if(comp->propertyGroup())
            comp->propertyGroup()->hide();
      }
   }

   QPointF delta = active->mapToScene(pos) - centerOfItems(m_insertibles);

   foreach(QucsItem *item, m_insertibles) {
      item->moveBy(delta.x(), delta.y());
   }
}

/*!align element 
  \param alignment: alignement used
  \todo use smart alignment ie: port alignement
  \todo implement snap on grid
  \todo string of undo
*/

bool SchematicScene::alignElements(const Qt::Alignment alignment)
{
   QList<QGraphicsItem*> gItems = selectedItems();
   QList<QucsItem*> items = filterItems<QucsItem>(gItems, DontRemoveItems);
   if(items.size() < 2) return false;

   bool debugMsgPrinted = false;

   m_undoStack->beginMacro("");
   disconnectItems(items, Qucs::PushUndoCmd);

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
            delta.rx() =  rect.left() - itemRect.left(); break;

         case Qt::AlignRight :
            delta.rx() = rect.right() - itemRect.right(); break;

         case Qt::AlignTop :
            delta.ry() = rect.top() - itemRect.top(); break;

         case Qt::AlignBottom :
            delta.ry() = rect.bottom() - itemRect.bottom(); break;

         case Qt::AlignHCenter :
            delta.rx() = rect.center().x() - itemRect.center().x(); break;

         case Qt::AlignVCenter :
            delta.ry() = rect.center().y() - itemRect.center().y(); break;

         default:
            if(!debugMsgPrinted) {
               qDebug() << Q_FUNC_INFO << "Wrong alignment flag " << alignment;
               debugMsgPrinted = true;
            }
      }

      QPointF itemPos = (*it)->pos();
      m_undoStack->push(new MoveCmd(*it, itemPos, itemPos + delta));;
      ++it;
   }

   connectItems(items, Qucs::PushUndoCmd);
   m_undoStack->endMacro();
   return true;
}

/*!\todo Document */
bool pointCmpFunction_X(QucsItem *lhs, QucsItem  *rhs)
{
   return lhs->pos().x() < rhs->pos().x();
}

/*!\todo Document */
bool pointCmpFunction_Y(QucsItem *lhs, QucsItem  *rhs)
{
   return lhs->pos().y() < rhs->pos().y();
}

/*!\todo document */
bool SchematicScene::distributeElements(Qt::Orientation orientation)
{
   QList<QGraphicsItem*> gItems = selectedItems();
   QList<QucsItem*> items = filterItems<QucsItem>(gItems);
   if(items.size() < 2) return false;

   m_undoStack->beginMacro(QString());
   disconnectItems(items, Qucs::PushUndoCmd);

   qSort(items.begin(), items.end(),
         orientation == Qt::Horizontal ? pointCmpFunction_X : pointCmpFunction_Y);

   qreal x1 = items.first()->pos().x(), y1 = items.first()->pos().y();
   qreal x2 = items.last()->pos().x(), y2 = items.last()->pos().y();

   qreal dx = (x2 - x1) / (items.size() - 1);
   qreal dy = (y2 - y1) / (items.size() - 1);

   qreal x = x1, y = y1;
   foreach(QucsItem *item, items) {
      if(item->isWire()) continue;

      QPointF newPos = item->pos();
      if(orientation == Qt::Horizontal) {
         newPos.setX(x);
         x += dx;
      }
      else {
         newPos.setY(y);
         y += dy;
      }

      m_undoStack->push(new MoveCmd(item, item->pos(), newPos));
   }

   connectItems(items, Qucs::PushUndoCmd);
   m_undoStack->endMacro();
   return true;
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
void SchematicScene::blockShortcuts(bool block)
{
   if(!activeView()) return;

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

/*! \todo document */
void SchematicScene::setModified(bool m)
{
   if(m_modified != m) {
      m_modified = m;
      emit modificationChanged(m_modified);
      emit titleToBeUpdated();
   }
}

/*! This function is called when a side bar item is clicked 
    \param itemName: name of item
    \param category: categoy name
    \todo Add tracing
    \todo Split in two function
*/
bool SchematicScene::sidebarItemClicked(const QString& itemName, const QString& category)
{
   if(itemName.isEmpty()) 
     return false;

   if(category == "paintings") {
      setCurrentMouseAction(PaintingDrawEvent);
      m_paintingDrawItem = Painting::fromName(itemName);
      if(!m_paintingDrawItem) {
         setCurrentMouseAction(Normal);
         return false;
      }
      m_paintingDrawItem->setPaintingRect(QRectF(-2, -2, 4, 4));
      return true;
   }

   else {
      QucsItem *item = itemForName(itemName, category);
      if(!item) 
	return false;

      addItem(item);
      setCurrentMouseAction(InsertingItems);
      beginInsertingItems(QList<QucsItem*>() << item);

      return true;
   }
}

/*! Draw background of schematic including grid
    \param painter: Where to draw
    \param rect: Visible area
    \todo Finish visual representation
    \bug Why multiply by two
*/
void SchematicScene::drawBackground(QPainter *painter, const QRectF& rect)
{
   if(!isGridVisible())
      return QGraphicsScene::drawBackground(painter,rect);

   painter->setPen(QPen(Qt::black,0));
   painter->setBrush(Qt::NoBrush);
   painter->setRenderHint(QPainter::Antialiasing,false);

   if(rect.contains(QPointF(0.0,0.0))) {
      painter->drawLine(QLineF(-3.0, 0.0, 3.0, 0.0));
      painter->drawLine(QLineF(0.0, -3.0, 0.0, 3.0));
   }

   /* XXX: why */
   int gridWidth = m_gridWidth*2, gridHeight = m_gridHeight*2;

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
   
   /* first grid poin */
   qreal left = int(rect.left()) + gridWidth - (int(rect.left()) % gridWidth);
   qreal top = int(rect.top()) + gridHeight - (int(rect.top()) % gridHeight);
   qreal right = int(rect.right()) - (int(rect.right()) % gridWidth);
   qreal bottom = int(rect.bottom()) - (int(rect.bottom()) % gridHeight);
   qreal x,y;

   /* draw grid */
   for( x = left; x <= right; x += gridWidth)
      for( y = top; y <=bottom; y += gridHeight)
         painter->drawPoint(QPointF(x,y));

   painter->setRenderHint(QPainter::Antialiasing,true);
}

/*! \todo Document */
bool SchematicScene::event(QEvent *event)
{
   if(m_currentMouseAction == InsertingItems) {
      if(event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
         bool visible = (event->type() == QEvent::Enter);
         foreach(QucsItem *item, m_insertibles)
            item->setVisible(visible);
         if(visible) {
            SchematicView *active = activeView();
            if(!active) return QGraphicsScene::event(event);

            QPoint pos = active->viewport()->mapFromGlobal(QCursor::pos());
            m_insertActionMousePos = active->mapToScene(pos);

            QPointF delta = m_insertActionMousePos - centerOfItems(m_insertibles);

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
      blockShortcuts(true);
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
         QPointF dest = m_snapToGrid ? nearingGridPoint(event->scenePos()) : event->scenePos();

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

/*!\brief Wiring event 
   \todo undo scheme 
   \todo document
   \todo remove tracing
   \todo change mouse action
*/
void SchematicScene::wiringEvent(MouseActionEvent *event)
{
   if(event->type() == QEvent::GraphicsSceneMousePress) {
      /* create new wire */
      if(!m_currentWiringWire) {
	 qDebug() << "create new wire";
         m_currentWiringWire = new Wire(event->scenePos(), event->scenePos(), false, this);
         m_currentWiringWire->hide();
         return;
      }

      if(m_currentWiringWire->port1()->scenePos() == m_currentWiringWire->port2()->scenePos())
         return;

      m_undoStack->beginMacro(QString());

      if(!m_isWireCmdAdded) {
	qDebug() << "first wire action";
         m_currentWiringWire->removeNullLines();

         m_undoStack->push(new AddWireCmd(m_currentWiringWire, this));
         m_isWireCmdAdded = true;
      }
      else {
	 qDebug() << "next wire action";
         m_currentWiringWire->removeNullLines();
         m_undoStack->push(
            new WireStateChangeCmd(m_currentWiringWire,m_currentWiringWire->storedState(),
                                   m_currentWiringWire->currentState()));

      }
      m_currentWiringWire->checkAndConnect(Qucs::PushUndoCmd);

      m_undoStack->endMacro();

      if(m_currentWiringWire->port2()->hasConnection()) {

         m_currentWiringWire->show();
         m_currentWiringWire->movePort1(m_currentWiringWire->port1()->pos());
         m_currentWiringWire->updateGeometry();

         m_currentWiringWire = 0;
         m_isWireCmdAdded = false;
      }
      else {
         m_currentWiringWire->storeState();

         WireLines& wLinesRef = m_currentWiringWire->wireLinesRef();
         WireLine toAppend(wLinesRef.last().p2(), wLinesRef.last().p2());
         wLinesRef << toAppend << toAppend;
      }
   }

   else if(event->type() == QEvent::GraphicsSceneMouseMove) {
      if(m_currentWiringWire) {
         QPointF newPos = m_currentWiringWire->mapFromScene(event->scenePos());
         m_currentWiringWire->movePort2(newPos);
      }
   }
}

/*! delete items 
  \todo document 
*/
void SchematicScene::deletingEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress ||
      !((event->buttons() | Qt::LeftButton) == Qt::LeftButton))
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   if(!_list.isEmpty()) {
      QList<QucsItem*> _items = filterItems<QucsItem>(_list);

      if(!_items.isEmpty()) {
         deleteItems(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
         setModified(true);
      }
   }
}

/*!\todo document */
void SchematicScene::markingEvent(MouseActionEvent *event)
{
   Q_UNUSED(event);
   //TODO:
}

/*!Rotate item 
  \todo implement left : trigonometric sense right inverse sense
*/  
void SchematicScene::rotatingEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      rotateItems(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd);
      setModified(true);
   }
}

/*! mirror X event 
   \todo implement left : X right Y
   \todo Factorie with Y
*/
void SchematicScene::mirroringXEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      mirrorXItems(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd);
      setModified(true);
   }
}

/*! mirror Y event 
   \todo implement left : Y right X
   \todo Factorie with X 
*/
void SchematicScene::mirroringYEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      mirrorYItems(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd);
      setModified(true);
   }
}

/*! Activate deactivate 
    \todo implement lef right behavior
*/
void SchematicScene::changingActiveStatusEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   QList<QucsItem*> qItems = filterItems<QucsItem>(_list, DontRemoveItems);
   if(!qItems.isEmpty()) {
      toggleActiveStatus(QList<QucsItem*>() << qItems.first(), Qucs::PushUndoCmd);
      setModified(true);
   }
}

/*! Set on grid event 
    \todo check correctness
*/
void SchematicScene::settingOnGridEvent(MouseActionEvent *event)
{
   if(event->type() != QEvent::GraphicsSceneMousePress)
      return;
   QList<QGraphicsItem*> _list = items(event->scenePos());
   if(!_list.isEmpty()) {
      QList<QucsItem*> _items = filterItems<QucsItem>(_list);

      if(!_list.isEmpty()) {
         setItemsOnGrid(QList<QucsItem*>() << _items.first(), Qucs::PushUndoCmd);
         setModified(true);
      }
   }
}

/*! Zoom at point event 
    \todo document
*/
void SchematicScene::zoomingAtPointEvent(MouseActionEvent *event)
{
   QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
   SchematicView *sv = qobject_cast<SchematicView*>(v);
   if(!sv) return;
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
         placeItem(copied, item->pos(), Qucs::PushUndoCmd);
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
         item->moveBy(delta.x(), delta.y());
      }
      m_insertActionMousePos = event->scenePos();
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
         QPointF delta = e->scenePos() - e->lastScenePos();
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
      storePos(item);
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
            wire->movePort(wire->port1()->connections(), other->scenePos());
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
            wire->movePort(wire->port2()->connections(), other->scenePos());
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
      m_undoStack->push(new MoveCmd(item, storedPos(item), item->scenePos()));
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

/*!\todo document */
void SchematicScene::placeItem(QucsItem *item, QPointF pos, Qucs::UndoOption opt)
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

/*!\todo document */
void SchematicScene::disconnectItems(const QList<QucsItem*> &qItems, Qucs::UndoOption opt)
{
   if(opt == Qucs::PushUndoCmd)
      m_undoStack->beginMacro(QString());

   foreach(QucsItem *item, qItems) {
      QList<Port*> ports;
      if(item->isComponent()) {
         ports = qucsitem_cast<Component*>(item)->ports();
      }
      else if(item->isWire()) {
         ports = qucsitem_cast<Wire*>(item)->ports();
      }

      foreach(Port *p, ports) {
         Port *other = p->getAnyConnectedPort();
         if(other) {
            if(opt == Qucs::PushUndoCmd)
               m_undoStack->push(new DisconnectCmd(p, other));
            else
               p->disconnectFrom(other);
         }
      }
   }

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->endMacro();
}

/*!\todo document */
void SchematicScene::connectItems(const QList<QucsItem*> &qItems, Qucs::UndoOption opt)
{
   if(opt == Qucs::PushUndoCmd)
      m_undoStack->beginMacro(QString());

   foreach(QucsItem *qItem, qItems) {
      if(qItem->isComponent())
         qucsitem_cast<Component*>(qItem)->checkAndConnect(opt);
      else if(qItem->isWire())
         qucsitem_cast<Wire*>(qItem)->checkAndConnect(opt);
   }

   if(opt == Qucs::PushUndoCmd)
      m_undoStack->endMacro();
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
