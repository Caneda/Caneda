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

#include "paintings.h"
#include "schematicscene.h"
#include "undocommands.h"

#include <QColorDialog>
#include <QPainter>
#include <QTimer>

PreviewWidget::PreviewWidget(int paintingType, QWidget *parent) :
    QWidget(parent),
    m_lightPixmap(10, 10),
    m_darkPixmap(10, 10),
    m_headStyle(1),
    m_headWidth(20),
    m_headHeight(40),
    m_startAngle(0),
    m_spanAngle(180),
    m_drawBackground(true),
    m_paintingType(paintingType)
{
    m_lightPixmap.fill(Qt::white);
    m_darkPixmap.fill(Qt::lightGray);
    setMinimumSize(QSize(140, 140));
    resize(140, 140);
    if(m_paintingType == Painting::ArrowType) {
        calcHeadPoints();
    }

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
    QTimer::singleShot(100, this, SLOT(update()));
}

void PreviewWidget::setPen(QPen pen)
{
    if(pen == m_pen) {
        return;
    }
    m_pen = pen;
    update();
}

void PreviewWidget::setBrush(QBrush brush)
{
    if(m_paintingType == Painting::GraphicLineType || brush == m_brush) {
        return;
    }
    m_brush = brush;
    update();
}

void PreviewWidget::setHeadStyle(int style)
{
    if(m_paintingType != Painting::ArrowType ||
            style == m_headStyle || (style != 0 && style != 1)) {
        return;
    }
    m_headStyle = style;
    update();
}

void PreviewWidget::setHeadWidth(int width)
{
    if(m_paintingType != Painting::ArrowType) {
        return;
    }
    m_headWidth = width;
    calcHeadPoints();
    update();
}

void PreviewWidget::setHeadHeight(int height)
{
    if(m_paintingType != Painting::ArrowType) {
        return;
    }
    m_headHeight = height;
    calcHeadPoints();
    update();
}

void PreviewWidget::setHeadSize(QSize size)
{
    if(m_paintingType != Painting::ArrowType) {
        return;
    }
    m_headWidth = size.width();
    m_headHeight = size.height();
    calcHeadPoints();
    update();
}

void PreviewWidget::setStartAngle(int angle)
{
    if(m_paintingType == Painting::EllipseArcType) {
        m_startAngle = angle;
        update();
    }
}

void PreviewWidget::setSpanAngle(int angle)
{
    if(m_paintingType == Painting::EllipseArcType) {
        m_spanAngle = angle;
        update();
    }
}

void PreviewWidget::resizeEvent(QResizeEvent *event)
{
    if(m_paintingType == Painting::ArrowType) {
        calcHeadPoints();
    }
    return QWidget::resizeEvent(event);
}

QRect PreviewWidget::adjustedRect() const
{
    QRect rect;
    if(m_paintingType == Painting::ArrowType) {
        rect = geometry();

        int sqr_size = qMin(rect.width(), rect.height());
        rect.setSize(QSize(sqr_size, sqr_size));

        rect.adjust(10, 10, -10, -10);
    }
    else {
        rect = geometry();
        rect.adjust(10, 10, -10, -10);
    }

    rect.moveCenter(QPoint(width()/2, height()/2));
    return rect;
}

void PreviewWidget::drawBackgroundBoxes(QPainter *painter)
{
    for(int i=0; i <= width(); i += 10) {
        for(int j=0; j <= height(); j += 10) {
            int shouldDrawLight = (i/10 + j/10) % 2;

            if(shouldDrawLight) {
                painter->drawPixmap(i, j, m_lightPixmap);
            }
            else {
                painter->drawPixmap(i, j, m_darkPixmap);
            }
        }
    }
}

void PreviewWidget::drawArrow(QPainter *painter)
{
    QRect rect = adjustedRect();

    painter->drawLine(rect.bottomLeft(), rect.topRight());
    if(m_headStyle == 1) {
        painter->drawConvexPolygon(m_headPolygon);
    }
    else {
        painter->drawLine(m_headPolygon[0], m_headPolygon[1]);
        painter->drawLine(m_headPolygon[1], m_headPolygon[2]);
    }
}

void PreviewWidget::drawEllipse(QPainter *painter)
{
    QRect rect = adjustedRect();
    painter->drawEllipse(rect);
}

void PreviewWidget::drawEllipseArc(QPainter *painter)
{
    QRect rect = adjustedRect();
    painter->drawArc(rect, m_startAngle*16, m_spanAngle*16);
}

void PreviewWidget::drawLine(QPainter *painter)
{
    QRect rect = adjustedRect();
    painter->drawLine(rect.bottomLeft(), rect.topRight());
}

void PreviewWidget::drawRectangle(QPainter *painter)
{
    QRect rect = adjustedRect();
    painter->drawRect(rect);
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(m_drawBackground) {
        drawBackgroundBoxes(&painter);
    }

    painter.setPen(pen());
    painter.setBrush(brush());

    switch(m_paintingType) {
        case Painting::ArrowType:
            drawArrow(&painter); break;

        case Painting::EllipseType:
            drawEllipse(&painter); break;

        case Painting::EllipseArcType:
            drawEllipseArc(&painter); break;

        case Painting::GraphicLineType:
            drawLine(&painter); break;

        case Painting::RectangleType:
            drawRectangle(&painter); break;

        default: ;
    }
}

