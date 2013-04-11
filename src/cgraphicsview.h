/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#ifndef C_GRAPHICS_VIEW_H
#define C_GRAPHICS_VIEW_H

#include "global.h"

#include <QGraphicsView>

namespace Caneda
{
    // Forward declarations
    class CGraphicsScene;

    class CGraphicsView : public QGraphicsView
    {
        Q_OBJECT
    public:
        CGraphicsView(CGraphicsScene *scene = 0);
        ~CGraphicsView();

        CGraphicsScene* cGraphicsScene() const;

        void zoomIn();
        void zoomOut();
        void zoomFitInBest();
        void zoomOriginal();

        qreal currentZoom() { return m_currentZoom; }
        void setZoom(int percentage);
        void zoomFitRect(const QRectF &rect);

    Q_SIGNALS:
        void cursorPositionChanged(const QString& newPos);
        void focussedIn(CGraphicsView *view);
        void focussedOut(CGraphicsView *view);

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void focusInEvent(QFocusEvent *event);
        void focusOutEvent(QFocusEvent *event);

    private Q_SLOTS:
        void onMouseActionChanged();

    private:
        void setZoomLevel(qreal zoomLevel, QPointF *toCenterOn = 0);

        const qreal m_zoomFactor;
        ZoomRange m_zoomRange;
        qreal m_currentZoom;

        //! \brief Auxiliary pan variables
        bool panMode;
        QPointF panStartPosition;
    };

} // namespace Caneda

#endif //C_GRAPHICS_VIEW_H
