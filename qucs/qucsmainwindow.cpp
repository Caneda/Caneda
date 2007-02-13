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

#include "global.h"
#include "componentssidebar.h"
#include "qucsmainwindow.h"
#include "schematicview.h"
#include "schematicscene.h"

#include <QtGui/QStatusBar>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QIcon>
#include <QtGui/QMenuBar>
#include <QtGui/QUndoGroup>
#include <QtGui/QUndoView>
#include <QtGui/QApplication>

QucsMainWindow::QucsMainWindow(QWidget *w) : DTabbedMainWindow(w)
{
   m_undoGroup = new QUndoGroup();
   //m_undoView = new QUndoView(m_undoGroup,this);
   statusBar()->show();
   initActions();
   m_componentsSidebar = new ComponentsSidebar(this);
   addToolView(m_componentsSidebar, Qt::LeftDockWidgetArea);
   m_componentsSidebar->show();
   connect(this,SIGNAL(widgetChanged(QWidget *)),this,SLOT(activateStackOf(QWidget *)));
   newView();
}

void QucsMainWindow::initActions()
{
   QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
   fileNewAction = fileMenu->addAction(QIcon(imageDirectory()+"filenew.png"),tr("&New"),this,SLOT(newView()));
   fileSaveAction = fileMenu->addAction(QIcon(imageDirectory()+"filesave.png"),tr("&Save"));
   fileMenu->addAction(tr("&Quit"),qApp,SLOT(quit()));

   QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
   QAction* ua = m_undoGroup->createUndoAction(this);
   ua->setIcon(QIcon(imageDirectory()+"undo.png"));
   QAction *ra = m_undoGroup->createRedoAction(this);
   ra->setIcon(QIcon(imageDirectory()+"redo.png"));
   editMenu->addAction(ua);
   editMenu->addAction(ra);

   QToolBar* toolBar = new QToolBar("main");
   toolBar->addAction(fileNewAction);
   toolBar->addAction(fileSaveAction);
   toolBar->addAction(ua);
   toolBar->addAction(ra);

   addToolBar(Qt::TopToolBarArea,toolBar);
}

void QucsMainWindow::newView()
{
   SchematicView *vv = new SchematicView();
   addView(vv);
   if(tabWidget()->count() == 1)
      m_undoGroup->setActiveStack(vv->schematicScene()->undoStack());
}

void QucsMainWindow::addView(SchematicView *view)
{
   m_undoGroup->addStack(view->schematicScene()->undoStack());
   DTabbedMainWindow::addWidget(view);
}

void QucsMainWindow::activateStackOf(QWidget *w)
{
   SchematicView *v = qobject_cast<SchematicView*>(w);
   if(v)
      m_undoGroup->setActiveStack(v->schematicScene()->undoStack());
}
