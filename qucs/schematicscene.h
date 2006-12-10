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

class Node;
class QUndoStack;

class SchematicScene : public QGraphicsScene
{
   public:
      SchematicScene(QObject *parent =0l);
      SchematicScene ( qreal x, qreal y, qreal width, qreal height, QObject * parent = 0 );
      ~SchematicScene(){}

      Node* nodeAt(qreal cx, qreal cy);
      Node *nodeAt(const QPointF& centre);
      Node* createNode(const QPointF& pos);
      void removeNode(Node* n);
      
      const QList<Node*>& circuitNodes() const;

      QUndoStack* undoStack();

      bool areItemsMoving() const;
      bool isCommonMovingNode(Node *n) const;
   protected:
      void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
      void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
      void dropEvent(QGraphicsSceneDragDropEvent * event);

      void mousePressEvent(QGraphicsSceneMouseEvent *e);
      void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
      void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);

   private:
      void init();
      void processBeforeMouseMove();
      void moveCommonNodes();
      
      QList<Node*> m_circuitNodes;
      QSet<Node*> m_commonMovingNodes;
      QUndoStack *m_undoStack;
      bool m_areItemsMoving;
      
};

#endif //__SCHEMATICSCENE_H
