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

#ifndef CANEDA_SCHEMATICWIDGET_H
#define CANEDA_SCHEMATICWIDGET_H

#include <QGraphicsView>

namespace Caneda
{
    // Forward declarations
    class SchematicItem;
    class SchematicScene;
    class SchematicView;

    class SchematicWidget : public QGraphicsView
    {
    Q_OBJECT
    public:
        static const qreal zoomFactor;

        SchematicWidget(SchematicView *view = 0);
        ~SchematicWidget();

        SchematicView* schematicView() const;
        SchematicScene* schematicScene() const;

        void saveScrollState();
        void restoreScrollState();

        qreal fit(const QRectF &rect);

    signals:
        void cursorPositionChanged(const QString& newPos);
        void focussed(SchematicWidget *view);

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void focusInEvent(QFocusEvent *event);

    private:
        SchematicView *m_schematicView;
        int m_horizontalScroll;
        int m_verticalScroll;
    };

} // namespace Caneda

#endif //CANEDA_SCHEMATICWIDGET_H
