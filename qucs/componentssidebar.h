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

#ifndef __COMPONENTSSIDEBAR_H
#define __COMPONENTSSIDEBAR_H

#include <QtGui/QWidget>

// Forward declarations
class QPixmap;
class FilterProxyModel;
class QLineEdit;
class SidebarModel;
class TreeView;
class QToolButton;

//! Represents sidebar which allows components to be selected.
class ComponentsSidebar : public QWidget
{
      Q_OBJECT;
   public:
      ComponentsSidebar(QWidget *parent = 0);
      ~ComponentsSidebar() {}

      void plugLibrary(QString str);

   private slots:
      void filterTextChanged();

   private:
      SidebarModel *m_model;
      FilterProxyModel *m_proxyModel;
      QLineEdit *m_filterEdit;
      TreeView *m_treeView;
      QToolButton *m_clearButton;
};

#endif
