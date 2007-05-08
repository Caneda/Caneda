/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __QUCSVIEW_H
#define __QUCSVIEW_H

#include <QtCore/QDateTime>
#include <QtCore/QString>

class QucsMainWindow;
class QPainter;

struct QucsView
{
      QucsView(QucsMainWindow *m);
      virtual ~QucsView() {}

      virtual void setFileName(const QString& name) = 0; // Emit signal while reimplementing
      virtual QString fileName() const = 0; // With path
      virtual bool load() = 0; // First set filename and then call load
      virtual bool save() = 0; // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  save
      virtual void print(QPainter *p, bool printAll, bool fitToPage) = 0;
      virtual void zoomIn() = 0;
      virtual void zoomOut() = 0;
      virtual void showAll() = 0;
      virtual void showNoZoom() = 0;

      QString tabText() const; // Returns text to be displayed on tab
      QDateTime lastSaved; // To check for external modification
      QucsMainWindow *mainWindow;
};

#endif //__QUCSVIEW_H
