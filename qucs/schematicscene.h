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

#ifndef __SCHEMATICSCENE_H
#define __SCHEMATICSCENE_H

#include <QtGui/QGraphicsScene>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtGui/QGraphicsItem>

class Node;
class QUndoStack;
class Component;
class ComponentPort;
class Wire;
class QucsItem;
class Diagram;
class Painting;

namespace Qucs
{
   enum Mode {
      SchematicMode,
      SymbolMode
   };
}

template<typename T> T qucsitem_cast(QGraphicsItem *item)
{
   bool firstCond = int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type);
   bool secondCond = !firstCond && item &&
      ((int(static_cast<T>(0)->Type) & item->type()) == (int(static_cast<T>(0)->Type)));
   bool result = firstCond | secondCond;
   return result ? static_cast<T>(item)  : 0;
}


typedef QGraphicsSceneMouseEvent MouseActionEvent;

class SchematicScene : public QGraphicsScene
{
      Q_OBJECT;
   public:
      enum MouseAction {
         Wiring,
         Deleting,
         Marking,
         Rotating,
         MirroringX,
         MirroringY,
         ChangingActiveStatus,
         SettingOnGrid,
         ZoomingAtPoint,
         InsertingItems,
         InsertingEquation,
         InsertingGround,
         InsertingPort,
         InsertingWireLabel,
         Normal
      };

      SchematicScene(QObject *parent =0);
      SchematicScene(qreal x, qreal y, qreal width, qreal height, QObject * parent = 0);
      ~SchematicScene();

      Node* nodeAt(qreal cx, qreal cy);
      Node* nodeAt(const QPointF& centre);
      Node* createNode(const QPointF& pos);
      void removeNode(Node *n);

      bool areItemsMoving() const { return m_areItemsMoving; }
      void setGrabbedWire(Wire *w);

      void insertComponent(Component *comp);
      void removeComponent(Component *comp);
      void insertWire(Wire *w);
      void removeWire(Wire *w);

      QList<Component*> components() const { return m_components; }
      QList<Painting*> paintings() const { return m_paintings; }
      QList<Wire*> wires() const { return m_wires; }
      QList<Diagram*> diagrams() const { return m_diagrams; }
      QList<Painting*> symbolPaintings() const { return m_symbolPaintings; }
      QList<Node*> nodes() const { return m_nodes; }

      void mirrorXItems(QList<QucsItem*>& items);
      void mirrorYItems(QList<QucsItem*>& items);
      void rotateItems(QList<QucsItem*>& items);
      void deleteItems(QList<QucsItem*>& items);
      void setItemsOnGrid(QList<QucsItem*>& items);
      void toggleActiveStatus(QList<QucsItem*>& components);

      QString fileName() const { return m_fileName; }
      void setFileName(const QString& fn);

      bool isModified() const { return m_modified; }

      QPointF nearingGridPoint(const QPointF &pos);

      QUndoStack* undoStack() { return m_undoStack; }

      uint gridWidth() const { return m_gridWidth; }
      void setGridWidth(uint width) { setGridSize(width, gridHeight()); }

      uint gridHeight() const { return m_gridHeight; }
      void setGridHeight(uint height) { setGridSize(gridWidth(), height); }

      void setGridSize(uint width, uint height);

      bool isGridVisible() const { return m_gridVisible; }
      void setGridVisible(bool visibility);

      QString dataSet() const { return m_dataSet; }
      void setDataSet(const QString& str);

      QString dataDisplay() const { return m_dataDisplay; }
      void setDataDisplay(const QString& disp);

      bool opensDataDisplay() const { return m_opensDataDisplay; }
      void setOpensDataDisplay(bool b);

      bool isFrameVisible() const { return m_frameVisible; }
      void setFrameVisible(bool vis);

      QStringList frameTexts() const { return m_frameTexts; }
      void setFrameTexts(const QStringList& texts);

      Qucs::Mode currentMode() const { return m_currentMode; }
      void setMode(Qucs::Mode mode);

      MouseAction currentMouseAction() const { return m_currentMouseAction; }
      void setCurrentMouseAction(MouseAction ma);

      void cutItems(QList<QucsItem*> items);
      void copyItems(QList<QucsItem*> items);
      void paste();

   public slots:
      void setModified(bool m = true);

   signals:
      void modificationChanged(bool changed);
      void fileNameChanged(const QString& file);
      void stateUpdated();

   protected:
      void drawBackground(QPainter *p, const QRectF& r);

      void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
      void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
      void dropEvent(QGraphicsSceneDragDropEvent * event);

      void mousePressEvent(QGraphicsSceneMouseEvent *e);
      void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
      void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
      void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

      // Custom handlers
      void wiringEvent(MouseActionEvent *e);
      void deletingEvent(MouseActionEvent *e);
      void markingEvent(MouseActionEvent *e);
      void rotatingEvent(MouseActionEvent *e);
      void mirroringXEvent(MouseActionEvent *e);
      void mirroringYEvent(MouseActionEvent *e);
      void changingActiveStatusEvent(MouseActionEvent *e);
      void settingOnGridEvent(MouseActionEvent *e);
      void zoomingAtPointEvent(MouseActionEvent *e);
      void insertingItemsEvent(MouseActionEvent *e);
      void insertingEquationEvent(MouseActionEvent *event);
      void insertingGroundEvent(MouseActionEvent *event);
      void insertingPortEvent(MouseActionEvent *event);
      void insertingWireLabelEvent(MouseActionEvent *event);
      void normalEvent(MouseActionEvent *e);

   private:
      void init();
      void sendMouseActionEvent(QGraphicsSceneMouseEvent *e);
      void connectNodes(Node *from, Node *to);
      // A very helpful recursive function to move components after connection
      void adjustPositions(Node *of,const QPointF& delta);
      void testInsertingItems();
      //These are helper variables (aka state holders)
      bool m_areItemsMoving;
      Wire *eventWire;
      QList<Wire*> createdWires;
      Wire *m_grabbedWire;
      Node *helperNode;
      QList<Node*> m_movingNodes;
      QList<Wire*> m_resizingWires;
      QList<Wire*> m_moveResizingWires;
      QList<QucsItem*> m_alreadyMoved;
      QList<QucsItem*> m_insertingItems;

      // These are the various qucs-items on scene
      QList<Component*> m_components;
      QList<Wire*> m_wires;
      QList<Diagram*> m_diagrams;
      QList<Node*> m_nodes;
      QList<Painting*> m_paintings;
      QList<Painting*> m_symbolPaintings;

      //Document properties
      QUndoStack *m_undoStack;
      MouseAction m_currentMouseAction;
      Qucs::Mode m_currentMode;

      uint m_gridWidth;
      uint m_gridHeight;
      bool m_gridVisible;

      QString m_dataSet;
      QString m_dataDisplay;
      QString m_fileName;
      QStringList m_frameTexts;

      bool m_modified;
      bool m_opensDataDisplay;
      bool m_frameVisible;
};

#endif //__SCHEMATICSCENE_H
