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

#ifndef SCHEMATIC_WIDGET_H
#define SCHEMATIC_WIDGET_H

#include "global.h"

#include <QGraphicsView>

namespace Caneda
{
    // Forward declarations
    class SchematicItem;
    class SchematicScene;

    class SchematicWidget : public QGraphicsView
    {
        Q_OBJECT
    public:
        static const qreal zoomFactor;

        SchematicWidget(SchematicScene *scene = 0);
        ~SchematicWidget();

        SchematicScene* schematicScene() const;

        void zoomIn();
        void zoomOut();
        void zoomFitInBest();
        void zoomOriginal();
        qreal currentZoom() { return m_currentZoom; };

        void zoomFitRect(const QRectF &rect);

    Q_SIGNALS:
        void cursorPositionChanged(const QString& newPos);
        void focussedIn(SchematicWidget *view);
        void focussedOut(SchematicWidget *view);

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void focusInEvent(QFocusEvent *event);
        void focusOutEvent(QFocusEvent *event);

    private Q_SLOTS:
        void onMouseActionChanged();

    private:
        void setZoomLevel(qreal zoomLevel, QPointF *toCenterOn = 0);

        ZoomRange m_zoomRange;
        qreal m_currentZoom;
    };

} // namespace Caneda

#endif //SCHEMATIC_WIDGET_H
