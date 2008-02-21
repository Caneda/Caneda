/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __STYLEDIALOG_H
#define __STYLEDIALOG_H

#include "ui_filldialog.h"
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPolygon>

class PreviewWidget : public QWidget
{
      Q_OBJECT;
   public:
      PreviewWidget(QWidget *widget = 0);

      QPen pen() const { return m_pen; }
      void setPen(QPen pen);

      QBrush brush() const { return m_brush; }
      void setBrush(QBrush brush);

      int headStyle() const { return m_headStyle; }
      void setHeadStyle(int style);

      void paintEvent(QPaintEvent *event);

      void calcHeadPoints(int width, int height);

   private:
      void drawHead(QPainter *painter);

      QPen m_pen;
      QBrush m_brush;
      QPixmap m_lightPixmap;
      QPixmap m_darkPixmap;

      int m_headStyle;
      QPolygon m_headPolygon;
};

class StyleDialog : public QDialog, public Ui::Dialog
{
      Q_OBJECT;

   public:
      StyleDialog(QWidget *parent = 0);

   public slots:
      void setupStyleWidgets();
      void updatePreview();

      void launchColorDialog();
   private:
      PreviewWidget *previewWidget;
      QColor lineColor;
      QColor fillColor;

      QPixmap lineColorPixmap;
      QPixmap fillColorPixmap;

      QPolygon headPolygon;
};

#endif //__DIALOG_H