void PreviewWidget::calcHeadPoints()
{
    QRect rect = adjustedRect();
    int angle = -45;

    QMatrix mapper;
    mapper.rotate(angle);

    QPoint arrowTipPos = mapper.map(rect.topRight());
    QPoint bottomLeft(arrowTipPos.x() - m_headWidth/2, arrowTipPos.y() + m_headHeight);
    QPoint bottomRight(arrowTipPos.x() + m_headWidth/2, arrowTipPos.y() + m_headHeight);

    mapper = mapper.inverted();

    if(m_headPolygon.size() != 3) {
        m_headPolygon.resize(3);
    }
    m_headPolygon[0] = mapper.map(bottomLeft);
    m_headPolygon[1] = mapper.map(arrowTipPos);
    m_headPolygon[2] = mapper.map(bottomRight);

    m_headPolygon.translate(rect.topRight() - mapper.map(arrowTipPos));
}

void PreviewWidget::toggleBackground(bool state)
{
    m_drawBackground = state;
    update();
}

StyleDialog::StyleDialog(Painting *_painting, Qucs::UndoOption option, QWidget *parent) :
    QDialog(parent),
    lineColor(defaultPaintingPen.color()),
    fillColor(Qt::white),
    lineColorPixmap(32, 32),
    fillColorPixmap(32, 32),
    painting(_painting),
    undoOption(option)
{
    lineColorPixmap.fill(lineColor);
    fillColorPixmap.fill(fillColor);

    setupUi(this);
    setupStyleWidgets();
    adjustSize();
}

void StyleDialog::setupStyleWidgets()
{
    QPen pen = painting->pen();
    QBrush brush = painting->brush();

    lineWidthSpinBox->setValue(pen.width());
    lineColor = pen.color();
    lineColorPixmap.fill(lineColor);
    lineStyleComboBox->setCurrentIndex(pen.style());

    fillColor = brush.color();
    fillColorPixmap.fill(brush.color());
    fillStyleComboBox->setCurrentIndex(brush.style());

    if(painting->type() == Painting::ArrowType) {
        Arrow *arrow = qucsitem_cast<Arrow*>(painting);
        arrowStyleComboBox->setCurrentIndex(arrow->headStyle());
        arrowWidthSpinBox->setValue(static_cast<int>(arrow->headWidth()));
        arrowHeightSpinBox->setValue(static_cast<int>(arrow->headHeight()));
    }
    else {
        arrowGroupBox->hide();
    }

    if(painting->type() == Painting::EllipseArcType) {
        EllipseArc *arc = qucsitem_cast<EllipseArc*>(painting);
        startAngleSpinBox->setValue(arc->startAngle());
        spanAngleSpinBox->setValue(arc->spanAngle());
        fillGroupBox->hide();
    }
    else {
        arcGroupBox->hide();
    }

    if(painting->type() == Painting::GraphicLineType) {
        fillGroupBox->hide();
    }

    lineColorButton->setIcon(lineColorPixmap);
    fillColorButton->setIcon(fillColorPixmap);

    connect(startAngleSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
    connect(spanAngleSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePreview()));

    connect(arrowStyleComboBox, SIGNAL(activated(int)), SLOT(updatePreview()));
    connect(arrowWidthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
    connect(arrowHeightSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePreview()));

    connect(lineWidthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
    connect(lineColorButton, SIGNAL(clicked()), SLOT(launchColorDialog()));
    connect(lineStyleComboBox, SIGNAL(activated(int)), SLOT(updatePreview()));

    connect(fillColorButton, SIGNAL(clicked()), SLOT(launchColorDialog()));
    connect(fillStyleComboBox, SIGNAL(activated(int)), SLOT(updatePreview()));

    connect(this, SIGNAL(accepted()), SLOT(applySettings()));

    previewWidget = new PreviewWidget(painting->type());

    QHBoxLayout *layout = new QHBoxLayout(previewGroupBox);
    layout->addWidget(previewWidget);
    connect(backgroundCheckBox, SIGNAL(toggled(bool)), previewWidget, SLOT(toggleBackground(bool)));

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
    previewWidget->setHeadSize(QSize(arrowWidthSpinBox->value(), arrowHeightSpinBox->value()));
    previewWidget->setPen(pen);
    previewWidget->setBrush(brush);
    previewWidget->setStartAngle(startAngleSpinBox->value());
    previewWidget->setSpanAngle(spanAngleSpinBox->value());
    previewWidget->update();
}

void StyleDialog::launchColorDialog()
{
    QToolButton *button = qobject_cast<QToolButton*>(sender());
    if(!button) {
        return;
    }

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

void StyleDialog::applySettings()
{
    QString saveData = painting->saveDataText();

    painting->setPen(previewWidget->pen());
    if(painting->type() != Painting::GraphicLineType) {
        painting->setBrush(previewWidget->brush());
    }

    if(painting->type() == Painting::ArrowType) {
        Arrow *arrow = qucsitem_cast<Arrow*>(painting);
        arrow->setHeadStyle(static_cast<Arrow::HeadStyle>(previewWidget->headStyle()));
        arrow->setHeadHeight(previewWidget->headHeight());
        arrow->setHeadWidth(previewWidget->headWidth());
    }
    else if(painting->type() == Painting::EllipseArcType) {
        EllipseArc *arc = static_cast<EllipseArc*>(painting);
        arc->setStartAngle(previewWidget->startAngle());
        arc->setSpanAngle(previewWidget->spanAngle());
    }
    if(painting->schematicScene()) {
        QUndoCommand *cmd = new PaintingPropertyChangeCmd(painting, saveData);
        painting->schematicScene()->undoStack()->push(cmd);
    }
}
