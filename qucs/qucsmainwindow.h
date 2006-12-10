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

#ifndef __QUCSMAINWINDOW_H
#define __QUCSMAINWINDOW_H

#include "ideality/dtabbedmainwindow.h"
class ComponentsSidebar;
class QUndoGroup;
class SchematicView;
class QucsMainWindow : public DTabbedMainWindow
{
Q_OBJECT
   public:
      QucsMainWindow(QWidget *w=0l);
      ~QucsMainWindow() {}

      void addView(SchematicView *view);

   private slots:
      void activateStackOf(QWidget *w);
      void newView();
      
   private:
      void initActions();

      QAction *fileNewAction;
      QAction *fileSaveAction;

      //QUndoView *m_undoView;
      QUndoGroup *m_undoGroup;
      ComponentsSidebar *m_componentsSidebar;
};

#endif //__QUCSMAINWINDOW_H
