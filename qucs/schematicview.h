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

#ifndef __SCHEMATICVIEW_H
#define __SCHEMATICVIEW_H

#include "qucsview.h"
#include <QtGui/QGraphicsView>

class SchematicScene;
class QucsMainWindow;
class QucsItem;

class SchematicView : public QGraphicsView, public QucsView
{      Q_OBJECT;
   public:
      static const qreal zoomFactor;

      SchematicView(SchematicScene *sc = 0,QucsMainWindow *parent = 0);
      ~SchematicView();
      void test();

      SchematicScene* schematicScene() const;

      //reimplemented virtuals from QucsView
      void setFileName(const QString& name);
      QString fileName() const;

      bool load();
      bool save();

      void print(QPainter *p, bool printAll, bool fitToPage);

      void zoomIn();
      void zoomOut();

      void showAll();
      void showNoZoom();

      bool isSchematicView() const { return true; }

      QWidget* toWidget() const;
      SchematicView* toSchematicView() const;

      bool isModified() const;

      void copy() const;
      void cut();
      void paste();

      void resetState();

      void saveScrollState();
      void restoreScrollState();

   public slots:
      void setModified(bool m);
      //update tab's text and modification status
      void updateTabs();

   signals:
      void modificationChanged(bool modified);
      void fileNameChanged(const QString& file);
      void titleToBeUpdated();

   private:
      void repaintWires();

      int m_horizontalScroll;
      int m_verticalScroll;

   private slots:
      void addTestComponents();
};

#endif //__SCHEMATICVIEW_H
