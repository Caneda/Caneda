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
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QPair>

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

class SchematicScene : public QGraphicsScene
{
   public:
      SchematicScene(QObject *parent =0);
      SchematicScene ( qreal x, qreal y, qreal width, qreal height, QObject * parent = 0 );
      ~SchematicScene(){}

      Node* nodeAt(qreal cx, qreal cy);
      Node *nodeAt(const QPointF& centre);
      Node* createNode(const QPointF& pos);

      bool areItemsMoving() const;
      QUndoStack* undoStack();

      void setGrabbedWire(Wire *w);
      void insertComponent(Component *comp);
      void removeComponent(Component *comp);
      void insertWire(Wire *w, Node* n1, Node *n2);
      void insertWire(Wire *w, const QPointF& n1Pos, const QPointF& n2Pos);
      void removeWire(Wire *w);

      QList<Component*> components() const { return m_components; }
      QList<Painting*> paintings() const { return m_paintings; }
      QList<Wire*> wires() const { return m_wires; }
      QList<Diagram*> diagrams() const { return m_diagrams; }
      QList<Painting*> symbolPaintings() const { return m_symbolPaintings; }
      QList<Node*> nodes() const { return m_nodes; }

      int xGridSize() const { return m_xGridSize; }
      int yGridSize() const { return m_yGridSize; }
      bool isGridShown() const { return m_gridShown; }
      QString dataSet() const { return m_dataSet; }
      QString dataDisplay() const { return m_dataDisplay; }
      QString fileName() const { return m_fileName; }
      void setFileName(const QString& fn) { m_fileName = fn; }
      bool simOpenDpl() const { return m_simOpenDpl; }
      bool isFrameShown() const { return m_frameShown; }
      QString frameText0() const { return m_frameText0; }
      QString frameText1() const { return m_frameText1; }
      QString frameText2() const { return m_frameText2; }
      QString frameText3() const { return m_frameText3; }
      Qucs::Mode currentMode() const { return m_currentMode; }
      void setMode(Qucs::Mode mode);

      void save();

   protected:
      void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
      void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
      void dropEvent(QGraphicsSceneDragDropEvent * event);

      void mousePressEvent(QGraphicsSceneMouseEvent *e);
      void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
      void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
      void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
      void drawBackground(QPainter *p, const QRectF& r);

   private:
      void init();
      void connect(Node *from, Node *to);
      void adjustPositions(Node *of,const QPointF& delta);

      QSet<Node*> m_movingNodes;
      QSet<Wire*> m_resizingWires;
      QSet<Wire*> m_moveResizingWires;
      QSet<QucsItem*> m_alreadyMoved;

      QList<Component*> m_components;
      QList<Wire*> m_wires;
      QList<Diagram*> m_diagrams;
      QList<Node*> m_nodes;
      QList<Painting*> m_paintings;
      QList<Painting*> m_symbolPaintings;

      QUndoStack *m_undoStack;
      bool m_areItemsMoving;
      Wire *m_grabbedWire;

      //Document properties
      int m_xGridSize;
      int m_yGridSize;
      bool m_gridShown;
      QString m_dataSet;
      QString m_dataDisplay;
      QString m_fileName;
      bool m_simOpenDpl;
      bool m_frameShown;
      QString m_frameText0,m_frameText1;
      QString m_frameText2,m_frameText3;
      Qucs::Mode m_currentMode;
};

#endif //__SCHEMATICSCENE_H
