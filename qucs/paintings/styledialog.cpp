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

#include "styledialog.h"
#include "painting.h"

#include <QtGui/QColorDialog>
#include <QtGui/QPainter>

PreviewWidget::PreviewWidget(QWidget *parent) :
   QWidget(parent),
   m_lightPixmap(10, 10),
   m_darkPixmap(10, 10),
   m_headStyle(1)

{
   m_lightPixmap.fill(Qt::white);
   m_darkPixmap.fill(Qt::lightGray);
   resize(140, 140);
   calcHeadPoints(20, 40);
   setMinimumSize(QSize(140, 140));
}

void PreviewWidget::setPen(QPen pen)
{
   if(pen == m_pen) return;
   m_pen = pen;
   update();
}

void PreviewWidget::setBrush(QBrush brush)
{
   if(brush == m_brush) return;
   m_brush = brush;
   update();
}

void PreviewWidget::setHeadStyle(int style)
{
   if(style == m_headStyle || (style != 0 && style != 1))
      return;
   m_headStyle = style;
   update();
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   for(int i=0; i <= width(); i += 10) {
      for(int j=0; j <= height(); j += 10) {
         QPixmap pix = (((i/10) + (j/10)) % 2) == 0 ? m_darkPixmap : m_lightPixmap;
         painter.drawPixmap(QPoint(i, j), pix);
      }
   }
   painter.setPen(pen());
   painter.setBrush(brush());
//   int adjust = 15;
//   QRect rect(frameGeometry().adjusted(adjust, adjust, -adjust, -adjust));
//   rect.moveCenter(QPoint(width()/2, height()/2));

   QRect rect = QRect(0,0, 100, 100);
   rect.moveCenter(QPoint(width()/2, height()/2));
   painter.drawLine(rect.bottomLeft(), rect.topRight());
   drawHead(&painter);
}

void PreviewWidget::calcHeadPoints(int headWidth, int headHeight)
{
   QRect rect = QRect(0, 0, 100, 100);
   rect.moveCenter(QPoint(width()/2, height()/2));
   int angle = -45;

   QMatrix mapper;
   mapper.rotate(angle);

   QPoint arrowTipPos = mapper.map(rect.topRight());
   QPoint bottomLeft(arrowTipPos.x() - headWidth/2, arrowTipPos.y() + headHeight);
   QPoint bottomRight(arrowTipPos.x() + headWidth/2, arrowTipPos.y() + headHeight);

   mapper = mapper.inverted();

   if(m_headPolygon.size() != 3)
      m_headPolygon.resize(3);
   m_headPolygon[0] = mapper.map(bottomLeft);
   m_headPolygon[1] = mapper.map(arrowTipPos);
   m_headPolygon[2] = mapper.map(bottomRight);

   m_headPolygon.translate(rect.topRight() - mapper.map(arrowTipPos));
}

void PreviewWidget::drawHead(QPainter *painter)
{
   if(m_headStyle == 1) {
      painter->drawConvexPolygon(m_headPolygon);
   }
   else {
      painter->drawLine(m_headPolygon[0], m_headPolygon[1]);
      painter->drawLine(m_headPolygon[1], m_headPolygon[2]);
   }
}

StyleDialog::StyleDialog(QWidget *parent) :
   QDialog(parent),
   lineColor(defaultPaintingPen.color()),
   fillColor(Qt::white),
   lineColorPixmap(32, 32),
   fillColorPixmap(32, 32)
{
   lineColorPixmap.fill(lineColor);
   fillColorPixmap.fill(fillColor);

   setupUi(this);
   setupStyleWidgets();
}

void StyleDialog::setupStyleWidgets()
{
   previewWidget = new PreviewWidget();
   QHBoxLayout *layout = new QHBoxLayout(previewGroupBox);
   layout->addWidget(previewWidget);

   lineColorButton->setIcon(lineColorPixmap);
   fillColorButton->setIcon(fillColorPixmap);

   arrowWidthSpinBox->setValue(12);
   arrowHeightSpinBox->setValue(20);

   connect(arrowStyleComboBox, SIGNAL(activated(int)), this, SLOT(updatePreview()));
   connect(arrowWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updatePreview()));
   connect(arrowHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updatePreview()));

   connect(lineWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updatePreview()));
   connect(lineColorButton, SIGNAL(clicked()), this, SLOT(launchColorDialog()));
   connect(lineStyleComboBox, SIGNAL(activated(int)), this, SLOT(updatePreview()));

   connect(fillColorButton, SIGNAL(clicked()), this, SLOT(launchColorDialog()));
   connect(fillStyleComboBox, SIGNAL(activated(int)), this, SLOT(updatePreview()));

   lineStyleComboBox->setCurrentIndex(1);
   updatePreview();
}

void StyleDialog::updatePreview()
{
   QPen pen(lineColor);
   pen.setWidth(lineWidthSpinBox->value());
   pen.setStyle((Qt::PenStyle)lineStyleComboBox->currentIndex());

   QColor color(fillColor);
   QBrush brush(color);
   brush.setStyle((Qt::BrushStyle)fillStyleComboBox->currentIndex());

   previewWidget->setHeadStyle(arrowStyleComboBox->currentIndex());
   previewWidget->calcHeadPoints(arrowWidthSpinBox->value(), arrowHeightSpinBox->value());
   previewWidget->setPen(pen);
   previewWidget->setBrush(brush);
   previewWidget->update();
}

void StyleDialog::launchColorDialog()
{
   QToolButton *button = qobject_cast<QToolButton*>(sender());
   if(!button) return;

   bool lineButtonClicked = (button == lineColorButton);
   QColor defaultColor = lineButtonClicked ? lineColor : fillColor;

   QColor color = QColorDialog::getColor(defaultColor);
   if(color.isValid()) {
      if(lineButtonClicked && lineColor != color) {
         lineColor = color;
         lineColorPixmap.fill(lineColor);
         lineColorButton->setIcon(lineColorPixmap);
         updatePreview();
      }
      else if(!lineButtonClicked && fillColor != color) {
         fillColor = color;
         fillColorPixmap.fill(fillColor);
         fillColorButton->setIcon(fillColorPixmap);
         updatePreview();
      }
   }
}
